// Reuse the established real optical/XA fixture; do not add a second decoder.
#define main ExistingXaTransportMain
#include "test_stage2_xa_transport.cpp"
#undef main

// 回归：消费者完成 DMA 时不应重新启动光盘周期，否则每个扇区都会额外变慢。
void DeadlineAfterDmaCheck(const std::filesystem::path& path){
    Context c(path);Start(c.cd);
    Advance(c.cd);Advance(c.cd);
    Check(c.cd.Flags()==1u,"Missing actual video sector before deadline test");
    Ack(c.cd);Bank(c.cd,0);Write(c.cd,3,128u);
    while(c.cd.DmaBytesAvailable())(void)c.cd.ReadDmaWord();
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    const auto reads=c.cd.SectorReads();
    c.cd.ReleaseConsumedSector();
    Check(c.cd.Service()&&c.cd.SectorReads()==reads+1u,
          "DMA release reset the elapsed optical deadline");
}

int main(int argc,char** argv){
    try{
        Check(argc==2,"Arguments: original optical image");
        const auto path=std::filesystem::u8path(argv[1]);
        DeadlineAfterDmaCheck(path);
        uint32_t gatedObservations=0;
        for(bool initiallyAllowed:{false,true}){
            Context c(path);Start(c.cd);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            const auto serial=c.cd.Serial(),commands=c.cd.Commands();
            for(uint32_t i=0;i<128u;++i){
                Check(!c.cd.Service(false),"Closed native read gate accepted another sector");
                Check(c.cd.SectorReads()==0u&&c.cd.Serial()==serial&&c.cd.Flags()==0u,
                      "Closed read gate fabricated data, progress or an interrupt");
                Check(c.xa.Sectors()==0u&&c.spu.CdFramesQueued()==0u,"Read gate consumed future XA audio");
                ++gatedObservations;
            }
            // The same closed gate must not deadlock an already accepted SDK
            // command. This is a producer scheduling gate, not an IRQ mask.
            Bank(c.cd,0);Write(c.cd,1,1u);
            Check(c.cd.Commands()==commands+1u,"Status command was not captured");
            Check(c.cd.Service(false)&&c.cd.Flags()==3u,"Read gate blocked an accepted command reply");
            Check(c.cd.SectorReads()==0u,"Command reply read an unrelated sector");Ack(c.cd);
            if(initiallyAllowed)Check(c.cd.Service(true),"Reopened read gate did not perform due optical work");
            else Advance(c.cd);
            Check(c.cd.SectorReads()==1u&&c.xa.Sectors()==1u&&c.spu.CdFramesQueued()==4704u,
                  "Reopening changed source identity or duplicated audio samples");
            const auto done=c.cd.Serial();
            for(uint32_t i=0;i<128u;++i)Check(!c.cd.Service(false)&&c.cd.Serial()==done&&c.cd.SectorReads()==1u,
                  "Closing after completed XA advanced the following video sector");
        }
        {
            Context c(path);Start(c.cd);
            Check(c.spu.QueueCdAudio(std::vector<int16_t>(88200u,321)),"Cannot establish bounded output backpressure");
            Advance(c.cd);Check(c.cd.SectorReads()==1u&&c.xa.Sectors()==0u,"Backpressured audio was not retained");
            Check(!c.cd.Service(false),"Full output queue should remain backpressured");
            std::vector<int16_t> pcm(88200u);c.spu.Render(pcm.data(),44100u);
            Check(c.cd.Service(false),"Closed new-read gate blocked completion of owned audio bytes");
            Check(c.cd.SectorReads()==1u&&c.xa.Sectors()==1u&&c.cd.Flags()==0u,
                  "Owned audio completion performed another read or raised a video IRQ");
        }
        std::cout<<"native-stream-pacing gated-observations="<<gatedObservations
                 <<" command-replies=2 pending-audio-completions=1 no-source-clock-write\n";
        std::cout<<"PASS stream pacing "<<checks<<" assertions\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
}
