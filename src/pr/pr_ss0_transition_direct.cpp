#include "pr_ss0_transition_direct.h"

#include <algorithm>
#include <limits>

namespace PrSS0TransitionDirect {
namespace {

static bool Append(TransitionPlan& plan, const TransitionAction& action)
{
    if (plan.count >=
        sizeof(plan.actions) / sizeof(plan.actions[0])) {
        plan.truncated = true;
        return false;
    }
    plan.actions[plan.count++] = action;
    return true;
}

static void AppendCall(TransitionPlan& plan,
                       TransitionActionKind kind,
                       uint32_t psxFunction,
                       uint32_t arg0 = 0,
                       uint32_t arg1 = 0,
                       uint32_t arg2 = 0,
                       uint32_t arg3 = 0,
                       int32_t repeatCount = 1,
                       bool repeatKnown = true)
{
    TransitionAction action{};
    action.kind = kind;
    action.psxFunction = psxFunction;
    action.arg0 = arg0;
    action.arg1 = arg1;
    action.arg2 = arg2;
    action.arg3 = arg3;
    action.repeatCount = repeatCount;
    action.repeatKnown = repeatKnown;
    (void)Append(plan, action);
}

static bool IsKnownMode8001EA74(int32_t mode)
{
    return mode == 1 || mode == 2 || mode == 3 || mode == 4 || mode == 5 ||
           mode == 6;
}

static uint32_t PackFfd4Args(int32_t preFfd4Arg, int32_t postFfd4Arg)
{
    return ((static_cast<uint32_t>(preFfd4Arg) & 0xFFFFu) << 16) |
           (static_cast<uint32_t>(postFfd4Arg) & 0xFFFFu);
}

static void AppendLoopBody80020110(TransitionPlan& plan,
                                   uint32_t ctxAddress,
                                   int32_t mode,
                                   bool word800916DC)
{
    const bool repeatKnown =
        HasKnownModeBodyTicks8001EA74(mode, word800916DC);
    const int32_t repeatCount =
        repeatKnown ? KnownModeBodyTicks8001EA74(mode, word800916DC) : 0;
    AppendCall(plan,
               TransitionActionKind::Call8001EA74,
               kFn8001EA74,
               ctxAddress,
               static_cast<uint32_t>(mode),
               word800916DC ? 1 : 0,
               0,
               repeatCount,
               repeatKnown);
    AppendCall(plan,
               TransitionActionKind::Call80027194,
               kFn80027194,
               30,
               0,
               0,
               0,
               repeatCount,
               repeatKnown);
    AppendCall(plan,
               TransitionActionKind::Call80035560,
               kFn80035560,
               2,
               0,
               0,
               0,
               repeatCount,
               repeatKnown);
    AppendCall(plan,
               TransitionActionKind::Call8001EBF4,
               kFn8001EBF4,
               ctxAddress,
               0,
               0,
               0,
               repeatCount,
               repeatKnown);
    AppendCall(plan,
               TransitionActionKind::Call8001F518,
               kFn8001F518,
               0,
               0,
               0,
               0,
               repeatCount,
               repeatKnown);
}

static void AppendLoopBody800201AC(TransitionPlan& plan,
                                   uint32_t ctxAddress,
                                   int32_t mode,
                                   bool word800916DC)
{
    const bool repeatKnown =
        HasKnownModeBodyTicks8001EA74(mode, word800916DC);
    const int32_t repeatCount =
        repeatKnown ? KnownModeBodyTicks8001EA74(mode, word800916DC) : 0;
    AppendCall(plan,
               TransitionActionKind::Call8001EA74,
               kFn8001EA74,
               ctxAddress,
               static_cast<uint32_t>(mode),
               word800916DC ? 1 : 0,
               0,
               repeatCount,
               repeatKnown);
    AppendCall(plan,
               TransitionActionKind::Call80035560,
               kFn80035560,
               2,
               0,
               0,
               0,
               repeatCount,
               repeatKnown);
    AppendCall(plan,
               TransitionActionKind::Call8001EBF4,
               kFn8001EBF4,
               ctxAddress,
               0,
               0,
               0,
               repeatCount,
               repeatKnown);
    AppendCall(plan,
               TransitionActionKind::Call8001F518,
               kFn8001F518,
               0,
               0,
               0,
               0,
               repeatCount,
               repeatKnown);
}

static constexpr FastTransitionSpriteTemplate800201AC
    kFastTransitionTemplates8001B25C[10] = {
        {true, 0x80050380u, 0x10000040u, 0x03A8u, 0x014Bu,
         0x0014u, 0x00A0u, 0x0110u, 0x01E8u},
        {true, 0x80050390u, 0x10000040u, 0x03ADu, 0x014Bu,
         0x0014u, 0x00A0u, 0x0110u, 0x01E9u},
        {true, 0x800503A0u, 0x10000040u, 0x0380u, 0x014Bu,
         0x0078u, 0x000Cu, 0x0110u, 0x01EAu},
        {true, 0x800503B0u, 0x10000040u, 0x0380u, 0x0157u,
         0x0078u, 0x000Cu, 0x0110u, 0x01EBu},
        {true, 0x800503C0u, 0x10000040u, 0x0380u, 0x0177u,
         0x0078u, 0x0014u, 0x0110u, 0x01ECu},
        {true, 0x800503D0u, 0x10000040u, 0x0380u, 0x018Bu,
         0x0078u, 0x0014u, 0x0110u, 0x01EDu},
        {true, 0x800503E0u, 0x10000040u, 0x0380u, 0x0163u,
         0x0014u, 0x0014u, 0x0110u, 0x01EEu},
        {true, 0x800503F0u, 0x10000040u, 0x0385u, 0x0163u,
         0x0014u, 0x0014u, 0x0110u, 0x01EFu},
        {true, 0x80050400u, 0x10000040u, 0x038Au, 0x0163u,
         0x0014u, 0x0014u, 0x0110u, 0x01F0u},
        {true, 0x80050410u, 0x10000040u, 0x038Fu, 0x0163u,
         0x0014u, 0x0014u, 0x0110u, 0x01F1u},
};

static constexpr FastTransitionSpriteTemplate800201AC
    kFastTransitionTemplatesScene0Frames[44] = {
        {true, 0x8004E860u, 0x10000040u, 0x0380u, 0x0163u,
         0x0014u, 0x0014u, 0x0110u, 0x01E4u},
        {true, 0x8004E870u, 0x10000040u, 0x0385u, 0x0163u,
         0x0014u, 0x0014u, 0x0110u, 0x01E5u},
        {true, 0x8004E880u, 0x10000040u, 0x038Au, 0x0163u,
         0x0014u, 0x0014u, 0x0110u, 0x01E6u},
        {true, 0x8004E890u, 0x10000040u, 0x038Fu, 0x0163u,
         0x0014u, 0x0014u, 0x0110u, 0x01E7u},
        {true, 0x8004E8A0u, 0x10000040u, 0x03A8u, 0x014Bu,
         0x0014u, 0x00A0u, 0x0110u, 0x01E8u},
        {true, 0x8004E8B0u, 0x10000040u, 0x03ADu, 0x014Bu,
         0x0014u, 0x00A0u, 0x0110u, 0x01E9u},
        {true, 0x8004E8C0u, 0x10000040u, 0x0380u, 0x014Bu,
         0x0078u, 0x000Cu, 0x0110u, 0x01EAu},
        {true, 0x8004E8D0u, 0x10000040u, 0x0380u, 0x0157u,
         0x0078u, 0x000Cu, 0x0110u, 0x01EBu},
        {true, 0x8004E8E0u, 0x10000040u, 0x0380u, 0x0177u,
         0x0078u, 0x0014u, 0x0110u, 0x01ECu},
        {true, 0x8004E8F0u, 0x10000040u, 0x0380u, 0x018Bu,
         0x0078u, 0x0014u, 0x0110u, 0x01EDu},
        {true, 0x8004E900u, 0x10000040u, 0x0380u, 0x0163u,
         0x0014u, 0x0014u, 0x0110u, 0x01EEu},
        {true, 0x8004E910u, 0x10000040u, 0x0385u, 0x0163u,
         0x0014u, 0x0014u, 0x0110u, 0x01EFu},
        {true, 0x8004E920u, 0x10000040u, 0x038Au, 0x0163u,
         0x0014u, 0x0014u, 0x0110u, 0x01F0u},
        {true, 0x8004E930u, 0x10000040u, 0x038Fu, 0x0163u,
         0x0014u, 0x0014u, 0x0110u, 0x01F1u},
        {true, 0x8004E940u, 0x00000040u, 0x03B2u, 0x014Bu,
         0x0014u, 0x0014u, 0x0110u, 0x01F2u},
        {true, 0x8004E950u, 0x00000040u, 0x03B2u, 0x015Fu,
         0x0014u, 0x0014u, 0x0110u, 0x01F3u},
        {true, 0x8004E960u, 0x00000040u, 0x03B2u, 0x0173u,
         0x0014u, 0x0014u, 0x0110u, 0x01F4u},
        {true, 0x8004E970u, 0x00000040u, 0x03B2u, 0x0187u,
         0x0014u, 0x0014u, 0x0110u, 0x01F5u},
        {true, 0x8004E980u, 0x10000040u, 0x039Eu, 0x014Bu,
         0x0014u, 0x00A0u, 0x0110u, 0x01E0u},
        {true, 0x8004E990u, 0x10000040u, 0x03B2u, 0x019Bu,
         0x0014u, 0x0014u, 0x0110u, 0x01E1u},
        {true, 0x8004E9A0u, 0x10000040u, 0x03B2u, 0x01AFu,
         0x0014u, 0x0014u, 0x0110u, 0x01E2u},
        {true, 0x8004E9B0u, 0x10000040u, 0x03B2u, 0x01C3u,
         0x0014u, 0x0014u, 0x0110u, 0x01E1u},
        {true, 0x8004E9C0u, 0x10000040u, 0x03B2u, 0x01D7u,
         0x0014u, 0x0014u, 0x0110u, 0x01E2u},
        {true, 0x8004E9D0u, 0x10000040u, 0x03B7u, 0x014Bu,
         0x0014u, 0x0014u, 0x0110u, 0x01E1u},
        {true, 0x8004E9E0u, 0x10000040u, 0x03B7u, 0x015Fu,
         0x0014u, 0x0014u, 0x0110u, 0x01E2u},
        {true, 0x8004E9F0u, 0x10000040u, 0x03B7u, 0x0173u,
         0x0014u, 0x0014u, 0x0110u, 0x01E1u},
        {true, 0x8004EA00u, 0x10000040u, 0x03B7u, 0x0187u,
         0x0014u, 0x0014u, 0x0110u, 0x01E2u},
        {true, 0x8004EA10u, 0x10000040u, 0x03B7u, 0x019Bu,
         0x0014u, 0x0014u, 0x0110u, 0x01E1u},
        {true, 0x8004EA20u, 0x10000040u, 0x03B7u, 0x01AFu,
         0x0014u, 0x0014u, 0x0110u, 0x01E2u},
        {true, 0x8004EA30u, 0x10000040u, 0x03B7u, 0x01C3u,
         0x0014u, 0x0014u, 0x0110u, 0x01E1u},
        {true, 0x8004EA40u, 0x10000040u, 0x03B7u, 0x01D7u,
         0x0014u, 0x0014u, 0x0110u, 0x01E2u},
        {true, 0x8004EA50u, 0x10000040u, 0x03A3u, 0x014Bu,
         0x0014u, 0x00A0u, 0x0110u, 0x01E3u},
        {true, 0x8004EA60u, 0x10000040u, 0x0394u, 0x0163u,
         0x0014u, 0x0014u, 0x0110u, 0x01F0u},
        {true, 0x8004EA70u, 0x10000040u, 0x0399u, 0x0163u,
         0x0014u, 0x0014u, 0x0110u, 0x01F1u},
        {true, 0x8004EA80u, 0x10000040u, 0x0394u, 0x019Fu,
         0x0014u, 0x0014u, 0x0110u, 0x01F0u},
        {true, 0x8004EA90u, 0x10000040u, 0x0399u, 0x019Fu,
         0x0014u, 0x0014u, 0x0110u, 0x01F1u},
        {true, 0x8004EAA0u, 0x50000040u, 0x0394u, 0x01B3u,
         0x0014u, 0x0014u, 0x0110u, 0x01F0u},
        {true, 0x8004EAB0u, 0x50000040u, 0x0399u, 0x01B3u,
         0x0014u, 0x0014u, 0x0110u, 0x01F1u},
        {true, 0x8004EAC0u, 0x50000040u, 0x0394u, 0x01C7u,
         0x0014u, 0x0014u, 0x0110u, 0x01F0u},
        {true, 0x8004EAD0u, 0x50000040u, 0x0399u, 0x01C7u,
         0x0014u, 0x0014u, 0x0110u, 0x01F1u},
        {true, 0x8004EAE0u, 0x50000040u, 0x0394u, 0x01DBu,
         0x0014u, 0x0014u, 0x0110u, 0x01F0u},
        {true, 0x8004EAF0u, 0x50000040u, 0x0399u, 0x01DBu,
         0x0014u, 0x0014u, 0x0110u, 0x01F1u},
        {true, 0x8004EB00u, 0x50000040u, 0x039Eu, 0x01EBu,
         0x0014u, 0x0014u, 0x0110u, 0x01F0u},
        {true, 0x8004EB10u, 0x50000040u, 0x03A3u, 0x01EBu,
         0x0014u, 0x0014u, 0x0110u, 0x01F1u},
};

static constexpr uint8_t kTileOrderF180Row[192] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 9, 8, 7, 6,
    5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 9, 8, 7, 6, 5,
    4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 3, 4, 5, 6, 7, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 8, 7, 6, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 3, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 7, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 4, 4,
    4, 4, 4, 4, 5, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5,
};

static constexpr uint8_t kTileOrderF180Col[192] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4,
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5,
    4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3,
    4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 13, 13, 13, 13, 13, 13, 13, 12, 11, 10, 9, 8, 7,
    6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 12, 12, 12, 12, 12, 11, 10, 9, 8, 7, 6, 5, 4,
    4, 4, 4, 4, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11, 10, 9,
    8, 7, 6, 5, 5, 5, 6, 7, 8, 9, 10, 10, 9, 8, 7, 6,
};

