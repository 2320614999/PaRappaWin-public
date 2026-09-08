#include "pr/pr_stage1_save_entry_direct.h"
#include "pr/pr_psx_event_frame_direct.h"
#include <cassert>
#include <fstream>
#include <iterator>
#include <iostream>
#include <memory>
#include <vector>

static uint32_t Read32(const std::vector<uint8_t>& data, std::size_t offset) {
    assert(offset + 4u <= data.size());
    return data[offset] | uint32_t(data[offset + 1]) << 8u |
        uint32_t(data[offset + 2]) << 16u | uint32_t(data[offset + 3]) << 24u;
}

int main(int argc, char** argv) {
    assert(argc == 2);
    std::ifstream stream(argv[1], std::ios::binary);
    const std::vector<uint8_t> scus((std::istreambuf_iterator<char>(stream)), {});
    const uint32_t base = Read32(scus, 0x18u);
    const auto tableOffset = std::size_t(0x8004F180u - base + 0x800u);
    namespace E = PrStage1SaveEntryDirect;
    namespace P = PrPsxEventFrameDirect;
    E::State entry;
    assert(!E::Advance(entry, 100));
    assert(E::Begin(entry));
    auto page = std::make_unique<P::EventFrameState8001E750>();
    P::ResetEventFrameState8003FB9C(*page, 320u, 240u);
    PrPsxVSyncDirect::PsxVSyncState80035560 vsync{};
    for (uint32_t i = 0; i < 28u; ++i) {
        const auto tick = 100u + i;
        assert(!E::ReadyForDispatcher(entry, tick));
        assert(E::Advance(entry, tick));
        assert(!E::Advance(entry, tick)); // extra 60-Hz render pass
        assert(entry.presentations == i + 1u);
        assert(entry.runtime.hostTicksCompleted == (i + 1u) * 2u);
        const auto& visual = entry.frame.visualFrame;
        std::array<uint8_t, 192> expected{};
        if (i < 24u) {
            expected.fill(1u);
            for (uint32_t n = 0; n < (i + 1u) * 8u; ++n) {
                // Exact original table: 8001F698 case 4 reads 191-gp[49].
                const auto offset = tableOffset + (191u - n) * 8u;
                const auto row = Read32(scus, offset);
                const auto col = Read32(scus, offset + 4u);
                assert(row < 12u && col < 16u);
                expected[row * 16u + col] = 0u;
            }
        }
        for (std::size_t cell = 0; cell < expected.size(); ++cell)
            assert(visual.grid[cell] == expected[cell]);
        assert(!P::PsxCall8001E750_SaveUiEventFrame(*page, 11, 0, 0, 0, 0,
            nullptr, nullptr, &visual));
        uint32_t tiles = 0;
        for (const auto& packet : page->fastSpritePageRuntime8003FA20.runtime.packetWrites) {
            if (packet.valid && packet.provenance.helper == 0x8001FDC0u) {
                ++tiles;
                assert(packet.word2CommandKnown);
                assert((packet.word2CommandCode & 1u) != 0u); // raw-texture path
            }
        }
        assert(tiles == visual.activeCount);
        const auto wait = P::PsxCall80035560_WaitFrameDetailed(*page, vsync, 2);
        assert(wait.sourceKnown && wait.waitPending);
        const auto consumed = P::PsxConsume80035560_WaitFrameHostVblankDetailed(*page, vsync, 2);
        assert(consumed.softwareStateCommitted && consumed.softwareWaitComplete);
        const auto end = P::PsxCall8001EBF4_SaveEntryEndFrame(*page);
        assert(end.completeWithinLimits && end.workListSubmitted);
        assert(end.clearImage.r == 255 && end.clearImage.g == 255 && end.clearImage.b == 255);
        // Inspect the committed page consumed by the renderer, not merely
        // the pre-commit packet accumulator.
        PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 committed{};
        assert(PrPsxGraphOwnerDirect::BuildRuntimeState8003FA20FromPageWork(
            page->graph, page->graph.mainPageWorkLists80087288[page->gp368WorkSlot & 1u], committed));
        uint32_t committedTiles = 0u;
        for (const auto& packet : committed.packetWrites) {
            if (!packet.valid || packet.provenance.helper != 0x8001FDC0u) continue;
            ++committedTiles;
            assert(packet.provenance.active && packet.provenance.priority == 0u);
            assert(packet.wordCount == 6u && packet.wordKnown[1] && packet.wordKnown[3] &&
                packet.wordKnown[4] && packet.wordKnown[5]);
            assert(packet.word2CommandKnown && (packet.word2CommandCode & 1u));
            if (i == 0u && committedTiles == 1u) {
                std::cout << "First committed tile: mode/xy/uvclut/wh=" << std::hex
                    << packet.words[1] << '/' << packet.words[3] << '/'
                    << packet.words[4] << '/' << packet.words[5] << std::dec
                    << " drawOffset=" << committed.drawEnvOffsetX80091738 << ','
                    << committed.drawEnvOffsetY8009173A << '\n';
            }
        }
        assert(committedTiles == tiles);
        assert(!E::ReadyForDispatcher(entry, tick));
    }
    assert(entry.frame.complete && entry.runtime.loopIterationsCompleted == 24u &&
        entry.runtime.tailIterationsCompleted == 4u);
    assert(E::ReadyForDispatcher(entry, 128));
    assert(!E::Advance(entry, 128));
    // Ordinary Save? remains a normal event page, not a lingering tile overlay.
    assert(!P::PsxCall8001E750_SaveUiEventFrame(*page, 11, 0, 0, 0, 0));
    for (const auto& packet : page->fastSpritePageRuntime8003FA20.runtime.packetWrites)
        assert(!packet.valid || packet.provenance.helper != 0x8001FDC0u);
    assert(E::Begin(entry) && entry.presentations == 0u && !entry.tickKnown);
    assert(E::Advance(entry, 128) && entry.frame.visualFrame.activeCount == 184u);
    auto bad = entry.frame.visualFrame;
    bad.grid[0] ^= 1u;
    assert(P::PsxCall8001E750_SaveUiEventFrame(*page, 11, 0, 0, 0, 0, nullptr, nullptr, &bad));
    std::cout << "PASS: native reverse table, 24+4 presented frames, duplicate redraw guard, white clear, real tile packets, clean dispatcher handoff\n";
}
