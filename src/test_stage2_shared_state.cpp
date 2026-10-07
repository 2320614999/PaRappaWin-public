#include "pr/pr_stage2_shared_state.h"
#include <iostream>
#include <vector>

namespace {
namespace P=PrStagePayloadBankDirect;
namespace S=PrStage2SharedState;
uint32_t checks=0;
// 检查完整共享状态交接，失败时保留具体原因。
void Check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
struct Memory final:PrStage2LifecycleDirect::Services {
    std::vector<uint8_t> bytes=std::vector<uint8_t>(0x200000u,0xA5u);
    uint32_t writes=0;
    PrPsxGteDirect::MatrixRegisters gte{};
    // 测试只允许访问真实 PSX RAM，意外调用设备立即失败。
    size_t Index(uint32_t a){if(a<0x80000000u||a>=0x80200000u)throw std::runtime_error("越界访问");return a-0x80000000u;}
    uint8_t Read8(uint32_t a)override{return bytes.at(Index(a));}
    uint16_t Read16(uint32_t a)override{return uint16_t(Read8(a)|(uint16_t(Read8(a+1u))<<8u));}
    uint32_t Read32(uint32_t a)override{return Read16(a)|(uint32_t(Read16(a+2u))<<16u);}
    void Write8(uint32_t a,uint8_t v)override{++writes;bytes.at(Index(a))=v;}
    void Write16(uint32_t a,uint16_t v)override{Write8(a,uint8_t(v));Write8(a+1u,uint8_t(v>>8u));}
    void Write32(uint32_t a,uint32_t v)override{Write16(a,uint16_t(v));Write16(a+2u,uint16_t(v>>16u));}
    int32_t Call(uint32_t,std::initializer_list<uint32_t>)override{throw std::logic_error("交接不应调用设备");}
    PrStage2LifecycleDirect::Words64 Call64(uint32_t,std::initializer_list<uint32_t>)override{throw std::logic_error("意外宽调用");}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32)override{throw std::logic_error("意外坐标运算");}
    int32_t SetCdLocation800367A4(uint32_t)override{throw std::logic_error("意外读盘");}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect,uint32_t)override{throw std::logic_error("意外上传");}
    PrPsxGteDirect::MatrixRegisters& MatrixGte()override{return gte;}
    [[noreturn]]void Exit(uint32_t,std::initializer_list<uint32_t>)override{throw std::logic_error("意外退出");}
    [[noreturn]]void Break(uint32_t,uint32_t)override{throw std::logic_error("意外断点");}
};

// 不同图案区分当前存档和旧备份，防止复制方向错误或备份被重建。
S::Entry Entry(){
    S::Entry entry;
    Check(P::InitSavePayload80015CC4(entry.payload).ok,"原保存初始化失败");
    for(size_t i=0;i<entry.payload.savePayloadBank.size();++i){
        entry.payload.savePayloadBank[i]=uint8_t(i*37u+11u);
        entry.payload.saveStatusBackup[i]=uint8_t(i*19u+97u);
    }
    entry.payload.saveStatusBackupKnown80079008=true;
    entry.payload.saveStatusBackupStatusBankKnown80092F1D=true;
    entry.savePolicyKnown=true;
    entry.mode=0;entry.easy=1;entry.language=5;entry.subtitles=1;entry.exitReason=0x8000;entry.savePolicy=0xFFFF;
    return entry;
}

// 逐字节比对完整 RAM，确认没有覆盖未声明的共享区或改写入参。
void Import(bool backup){
    auto entry=Entry();
    entry.payload.saveStatusBackupKnown80079008=backup;
    entry.payload.replayMirror.fill(0x5Au); // 没有来源凭证的回放不能被导入。
    entry.payload.replayMirrorKnown8008EEF8=true;
    Memory memory,expected;
    const auto original=entry.payload;
    for(size_t i=0;i<original.savePayloadBank.size();++i){
        expected.bytes[0x92F10u+i]=original.savePayloadBank[i];
        if(backup)expected.bytes[0x79008u+i]=original.saveStatusBackup[i];
    }
    expected.Write16(0x800916D0u,entry.mode);expected.Write16(0x800916D8u,entry.language);
    expected.Write16(0x800916DAu,entry.easy);expected.Write16(0x800916DCu,entry.subtitles);
    expected.Write16(0x800916E0u,entry.exitReason);expected.Write16(0x800916F0u,entry.savePolicy);
    S::ImportEntry(memory,entry);
    Check(memory.bytes==expected.bytes,"共享状态内容或写入范围错误");
    Check(entry.payload.savePayloadBank==original.savePayloadBank&&entry.payload.saveStatusBackup==original.saveStatusBackup,
          "交接改变了菜单持有的存档或备份");
    Check(memory.writes==original.savePayloadBank.size()*(backup?2u:1u)+12u,"交接写入次数异常");
}

// 任一关键来源缺失时，在第一次 RAM 写入前拒绝交接。
void RejectIncomplete(){
    for(int which=0;which<5;++which){
        auto entry=Entry();
        if(which==0)entry.savePolicyKnown=false;
        if(which==1)entry.payload.savePayloadBankKnown=false;
        if(which==2)entry.payload.statusBankKnown80092F1D=false;
        if(which==3)entry.payload.boundsFault=true;
        if(which==4)entry.mode=2; // 有字节及备份标记，但没有真实载入的回放凭证。
        Memory memory;const auto original=memory.bytes;bool rejected=false;
        try{S::ImportEntry(memory,entry);}catch(const std::runtime_error&){rejected=true;}
        Check(rejected,"缺少共享来源仍被当作可用状态");
        Check(memory.writes==0&&memory.bytes==original,"失败交接留下了部分写入");
    }
}
}

// 独立运行，覆盖正常入场和拒绝路径；不代表真实回放文件载入验收。
int main(){
    try{Import(false);Import(true);RejectIncomplete();std::cout<<"shared-state-pass "<<checks<<" checks\n";return 0;}
    catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
}