static bool AppendFastTransitionSprite8001B25C(
    FastTransitionFramePlan800201AC& plan,
    int16_t x,
    int16_t y,
    uint32_t templateAddress,
    uint16_t priority)
{
    const FastTransitionSpriteTemplate800201AC spriteTemplate =
        ResolveFastTransitionSpriteTemplate800201AC(templateAddress);
    if (!spriteTemplate.known ||
        plan.commandCount >= kFastTransitionMaxSpriteCommands800201AC) {
        plan.truncated = true;
        return false;
    }
    FastTransitionSpriteCommand800201AC& command =
        plan.commands[plan.commandCount];
    command.active = true;
    command.rawTextureKnown = true;
    command.rawTexture = (spriteTemplate.attr & 0x40u) != 0u;
    command.semiTransparentKnown = true;
    command.semiTransparent =
        (spriteTemplate.attr & 0x40000000u) != 0u;
    command.abrKnown = true;
    command.abr = 1u;
    command.x = x;
    command.y = y;
    command.priority = priority;
    command.order = static_cast<uint16_t>(plan.commandCount + 1u);
    command.spriteTemplate = spriteTemplate;
    ++plan.commandCount;
    return true;
}

static bool AppendOutroNoSubboxSprite8001F230(
    FastTransitionFramePlan800201AC& plan,
    int16_t x,
    int16_t y,
    uint32_t templateAddress)
{
    return AppendFastTransitionSprite8001B25C(
        plan, x, y, templateAddress, 5u);
}

static bool AppendFinalNoVideoSprite8001FEB4(
    FastTransitionFramePlan800201AC& plan,
    int16_t x,
    int16_t y,
    uint32_t templateIndex)
{
    static constexpr uint32_t kTemplateAddresses[4] = {
        0x800503E0u, 0x800503F0u, 0x80050400u, 0x80050410u};
    if (templateIndex >= 4u) {
        plan.truncated = true;
        return false;
    }
    return AppendFastTransitionSprite8001B25C(
        plan, x, y, kTemplateAddresses[templateIndex], 0u);
}

static bool AppendScene0FrameSprite8001B590(
    FastTransitionFramePlan800201AC& plan,
    int16_t x,
    int16_t y,
    uint32_t templateAddress)
{
    return AppendFastTransitionSprite8001B25C(
        plan, x, y, templateAddress, 5u);
}

static bool AppendSubtitleFrameSprites8001C864(
    FastTransitionFramePlan800201AC& plan)
{
    if (!AppendScene0FrameSprite8001B590(
            plan, 280, 200, 0x8004E940u) ||
        !AppendScene0FrameSprite8001B590(
            plan, 20, 200, 0x8004E950u) ||
        !AppendScene0FrameSprite8001B590(
            plan, 280, 180, 0x8004E960u) ||
        !AppendScene0FrameSprite8001B590(
            plan, 20, 180, 0x8004E970u)) {
        return false;
    }

    for (int i = 0; i < 7; ++i) {
        const int16_t x0 = static_cast<int16_t>(40 + 40 * i);
        const int16_t x1 = static_cast<int16_t>(20 + 40 * i);
        if (!AppendScene0FrameSprite8001B590(
                plan, x0, 0, 0x8004E900u) ||
            !AppendScene0FrameSprite8001B590(
                plan, x1, 0, 0x8004E910u) ||
            !AppendScene0FrameSprite8001B590(
                plan, x0, 220, 0x8004E920u) ||
            !AppendScene0FrameSprite8001B590(
                plan, x1, 220, 0x8004E930u)) {
            return false;
        }
    }

    for (int i = 0; i < 6; ++i) {
        const int16_t y0 = static_cast<int16_t>(20 + 40 * i);
        const int16_t y1 = static_cast<int16_t>(40 * i);
        const int16_t x0 = static_cast<int16_t>(40 + 40 * i);
        const int16_t x1 = static_cast<int16_t>(60 + 40 * i);
        if (!AppendScene0FrameSprite8001B590(
                plan, 0, y0, 0x8004E920u) ||
            !AppendScene0FrameSprite8001B590(
                plan, 0, y1, 0x8004E900u) ||
            !AppendScene0FrameSprite8001B590(
                plan, 300, y0, 0x8004E930u) ||
            !AppendScene0FrameSprite8001B590(
                plan, 300, y1, 0x8004E910u) ||
            !AppendScene0FrameSprite8001B590(
                plan, x0, 200, 0x8004E860u) ||
            !AppendScene0FrameSprite8001B590(
                plan, x1, 200, 0x8004E870u) ||
            !AppendScene0FrameSprite8001B590(
                plan, x0, 180, 0x8004E880u) ||
            !AppendScene0FrameSprite8001B590(
                plan, x1, 180, 0x8004E890u)) {
            return false;
        }
    }

    return AppendScene0FrameSprite8001B590(
               plan, 40, 160, 0x8004E8E0u) &&
           AppendScene0FrameSprite8001B590(
               plan, 160, 160, 0x8004E8F0u) &&
           AppendScene0FrameSprite8001B590(
               plan, 40, 20, 0x8004E8C0u) &&
           AppendScene0FrameSprite8001B590(
               plan, 160, 20, 0x8004E8D0u) &&
           AppendScene0FrameSprite8001B590(
               plan, 20, 20, 0x8004E8A0u) &&
           AppendScene0FrameSprite8001B590(
               plan, 280, 20, 0x8004E8B0u);
}

static bool AppendNoSubtitleFrameSprites8001CE30(
    FastTransitionFramePlan800201AC& plan)
{
    for (int i = 0; i < 7; ++i) {
        const int16_t x0 = static_cast<int16_t>(40 + 40 * i);
        const int16_t x1 = static_cast<int16_t>(20 + 40 * i);
        if (!AppendScene0FrameSprite8001B590(
                plan, x0, 0, 0x8004E900u) ||
            !AppendScene0FrameSprite8001B590(
                plan, x1, 0, 0x8004E910u) ||
            !AppendScene0FrameSprite8001B590(
                plan, x0, 220, 0x8004E920u) ||
            !AppendScene0FrameSprite8001B590(
                plan, x1, 220, 0x8004E930u) ||
            !AppendScene0FrameSprite8001B590(
                plan, x0, 20, 0x8004E920u) ||
            !AppendScene0FrameSprite8001B590(
                plan, x1, 20, 0x8004E930u) ||
            !AppendScene0FrameSprite8001B590(
                plan, x0, 200, 0x8004E900u) ||
            !AppendScene0FrameSprite8001B590(
                plan, x1, 200, 0x8004E910u)) {
            return false;
        }
    }

    for (int i = 0; i < 6; ++i) {
        const int16_t y0 = static_cast<int16_t>(20 + 40 * i);
        const int16_t y1 = static_cast<int16_t>(40 * i);
        if (!AppendScene0FrameSprite8001B590(
                plan, 0, y0, 0x8004E920u) ||
            !AppendScene0FrameSprite8001B590(
                plan, 0, y1, 0x8004E900u) ||
            !AppendScene0FrameSprite8001B590(
                plan, 300, y0, 0x8004E930u) ||
            !AppendScene0FrameSprite8001B590(
                plan, 300, y1, 0x8004E910u)) {
            return false;
        }
    }

    static constexpr struct {
        int16_t x;
        uint32_t templateAddress;
    } kBottomRow[12] = {
        {40, 0x8004EA60u},
        {60, 0x8004EA70u},
        {80, 0x8004EA80u},
        {100, 0x8004EA90u},
        {120, 0x8004EAA0u},
        {140, 0x8004EAB0u},
        {160, 0x8004EAC0u},
        {180, 0x8004EAD0u},
        {200, 0x8004EAE0u},
        {220, 0x8004EAF0u},
        {240, 0x8004EB00u},
        {260, 0x8004EB10u},
    };
    for (const auto& sprite : kBottomRow) {
        if (!AppendScene0FrameSprite8001B590(
                plan, sprite.x, 180, sprite.templateAddress)) {
            return false;
        }
    }

    static constexpr struct {
        int16_t x;
        uint32_t templateAddress;
    } kCenterRow[14] = {
        {280, 0x8004E980u},
        {20, 0x8004EA50u},
        {160, 0x8004E9F0u},
        {140, 0x8004E9E0u},
        {180, 0x8004EA00u},
        {120, 0x8004E9D0u},
        {200, 0x8004EA10u},
        {100, 0x8004E9C0u},
        {240, 0x8004EA30u},
        {80, 0x8004E9B0u},
        {220, 0x8004EA20u},
        {60, 0x8004E9A0u},
        {260, 0x8004EA40u},
        {40, 0x8004E990u},
    };
    for (const auto& sprite : kCenterRow) {
        if (!AppendScene0FrameSprite8001B590(
                plan, sprite.x, 40, sprite.templateAddress)) {
            return false;
        }
    }
    return true;
}

static uint32_t ResolveVisualFrameFunction8001EA74(
    FastTransitionFrameKind800201AC kind)
{
    switch (kind) {
    case FastTransitionFrameKind800201AC::NoSubtitleFrame8001CE30:
        return kFn8001CE30;
    case FastTransitionFrameKind800201AC::SubtitleFrame8001C864:
        return kFn8001C864;
    case FastTransitionFrameKind800201AC::OutroNoSubboxFrame8001F230:
        return kFn8001F230;
    case FastTransitionFrameKind800201AC::FinalNoVideoFrame8001FEB4:
        return kFn8001FEB4;
    default:
        return 0u;
    }
}

static bool AppendFastTransitionPresentAction8001EBF4(
    FastTransitionPresentPlan8001EBF4& plan,
    FastTransitionPresentActionKind8001EBF4 kind,
    uint32_t psxFunction,
    uint32_t arg0 = 0u,
    uint32_t arg1 = 0u,
    uint32_t arg2 = 0u)
{
    if (plan.actionCount >= kFastTransitionMaxPresentActions8001EBF4) {
        plan.truncated = true;
        return false;
    }
    FastTransitionPresentAction8001EBF4& action =
        plan.actions[plan.actionCount++];
    action.kind = kind;
    action.psxFunction = psxFunction;
    action.arg0 = arg0;
    action.arg1 = arg1;
    action.arg2 = arg2;
    return true;
}

static constexpr uint32_t kSlowTransitionBodyIterations80020110 = 24u;
static constexpr uint32_t kSlowTransitionTailIterations80020110 = 4u;

// SCUS_941.83 0x80050730..0x800508AF.  Each style is 12 rows by two
// 32-bit words.  Keep these tables beside the translated 8001EF40 state
// machine so the direct runtime never borrows the former Windows-S0 curtain.
static constexpr uint32_t kLoadingPatternMasks8001EF40[4][12][2] = {
    {
        {0x00000000u, 0x00000000u}, {0x00000000u, 0x00000000u},
        {0x90210000u, 0x00000000u}, {0x90210000u, 0x00000000u},
        {0x93270CE0u, 0x00000000u}, {0xF4A91290u, 0x00000000u},
        {0x94A91290u, 0x00000000u}, {0x94A91290u, 0x00000000u},
        {0x93270C95u, 0x55555554u}, {0x00000000u, 0x00000000u},
        {0x00000000u, 0x00000000u}, {0x00000000u, 0x00000000u},
    },
    {
        {0x00000000u, 0x00000000u}, {0x00000000u, 0x00000000u},
        {0x90000000u, 0x00000000u}, {0x93000000u, 0x00000000u},
        {0x90B8E19Cu, 0x00000000u}, {0xF3A52252u, 0x00000000u},
        {0x94A52252u, 0x00000000u}, {0x94A52252u, 0x00000000u},
        {0x93A4E192u, 0xAAAAAAAAu}, {0x00002000u, 0x00000000u},
        {0x0000C000u, 0x00000000u}, {0x00000000u, 0x00000000u},
    },
    {
        {0x00000000u, 0x00000000u}, {0x00000000u, 0x00000000u},
        {0x88008000u, 0x00000000u}, {0x89888060u, 0x00000000u},
        {0x8841C010u, 0x1CC60000u}, {0x89C88070u, 0x21290000u},
        {0xAA488090u, 0x19E80000u}, {0xDA488090u, 0x05090000u},
        {0x89C84070u, 0x38C65554u}, {0x00000000u, 0x00000000u},
        {0x00000000u, 0x00000000u}, {0x00000000u, 0x00000000u},
    },
    {
        {0x00000000u, 0x00000000u}, {0x00000000u, 0x00000000u},
        {0x48090000u, 0x00000000u}, {0x48090000u, 0x00000000u},
        {0x49891800u, 0x00000000u}, {0x7A492400u, 0x00000000u},
        {0x4BC92400u, 0x00000000u}, {0x4A092400u, 0x00000000u},
        {0x498918AAu, 0xAAAAAAA8u}, {0x00000000u, 0x00000000u},
        {0x00000000u, 0x00000000u}, {0x00000000u, 0x00000000u},
    },
};

