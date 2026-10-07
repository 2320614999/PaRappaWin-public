#include "pr_stage2_shared_state.h"
#include <stdexcept>

namespace PrStage2SharedState {
namespace P=PrStagePayloadBankDirect;
// 在原初始化之前导入菜单拥有的进度、选项及回放，保留原备份不重新覆盖。
void ImportEntry(PrStage2LifecycleDirect::Services& s,const Entry& entry) {
    const auto& bank=entry.payload;
    if(!entry.savePolicyKnown || !bank.savePayloadBankKnown ||
       !bank.statusBankKnown80092F1D || bank.boundsFault)
        throw std::runtime_error("S2 scene entry has no readable shared progress/options");
    if(entry.mode==2u) {
        if(!P::ReplayPayloadBackingAuthorityMatchesState80092F5C(bank) ||
           !bank.saveStatusBackupKnown80079008 ||
           !bank.saveStatusBackupStatusBankKnown80092F1D)
            throw std::runtime_error("S2 replay entry requires the loaded replay and pre-load backup");
    }
    // 保留名字、未使用的回放槽与独立备份；此处重新备份会把载入前进度覆盖成回放数据。
    for(uint32_t i=0;i<bank.savePayloadBank.size();++i)
        s.Write8(P::kBaseAddress80092F10+i,bank.savePayloadBank[i]);
    if(bank.saveStatusBackupKnown80079008)
        for(uint32_t i=0;i<bank.saveStatusBackup.size();++i)
            s.Write8(0x80079008u+i,bank.saveStatusBackup[i]);
    if(P::ReplayMirrorSourceAuthorityMatchesState8008EEF8(bank)) {
        for(uint32_t i=0;i<bank.replayMirror.size();++i)
            s.Write8(P::kMirrorSrcAddress8008EEF8+i,bank.replayMirror[i]);
        if(bank.replayMirrorPublishedCountKnown800901BC)
            s.Write32(0x800901BCu,bank.replayMirrorPublishedCount800901BC);
    }
    s.Write16(0x800916D0u,entry.mode);
    s.Write16(0x800916D8u,entry.language);
    s.Write16(0x800916DAu,entry.easy);
    s.Write16(0x800916DCu,entry.subtitles);
    s.Write16(0x800916E0u,entry.exitReason);
    s.Write16(0x800916F0u,entry.savePolicy);
}
}
