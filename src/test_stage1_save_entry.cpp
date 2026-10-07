#include "pr/pr_stage1_save_entry_direct.h"
#include "pr/pr_psx_event_frame_direct.h"
#include "pr/pr_ss0_raw_sprite_packet_direct.h"
#include "pr/pr_ss0_card_read_callback_direct.h"
#include "pr/pr_ss0_directory_pages_render_direct.h"
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
    // Initial LOAD/REPLAY errors share the native event12/18 prompt packets.
    // Raw texture packets need command authority, not unused RGB authority.
    for (int event : {12, 18}) {
        for (int language = 0; language < 5; ++language) {
          std::vector<std::pair<int32_t, int32_t>> firstPagePositions;
          for (unsigned slot : {0u, 1u}) {
            P::ResetEventFrameState8003FB9C(*page, 320u, 240u);
            namespace G = PrPsxGraphOwnerDirect;
            G::PsxCall8003FC14_ApplyGraphModeFlags(page->graph, 4u);
            G::PsxCall80040AE4_SetDoubleBufferOffsets(page->graph, 0, 0, 0, 240);
            G::PsxCall80040B84_ApplyScreenCenterAndDrawOffset(page->graph);
            if (slot) G::PsxCall80040370_FlipGraph(page->graph);
            assert(!P::PsxCall8001E750_SaveUiEventFrame(*page, event, 1, 0, 0, language));
            unsigned rawCount = 0, partialCount = 0;
            for (const auto& packet : page->graph.mainPageWorkLists80087288[
                     page->gp368WorkSlot & 1u].work.packetWriteMirror) {
                if (!packet.valid || packet.wordCount != 6u) continue;
                uint8_t command = 0;
                assert(PrSS0RawSpritePacketDirect::ResolveCommand8003FA20(packet, command));
                int32_t x = 0, y = 0;
                assert(PrSS0RawSpritePacketDirect::ResolvePagePosition8003FA20(packet, page->graph, x, y));
                if (!slot) firstPagePositions.emplace_back(x, y);
                else assert(firstPagePositions.at(rawCount) == std::make_pair(x, y));
                // All page sprites are authored in the native320x240 page.
                assert(x >= 0 && x < 320 && y >= 0 && y < 240);
                PrSS0RawSpritePacketDirect::TextureOrigin origin{};
                assert(PrSS0RawSpritePacketDirect::ResolveTextureOrigin8003FA20(packet, origin));
                assert(origin.x < 1024 && origin.y < 512 && origin.clutX < 1024 && origin.clutY < 512);
                ++rawCount;
                if (!packet.wordKnown[2]) ++partialCount;
                auto badPacket = packet;
                badPacket.word2CommandKnown = false;
                assert(!PrSS0RawSpritePacketDirect::ResolveCommand8003FA20(badPacket, command));
                for (unsigned word : {0u, 1u, 3u, 4u, 5u}) {
                    badPacket = packet;
                    badPacket.wordKnown[word] = false;
                    assert(!PrSS0RawSpritePacketDirect::ResolveCommand8003FA20(badPacket, command));
                }
                badPacket = packet;
                badPacket.word2CommandCode = 0x64u;
                assert(!PrSS0RawSpritePacketDirect::ResolveCommand8003FA20(badPacket, command));
                badPacket = packet;
                badPacket.wordKnown[2] = true;
                badPacket.words[2] = 0;
                assert(!PrSS0RawSpritePacketDirect::ResolveCommand8003FA20(badPacket, command));
            }
            assert(rawCount && partialCount);
          }
        }
    }
    for (int language = 0; language < 5; ++language) {
        P::ResetEventFrameState8003FB9C(*page, 320u, 240u);
        namespace G = PrPsxGraphOwnerDirect;
        G::PsxCall8003FC14_ApplyGraphModeFlags(page->graph, 4u);
        G::PsxCall80040AE4_SetDoubleBufferOffsets(page->graph, 0, 0, 0, 240);
        G::PsxCall80040B84_ApplyScreenCenterAndDrawOffset(page->graph);
        PrSS0CardReadCallbackDirect::State80017F38 callback{};
        PrSS0DirectoryPagesRenderDirect::MainDirectoryState80021E60 menu{};
        menu.language = language;
        menu.cursor = 1; // Native HI-SCORE selection includes the I/O banner.
        menu.itemValue[1] = 0; // Confirmed choice, not the unconfirmed-1 focus.
        int32_t blink = 1;
        unsigned preparedSlot = 0;
        for (unsigned tick = 0; tick < 300; ++tick) {
            const bool ok = PrSS0CardReadCallbackDirect::Tick80017F38(callback, blink, [&]() {
                menu.blinkOnOff = blink;
                assert(!P::PsxCall8001E750_MainMenuFrameCloseBlocked(*page, menu));
                preparedSlot = page->gp368WorkSlot;
                unsigned mainSprites = 0, boxes = 0;
                for (const auto& packet : page->graph.mainPageWorkLists80087288[preparedSlot].work.packetWriteMirror) {
                    if (!packet.valid) continue;
                    if (packet.wordCount == 6u) {
                        uint8_t command = 0;
                        assert(PrSS0RawSpritePacketDirect::ResolveCommand8003FA20(packet, command));
                        int32_t x = 0, y = 0;
                        assert(PrSS0RawSpritePacketDirect::ResolvePagePosition8003FA20(packet, page->graph, x, y));
                        assert(x >= 0 && x < 320 && y >= 0 && y < 240);
                    }
                    if (packet.provenance.helper == 0x80021E60u) ++mainSprites;
                    if (packet.wordCount == 5u) {
                        int32_t x = 0, y = 0;
                        assert(PrSS0RawSpritePacketDirect::ResolvePagePosition8003FA20(packet, page->graph, x, y));
                        assert(x >= 0 && x < 320 && y >= 115 && y <= 132);
                        // The raw centeredY is negative: using it as pageY
                        // would move the prompt's pseudo-transparent box to0,0.
                        assert(static_cast<int16_t>(packet.words[3] >> 16u) < 20);
                        ++boxes;
                    }
                }
                if (mainSprites != 26 || boxes != 3)
                    std::cerr << "event3 packet counts: language=" << language << " tick=" << tick
                              << " main=" << mainSprites << " boxes=" << boxes << "\n";
                assert(mainSprites == 26 && boxes == 3);
                return true;
            }, [&]() {
                const auto end = P::PsxCall8001EA00_EndFrameDetailed(*page, 0);
                assert(end.completeWithinLimits && end.graphFlipExecuted && end.clearImageExecuted && end.workListSubmitted);
                assert(end.submitSlotFromGp368BeforeFlip == preparedSlot);
                assert(page->graph.word_80096590 == (preparedSlot ^ 1u));
                return true;
            });
            assert(ok);
            assert(callback.gp740 == int32_t((tick + 1) % 20));
            if (tick % 20 == 19) assert(callback.returnValue == 20);
        }
        assert(callback.invocations == 300 && callback.prepares == 15 && callback.ends == 15);
        assert(callback.gp736 == 0 && callback.gp740 == 0 && blink == 0);
    }
    std::cout << "PASS: native reverse table, save entry, event12/18 raw packets, and300VBlank event3 read callbacks in all five languages\n";
}