static constexpr uint32_t kLoadingPatternColorCode800508B0[4] = {
    0x401C0B5Au,
    0x40003F1Eu,
    0x40AE2A02u,
    0x4053005Du,
};

static constexpr uint32_t kLoadingPatternWordLimit800508B4[4] = {
    2u, 2u, 2u, 2u,
};

static uint64_t HashLoadingPatternGridFnv1a8001EF40(
    const std::array<uint8_t, kLoadingPatternGridCells8001EF40>& grid)
{
    uint64_t hash = 1469598103934665603ull;
    for (const uint8_t cell : grid) {
        hash ^= static_cast<uint64_t>(cell);
        hash *= 1099511628211ull;
    }
    return hash;
}

static bool IsKnownSlowTransitionTuple80020110(uint32_t ctxAddress,
                                               int32_t mode,
                                               int32_t preFfd4Arg,
                                               int32_t postFfd4Arg)
{
    if (ctxAddress == 0u) {
        return (mode == 3 || mode == 4) && preFfd4Arg == 2 &&
               postFfd4Arg == 1;
    }
    if (ctxAddress != kScene0WorkAddress) {
        return false;
    }
    return (mode == 1 && preFfd4Arg == 2 && postFfd4Arg == 1) ||
           (mode == 2 && preFfd4Arg == 1 && postFfd4Arg == 2);
}

static uint32_t ResolveSlowTransitionStartFrame80020110(int32_t mode)
{
    if (mode == 1) {
        return kScene0TitleIntroTransitionStartFrame6815;
    }
    if (mode == 2) {
        return kScene0TitleExitStartFrame7854;
    }
    if (mode == 3 || mode == 4) {
        return kScene0MainMenuEntryTransitionRelativeTickOrigin;
    }
    return 0u;
}

static uint32_t ResolveSlowTransitionVisualFunction80020110(int32_t mode)
{
    if (mode == 1) {
        return kFn8001F524;
    }
    if (mode == 2 || mode == 3 || mode == 4) {
        return kFn8001FDC0;
    }
    return 0u;
}

} // namespace

bool RuntimeCutoverAllowed()
{
    return false;
}

uint8_t ResolveLoadingPatternStyle8001EF40(int16_t word800916E0)
{
    if (word800916E0 == 2) {
        return 1u;
    }
    if (word800916E0 == 3) {
        return 2u;
    }
    if (word800916E0 == 4) {
        return 3u;
    }
    return 0u;
}

bool ResolveSceneEntryLoadingMode801C7284(int sceneIndex, int16_t& outMode)
{
    if (sceneIndex < 0 ||
        static_cast<std::size_t>(sceneIndex) >= kSceneEntryLoadingModes801CCBBC.size()) {
        return false;
    }
    outMode = kSceneEntryLoadingModes801CCBBC[static_cast<std::size_t>(sceneIndex)];
    return true;
}

void ResetLoadingPatternState8001EF14(LoadingPatternRuntime8001EF40& runtime)
{
    // Unlike 8001FFD4, 8001EF14 clears all three cursors and the grid.
    // Callback ownership is independent (80015408/8001545C).
    runtime.countersKnown = true;
    runtime.bitCursorGp49 = 0u;
    runtime.wordCursorGp50 = 0u;
    runtime.frameCounterGp51 = 0u;
    runtime.liveGrid.fill(0u);
}

bool BeginLoadingPatternRuntimeAfter8001FFD4(
    LoadingPatternRuntime8001EF40& runtime,
    int16_t word800916E0,
    const uint8_t* completedLiveGrid,
    std::size_t completedLiveGridCount)
{
    if (!completedLiveGrid ||
        completedLiveGridCount != kLoadingPatternGridCells8001EF40) {
        return false;
    }
    for (std::size_t i = 0u; i < completedLiveGridCount; ++i) {
        if (completedLiveGrid[i] > 1u) {
            return false;
        }
    }

    // 8001FFD4 resets gp+0xC4 (gp[49]) and republishes the live grid, but it
    // does not clear gp[50]/gp[51].  Preserve those two counters across
    // Loading pages after their process-start zero initialization.
    if (!runtime.countersKnown) {
        runtime.wordCursorGp50 = 0u;
        runtime.frameCounterGp51 = 0u;
        runtime.mutationSerial = 0u;
        runtime.countersKnown = true;
    }
    runtime.active = true;
    runtime.style = ResolveLoadingPatternStyle8001EF40(word800916E0);
    runtime.bitCursorGp49 = 0u;
    std::copy(completedLiveGrid,
              completedLiveGrid + completedLiveGridCount,
              runtime.liveGrid.begin());
    return true;
}

LoadingPatternFrame8001EF40 TickLoadingPatternRuntime8001EF40(
    LoadingPatternRuntime8001EF40& runtime)
{
    LoadingPatternFrame8001EF40 out{};
    if (!runtime.countersKnown || !runtime.active || runtime.style >= 4u) {
        return out;
    }

    // Exact 8001EF40 order: test gp[51] before incrementing it.  The first
    // callback therefore mutates the grid when process-start gp[51] is zero.
    if (runtime.frameCounterGp51 ==
        3u * (runtime.frameCounterGp51 / 3u)) {
        if (runtime.bitCursorGp49 >= 32u) {
            runtime.bitCursorGp49 = 0u;
            ++runtime.wordCursorGp50;
        }
        if (runtime.wordCursorGp50 ==
            kLoadingPatternWordLimit800508B4[runtime.style]) {
            runtime.wordCursorGp50 = 0u;
            runtime.bitCursorGp49 = 0u;
        }

        const uint32_t bit = 31u - runtime.bitCursorGp49;
        for (std::size_t row = 0u;
             row < kLoadingPatternGridRows8001EF40;
             ++row) {
            const std::size_t base =
                row * kLoadingPatternGridColumns8001EF40;
            for (std::size_t col = 0u;
                 col + 1u < kLoadingPatternGridColumns8001EF40;
                 ++col) {
                runtime.liveGrid[base + col] =
                    runtime.liveGrid[base + col + 1u];
            }
            const uint32_t mask =
                kLoadingPatternMasks8001EF40[runtime.style][row]
                                                [runtime.wordCursorGp50];
            runtime.liveGrid[
                base + kLoadingPatternGridColumns8001EF40 - 1u] =
                (mask & (1u << bit)) != 0u ? 1u : 0u;
        }
        ++runtime.bitCursorGp49;
        ++runtime.mutationSerial;
        out.mutationApplied = true;
    }

    ++runtime.frameCounterGp51;
    out.known = true;
    out.style = runtime.style;
    out.sourceFunction = kFn8001EF40;
    out.drawHighlightFunction = kFn8001C4EC;
    out.drawTileFunction = kFn8001C550;
    out.boxFillAttr8001B6C4 =
        kLoadingPatternColorCode800508B0[runtime.style];
    const uint32_t attr = out.boxFillAttr8001B6C4;
    const uint32_t command = ((attr >> 29u) & 0x02u) | 0x60u;
    out.boxFillGpuColorCode8003EE84 =
        (command << 24u) |
        ((attr & 0xFFu) << 16u) |
        (attr & 0x0000FF00u) |
        ((attr >> 16u) & 0xFFu);
    out.bitCursorGp49 = runtime.bitCursorGp49;
    out.wordCursorGp50 = runtime.wordCursorGp50;
    out.frameCounterGp51 = runtime.frameCounterGp51;
    out.mutationSerial = runtime.mutationSerial;
    out.liveGrid = runtime.liveGrid;
    for (const uint8_t cell : out.liveGrid) {
        out.highlightCount += cell != 0u ? 1u : 0u;
    }
    out.liveGridFnv1a = HashLoadingPatternGridFnv1a8001EF40(out.liveGrid);
    return out;
}

void StopLoadingPatternRuntime8001EF40(
    LoadingPatternRuntime8001EF40& runtime)
{
    // The callback is removed by 8001545C, but gp[50]/gp[51] remain global
    // process state for the next 8001EF40 activation.
    runtime.active = false;
}

bool HasKnownModeBodyTicks8001EA74(int32_t mode, bool word800916DC)
{
    if (mode == 1 || mode == 2 || mode == 3 || mode == 4) {
        return true;
    }
    if (mode == 5 || mode == 6) {
        return true;
    }
    (void)word800916DC;
    return false;
}

int32_t KnownModeBodyTicks8001EA74(int32_t mode, bool word800916DC)
{
    if (mode == 1 || mode == 2 || mode == 3 || mode == 4) {
        return 24;
    }
    if (mode == 5 || mode == 6) {
        return word800916DC ? 31 : 16;
    }
    return 0;
}

bool BeginFastTransitionRuntime800201AC(
    FastTransitionRuntime800201AC& runtime,
    uint32_t ctxAddress,
    int32_t mode,
    int32_t preFfd4Arg,
    int32_t postFfd4Arg,
    bool word800916DC)
{
    runtime = FastTransitionRuntime800201AC{};
    if (ctxAddress != kScene0WorkAddress ||
        !HasKnownModeBodyTicks8001EA74(mode, word800916DC) ||
        (mode != 5 && mode != 6) ||
        preFfd4Arg != 1 || postFfd4Arg != 2) {
        return false;
    }

    runtime.active = true;
    runtime.ctxAddress = ctxAddress;
    runtime.mode = mode;
    runtime.preFfd4Arg = preFfd4Arg;
    runtime.postFfd4Arg = postFfd4Arg;
    runtime.word800916DC = word800916DC;
    runtime.phase = FastTransitionRuntimePhase800201AC::Loop8001EA74;
    runtime.loopIterationsRequired = static_cast<uint32_t>(
        KnownModeBodyTicks8001EA74(mode, word800916DC));
    runtime.tileMaskKnown8001EEAC = true;
    runtime.postFfd4Applied = false;
    runtime.gp196 = 0u;
    runtime.tileMaskMutationSerial8001EEAC = 1u;
    std::fill(runtime.tileMask8001EEAC.begin(),
              runtime.tileMask8001EEAC.end(),
              preFfd4Arg == 1 ? 0u : 1u);
    return true;
}

