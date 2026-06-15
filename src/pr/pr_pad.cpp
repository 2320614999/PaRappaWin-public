#include "pr_pad.h"
#include "../app_config.h"

#include <windows.h>
#include <array>
#include <chrono>

namespace {

using PadClock = std::chrono::steady_clock;

static PrPadState g_state[2]{};
static uint16_t g_prevHeld[2] = {0, 0};
static uint16_t g_polledHeld[2] = {0, 0};
static uint16_t g_latchedPressed[2] = {0, 0};
static uint16_t g_latchedReleased[2] = {0, 0};
static std::array<std::array<PadClock::time_point, 16>, 2> g_pressedAt{};
static uint16_t g_pressedAtKnown[2] = {0, 0};
static KeyBindings g_kb; // current bindings (defaults match AppConfig defaults)

static bool KeyDown(int vk) {
    if (vk <= 0) return false;
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

static uint16_t BuildKeyboardMask() {
    uint16_t m = 0;

    if (KeyDown(g_kb.up)    || KeyDown(g_kb.up_alt))    m |= (uint16_t)PrPadButton::Up;
    if (KeyDown(g_kb.down)  || KeyDown(g_kb.down_alt))  m |= (uint16_t)PrPadButton::Down;
    if (KeyDown(g_kb.left)  || KeyDown(g_kb.left_alt))  m |= (uint16_t)PrPadButton::Left;
    if (KeyDown(g_kb.right) || KeyDown(g_kb.right_alt)) m |= (uint16_t)PrPadButton::Right;

    if (KeyDown(g_kb.triangle)) m |= (uint16_t)PrPadButton::Triangle;
    if (KeyDown(g_kb.circle))   m |= (uint16_t)PrPadButton::Circle;
    if (KeyDown(g_kb.cross) || KeyDown(g_kb.cross_alt) || KeyDown(g_kb.cross_alt2))
        m |= (uint16_t)PrPadButton::Cross;
    if (KeyDown(g_kb.square)) m |= (uint16_t)PrPadButton::Square;

    if (KeyDown(g_kb.l1)) m |= (uint16_t)PrPadButton::L1;
    if (KeyDown(g_kb.r1)) m |= (uint16_t)PrPadButton::R1;
    if (KeyDown(g_kb.l2)) m |= (uint16_t)PrPadButton::L2;
    if (KeyDown(g_kb.r2)) m |= (uint16_t)PrPadButton::R2;

    if (KeyDown(g_kb.start))  m |= (uint16_t)PrPadButton::Start;
    if (KeyDown(g_kb.select)) m |= (uint16_t)PrPadButton::Select;

    return m;
}

} // namespace

void PrPad::Init() {
    g_state[0] = {};
    g_state[1] = {};
    g_prevHeld[0] = 0;
    g_prevHeld[1] = 0;
    g_polledHeld[0] = 0;
    g_polledHeld[1] = 0;
    g_latchedPressed[0] = 0;
    g_latchedPressed[1] = 0;
    g_latchedReleased[0] = 0;
    g_latchedReleased[1] = 0;
    g_pressedAtKnown[0] = 0;
    g_pressedAtKnown[1] = 0;
    g_pressedAt = {};
}

void PrPad::SetKeyBindings(const KeyBindings& kb) {
    g_kb = kb;
}

void PrPad::Poll(bool focused) {
    const uint16_t held0 = focused ? BuildKeyboardMask() : 0;
    const uint16_t prev0 = g_prevHeld[0];
    const uint16_t pressed0 = (uint16_t)(held0 & ~prev0);
    const PadClock::time_point now = PadClock::now();

    g_polledHeld[0] = held0;
    g_latchedPressed[0] |= pressed0;
    g_latchedReleased[0] |= (uint16_t)(prev0 & ~held0);
    g_prevHeld[0] = held0;
    for (int bit = 0; bit < 16; ++bit) {
        const uint16_t mask = (uint16_t)(1u << bit);
        if ((pressed0 & mask) != 0u) {
            g_pressedAt[0][(size_t)bit] = now;
            g_pressedAtKnown[0] |= mask;
        }
    }

    const uint16_t held1 = 0;
    const uint16_t prev1 = g_prevHeld[1];

    g_polledHeld[1] = held1;
    g_latchedPressed[1] |= (uint16_t)(held1 & ~prev1);
    g_latchedReleased[1] |= (uint16_t)(prev1 & ~held1);
    g_prevHeld[1] = held1;
}

void PrPad::CommitLogicFrame() {
    for (int port = 0; port < 2; ++port) {
        g_state[port].held = g_polledHeld[port];
        g_state[port].pressed = g_latchedPressed[port];
        g_state[port].released = g_latchedReleased[port];
        g_latchedPressed[port] = 0;
        g_latchedReleased[port] = 0;
    }
}

void PrPad::Update(bool focused) {
    Poll(focused);
    CommitLogicFrame();
}

PrPadState PrPad::GetState(int port) {
    if (port < 0 || port > 1) {
        return {};
    }
    return g_state[port];
}

double PrPad::GetPressedAgeSeconds(int port, uint16_t mask) {
    if (port < 0 || port > 1 || mask == 0u) {
        return 0.0;
    }

    const uint16_t known = (uint16_t)(g_pressedAtKnown[port] & mask);
    if (known == 0u) {
        return 0.0;
    }

    const PadClock::time_point now = PadClock::now();
    bool found = false;
    double youngest = 0.0;
    for (int bit = 0; bit < 16; ++bit) {
        const uint16_t bitMask = (uint16_t)(1u << bit);
        if ((known & bitMask) == 0u) {
            continue;
        }
        double age =
            std::chrono::duration<double>(
                now - g_pressedAt[port][(size_t)bit]).count();
        if (age < 0.0) {
            age = 0.0;
        }
        if (!found || age < youngest) {
            youngest = age;
            found = true;
        }
    }
    return found ? youngest : 0.0;
}
