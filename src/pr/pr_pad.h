#pragma once

#include <cstdint>

struct KeyBindings;

enum class PrPadButton : uint16_t {
    Up = 0x0001,
    Down = 0x0002,
    Left = 0x0004,
    Right = 0x0008,
    Triangle = 0x0010,
    Circle = 0x0020,
    Cross = 0x0040,
    Square = 0x0080,
    L1 = 0x0100,
    R1 = 0x0200,
    L2 = 0x0400,
    R2 = 0x0800,
    Start = 0x1000,
    Select = 0x2000,
};

struct PrPadState {
    uint16_t held = 0;
    uint16_t pressed = 0;
    uint16_t released = 0;
};

struct PrPad {
    static void Init();
    static void SetKeyBindings(const KeyBindings& kb);
    static void Poll(bool focused = true);
    static void CommitLogicFrame();
    static void Update(bool focused = true);
    static PrPadState GetState(int port);
    static double GetPressedAgeSeconds(int port, uint16_t mask);
};