FastTransitionTickResult800201AC TickFastTransitionRuntime800201AC(
    FastTransitionRuntime800201AC& runtime)
{
    FastTransitionTickResult800201AC out{};
    out.phaseBefore = runtime.phase;
    out.phaseAfter = runtime.phase;
    out.loopIterationsCompleted = runtime.loopIterationsCompleted;
    out.tailIterationsCompleted = runtime.tailIterationsCompleted;
    out.waitVblanksCompleted = runtime.waitVblanksCompleted;
    out.tileMaskMutationSerial8001EEAC =
        runtime.tileMaskMutationSerial8001EEAC;
    out.gp196 = runtime.gp196;
    if (!runtime.active ||
        runtime.ctxAddress != kScene0WorkAddress ||
        runtime.loopIterationsRequired == 0u ||
        (runtime.mode != 5 && runtime.mode != 6) ||
        runtime.preFfd4Arg != 1 || runtime.postFfd4Arg != 2 ||
        (runtime.phase !=
             FastTransitionRuntimePhase800201AC::Loop8001EA74 &&
         runtime.phase !=
             FastTransitionRuntimePhase800201AC::Tail80020090) ||
        runtime.waitVblanksCompleted >= 2u ||
        !runtime.tileMaskKnown8001EEAC) {
        return out;
    }

    if (runtime.phase ==
        FastTransitionRuntimePhase800201AC::Loop8001EA74) {
        const uint32_t expectedGp196 = runtime.waitVblanksCompleted == 0u
            ? runtime.loopIterationsCompleted
            : (runtime.loopIterationsCompleted + 1u >=
                       runtime.loopIterationsRequired
                   ? 192u
                   : runtime.loopIterationsCompleted + 1u);
        if (runtime.gp196 != expectedGp196 || runtime.postFfd4Applied) {
            return FastTransitionTickResult800201AC{};
        }
    } else if (runtime.gp196 != 192u ||
               (runtime.waitVblanksCompleted != 0u &&
                !runtime.postFfd4Applied)) {
        return FastTransitionTickResult800201AC{};
    }

    out.accepted = true;
    if (runtime.phase ==
        FastTransitionRuntimePhase800201AC::Loop8001EA74) {
        // SCUS 80020308/80020248 call 800271E4 before drawing the visual
        // body and before the two-VBlank wait owned by 800201AC.  Publish
        // the cue only on the first VBlank half of the current iteration so
        // a caller that models both halves cannot play it twice.
        if (runtime.waitVblanksCompleted == 0u) {
            if (runtime.mode == 5 && runtime.loopIterationsCompleted == 0u &&
                !runtime.initialCue800271E4Dispatched) {
                out.sub800271E4CallRequired = true;
                out.sub800271E4CueIndex = 0u;
            } else if (runtime.mode == 5 && runtime.word800916DC &&
                       runtime.loopIterationsCompleted == 15u) {
                out.sub800271E4CallRequired = true;
                out.sub800271E4CueIndex = 1u;
            } else if (runtime.mode == 6 &&
                       runtime.loopIterationsCompleted == 0u &&
                       !runtime.initialCue800271E4Dispatched) {
                out.sub800271E4CallRequired = true;
                out.sub800271E4CueIndex = 1u;
            } else if (runtime.mode == 6 && runtime.word800916DC &&
                       runtime.loopIterationsCompleted == 15u) {
                out.sub800271E4CallRequired = true;
                out.sub800271E4CueIndex = 0u;
            }
            if (out.sub800271E4CallRequired) {
                out.sub800271E4CueAddress =
                    kCueGlobal9441C +
                    6u * static_cast<uint32_t>(out.sub800271E4CueIndex);
                out.sub800271E4SourceFunction = kFn800271E4;
                if (runtime.loopIterationsCompleted == 0u) {
                    runtime.initialCue800271E4Dispatched = true;
                }
            }
        }
        out.visualFrame = ResolveFastTransitionVisualFrame800201AC(
            runtime.mode,
            runtime.word800916DC,
            false,
            runtime.loopIterationsCompleted);
        if (runtime.waitVblanksCompleted == 0u) {
            const uint32_t nextGp196 =
                runtime.loopIterationsCompleted + 1u;
            runtime.gp196 = nextGp196 >= runtime.loopIterationsRequired
                ? 192u
                : nextGp196;
        }
    } else {
        if (runtime.waitVblanksCompleted == 0u &&
            !runtime.postFfd4Applied) {
            // 800201AC calls 8001FFD4(a4), then explicitly writes 190 to
            // gp+0xC4 before entering 80020090.  The first tail 8001EA74
            // call immediately advances/resets that counter to 192.
            runtime.gp196 = 0u;
            std::fill(runtime.tileMask8001EEAC.begin(),
                      runtime.tileMask8001EEAC.end(),
                      runtime.postFfd4Arg == 1 ? 0u : 1u);
            runtime.postFfd4Applied = true;
            ++runtime.tileMaskMutationSerial8001EEAC;
            out.tileMaskMutationApplied8001EEAC = true;
            runtime.gp196 = 190u;
        }
        out.visualFrame = ResolveFastTransitionVisualFrame800201AC(
            runtime.mode,
            runtime.word800916DC,
            true,
            runtime.tailIterationsCompleted);
        if (runtime.waitVblanksCompleted == 0u) {
            runtime.gp196 = 192u;
        }
    }
    ++runtime.waitVblanksCompleted;
    if (runtime.waitVblanksCompleted < 2u) {
        out.waitVblanksCompleted = runtime.waitVblanksCompleted;
        out.tileMaskMutationSerial8001EEAC =
            runtime.tileMaskMutationSerial8001EEAC;
        out.gp196 = runtime.gp196;
        return out;
    }
    runtime.waitVblanksCompleted = 0u;

    if (runtime.phase ==
        FastTransitionRuntimePhase800201AC::Loop8001EA74) {
        ++runtime.loopIterationsCompleted;
        out.loopIterationCompleted = true;
        if (runtime.loopIterationsCompleted >=
            runtime.loopIterationsRequired) {
            runtime.phase =
                FastTransitionRuntimePhase800201AC::Tail80020090;
        }
    } else if (runtime.phase ==
               FastTransitionRuntimePhase800201AC::Tail80020090) {
        ++runtime.tailIterationsCompleted;
        out.tailIterationCompleted = true;
        if (runtime.tailIterationsCompleted >= 4u) {
            runtime.phase = FastTransitionRuntimePhase800201AC::Complete;
            runtime.active = false;
            out.complete = true;
        }
    }

    out.phaseAfter = runtime.phase;
    out.loopIterationsCompleted = runtime.loopIterationsCompleted;
    out.tailIterationsCompleted = runtime.tailIterationsCompleted;
    out.waitVblanksCompleted = runtime.waitVblanksCompleted;
    out.tileMaskMutationSerial8001EEAC =
        runtime.tileMaskMutationSerial8001EEAC;
    out.gp196 = runtime.gp196;
    return out;
}

void ResetFastTransitionRuntime800201AC(
    FastTransitionRuntime800201AC& runtime)
{
    runtime = FastTransitionRuntime800201AC{};
}

FastTransitionVisualFrame800201AC ResolveFastTransitionVisualFrame800201AC(
    int32_t mode,
    bool word800916DC,
    bool tail,
    uint32_t iteration)
{
    FastTransitionVisualFrame800201AC out{};
    const uint32_t loopLimit = word800916DC ? 31u : 16u;
    if ((mode != 5 && mode != 6) || (!tail && iteration >= loopLimit)) {
        return out;
    }

    out.known = true;
    out.tail = tail;
    out.mode = mode;
    out.word800916DC = word800916DC;
    out.iteration = tail ? 190u : iteration;
    out.sourceFunction = mode == 5 ? kFn80020308 : kFn80020248;
    if (mode == 5) {
        out.kind = !word800916DC || out.iteration >= 15u
            ? FastTransitionFrameKind800201AC::FinalNoVideoFrame8001FEB4
            : FastTransitionFrameKind800201AC::OutroNoSubboxFrame8001F230;
    } else {
        out.kind = !word800916DC
            ? FastTransitionFrameKind800201AC::NoSubtitleFrame8001CE30
            : (out.iteration >= 15u
                ? FastTransitionFrameKind800201AC::SubtitleFrame8001C864
                : FastTransitionFrameKind800201AC::OutroNoSubboxFrame8001F230);
    }
    out.clearColorKnown = mode == 6 ||
                          (mode == 5 && word800916DC &&
                           out.iteration >= 15u);
    if (out.clearColorKnown) {
        out.clearR = 0xFFu;
        out.clearG = 0xFFu;
        out.clearB = 0xFFu;
    }
    return out;
}

FastTransitionSpriteTemplate800201AC
ResolveFastTransitionSpriteTemplate800201AC(uint32_t sourceAddress)
{
    for (const FastTransitionSpriteTemplate800201AC& spriteTemplate :
         kFastTransitionTemplates8001B25C) {
        if (spriteTemplate.sourceAddress == sourceAddress) {
            return spriteTemplate;
        }
    }
    for (const FastTransitionSpriteTemplate800201AC& spriteTemplate :
         kFastTransitionTemplatesScene0Frames) {
        if (spriteTemplate.sourceAddress == sourceAddress) {
            return spriteTemplate;
        }
    }
    return FastTransitionSpriteTemplate800201AC{};
}

FastTransitionFramePlan800201AC BuildFastTransitionFramePlan800201AC(
    const FastTransitionVisualFrame800201AC& visualFrame)
{
    FastTransitionFramePlan800201AC plan{};
    plan.kind = visualFrame.kind;
    if (!visualFrame.known) {
        return plan;
    }

    if (visualFrame.kind ==
        FastTransitionFrameKind800201AC::SubtitleFrame8001C864) {
        if (!AppendSubtitleFrameSprites8001C864(plan)) {
            return plan;
        }
        plan.known = plan.commandCount == 86u && !plan.truncated;
        return plan;
    }

    if (visualFrame.kind ==
        FastTransitionFrameKind800201AC::NoSubtitleFrame8001CE30) {
        if (!AppendNoSubtitleFrameSprites8001CE30(plan)) {
            return plan;
        }
        plan.known = plan.commandCount == 106u && !plan.truncated;
        return plan;
    }

    if (visualFrame.kind ==
        FastTransitionFrameKind800201AC::FinalNoVideoFrame8001FEB4) {
        for (int col = 0; col < 8; ++col) {
            for (int row = 0; row < 6; ++row) {
                for (uint32_t tile = 0; tile < 4u; ++tile) {
                    if (!AppendFinalNoVideoSprite8001FEB4(
                            plan,
                            static_cast<int16_t>(
                                40 * col +
                                (static_cast<int>(tile) & 1) * 20),
                            static_cast<int16_t>(
                                40 * row +
                                ((static_cast<int>(tile) >> 1) & 1) * 20),
                            tile)) {
                        return plan;
                    }
                }
            }
        }
        plan.known = plan.commandCount == 192u && !plan.truncated;
        return plan;
    }

    if (visualFrame.kind !=
        FastTransitionFrameKind800201AC::OutroNoSubboxFrame8001F230) {
        return plan;
    }

    static constexpr struct {
        int16_t x;
        int16_t y;
        uint32_t templateAddress;
    } kFixedHead[6] = {
        {20, 20, 0x80050380u},
        {280, 20, 0x80050390u},
        {40, 20, 0x800503A0u},
        {160, 20, 0x800503B0u},
        {40, 160, 0x800503C0u},
        {160, 160, 0x800503D0u},
    };
    for (const auto& sprite : kFixedHead) {
        if (!AppendOutroNoSubboxSprite8001F230(
                plan, sprite.x, sprite.y, sprite.templateAddress)) {
            return plan;
        }
    }

    for (int i = 0; i < 7; ++i) {
        const int16_t x0 = static_cast<int16_t>(40 + 40 * i);
        const int16_t x1 = static_cast<int16_t>(20 + 40 * i);
        if (!AppendOutroNoSubboxSprite8001F230(
                plan, x0, 0, 0x800503E0u) ||
            !AppendOutroNoSubboxSprite8001F230(
                plan, x1, 0, 0x800503F0u) ||
            !AppendOutroNoSubboxSprite8001F230(
                plan, x0, 220, 0x80050400u) ||
            !AppendOutroNoSubboxSprite8001F230(
                plan, x1, 220, 0x80050410u)) {
            return plan;
        }
    }

    for (int i = 0; i < 6; ++i) {
        const int16_t y0 = static_cast<int16_t>(20 + 40 * i);
        const int16_t y1 = static_cast<int16_t>(40 * i);
        const int16_t x0 = static_cast<int16_t>(40 + 40 * i);
        const int16_t x1 = static_cast<int16_t>(60 + 40 * i);
        if (!AppendOutroNoSubboxSprite8001F230(
                plan, 0, y0, 0x80050400u) ||
            !AppendOutroNoSubboxSprite8001F230(
                plan, 0, y1, 0x800503E0u) ||
            !AppendOutroNoSubboxSprite8001F230(
                plan, 300, y0, 0x80050410u) ||
            !AppendOutroNoSubboxSprite8001F230(
                plan, 300, y1, 0x800503F0u) ||
            !AppendOutroNoSubboxSprite8001F230(
                plan, x0, 200, 0x800503E0u) ||
            !AppendOutroNoSubboxSprite8001F230(
                plan, x1, 200, 0x800503F0u) ||
            !AppendOutroNoSubboxSprite8001F230(
                plan, x0, 180, 0x80050400u) ||
            !AppendOutroNoSubboxSprite8001F230(
                plan, x1, 180, 0x80050410u)) {
            return plan;
        }
    }

    if (!AppendOutroNoSubboxSprite8001F230(
            plan, 280, 200, 0x800503E0u) ||
        !AppendOutroNoSubboxSprite8001F230(
            plan, 20, 200, 0x800503F0u) ||
        !AppendOutroNoSubboxSprite8001F230(
            plan, 280, 180, 0x80050400u) ||
        !AppendOutroNoSubboxSprite8001F230(
            plan, 20, 180, 0x80050410u)) {
        return plan;
    }

    plan.known = plan.commandCount == 86u && !plan.truncated;
    return plan;
}

