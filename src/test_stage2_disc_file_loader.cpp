#include "pr/pr_stage2_int_loader_direct.h"
#include "pr/pr_stage2_disc_file_device.h"
#include "pr/pr_stage2_vram_device.h"
#ifdef S2_VERIFY_NATIVE_ATLAS
#include "test_stage2_vram_atlas_checks.h"
#endif

#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include "test_stage2_disc_file_services.h"
int main(int argc,char** argv) {
    try {
#ifdef S2_VERIFY_RUNTIME_TIM
        if (argc != 4) return 1;
#else
        if (argc != 3) return 1;
#endif
        uint32_t count, repeats;
        if (!(std::cin >> count >> repeats) || count > 1024u || repeats > 2u || repeats == 0) return 1;
        std::vector<uint32_t> sizes(count); uint32_t total = 0;
        for (auto& size : sizes) { if (!(std::cin >> size) || size > 0x200000u-total) return 1; total += size; }
        DiskServices s(argv[1],argv[2]);
        constexpr uint32_t record = 0x80054A60u; // Live IDA xref of \S2\COMPO02.INT;1.
        if (s.Read32(record) != 0x800116F8u) throw std::runtime_error("wrong original S2 record");
        for (uint32_t pass = 0; pass < repeats; ++pass) {
            const auto result = PrStage2IntLoaderDirect::LoadInt8001AC18(s,record,0u);
            std::cout << "pass " << pass << " return " << result << '\n';
            if (count == 0) {
                if (result != 0 || s.disc.ReadRequests() != 0) return 2;
                continue;
            }
            if (result != 1 || s.Read32(0x8006EB84u) != count) return 3;
            for (uint32_t slot = 1; slot <= count; ++slot) {
                const uint32_t address = s.Read32(0x80091858u+slot*4u), size = sizes[slot-1];
                uint64_t hash = 14695981039346656037ull;
                for (uint32_t i = 0; i < size; ++i) hash = (hash^s.Read8(address+i))*1099511628211ull;
                std::cout << "resident " << slot << ' ' << address << ' ' << size << ' ' << hash << '\n';
            }
            std::cout << "descriptor " << s.Read32(record+16u) << ' ' << s.Read32(record+20u)
                      << ' ' << s.Read32(record+40u) << ' ' << s.Read32(record+44u) << '\n';
            // Hash every word and its known bit: no unobserved black fill can
            // stand in for a missing palette/image write.
            uint64_t vramHash = 14695981039346656037ull;
            for (std::size_t i = 0; i < s.vram.WordCount; ++i) {
                const uint16_t word = s.vram.Words()[i];
                for (uint8_t byte : {static_cast<uint8_t>(word),static_cast<uint8_t>(word>>8u),s.vram.Known()[i]})
                    vramHash = (vramHash^byte)*1099511628211ull;
            }
            std::cout << "vram " << pass << ' ' << s.vram.UploadCount() << ' ' << s.vram.SyncCount()
                      << ' ' << s.vram.KnownWords() << ' ' << s.vram.WrittenWords() << ' ' << vramHash << '\n';
#ifdef S2_VERIFY_NATIVE_ATLAS
            RunStage2VramAtlasChecks(s, s.vram, sizes, pass);
#endif
        }
        if (count != 0) {
            // Real filesystem short read: leave RAM and the head unchanged,
            // then retry one valid sector from that same position.
            PrStage2DiscFileDevice::Device failureProbe(argv[2]);
            const auto sectors = std::filesystem::file_size(argv[2])/2352u;
            if (sectors == 0 || sectors > 0x7FFFFFFFu) return 4;
            constexpr uint32_t location = 0x801FF100u, destination = 0x801FE000u;
            PrStage2IntLoaderDirect::LbaToLocation80036974(s,static_cast<uint32_t>(sectors-1u),location);
            if (failureProbe.SetLocation(s.Read32(location)&0xFFFFFFu) != 1) return 4;
            for (uint32_t i = 0; i < 4096u; ++i) s.Write8(destination+i,static_cast<uint8_t>(i^0xA5u));
            if (failureProbe.StartRead(s,2u,destination,0u) != 1 || failureProbe.PollRead() != -1 ||
                failureProbe.BytesTransferred() != 0u) return 4;
            for (uint32_t i = 0; i < 4096u; ++i)
                if (s.Read8(destination+i) != static_cast<uint8_t>(i^0xA5u)) return 4;
            if (failureProbe.StartRead(s,1u,destination,128u) != 1 || failureProbe.PollRead() != 0 ||
                failureProbe.BytesTransferred() != 2048u) return 4;
            uint64_t hash = 14695981039346656037ull;
            for (uint32_t i = 0; i < 2048u; ++i) hash = (hash^s.Read8(destination+i))*1099511628211ull;
            for (uint32_t i = 2048u; i < 4096u; ++i)
                if (s.Read8(destination+i) != static_cast<uint8_t>(i^0xA5u)) return 4;
            std::cout << "boundary short_read_preserves_ram_and_head " << hash << '\n';
        }
        if (count != 0)
            std::cout << "gpu-sync-device " << s.gpuSyncQueries << ' ' << s.gpuStatusReads << '\n';
        std::cout << "io " << s.disc.LookupRequests() << ' ' << s.disc.ReadRequests() << ' '
                  << s.disc.BytesTransferred() << ' ' << s.vram.UploadCount() << ' ' << s.vab << ' ' << s.waits << '\n';
#ifdef S2_VERIFY_RUNTIME_TIM
        if (count != 0) RunStage2RuntimeTimChecks(s, s.vram, sizes, argv[3]);
#endif
        // Rejected arguments and unreadable transactions are not success.
        if (s.disc.SetLocation(0x00FFFFFFu) != 0) return 4;
        if (s.disc.StartRead(s,0xFFFFFFFFu,0x800965B0u,0u) != 0 || s.disc.PollRead() != -1) return 4;
        return 0;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 5; }
}
