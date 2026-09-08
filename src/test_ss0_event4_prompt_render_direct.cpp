#include "pr/pr_ss0_event4_prompt_render_direct.h"

#include <cstdio>
#include <initializer_list>
#include <limits>

namespace {

using namespace PrSS0Event4PromptRenderDirect;

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            ++g_failed;                                                       \
            std::printf("FAIL:%d: %s\n", __LINE__, #expr);                  \
        }                                                                     \
    } while (false)

Event4PromptInput800203D4 MakeInput(int32_t ctx0 = -1) {
    Event4PromptInput800203D4 input{};
    input.requestBound = true;
    input.choiceSourceKnown = true;
    input.ctx0 = ctx0;
    input.rgb[0] = {true, 0x10u, 0x20u, 0x30u, 0x800203D4u, 0x80050950u};
    input.rgb[1] = {true, 0x11u, 0x21u, 0x31u, 0x800203D4u, 0x80050960u};
    input.rgb[2] = {true, 0x12u, 0x22u, 0x32u, 0x800203D4u, 0x80050970u};
    return input;
}

void TestRequestAndChoiceSourceFailClosed() {
    Event4PromptInput800203D4 input = MakeInput();
    input.requestBound = false;
    CHECK(!BuildEvent4PromptDrawList800203D4(input).accepted);

    input = MakeInput();
    input.choiceSourceKnown = false;
    CHECK(!BuildEvent4PromptDrawList800203D4(input).accepted);
}

void TestRawTextureAcceptsUnknownStackRgb() {
    Event4PromptInput800203D4 input{};
    input.requestBound = true;
    input.choiceSourceKnown = true;
    input.ctx0 = -1;
    input.rgb[0] = {false, 0xAAu, 0xBBu, 0xCCu, 0u, 0u};
    input.rgb[1] = {false, 0x11u, 0x22u, 0x33u, 0u, 0u};
    input.rgb[2] = {false, 0x44u, 0x55u, 0x66u, 0u, 0u};
    const auto list = BuildEvent4PromptDrawList800203D4(input);
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.count == 3u);
    CHECK(!list.rgbSourceKnown);
    CHECK(list.rgbTailUnresolvedWithoutInput);
    CHECK(list.rawTextureOnly);
    CHECK(list.visibleColorIndependentOfRgb);
    for (const auto& sprite : list.sprites) {
        CHECK(sprite.rawTextureKnown);
        CHECK(sprite.rawTexture);
        CHECK(!sprite.rgbKnown);
        CHECK(sprite.word2CommandKnown);
        CHECK(sprite.word2CommandCode == 0x67u);
        CHECK(!sprite.packetWord2ColorKnown);
        CHECK(sprite.packetWord2ColorCode == 0u);
        CHECK(sprite.r == 0u && sprite.g == 0u && sprite.b == 0u);
    }
}

void TestDefaultPrompt() {
    const auto list = BuildEvent4PromptDrawList800203D4(MakeInput(-1));
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.count == 3u);
    CHECK(list.sprites[0].templateAddress == 0x80050950u);
    CHECK(list.sprites[1].templateAddress == 0x80050960u);
    CHECK(list.sprites[2].templateAddress == 0x80050970u);
    CHECK(!list.sprites[1].selected);
    CHECK(!list.sprites[2].selected);
    CHECK(list.sprites[0].screenX == 56 && list.sprites[0].screenY == 57);
    CHECK(list.sprites[1].localX == -90 && list.sprites[1].localY == 29);
    CHECK(list.sprites[2].localX == 18 && list.sprites[2].localY == 32);
    CHECK(list.sprites[0].tpage == 0x3Fu);
    CHECK(list.sprites[0].u == 0x00u && list.sprites[0].v == 0xCBu);
    CHECK(list.sprites[0].packetWord2ColorCode == 0x67302010u);
    CHECK(list.sprites[0].packetWord2ColorKnown);
    CHECK(list.sprites[0].psxCallChain[0] == kFn8001C550_FastSpriteChain);
    CHECK(list.sprites[0].psxCallChain[3] == kFn8003FA20_FastSpriteSubmit);
}

void TestChoiceTemplates() {
    const auto left = BuildEvent4PromptDrawList800203D4(MakeInput(0));
    CHECK(left.accepted);
    CHECK(left.sprites[1].templateAddress == 0x80050980u);
    CHECK(left.sprites[2].templateAddress == 0x80050970u);
    CHECK(left.sprites[1].selected);
    CHECK(!left.sprites[2].selected);

    const auto right = BuildEvent4PromptDrawList800203D4(MakeInput(1));
    CHECK(right.accepted);
    CHECK(right.sprites[1].templateAddress == 0x80050960u);
    CHECK(right.sprites[2].templateAddress == 0x80050990u);
    CHECK(!right.sprites[1].selected);
    CHECK(right.sprites[2].selected);
}

void TestNoncanonicalChoiceUsesOriginalDefaultTemplates() {
    for (const int32_t ctx0 : {
             std::numeric_limits<int32_t>::min(),
             -2,
             2,
             std::numeric_limits<int32_t>::max(),
         }) {
        const auto list = BuildEvent4PromptDrawList800203D4(MakeInput(ctx0));
        CHECK(list.accepted);
        CHECK(list.complete);
        CHECK(list.count == 3u);
        CHECK(list.ctx0 == ctx0);
        CHECK(list.sprites[1].templateAddress == 0x80050960u);
        CHECK(list.sprites[2].templateAddress == 0x80050970u);
        CHECK(!list.sprites[1].selected);
        CHECK(!list.sprites[2].selected);
    }
}

}  // namespace

int main() {
    TestRequestAndChoiceSourceFailClosed();
    TestRawTextureAcceptsUnknownStackRgb();
    TestDefaultPrompt();
    TestChoiceTemplates();
    TestNoncanonicalChoiceUsesOriginalDefaultTemplates();
    if (g_failed != 0) {
        std::printf("test_ss0_event4_prompt_render_direct: failed checks=%d\n",
                    g_failed);
        return 1;
    }
    std::printf("test_ss0_event4_prompt_render_direct: ok\n");
    return 0;
}