FastTransitionPresentPlan8001EBF4 BuildFastTransitionPresentPlan8001EBF4(
    const FastTransitionVisualFrame800201AC& visualFrame,
    const FastTransitionGraphInput8001EA74& graphInput)
{
    FastTransitionPresentPlan8001EBF4 plan{};
    const uint32_t visualFunction =
        ResolveVisualFrameFunction8001EA74(visualFrame.kind);
    if (!visualFrame.known || visualFunction == 0u || !graphInput.known ||
        graphInput.drawSlot8004019C > 1u) {
        return plan;
    }

    static constexpr uint32_t kExpectedAllocator[2] = {
        0x801AE430u, 0x801B8CF0u};
    static constexpr uint32_t kExpectedWork[2] = {
        0x80087288u, 0x8008729Cu};
    static constexpr uint32_t kExpectedOtHead[2] = {
        0x80088288u, 0x80098288u};
    const uint16_t slot = graphInput.drawSlot8004019C;
    if (graphInput.packetAllocator8006ED50 != kExpectedAllocator[slot] ||
        graphInput.mainPageWorkAddress80087288 != kExpectedWork[slot] ||
        graphInput.mainPageOtHeadAddress80088288 != kExpectedOtHead[slot] ||
        graphInput.mainPageWorkHeadAddress80040CC8 !=
            kExpectedOtHead[slot] ||
        graphInput.mainPageWorkOrder != 14u) {
        return plan;
    }

    plan.drawSlotBefore = slot;
    plan.drawSlotAfter = static_cast<uint16_t>(slot ^ 1u);
    plan.packetAllocator8006ED50 = graphInput.packetAllocator8006ED50;
    plan.mainPageWorkAddress80087288 =
        graphInput.mainPageWorkAddress80087288;
    plan.mainPageOtHeadAddress80088288 =
        graphInput.mainPageOtHeadAddress80088288;
    plan.clearColorRequested80040420 = visualFrame.clearColorKnown;
    plan.mainPageWorkSubmitRequested80040CA4 = true;

    if (!AppendFastTransitionPresentAction8001EBF4(
            plan,
            FastTransitionPresentActionKind8001EBF4::GetDrawBuffer8004019C,
            kFn8004019C) ||
        !AppendFastTransitionPresentAction8001EBF4(
            plan,
            FastTransitionPresentActionKind8001EBF4::SetPacketAllocator80040F90,
            kFn80040F90,
            graphInput.packetAllocator8006ED50) ||
        !AppendFastTransitionPresentAction8001EBF4(
            plan,
            FastTransitionPresentActionKind8001EBF4::ClearMainPageWork80040CC8,
            kFn80040CC8,
            0u,
            0u,
            graphInput.mainPageWorkAddress80087288) ||
        !AppendFastTransitionPresentAction8001EBF4(
            plan,
            FastTransitionPresentActionKind8001EBF4::BuildVisualFrame8001EA74,
            visualFunction) ||
        !AppendFastTransitionPresentAction8001EBF4(
            plan,
            FastTransitionPresentActionKind8001EBF4::FlipGraph80040370,
            kFn80040370)) {
        return plan;
    }
    if (visualFrame.clearColorKnown &&
        !AppendFastTransitionPresentAction8001EBF4(
            plan,
            FastTransitionPresentActionKind8001EBF4::ClearColor80040420,
            kFn80040420,
            visualFrame.clearR,
            visualFrame.clearG,
            visualFrame.clearB)) {
        return plan;
    }
    if (!AppendFastTransitionPresentAction8001EBF4(
            plan,
            FastTransitionPresentActionKind8001EBF4::SubmitMainPageWork80040CA4,
            kFn80040CA4,
            graphInput.mainPageWorkAddress80087288)) {
        return plan;
    }
    plan.known = !plan.truncated;
    return plan;
}

bool BeginSlowTransitionRuntime80020110(
    SlowTransitionRuntime80020110& runtime,
    uint32_t ctxAddress,
    int32_t mode,
    int32_t preFfd4Arg,
    int32_t postFfd4Arg)
{
    runtime = SlowTransitionRuntime80020110{};
    if (!IsKnownSlowTransitionTuple80020110(
            ctxAddress, mode, preFfd4Arg, postFfd4Arg)) {
        return false;
    }

    runtime.active = true;
    runtime.ctxAddress = ctxAddress;
    runtime.mode = mode;
    runtime.preFfd4Arg = preFfd4Arg;
    runtime.postFfd4Arg = postFfd4Arg;
    runtime.phase = SlowTransitionRuntimePhase80020110::Loop8001EA74;
    runtime.loopIterationsRequired = kSlowTransitionBodyIterations80020110;
    runtime.sceneFrame = ResolveSlowTransitionStartFrame80020110(mode);
    // 8001FFD4 resets gp+0xC4 and delegates to 8001EEAC.  The original
    // initializes an all-clear mask for arg==1 and an all-set mask otherwise;
    // keep that software state explicit before the first 8001EA74 body tick.
    runtime.tileMaskKnown = true;
    runtime.postFfd4Applied = false;
    runtime.gp196 = 0u;
    runtime.tileMaskMutationSerial = 1u;
    std::fill(runtime.tileMask.begin(),
              runtime.tileMask.end(),
              preFfd4Arg == 1 ? 0u : 1u);
    return true;
}

Sub80027194CadenceStep AdvanceSub80027194Cadence(
    Sub80027194CadenceState& state)
{
    Sub80027194CadenceStep out{};
    out.previousCounter = state.counter;
    out.cueRequired = state.counter >= 2;
    out.flushRequired = out.cueRequired;
    out.nextCounter = out.cueRequired ? 1 : state.counter + 1;
    state.counter = out.nextCounter;
    out.known = true;
    return out;
}

SlowTransitionTickResult80020110 TickSlowTransitionRuntime80020110(
    SlowTransitionRuntime80020110& runtime)
{
    SlowTransitionTickResult80020110 out{};
    out.phaseBefore = runtime.phase;
    out.phaseAfter = runtime.phase;
    out.loopIterationsCompleted = runtime.loopIterationsCompleted;
    out.tailIterationsCompleted = runtime.tailIterationsCompleted;
    out.waitVblanksCompleted = runtime.waitVblanksCompleted;
    out.hostTicksCompleted = runtime.hostTicksCompleted;
    out.sceneFrame = runtime.sceneFrame;
    out.tileMaskMutationSerial = runtime.tileMaskMutationSerial;

    const uint32_t startFrame =
        ResolveSlowTransitionStartFrame80020110(runtime.mode);
    const bool loopPhaseValid =
        runtime.phase ==
            SlowTransitionRuntimePhase80020110::Loop8001EA74 &&
        runtime.loopIterationsCompleted <
            kSlowTransitionBodyIterations80020110 &&
        runtime.tailIterationsCompleted == 0u;
    const bool tailPhaseValid =
        runtime.phase ==
            SlowTransitionRuntimePhase80020110::Tail80020008 &&
        runtime.loopIterationsCompleted ==
            kSlowTransitionBodyIterations80020110 &&
        runtime.tailIterationsCompleted <
            kSlowTransitionTailIterations80020110;
    const uint32_t expectedHostTicks =
        (runtime.loopIterationsCompleted +
         runtime.tailIterationsCompleted) * 2u +
        runtime.waitVblanksCompleted;
    if (!runtime.active ||
        !IsKnownSlowTransitionTuple80020110(
            runtime.ctxAddress,
            runtime.mode,
            runtime.preFfd4Arg,
            runtime.postFfd4Arg) ||
        (startFrame == 0u &&
         !(runtime.ctxAddress == 0u && runtime.mode == 3)) ||
        runtime.loopIterationsRequired !=
            kSlowTransitionBodyIterations80020110 ||
        runtime.waitVblanksCompleted >= 2u ||
        runtime.hostTicksCompleted != expectedHostTicks ||
        runtime.sceneFrame != startFrame + expectedHostTicks ||
        !runtime.tileMaskKnown ||
        (!loopPhaseValid && !tailPhaseValid)) {
        return out;
    }

    // 8001EA74 mutates the software mask once per transition iteration, not
    // once per VBlank.  The host adapter calls this function twice per logic
    // tick, so waitVblanksCompleted==0 is the exact mutation boundary.  The
    // post-loop 8001FFD4 is likewise applied at the first tail VBlank.
    if (runtime.waitVblanksCompleted == 0u) {
        bool mutationApplied = false;
        if (loopPhaseValid) {
            const uint32_t iteration = runtime.loopIterationsCompleted;
            if (runtime.gp196 != iteration * 8u) {
                return out;
            }
            const uint32_t begin = iteration * 8u;
            const uint32_t end = begin + 8u;
            if (runtime.mode == 1) {
                for (uint32_t orderIndex = begin; orderIndex < end;
                     ++orderIndex) {
                    const SlowTransitionGridCoordinate8004EB80 coordinate =
                        ResolveSlowTransitionMode1SpiralCoordinate8004EB80(
                            orderIndex);
                    if (!coordinate.known) {
                        return out;
                    }
                    runtime.tileMask[
                        static_cast<std::size_t>(coordinate.row) *
                            kSlowTransitionGridColumns8001FDC0 +
                        coordinate.column] = 0u;
                }
                mutationApplied = true;
            } else if (runtime.mode == 2 || runtime.mode == 4) {
                const bool setActive = runtime.mode == 2;
                for (uint32_t orderIndex = begin; orderIndex < end;
                     ++orderIndex) {
                    const std::size_t reverseIndex =
                        kSlowTransitionGridCells8001FDC0 - 1u - orderIndex;
                    const std::size_t row = kTileOrderF180Row[reverseIndex];
                    const std::size_t col = kTileOrderF180Col[reverseIndex];
                    if (row >= kSlowTransitionGridRows8001FDC0 ||
                        col >= kSlowTransitionGridColumns8001FDC0) {
                        return out;
                    }
                    runtime.tileMask[
                        row * kSlowTransitionGridColumns8001FDC0 + col] =
                        setActive ? 1u : 0u;
                }
                mutationApplied = true;
            } else if (runtime.mode == 3) {
                // 8001EA74 case 3 performs 8001FC40(4,8), whose eight
                // 8001F698(4,0) calls consume the reverse F180 order before
                // 8001FDC0 redraws the remaining active tiles.
                for (uint32_t orderIndex = begin; orderIndex < end;
                     ++orderIndex) {
                    const std::size_t reverseIndex =
                        kSlowTransitionGridCells8001FDC0 - 1u - orderIndex;
                    const std::size_t row = kTileOrderF180Row[reverseIndex];
                    const std::size_t col = kTileOrderF180Col[reverseIndex];
                    if (row >= kSlowTransitionGridRows8001FDC0 ||
                        col >= kSlowTransitionGridColumns8001FDC0) {
                        return out;
                    }
                    runtime.tileMask[
                        row * kSlowTransitionGridColumns8001FDC0 + col] = 0u;
                }
                mutationApplied = true;
            }
            if (!mutationApplied) {
                return out;
            }
            runtime.gp196 = end;
        } else if (!runtime.postFfd4Applied) {
            // 8001FFD4(a4) resets gp+0xC4 before entering 80020008.
            runtime.gp196 = 0u;
            std::fill(runtime.tileMask.begin(),
                      runtime.tileMask.end(),
                      runtime.postFfd4Arg == 1 ? 0u : 1u);
            runtime.postFfd4Applied = true;
            mutationApplied = true;
        }
        if (mutationApplied) {
            ++runtime.tileMaskMutationSerial;
            out.tileMaskMutationApplied = true;
        }
    }

    out.visualFrame = ResolveSlowTransitionVisualFrame80020110(
        runtime.ctxAddress,
        runtime.mode,
        runtime.preFfd4Arg,
        runtime.postFfd4Arg,
        tailPhaseValid,
        tailPhaseValid ? runtime.tailIterationsCompleted
                       : runtime.loopIterationsCompleted);
    if (!out.visualFrame.known) {
        return out;
    }
    for (std::size_t i = 0u; i < kSlowTransitionGridCells8001FDC0; ++i) {
        if (out.visualFrame.grid[i] != runtime.tileMask[i]) {
            return SlowTransitionTickResult80020110{};
        }
    }

    out.accepted = true;
    out.sub80027194CallRequired = runtime.waitVblanksCompleted == 0u;
    out.presentRequired = runtime.waitVblanksCompleted == 1u;
    ++runtime.waitVblanksCompleted;
    ++runtime.hostTicksCompleted;
    ++runtime.sceneFrame;
    if (runtime.waitVblanksCompleted == 2u) {
        runtime.waitVblanksCompleted = 0u;
        if (loopPhaseValid) {
            ++runtime.loopIterationsCompleted;
            out.loopIterationCompleted = true;
            if (runtime.loopIterationsCompleted ==
                kSlowTransitionBodyIterations80020110) {
                runtime.phase =
                    SlowTransitionRuntimePhase80020110::Tail80020008;
            }
        } else {
            ++runtime.tailIterationsCompleted;
            out.tailIterationCompleted = true;
            if (runtime.tailIterationsCompleted ==
                kSlowTransitionTailIterations80020110) {
                runtime.phase =
                    SlowTransitionRuntimePhase80020110::Complete;
                runtime.active = false;
                out.complete = true;
            }
        }
    }

    out.phaseAfter = runtime.phase;
    out.loopIterationsCompleted = runtime.loopIterationsCompleted;
    out.tailIterationsCompleted = runtime.tailIterationsCompleted;
    out.waitVblanksCompleted = runtime.waitVblanksCompleted;
    out.hostTicksCompleted = runtime.hostTicksCompleted;
    out.sceneFrame = runtime.sceneFrame;
    out.tileMaskMutationSerial = runtime.tileMaskMutationSerial;
    return out;
}

void ResetSlowTransitionRuntime80020110(
    SlowTransitionRuntime80020110& runtime)
{
    runtime = SlowTransitionRuntime80020110{};
}

