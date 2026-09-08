#include "pr_ss0_event_backdrop_render_direct.h"

namespace PrSS0EventBackdropRenderDirect {
namespace {

constexpr uint32_t kEventBackdropTileTemplate8004E7D0 = 0x8004E7D0u;

constexpr EventBackdropSpriteTemplate8001B25C kEventBackdropTemplates[] = {
    {true, 0x8004E7D0u, 0x50000040u, 0x0332u, 0x0100u, 0x0028u, 0x0028u, 0x0100u, 0x01EAu},
    {true, 0x8004E7E0u, 0x50000040u, 0x0300u, 0x0100u, 0x0014u, 0x0064u, 0x0100u, 0x01EBu},
    {true, 0x8004E7F0u, 0x50000040u, 0x0305u, 0x0100u, 0x0014u, 0x0064u, 0x0100u, 0x01ECu},
    {true, 0x8004E800u, 0x50000040u, 0x030Au, 0x0100u, 0x0014u, 0x0064u, 0x0100u, 0x01EDu},
    {true, 0x8004E810u, 0x50000040u, 0x030Fu, 0x0100u, 0x0014u, 0x0064u, 0x0100u, 0x01EEu},
    {true, 0x8004E820u, 0x50000040u, 0x0314u, 0x0100u, 0x0078u, 0x0014u, 0x0100u, 0x01EFu},
    {true, 0x8004E830u, 0x50000040u, 0x0314u, 0x0114u, 0x0078u, 0x0014u, 0x0100u, 0x01F0u},
    {true, 0x8004E840u, 0x50000040u, 0x0314u, 0x0128u, 0x0078u, 0x0014u, 0x0100u, 0x01F1u},
    {true, 0x8004E850u, 0x50000040u, 0x0314u, 0x013Cu, 0x0078u, 0x0014u, 0x0100u, 0x01F2u},
    {true, 0x8004E900u, 0x10000040u, 0x0380u, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01EEu},
    {true, 0x8004E910u, 0x10000040u, 0x0385u, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01EFu},
    {true, 0x8004E920u, 0x10000040u, 0x038Au, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01F0u},
    {true, 0x8004E930u, 0x10000040u, 0x038Fu, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01F1u},
};

bool AppendEventBackdropSprite8001D74C(
    EventBackdropDrawList8001D74C& out,
    uint16_t priority,
    int16_t x,
    int16_t y,
    uint32_t templateAddress) {
    if (out.count >= kEventBackdropSpriteCapacity8001D74C) {
        out.truncated = true;
        return false;
    }

    EventBackdropSpriteCommand8001D74C command{};
    command.sprite =
        ResolveEventBackdropSpriteTemplate8001B25C(templateAddress);
    if (!command.sprite.known) {
        return false;
    }
    command.known = true;
    command.x = x;
    command.y = y;
    command.priority = priority;
    command.callOrder = out.count;
    out.commands[out.count++] = command;
    out.rawTextureOnly =
        out.rawTextureOnly && (command.sprite.attr & 0x40u) != 0u;
    return true;
}

}  // namespace

EventBackdropSpriteTemplate8001B25C
ResolveEventBackdropSpriteTemplate8001B25C(uint32_t psxAddress) {
    for (const auto& sprite : kEventBackdropTemplates) {
        if (sprite.psxAddress == psxAddress) {
            return sprite;
        }
    }
    return {};
}

EventBackdropDrawList8001D74C BuildEventBackdropDrawList8001D74C(
    uint16_t priority) {
    EventBackdropDrawList8001D74C out{};
    out.sourceKnown = true;
    out.rawTextureOnly = true;
    bool ok = true;

    ok = AppendEventBackdropSprite8001D74C(
             out, priority, 20, 20, 0x8004E7E0u) && ok;
    ok = AppendEventBackdropSprite8001D74C(
             out, priority, 20, 120, 0x8004E7F0u) && ok;
    ok = AppendEventBackdropSprite8001D74C(
             out, priority, 280, 20, 0x8004E800u) && ok;
    ok = AppendEventBackdropSprite8001D74C(
             out, priority, 280, 120, 0x8004E810u) && ok;
    ok = AppendEventBackdropSprite8001D74C(
             out, priority, 40, 20, 0x8004E820u) && ok;
    ok = AppendEventBackdropSprite8001D74C(
             out, priority, 160, 20, 0x8004E830u) && ok;
    ok = AppendEventBackdropSprite8001D74C(
             out, priority, 40, 200, 0x8004E840u) && ok;
    ok = AppendEventBackdropSprite8001D74C(
             out, priority, 160, 200, 0x8004E850u) && ok;

    for (int32_t x = 40; x < 280; x += 40) {
        for (int32_t y = 40; y < 200; y += 40) {
            ok = AppendEventBackdropSprite8001D74C(
                     out,
                     priority,
                     static_cast<int16_t>(x),
                     static_cast<int16_t>(y),
                     kEventBackdropTileTemplate8004E7D0) && ok;
        }
    }

    for (int32_t x = 40; x < 320; x += 40) {
        ok = AppendEventBackdropSprite8001D74C(
                 out, priority, static_cast<int16_t>(x), 0,
                 0x8004E900u) && ok;
        ok = AppendEventBackdropSprite8001D74C(
                 out, priority, static_cast<int16_t>(x - 20), 0,
                 0x8004E910u) && ok;
        ok = AppendEventBackdropSprite8001D74C(
                 out, priority, static_cast<int16_t>(x), 220,
                 0x8004E920u) && ok;
        ok = AppendEventBackdropSprite8001D74C(
                 out, priority, static_cast<int16_t>(x - 20), 220,
                 0x8004E930u) && ok;
    }

    for (int32_t y = 0; y < 240; y += 40) {
        ok = AppendEventBackdropSprite8001D74C(
                 out, priority, 0, static_cast<int16_t>(y),
                 0x8004E900u) && ok;
        ok = AppendEventBackdropSprite8001D74C(
                 out, priority, 300, static_cast<int16_t>(y),
                 0x8004E910u) && ok;
        ok = AppendEventBackdropSprite8001D74C(
                 out, priority, 0, static_cast<int16_t>(y + 20),
                 0x8004E920u) && ok;
        ok = AppendEventBackdropSprite8001D74C(
                 out, priority, 300, static_cast<int16_t>(y + 20),
                 0x8004E930u) && ok;
    }

    out.accepted = ok && !out.truncated && out.rawTextureOnly &&
                   out.count == kEventBackdropSpriteCapacity8001D74C;
    out.complete = out.sourceKnown && out.accepted;
    return out;
}

}  // namespace PrSS0EventBackdropRenderDirect
