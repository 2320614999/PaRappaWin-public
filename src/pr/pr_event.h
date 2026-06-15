#pragma once

#include <cstdint>

#include "pr_psx_event_frame_direct.h"

struct PrGameContext;

using PrEventId = uint32_t;

enum class PrDispatcherState : uint8_t {
    Idle = 0,
    Running,
    Done
};

struct PrEventDispatcherContext {
    int16_t menuIndex = 0;
    int16_t menuCount = 0;
    int16_t selections[9] = {0};  // event=2 需要 8 个 enabled 槽位
    int32_t confirmFlag = 0;
    int32_t inputCooldown = 0;
    int32_t outNextScene = -1;    // event=2 产出的下一场景
    int32_t pauseChoice = -1;     // event=4 产出的选择 (0=Cross, 1=Circle)
};

struct PrEv16Ctx {
    int32_t frame;
    int32_t tick;
    int32_t round;
    int32_t frameInRound;
    int32_t mode;
    int32_t hits;
    int32_t hitGood;
    int32_t hitEarly;
    int32_t hitLate;
    int32_t flags00;
    int32_t state1C;
    int32_t blink4C;
    int32_t exit48;
    int32_t padStopFrames;

    int32_t seqMode8A;
    int32_t seqCurA8C;
    int32_t seqEnA90;
    int32_t seqCurB9E;
    int32_t seqEnBA2;

    const int8_t* streamA94;
    const int8_t* streamBA4;
};

struct PrEvMemCardCtx {
    int32_t eventId = 0;
    int32_t exitFrameOn = 0;
    int32_t exitTextOn = 0;
    int16_t rows = 5;
    int16_t cols = 3;
    int16_t itemCount = 16;
    int16_t selected = 15;
    int16_t enabled[15] = {0};
    int16_t entryBlockIndex[15] = {0}; // physical memcard directory entry, -1 when unknown
    char slotText[15][32] = {};
    int32_t ioMessage = 0;
    int32_t exitSelected = 0;
    int32_t replayResolvedSlot = -1;
    int32_t replayResolvedScene = -1;
};

struct PrEvent {
    static void Init();
    static void ClearDispatcherResidueForDirectCutover();
    static void Update(PrGameContext& ctx);

    static bool StartDispatcher(PrEventId eventId, int16_t* transitionStatePtr, PrGameContext& ctx);
    static bool StartDispatcherEx(PrEventId eventId, void* argPtr, PrGameContext& ctx);
    static bool IsDispatcherRunning();
    static int  ConsumeDispatcherResult();
    static PrEventDispatcherContext& GetDispatcherContext();
    static int* GetEv6DoneFlagPtr();
    static int* GetDispTimeoutPtr();
    static int* GetDispFrameCountdownPtr();
    static uint16_t* GetDispLastInputPtr();
    static bool IsEvMemCardArgBacked();
    static int* GetEv17BlinkPtr();
    static int* GetEv2BlinkPtr();
    static int* GetEv3BlinkPtr();
    static int GetStageSelectStatus(int stage);
    static int GetMemCardReplayResolvedSlot();
    static int GetMemCardReplayResolvedScene();
    static int16_t* GetDispMenuIndexPtr();
    static int* GetDispEventIdPtr();
    static void SetRecordsModePtr(int16_t* ptr);  // word_800916DA: records mode flag
    static void DebugUnlockNextStage();
    static void DebugFirstClearSelectableStages();

    static PrEv16Ctx* GetEv16CtxPtr();
    static PrEvMemCardCtx* GetEvMemCardCtxPtr();
    static const PrPsxEventFrameDirect::EventFrameState8001E750*
        GetActiveEventFrameState8001E750();
};