SlowTransitionGridCoordinate8004EB80
ResolveSlowTransitionMode1SpiralCoordinate8004EB80(std::size_t orderIndex)
{
    SlowTransitionGridCoordinate8004EB80 out{};
    if (orderIndex >= kSlowTransitionGridCells8001FDC0) {
        return out;
    }

    std::size_t cursor = 0u;
    const auto visit = [&](std::size_t row, std::size_t column) {
        if (cursor++ != orderIndex) {
            return false;
        }
        out.known = true;
        out.row = static_cast<uint8_t>(row);
        out.column = static_cast<uint8_t>(column);
        return true;
    };

    for (std::size_t layer = 0u;
         layer * 2u < kSlowTransitionGridRows8001FDC0 &&
         layer * 2u < kSlowTransitionGridColumns8001FDC0;
         ++layer) {
        const std::size_t top = layer;
        const std::size_t bottom =
            kSlowTransitionGridRows8001FDC0 - 1u - layer;
        const std::size_t left = layer;
        const std::size_t right =
            kSlowTransitionGridColumns8001FDC0 - 1u - layer;

        for (std::size_t column = left; column <= right; ++column) {
            if (visit(top, column)) {
                return out;
            }
        }
        for (std::size_t row = top + 1u; row <= bottom; ++row) {
            if (visit(row, right)) {
                return out;
            }
        }
        if (bottom > top) {
            for (std::size_t column = right; column-- > left;) {
                if (visit(bottom, column)) {
                    return out;
                }
            }
        }
        if (right > left && bottom > top + 1u) {
            for (std::size_t row = bottom; row-- > top + 1u;) {
                if (visit(row, left)) {
                    return out;
                }
            }
        }
    }
    return SlowTransitionGridCoordinate8004EB80{};
}

SlowTransitionVisualFrame80020110
ResolveSlowTransitionVisualFrame80020110(
    uint32_t ctxAddress,
    int32_t mode,
    int32_t preFfd4Arg,
    int32_t postFfd4Arg,
    bool tail,
    uint32_t iteration)
{
    SlowTransitionVisualFrame80020110 out{};
    if (!IsKnownSlowTransitionTuple80020110(
            ctxAddress, mode, preFfd4Arg, postFfd4Arg) ||
        (!tail && iteration >= kSlowTransitionBodyIterations80020110) ||
        (tail && iteration >= kSlowTransitionTailIterations80020110)) {
        return out;
    }

    out.known = true;
    out.tail = tail;
    out.ctxAddress = ctxAddress;
    out.mode = mode;
    out.preFfd4Arg = preFfd4Arg;
    out.postFfd4Arg = postFfd4Arg;
    out.iteration = iteration;
    out.sourceFunction = ResolveSlowTransitionVisualFunction80020110(mode);

    if (mode == 1 || mode == 3) {
        out.activeCount = tail
            ? 0u
            : static_cast<uint32_t>(kSlowTransitionGridCells8001FDC0) -
                  (iteration + 1u) * 8u;
        if (tail) {
            return out;
        }
        for (std::size_t i = 0u;
             i < kSlowTransitionGridCells8001FDC0;
             ++i) {
            out.grid[i] = 1u;
        }
        const uint32_t clearCount = (iteration + 1u) * 8u;
        for (uint32_t orderIndex = 0u; orderIndex < clearCount;
             ++orderIndex) {
            std::size_t row = 0u;
            std::size_t col = 0u;
            if (mode == 1) {
                const SlowTransitionGridCoordinate8004EB80 coordinate =
                    ResolveSlowTransitionMode1SpiralCoordinate8004EB80(
                        orderIndex);
                if (!coordinate.known) {
                    return SlowTransitionVisualFrame80020110{};
                }
                row = coordinate.row;
                col = coordinate.column;
            } else {
                // Current SCUS 8001F698 case 4 indexes 191 - gp[49].
                // Both mode 3 and mode 4 clear this same reverse order.
                const std::size_t reverseIndex =
                    kSlowTransitionGridCells8001FDC0 - 1u - orderIndex;
                row = kTileOrderF180Row[reverseIndex];
                col = kTileOrderF180Col[reverseIndex];
            }
            if (row >= kSlowTransitionGridRows8001FDC0 ||
                col >= kSlowTransitionGridColumns8001FDC0) {
                return SlowTransitionVisualFrame80020110{};
            }
            const std::size_t gridIndex =
                row * kSlowTransitionGridColumns8001FDC0 + col;
            if (out.grid[gridIndex] == 0u) {
                return SlowTransitionVisualFrame80020110{};
            }
            out.grid[gridIndex] = 0u;
        }
        return out;
    }

    if (mode == 4) {
        out.activeCount = tail
            ? 0u
            : static_cast<uint32_t>(kSlowTransitionGridCells8001FDC0) -
                  (iteration + 1u) * 8u;
        if (tail) {
            return out;
        }
        for (std::size_t i = 0u;
             i < kSlowTransitionGridCells8001FDC0;
             ++i) {
            out.grid[i] = 1u;
        }
        const uint32_t clearCount = (iteration + 1u) * 8u;
        for (uint32_t c4 = 0u; c4 < clearCount; ++c4) {
            const std::size_t orderIndex =
                kSlowTransitionGridCells8001FDC0 - 1u - c4;
            const std::size_t row = kTileOrderF180Row[orderIndex];
            const std::size_t col = kTileOrderF180Col[orderIndex];
            if (row >= kSlowTransitionGridRows8001FDC0 ||
                col >= kSlowTransitionGridColumns8001FDC0) {
                return SlowTransitionVisualFrame80020110{};
            }
            out.grid[row * kSlowTransitionGridColumns8001FDC0 + col] = 0u;
        }
        return out;
    }

    out.activeCount = tail
        ? static_cast<uint32_t>(kSlowTransitionGridCells8001FDC0)
        : (iteration + 1u) * 8u;

    if (tail) {
        for (std::size_t i = 0;
             i < kSlowTransitionGridCells8001FDC0;
             ++i) {
            out.grid[i] = 1u;
        }
        return out;
    }

    for (uint32_t c4 = 0; c4 < out.activeCount; ++c4) {
        const std::size_t orderIndex =
            kSlowTransitionGridCells8001FDC0 - 1u - c4;
        const std::size_t row = kTileOrderF180Row[orderIndex];
        const std::size_t col = kTileOrderF180Col[orderIndex];
        if (row >= kSlowTransitionGridRows8001FDC0 ||
            col >= kSlowTransitionGridColumns8001FDC0) {
            return SlowTransitionVisualFrame80020110{};
        }
        out.grid[row * kSlowTransitionGridColumns8001FDC0 + col] = 1u;
    }
    return out;
}

SlowTransitionFramePlan8001FDC0 BuildSlowTransitionFramePlan8001FDC0(
    const SlowTransitionVisualFrame80020110& visualFrame)
{
    SlowTransitionFramePlan8001FDC0 plan{};
    const SlowTransitionVisualFrame80020110 expected =
        ResolveSlowTransitionVisualFrame80020110(
            visualFrame.ctxAddress,
            visualFrame.mode,
            visualFrame.preFfd4Arg,
            visualFrame.postFfd4Arg,
            visualFrame.tail,
            visualFrame.iteration);
    if (!visualFrame.known || !expected.known ||
        visualFrame.activeCount != expected.activeCount ||
        visualFrame.sourceFunction != expected.sourceFunction) {
        return plan;
    }
    for (std::size_t i = 0;
         i < kSlowTransitionGridCells8001FDC0;
         ++i) {
        if (visualFrame.grid[i] != expected.grid[i]) {
            return plan;
        }
    }

    plan.tail = visualFrame.tail;
    plan.iteration = visualFrame.iteration;
    plan.activeCount = visualFrame.activeCount;
    static constexpr uint32_t kGridTemplateAddresses[4] = {
        0x800503E0u, 0x800503F0u, 0x80050400u, 0x80050410u};
    for (std::size_t row = 0;
         row < kSlowTransitionGridRows8001FDC0;
         ++row) {
        for (std::size_t col = 0;
             col < kSlowTransitionGridColumns8001FDC0;
             ++col) {
            const std::size_t gridIndex =
                row * kSlowTransitionGridColumns8001FDC0 + col;
            if (visualFrame.grid[gridIndex] == 0u) {
                continue;
            }
            if (plan.commandCount >=
                kSlowTransitionGridCells8001FDC0) {
                plan.truncated = true;
                return plan;
            }
            const uint32_t templateAddress =
                kGridTemplateAddresses[(row & 1u) * 2u + (col & 1u)];
            const FastTransitionSpriteTemplate800201AC spriteTemplate =
                ResolveFastTransitionSpriteTemplate800201AC(
                    templateAddress);
            if (!spriteTemplate.known) {
                plan.truncated = true;
                return plan;
            }
            FastTransitionSpriteCommand800201AC& command =
                plan.commands[plan.commandCount];
            command.active = true;
            command.rawTextureKnown = true;
            command.rawTexture = true;
            command.semiTransparentKnown = true;
            command.semiTransparent = false;
            command.abrKnown = true;
            command.abr = 1u;
            command.rgbKnown = false;
            command.x = static_cast<int16_t>(col * 20u);
            command.y = static_cast<int16_t>(row * 20u);
            command.priority = 0u;
            command.order =
                static_cast<uint16_t>(plan.commandCount + 1u);
            command.spriteTemplate = spriteTemplate;
            ++plan.commandCount;
        }
    }
    plan.known = !plan.truncated &&
                 plan.commandCount == plan.activeCount;
    return plan;
}

FastTransitionPresentPlan8001EBF4
BuildSlowTransitionPresentPlan8001EBF4(
    const SlowTransitionVisualFrame80020110& visualFrame,
    const FastTransitionGraphInput8001EA74& graphInput)
{
    FastTransitionPresentPlan8001EBF4 plan{};
    const SlowTransitionFramePlan8001FDC0 framePlan =
        BuildSlowTransitionFramePlan8001FDC0(visualFrame);
    if (!framePlan.known || !graphInput.known ||
        graphInput.drawSlot8004019C > 1u) {
        return plan;
    }

    static constexpr uint32_t kExpectedAllocator[2] = {
        0x801AE430u, 0x801B8CF0u};
    static constexpr uint32_t kExpectedWork[2] = {
        0x80087288u, 0x8008729Cu};
    static constexpr uint32_t kExpectedOtHead[2] = {
        0x80088288u, 0x80098288u};
    const uint16_t slot = graphInput.drawSlot8004019C;
    if (graphInput.packetAllocator8006ED50 != kExpectedAllocator[slot] ||
        graphInput.mainPageWorkAddress80087288 != kExpectedWork[slot] ||
        graphInput.mainPageOtHeadAddress80088288 !=
            kExpectedOtHead[slot] ||
        graphInput.mainPageWorkHeadAddress80040CC8 !=
            kExpectedOtHead[slot] ||
        graphInput.mainPageWorkOrder != 14u) {
        return plan;
    }

    plan.drawSlotBefore = slot;
    plan.drawSlotAfter = static_cast<uint16_t>(slot ^ 1u);
    plan.packetAllocator8006ED50 = graphInput.packetAllocator8006ED50;
    plan.mainPageWorkAddress80087288 =
        graphInput.mainPageWorkAddress80087288;
    plan.mainPageOtHeadAddress80088288 =
        graphInput.mainPageOtHeadAddress80088288;
    plan.clearColorRequested80040420 = false;
    plan.mainPageWorkSubmitRequested80040CA4 = true;

    if (!AppendFastTransitionPresentAction8001EBF4(
            plan,
            FastTransitionPresentActionKind8001EBF4::
                GetDrawBuffer8004019C,
            kFn8004019C) ||
        !AppendFastTransitionPresentAction8001EBF4(
            plan,
            FastTransitionPresentActionKind8001EBF4::
                SetPacketAllocator80040F90,
            kFn80040F90,
            graphInput.packetAllocator8006ED50) ||
        !AppendFastTransitionPresentAction8001EBF4(
            plan,
            FastTransitionPresentActionKind8001EBF4::
                ClearMainPageWork80040CC8,
            kFn80040CC8,
            0u,
            0u,
            graphInput.mainPageWorkAddress80087288) ||
        !AppendFastTransitionPresentAction8001EBF4(
            plan,
            FastTransitionPresentActionKind8001EBF4::
                BuildVisualFrame8001EA74,
            visualFrame.sourceFunction) ||
        !AppendFastTransitionPresentAction8001EBF4(
            plan,
            FastTransitionPresentActionKind8001EBF4::FlipGraph80040370,
            kFn80040370) ||
        !AppendFastTransitionPresentAction8001EBF4(
            plan,
            FastTransitionPresentActionKind8001EBF4::
                SubmitMainPageWork80040CA4,
            kFn80040CA4,
            graphInput.mainPageWorkAddress80087288)) {
        return plan;
    }
    plan.known = !plan.truncated;
    return plan;
}

TransitionPlan BuildMode8001EA74Plan(int32_t mode, bool word800916DC)
{
    TransitionPlan plan{};
    plan.kind = TransitionPlanKind::Mode8001EA74;
    plan.mode = mode;
    plan.word800916DC = word800916DC;
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();

    AppendCall(plan,
               TransitionActionKind::GateModeSource8001EA74,
               kFn8001EA74,
               static_cast<uint32_t>(mode),
               word800916DC ? 1u : 0u,
               IsKnownMode8001EA74(mode) ? 1u : 0u,
               HasKnownModeBodyTicks8001EA74(mode, word800916DC)
                   ? static_cast<uint32_t>(
                         KnownModeBodyTicks8001EA74(mode, word800916DC))
                   : 0u);
    AppendCall(plan,
               TransitionActionKind::Call8001EA74,
               kFn8001EA74,
               0,
               static_cast<uint32_t>(mode),
               word800916DC ? 1 : 0);

    if (mode == 2) {
        AppendCall(plan,
                   TransitionActionKind::Call8001FCBC,
                   kFn8001FCBC,
                   8,
                   4);
        AppendCall(plan,
                   TransitionActionKind::Call8001FDC0,
                   kFn8001FDC0,
                   0);
        AppendCall(plan, TransitionActionKind::Gap, 0, 3, 17);
        return plan;
    }

    if (mode == 3) {
        // COMOD0 8001EA74 case 3 is the card-save transition used by
        // 80019148: render the type-4 card prompt, draw the page frame, clear
        // eight tiles per 8001FC40(4,8) iteration, then redraw active tiles.
        AppendCall(plan,
                   TransitionActionKind::Call80022CBC,
                   kFn80022CBC,
                   4,
                   0);
        AppendCall(plan,
                   TransitionActionKind::Call8001D74C,
                   kFn8001D74C,
                   5);
        AppendCall(plan,
                   TransitionActionKind::Call8001FC40,
                   kFn8001FC40,
                   4,
                   8);
        AppendCall(plan,
                   TransitionActionKind::Call8001FDC0,
                   kFn8001FDC0,
                   0);
        return plan;
    }

    if (mode == 4) {
        AppendCall(plan,
                   TransitionActionKind::Call80021E60,
                   kFn80021E60,
                   0);
        AppendCall(plan,
                   TransitionActionKind::Call8001D74C,
                   kFn8001D74C,
                   5);
        AppendCall(plan,
                   TransitionActionKind::Call8001FC40,
                   kFn8001FC40,
                   4,
                   8);
        AppendCall(plan,
                   TransitionActionKind::Call8001FDC0,
                   kFn8001FDC0,
                   0);
        return plan;
    }

    if (mode == 5) {
        AppendCall(plan,
                   TransitionActionKind::Call80020308,
                   kFn80020308,
                   word800916DC ? 1 : 0);
        AppendCall(plan,
                   TransitionActionKind::SetGp792,
                   kFn8001EA74,
                   5);
        if (word800916DC) {
            AppendCall(plan,
                       TransitionActionKind::Call8001F230,
                       kFn8001F230,
                       5,
                       0,
                       0,
                       0,
                       15);
        }
        AppendCall(plan,
                   TransitionActionKind::Call8001FEB4,
                   kFn8001FEB4,
                   0);
        AppendCall(plan,
                   TransitionActionKind::SetGpC4,
                   kFn80020308,
                   192);
        return plan;
    }

    if (mode == 6) {
        AppendCall(plan,
                   TransitionActionKind::Call80020248,
                   kFn80020248,
                   word800916DC ? 1 : 0);
        if (word800916DC) {
            AppendCall(plan,
                       TransitionActionKind::Call8001F230,
                       kFn8001F230,
                       5,
                       0,
                       0,
                       0,
                       15);
            AppendCall(plan,
                       TransitionActionKind::Call8001C864,
                       kFn8001C864,
                       5);
        } else {
            AppendCall(plan,
                       TransitionActionKind::Call8001CE30,
                       kFn8001CE30,
                       5);
        }
        AppendCall(plan,
                   TransitionActionKind::SetGpC4,
                   kFn80020248,
                   192);
        return plan;
    }

    AppendCall(plan, TransitionActionKind::Gap, 0, static_cast<uint32_t>(mode));
    return plan;
}

TransitionPlan BuildSlow80020110Plan(uint32_t ctxAddress,
                                     int32_t mode,
                                     int32_t preFfd4Arg,
                                     int32_t postFfd4Arg,
                                     bool word800916DC)
{
    TransitionPlan plan{};
    plan.kind = TransitionPlanKind::Slow80020110;
    plan.mode = mode;
    plan.word800916DC = word800916DC;
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();

    const bool repeatKnown =
        HasKnownModeBodyTicks8001EA74(mode, word800916DC);
    const int32_t repeatCount =
        repeatKnown ? KnownModeBodyTicks8001EA74(mode, word800916DC) : 0;
    AppendCall(plan,
               TransitionActionKind::GateTransitionCallSource80020110,
               kFn80020110,
               ctxAddress,
               static_cast<uint32_t>(mode),
               PackFfd4Args(preFfd4Arg, postFfd4Arg),
               word800916DC ? 1u : 0u,
               repeatCount,
               repeatKnown);
    AppendCall(plan,
               TransitionActionKind::Call8001FFD4,
               kFn8001FFD4,
               preFfd4Arg);
    AppendLoopBody80020110(plan, ctxAddress, mode, word800916DC);
    AppendCall(plan,
               TransitionActionKind::Call8001FFD4,
               kFn8001FFD4,
               postFfd4Arg);
    AppendCall(plan,
               TransitionActionKind::Call80020008Tail,
               kFn80020008,
               ctxAddress,
               static_cast<uint32_t>(mode),
               kFn8001EA74,
               kFn8001EBF4,
               4);
    AppendCall(plan,
               TransitionActionKind::Call80027194,
               kFn80027194,
               30,
               0,
               0,
               0,
               4);
    return plan;
}

TransitionPlan BuildFast800201ACPlan(uint32_t ctxAddress,
                                     int32_t mode,
                                     int32_t preFfd4Arg,
                                     int32_t postFfd4Arg,
                                     bool word800916DC)
{
    TransitionPlan plan{};
    plan.kind = TransitionPlanKind::Fast800201AC;
    plan.mode = mode;
    plan.word800916DC = word800916DC;
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();

    const bool repeatKnown =
        HasKnownModeBodyTicks8001EA74(mode, word800916DC);
    const int32_t repeatCount =
        repeatKnown ? KnownModeBodyTicks8001EA74(mode, word800916DC) : 0;
    AppendCall(plan,
               TransitionActionKind::GateTransitionCallSource800201AC,
               kFn800201AC,
               ctxAddress,
               static_cast<uint32_t>(mode),
               PackFfd4Args(preFfd4Arg, postFfd4Arg),
               word800916DC ? 1u : 0u,
               repeatCount,
               repeatKnown);
    AppendCall(plan,
               TransitionActionKind::Call8001FFD4,
               kFn8001FFD4,
               preFfd4Arg);
    AppendLoopBody800201AC(plan, ctxAddress, mode, word800916DC);
    AppendCall(plan,
               TransitionActionKind::Call8001FFD4,
               kFn8001FFD4,
               postFfd4Arg);
    AppendCall(plan,
               TransitionActionKind::SetGpC4,
               kFn800201AC,
               190);
    AppendCall(plan,
               TransitionActionKind::Call80020090Tail,
               kFn80020090,
               ctxAddress,
               static_cast<uint32_t>(mode),
               kFn8001EA74,
               kFn8001EBF4,
               4);
    return plan;
}

Scene0OuterEntryTransaction801C4DC4
BuildScene0OuterEntryTransaction801C4DC4(
    const Scene0OuterEntryInput801C4DC4& input)
{
    Scene0OuterEntryTransaction801C4DC4 transaction{};
    if (!input.word800916D2Known) {
        return transaction;
    }
    if (!input.contextKnown ||
        input.contextAddress != kScene0WorkAddress) {
        transaction.status =
            Scene0OuterEntryStatus801C4DC4::InvalidContext;
        return transaction;
    }

    const bool initialSlowRequired = input.word800916D2 == 0u;
    transaction.status = initialSlowRequired
        ? Scene0OuterEntryStatus801C4DC4::AcceptedInitialSlow
        : Scene0OuterEntryStatus801C4DC4::AcceptedSkipInitialSlow;
    transaction.accepted = true;
    transaction.publishWord800916D2 = initialSlowRequired;
    transaction.nextWord800916D2 =
        initialSlowRequired ? 1u : input.word800916D2;
    transaction.initialSlowTransitionRequired80020110 =
        initialSlowRequired;
    transaction.slowCtxAddress80020110 = kScene0WorkAddress;
    transaction.slowMode80020110 = 2;
    transaction.slowPreFfd4Arg80020110 = 1;
    transaction.slowPostFfd4Arg80020110 = 2;
    transaction.fastCtxAddress800201AC = kScene0WorkAddress;
    transaction.fastMode800201AC = 6;
    transaction.fastPreFfd4Arg800201AC = 1;
    transaction.fastPostFfd4Arg800201AC = 2;
    return transaction;
}

TransitionPlan BuildScene0Outer801C4DC4Plan(bool word800916D2WasZero)
{
    TransitionPlan plan{};
    plan.kind = TransitionPlanKind::Scene0Outer801C4DC4;
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();

    AppendCall(plan,
               TransitionActionKind::GateScene0OuterSource801C4DC4,
               kFn801C4DC4,
               kScene0WorkAddress,
               word800916D2WasZero ? 1u : 0u,
               (kSceneEntryMovie0SegmentOffset << 16) |
                   kSceneEntryMovie0TSegmentOffset,
               0);
    if (word800916D2WasZero) {
        AppendCall(plan,
                   TransitionActionKind::Call80020110,
                   kFn80020110,
                   kScene0WorkAddress,
                   2,
                   1,
                   2);
    }
    AppendCall(plan,
               TransitionActionKind::Call800201AC,
               kFn800201AC,
               kScene0WorkAddress,
               6,
               1,
               2);
    AppendCall(plan,
               TransitionActionKind::Call801C44E0,
               kFn801C44E0,
               kSceneEntryMovie0SegmentOffset,
               0);
    AppendCall(plan,
               TransitionActionKind::Call801C455C,
               kFn801C455C,
               kSceneEntryMovie0SegmentOffset,
               kScene0WorkAddress,
               0);
    AppendCall(plan,
               TransitionActionKind::Call8001B120,
               kFn8001B120,
               1);
    AppendCall(plan,
               TransitionActionKind::Call800201AC,
               kFn800201AC,
               kScene0WorkAddress,
               5,
               1,
               2);
    AppendCall(plan,
               TransitionActionKind::Call801C4894TitleLoop801C4E84,
               kFn801C4894,
               kSceneEntryMovie0TSegmentOffset,
               kScene0WorkAddress,
               kScene0TitleLoopCallsite801C4E84,
               kScene0TitleLoopEntryFrame6810);
    AppendCall(plan,
               TransitionActionKind::Call80026FA4TitleExit801C4E8C,
               kFn80026FA4,
               kScene0TitleExitAudioResetCallsite801C4E8C,
               kScene0TitleExitStartFrame7854);
    AppendCall(plan,
               TransitionActionKind::GateScene0TitleExitFadeCallsite801C4EA0,
               kFn801C4DC4,
               kScene0TitleExitFadeCallsite801C4EA0,
               kScene0TitleExitStartFrame7854);
    AppendCall(plan,
               TransitionActionKind::Call80020110TitleExitFade,
               kFn80020110,
               kScene0WorkAddress,
               2,
               1,
               2);
    AppendCall(plan,
               TransitionActionKind::Call80020008TitleExitFadeTail,
               kFn80020008,
               kScene0TitleExitFadeTailCallsite80020188,
               kScene0TitleExitFadeTailFrame7902);
    AppendCall(plan,
               TransitionActionKind::Call8001EF14TitleExitSceneTail801C4EA8,
               kFn8001EF14,
               kScene0TitleExitSceneTailCallsite801C4EA8,
               kScene0TitleExitSceneTailFrame7910);
    return plan;
}

TransitionPlan BuildScene0TitleResult801C4DC4Plan(int32_t selectorResult,
                                                  int32_t currentScene,
                                                  int32_t lastRandomScene)
{
    TransitionPlan plan{};
    plan.kind = TransitionPlanKind::Scene0TitleResult801C4DC4;
    plan.mode = selectorResult;
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();

    AppendCall(plan,
               TransitionActionKind::GateScene0TitleResultSource801C4DC4,
               kFn801C4DC4,
               static_cast<uint32_t>(selectorResult),
               static_cast<uint32_t>(currentScene),
               static_cast<uint32_t>(lastRandomScene),
               kScene0TitleResultReturnJoinCallsite801C4F48);
    AppendCall(plan,
               TransitionActionKind::CompareTitleResultMenu801C4EB0,
               kFn801C4DC4,
               static_cast<uint32_t>(selectorResult),
               kTitleSelectorResultMenu801C4DC4,
               kScene0TitleResultCompareOneCallsite801C4EB0);
    if (selectorResult == static_cast<int32_t>(kTitleSelectorResultMenu801C4DC4)) {
        AppendCall(plan,
                   TransitionActionKind::WriteWord800916D0TitleMenuZero801C4EBC,
                   kFn801C4DC4,
                   kWord800916D0Address,
                   0,
                   kScene0TitleResultMenuWriteCallsite801C4EBC);
        AppendCall(plan,
                   TransitionActionKind::ReturnTitleMenuMinusOne801C4EC8,
                   kFn801C4DC4,
                   0xFFFFFFFFu,
                   kScene0TitleResultMenuReturnCallsite801C4EC8,
                   kScene0TitleResultReturnJoinCallsite801C4F48);
        return plan;
    }

    AppendCall(plan,
               TransitionActionKind::CompareTitleResultRandom801C4ECC,
               kFn801C4DC4,
               static_cast<uint32_t>(selectorResult),
               kTitleSelectorResultRandom801C4DC4,
               kScene0TitleResultCompareRandomCallsite801C4ECC);
    if (selectorResult == static_cast<int32_t>(kTitleSelectorResultRandom801C4DC4)) {
        AppendCall(plan,
                   TransitionActionKind::WriteWord800916D0TitleRandomOne801C4ED4,
                   kFn801C4DC4,
                   kWord800916D0Address,
                   1,
                   kScene0TitleResultRandomWriteCallsite801C4ED4);
        AppendCall(plan,
                   TransitionActionKind::CallRandTitleRandomScene801C4EE0,
                   0,
                   kScene0TitleResultRandCallsite801C4EE0,
                   6,
                   1);
        AppendCall(plan,
                   TransitionActionKind::GateTitleRandomModuloSixPlusOne801C4EE8,
                   kFn801C4DC4,
                   kScene0TitleResultRandomModuloCallsite801C4EE8,
                   6,
                   1);
        AppendCall(plan,
                   TransitionActionKind::GateTitleRandomAvoidLastScene800916EE,
                   kFn801C4DC4,
                   kWord800916EEAddress,
                   static_cast<uint32_t>(lastRandomScene),
                   kScene0TitleResultLastRandomReadCallsite801C4F08,
                   kScene0TitleResultAvoidLastBranchCallsite801C4F14);
        AppendCall(plan,
                   TransitionActionKind::ReturnTitleRandomScene801C4F48,
                   kFn801C4DC4,
                   1,
                   6,
                   kScene0TitleResultReturnJoinCallsite801C4F48);
        return plan;
    }

    const int32_t defaultResult = currentScene + 1;
    AppendCall(plan,
               TransitionActionKind::GateTitleDefaultBranch801C4F24,
               kFn801C4DC4,
               static_cast<uint32_t>(selectorResult),
               kScene0TitleResultDefaultCueLoadCallsite801C4F24);
    AppendCall(plan,
               TransitionActionKind::Call80026EF8TitleDefaultCue801C4F2C,
               kFn80026EF8,
               kCueGlobal94410,
               kScene0TitleResultDefaultCueLoadCallsite801C4F24,
               kScene0TitleResultDefaultCueCallsite801C4F2C);
    AppendCall(plan,
               TransitionActionKind::Call80026ECCTitleDefaultFlush801C4F34,
               kFn80026ECC,
               kScene0TitleResultDefaultFlushCallsite801C4F34);
    AppendCall(plan,
               TransitionActionKind::ReturnTitleDefaultScenePlusOne801C4F3C,
               kFn801C4DC4,
               static_cast<uint32_t>(currentScene),
               static_cast<uint32_t>(defaultResult),
               kScene0TitleResultDefaultReturnCallsite801C4F3C,
               kScene0TitleResultReturnJoinCallsite801C4F48);
    AppendCall(plan,
               TransitionActionKind::WriteWord800916D0TitleDefaultZero801C4F40,
               kFn801C4DC4,
               kWord800916D0Address,
               0,
               kScene0TitleResultDefaultWriteCallsite801C4F40);
    return plan;
}

Scene0TitleResultResolveResult801C4DC4
ResolveScene0TitleResult801C4DC4(
    const Scene0TitleResultResolveInput801C4DC4& input)
{
    Scene0TitleResultResolveResult801C4DC4 result{};
    if (!input.selectorResultKnown) {
        return result;
    }

    switch (input.selectorResult) {
    case static_cast<int32_t>(kTitleSelectorResultMenu801C4DC4):
        result.word800916D0Known = true;
        result.word800916D0 = 0u;
        result.returnSceneKnown = true;
        result.returnScene = -1;
        result.status = Scene0TitleResultResolveStatus801C4DC4::Accepted;
        result.accepted = true;
        return result;

    case static_cast<int32_t>(kTitleSelectorResultStart801C4DC4):
        if (!input.currentSceneKnown) {
            return result;
        }
        if (input.currentScene < 0 ||
            input.currentScene == std::numeric_limits<int32_t>::max()) {
            result.status =
                Scene0TitleResultResolveStatus801C4DC4::InvalidInput;
            return result;
        }
        result.word800916D0Known = true;
        result.word800916D0 = 0u;
        result.cue94410Required = true;
        result.flush26ECCRequired = true;
        result.returnSceneKnown = true;
        result.returnScene = input.currentScene + 1;
        result.status = Scene0TitleResultResolveStatus801C4DC4::Accepted;
        result.accepted = true;
        return result;

    case static_cast<int32_t>(kTitleSelectorResultRandom801C4DC4):
        result.word800916D0Known = true;
        result.word800916D0 = 1u;
        result.publishWord800916D0BeforeDrawConsumption = true;
        if (!input.previousSceneKnown || !input.injectedRandDrawsKnown) {
            return result;
        }
        if (input.injectedRandDrawCount != 0u &&
            input.injectedRandDraws == nullptr) {
            result.status =
                Scene0TitleResultResolveStatus801C4DC4::DrawSourceMissing;
            return result;
        }

        for (std::size_t index = 0u;
             index < input.injectedRandDrawCount;
             ++index) {
            const int32_t draw = input.injectedRandDraws[index];
            ++result.drawCount;
            if (draw < 0) {
                result.status =
                    Scene0TitleResultResolveStatus801C4DC4::InvalidDraw;
                return result;
            }

            const int32_t candidateScene = draw % 6 + 1;
            if (candidateScene == input.previousScene) {
                continue;
            }
            result.returnSceneKnown = true;
            result.returnScene = candidateScene;
            result.status =
                Scene0TitleResultResolveStatus801C4DC4::Accepted;
            result.accepted = true;
            return result;
        }

        result.status =
            Scene0TitleResultResolveStatus801C4DC4::DrawsExhausted;
        return result;

    default:
        result.status = Scene0TitleResultResolveStatus801C4DC4::InvalidInput;
        return result;
    }
}

const char* TransitionPlanKindName(TransitionPlanKind kind)
{
    switch (kind) {
    case TransitionPlanKind::Unknown:
        return "Unknown";
    case TransitionPlanKind::Mode8001EA74:
        return "Mode8001EA74";
    case TransitionPlanKind::Slow80020110:
        return "Slow80020110";
    case TransitionPlanKind::Fast800201AC:
        return "Fast800201AC";
    case TransitionPlanKind::Scene0Outer801C4DC4:
        return "Scene0Outer801C4DC4";
    case TransitionPlanKind::Scene0TitleResult801C4DC4:
        return "Scene0TitleResult801C4DC4";
    }
    return "Unknown";
}

const char* TransitionActionKindName(TransitionActionKind kind)
{
    switch (kind) {
    case TransitionActionKind::None:
        return "None";
    case TransitionActionKind::GateModeSource8001EA74:
        return "GateModeSource8001EA74";
    case TransitionActionKind::Call80020110:
        return "Call80020110";
    case TransitionActionKind::Call800201AC:
        return "Call800201AC";
    case TransitionActionKind::Call8001EA74:
        return "Call8001EA74";
    case TransitionActionKind::GateTransitionCallSource80020110:
        return "GateTransitionCallSource80020110";
    case TransitionActionKind::GateTransitionCallSource800201AC:
        return "GateTransitionCallSource800201AC";
    case TransitionActionKind::Call8001FFD4:
        return "Call8001FFD4";
    case TransitionActionKind::Call8001EBF4:
        return "Call8001EBF4";
    case TransitionActionKind::Call8001F518:
        return "Call8001F518";
    case TransitionActionKind::Call80020008Tail:
        return "Call80020008Tail";
    case TransitionActionKind::Call80020090Tail:
        return "Call80020090Tail";
    case TransitionActionKind::Call8001FCBC:
        return "Call8001FCBC";
    case TransitionActionKind::Call8001FC40:
        return "Call8001FC40";
    case TransitionActionKind::Call8001FDC0:
        return "Call8001FDC0";
    case TransitionActionKind::Call8001D74C:
        return "Call8001D74C";
    case TransitionActionKind::Call80022CBC:
        return "Call80022CBC";
    case TransitionActionKind::Call80021E60:
        return "Call80021E60";
    case TransitionActionKind::Call80020308:
        return "Call80020308";
    case TransitionActionKind::Call80020248:
        return "Call80020248";
    case TransitionActionKind::Call8001F230:
        return "Call8001F230";
    case TransitionActionKind::Call8001FEB4:
        return "Call8001FEB4";
    case TransitionActionKind::Call8001C864:
        return "Call8001C864";
    case TransitionActionKind::Call8001CE30:
        return "Call8001CE30";
    case TransitionActionKind::Call80027194:
        return "Call80027194";
    case TransitionActionKind::Call80035560:
        return "Call80035560";
    case TransitionActionKind::Call801C44E0:
        return "Call801C44E0";
    case TransitionActionKind::Call801C455C:
        return "Call801C455C";
    case TransitionActionKind::Call801C4894:
        return "Call801C4894";
    case TransitionActionKind::Call801C4894TitleLoop801C4E84:
        return "Call801C4894TitleLoop801C4E84";
    case TransitionActionKind::Call8001B120:
        return "Call8001B120";
    case TransitionActionKind::Call80026FA4:
        return "Call80026FA4";
    case TransitionActionKind::Call80026FA4TitleExit801C4E8C:
        return "Call80026FA4TitleExit801C4E8C";
    case TransitionActionKind::GateScene0TitleExitFadeCallsite801C4EA0:
        return "GateScene0TitleExitFadeCallsite801C4EA0";
    case TransitionActionKind::Call80020110TitleExitFade:
        return "Call80020110TitleExitFade";
    case TransitionActionKind::Call80020008TitleExitFadeTail:
        return "Call80020008TitleExitFadeTail";
    case TransitionActionKind::Call8001EF14:
        return "Call8001EF14";
    case TransitionActionKind::Call8001EF14TitleExitSceneTail801C4EA8:
        return "Call8001EF14TitleExitSceneTail801C4EA8";
    case TransitionActionKind::GateScene0OuterSource801C4DC4:
        return "GateScene0OuterSource801C4DC4";
    case TransitionActionKind::GateScene0TitleResultSource801C4DC4:
        return "GateScene0TitleResultSource801C4DC4";
    case TransitionActionKind::CompareTitleResultMenu801C4EB0:
        return "CompareTitleResultMenu801C4EB0";
    case TransitionActionKind::WriteWord800916D0TitleMenuZero801C4EBC:
        return "WriteWord800916D0TitleMenuZero801C4EBC";
    case TransitionActionKind::ReturnTitleMenuMinusOne801C4EC8:
        return "ReturnTitleMenuMinusOne801C4EC8";
    case TransitionActionKind::CompareTitleResultRandom801C4ECC:
        return "CompareTitleResultRandom801C4ECC";
    case TransitionActionKind::WriteWord800916D0TitleRandomOne801C4ED4:
        return "WriteWord800916D0TitleRandomOne801C4ED4";
    case TransitionActionKind::CallRandTitleRandomScene801C4EE0:
        return "CallRandTitleRandomScene801C4EE0";
    case TransitionActionKind::GateTitleRandomModuloSixPlusOne801C4EE8:
        return "GateTitleRandomModuloSixPlusOne801C4EE8";
    case TransitionActionKind::GateTitleRandomAvoidLastScene800916EE:
        return "GateTitleRandomAvoidLastScene800916EE";
    case TransitionActionKind::ReturnTitleRandomScene801C4F48:
        return "ReturnTitleRandomScene801C4F48";
    case TransitionActionKind::GateTitleDefaultBranch801C4F24:
        return "GateTitleDefaultBranch801C4F24";
    case TransitionActionKind::Call80026EF8TitleDefaultCue801C4F2C:
        return "Call80026EF8TitleDefaultCue801C4F2C";
    case TransitionActionKind::Call80026ECCTitleDefaultFlush801C4F34:
        return "Call80026ECCTitleDefaultFlush801C4F34";
    case TransitionActionKind::ReturnTitleDefaultScenePlusOne801C4F3C:
        return "ReturnTitleDefaultScenePlusOne801C4F3C";
    case TransitionActionKind::WriteWord800916D0TitleDefaultZero801C4F40:
        return "WriteWord800916D0TitleDefaultZero801C4F40";
    case TransitionActionKind::SetGpC4:
        return "SetGpC4";
    case TransitionActionKind::SetGp792:
        return "SetGp792";
    case TransitionActionKind::Gap:
        return "Gap";
    }
    return "Unknown";
}

} // namespace PrSS0TransitionDirect
