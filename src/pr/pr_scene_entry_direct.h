#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrSceneEntryDirect {

static constexpr uint32_t kSceneEntryTableBase8005474C = 0x8005474Cu;
static constexpr uint32_t kSceneEntryStride8005474C = 0x16Cu;
static constexpr uint32_t kSceneEntryFirstMovieSegmentOffset = 0x0Cu;
static constexpr uint32_t kSceneEntryMovieSegmentRowSize = 0x30u;
static constexpr uint32_t kSceneEntryMovieSegmentRowWordCount =
    kSceneEntryMovieSegmentRowSize / 4u;
static constexpr uint32_t kSceneEntryMovieSegmentRowCount = 7u;
static constexpr uint32_t kSceneEntrySceneCount = 7u;
static constexpr uint32_t kSceneCallbackTableSceneCount80048D28 = 9u;
static constexpr size_t kMainSceneRequestMax80015D18 = 8u;
static constexpr size_t kGenericSwitchRequestMax80015788 = 8u;
static constexpr size_t kHiScoreRowCount80019284 = 6u;
static constexpr size_t kHiScoreColumnCount80019284 = 3u;
static constexpr size_t kHiScoreCellCount80019284 =
    kHiScoreRowCount80019284 * kHiScoreColumnCount80019284;
static constexpr size_t kHiScoreRecordStride80019284 = 16u;
static constexpr size_t kHiScoreTableHeaderSize80019284 = 20u;
static constexpr size_t kHiScoreTableSize80019284 =
    kHiScoreTableHeaderSize80019284 +
    kHiScoreCellCount80019284 * kHiScoreRecordStride80019284;
static constexpr size_t kHiScoreInputRequiredSize80019284 = 5244u;
static constexpr size_t kHiScoreStatusPrefixSize80019284 = 4876u;
static constexpr size_t kHiScoreBankRowCount800164F8 = 6u;
static constexpr size_t kHiScoreBankVisibleSlotCount800164F8 = 3u;
static constexpr size_t kHiScoreBankSlotCount800164F8 = 4u;
static constexpr size_t kHiScoreBankSlotStride80016000 = 16u;
static constexpr size_t kHiScoreBankRowStride80016000 =
    kHiScoreBankSlotCount800164F8 * kHiScoreBankSlotStride80016000;
static constexpr size_t kHiScoreBankSize8009421C =
    kHiScoreBankRowCount800164F8 * kHiScoreBankRowStride80016000;
static constexpr size_t kHiScoreFullA1MemorySize80019284 =
    kHiScoreStatusPrefixSize80019284 + kHiScoreBankSize8009421C;
static constexpr size_t kHiScoreBankScratchSlotIndex800164F8 = 3u;
static constexpr size_t kHiScoreSavePayloadNameOffset800164F8 = 1u;
static constexpr size_t kHiScoreBankNameSize80016000 = 12u;
static constexpr size_t kHiScoreSavePayloadScoreBase800164F8 = 20u;
static constexpr size_t kHiScoreSavePayloadScoreStride800164F8 = 4u;
static constexpr size_t kHiScoreSavePayloadRequiredSize800164F8 =
    kHiScoreSavePayloadScoreBase800164F8 +
    kHiScoreBankRowCount800164F8 * kHiScoreSavePayloadScoreStride800164F8;
static constexpr size_t kHiScoreCardRowCount80019D7C = 15u;
struct SceneEntryKey {
    bool sceneIndexKnown = false;
    uint32_t sceneIndex = 0;
    bool baseKnown = false;
    uint32_t base = 0;
};

enum class SceneEntryRowRole : uint8_t {
    Unknown = 0,
    Comod,
    Compo,
    OpeningMovie,
    StageRuntime,
    ClearMovieA,
    ClearMovieB,
    ZCompo,
};

enum class SceneEntryPathRole : uint8_t {
    Unknown = 0,
    Comod,
    Compo,
    OpeningMovie,
    StageRuntime,
    ClearMovie,
    ZCompo,
};

struct SceneEntryPathIdentity {
    bool known = false;
    bool pathPtrKnown = false;
    uint32_t pathPtr = 0;
    SceneEntryPathRole role = SceneEntryPathRole::Unknown;
    const char* psxPath = nullptr;
    const char* relativeWinPath = nullptr;
};

struct SceneEntryRawRow {
    bool known = false;
    uint32_t words[kSceneEntryMovieSegmentRowWordCount]{};
    uint8_t bytes[kSceneEntryMovieSegmentRowSize]{};
    bool pathPtrKnown = false;
    uint32_t pathPtr = 0;
    bool opaque04Known = false;
    uint32_t opaque04 = 0;
    bool endBias08Known = false;
    int32_t endBias08 = 0;
    bool loadedState0CKnown = false;
    int32_t loadedState0C = 0;
};

struct SceneEntryRowIdentity {
    bool known = false;
    SceneEntryKey entry{};
    bool rowIndexKnown = false;
    uint32_t rowIndex = 0;
    bool rowOffsetKnown = false;
    uint32_t rowOffset = 0;
    bool rowAddrKnown = false;
    uint32_t rowAddr = 0;
    SceneEntryRowRole role = SceneEntryRowRole::Unknown;
    SceneEntryPathIdentity path{};
    SceneEntryRawRow raw{};
};

enum class MainSceneRequestKind80015D18 : uint8_t {
    None = 0,
    Call8001EF14,
    Call80015788,
    Call80025A34,
    Call8001A324,
    Call800154B0,
    CallSceneFn0,
    CallSceneFn1,
    CallSceneFn2,
    Call80015CC4,
    Call8001E34C,
};

struct MainSceneRequest80015D18 {
    bool valid = false;
    MainSceneRequestKind80015D18 kind = MainSceneRequestKind80015D18::None;
    uint32_t psxFunction = 0;
    bool psxFunctionSlotKnown = false;
    uint32_t psxFunctionSlot = 0;
    bool arg0Known = false;
    int32_t arg0 = 0;
    bool arg1Known = false;
    int32_t arg1 = 0;
};

struct MainSceneRequestTrace80015D18 {
    std::array<MainSceneRequest80015D18, kMainSceneRequestMax80015D18>
        requests{};
    size_t count = 0u;
    bool overflow = false;
};

struct MainSceneState80015D18 {
    bool word800916D0Known = false;
    uint16_t word800916D0 = 0;
    bool word800916D4Known = false;
    uint16_t word800916D4 = 0;
    bool word800916DEKnown = false;
    uint16_t word800916DE = 0;
    bool word800916E2Known = false;
    uint16_t word800916E2 = 0;
    bool word800916EEKnown = false;
    int32_t word800916EE = 0;
    bool currentSceneV0Known = false;
    int32_t currentSceneV0 = 0;
    bool previousSceneV1Known = false;
    int32_t previousSceneV1 = 0;
};

struct Scene0TitleSelectorResult801C4DC4 {
    bool known = false;
    bool valid = false;
    uint16_t word800916D0 = 0;
    int fn2ReturnScene = 0;
    int randomScene = 0;
    bool word800916EEWrite = false;
    int16_t word800916EE = 0;
};

struct MainSceneStepResult80015D18 {
    MainSceneState80015D18 state{};
    MainSceneRequestTrace80015D18 trace{};
    bool waitingFor80015788Result = false;
    bool waitingForSceneFn2Result = false;
    bool invalidCurrentSceneIndex = false;
    bool missingWord800916D0For15CC4Gate = false;
};

struct SceneCallbackTriplet80048D28 {
    bool known = false;
    uint32_t sceneIndex = 0;
    uint32_t fn0Slot = 0;
    uint32_t fn0 = 0;
    uint32_t fn1Slot = 0;
    uint32_t fn1 = 0;
    uint32_t fn2Slot = 0;
    uint32_t fn2 = 0;
};

enum class GenericSwitchPhase80015788 : uint8_t {
    Start = 0,
    WaitInput35510,
    WaitEvent3,
    WaitCall19414,
    WaitEvent6,
    WaitCall193F4,
    WaitCall161A8,
    WaitPractice2776C,
    WaitEvent2,
    WaitSave193B0,
    WaitEvent17,
    Done,
};

enum class GenericSwitchRequestKind80015788 : uint8_t {
    None = 0,
    Call80026FA4,
    Call80026EF8,
    Call80026ECC,
    Call80015590,
    Call80035510,
    Call80026B94,
    Call80019414,
    Call80015700,
    Call800193F4,
    Call800161A8,
    Call8002776C,
    Call800193B0,
};

struct GenericSwitchRequest80015788 {
    bool valid = false;
    GenericSwitchRequestKind80015788 kind =
        GenericSwitchRequestKind80015788::None;
    uint32_t psxFunction = 0;
    bool arg0Known = false;
    int32_t arg0 = 0;
    bool arg1Known = false;
    int32_t arg1 = 0;
};

struct GenericSwitchRequestTrace80015788 {
    std::array<GenericSwitchRequest80015788,
               kGenericSwitchRequestMax80015788>
        requests{};
    size_t count = 0u;
    bool overflow = false;
};

struct GenericSwitchState80015788 {
    GenericSwitchPhase80015788 phase = GenericSwitchPhase80015788::Start;
    bool prevSceneA1Known = false;
    int32_t prevSceneA1 = 0;
    bool word800916D0Known = false;
    uint16_t word800916D0 = 0;
    bool lastEventResultV2Known = false;
    int32_t lastEventResultV2 = 0;
    bool replaySlotV3Known = false;
    int32_t replaySlotV3 = 0;
    bool returnValueKnown = false;
    int32_t returnValue = 0;
};

struct GenericSwitchFeedback80015788 {
    bool poll35510Known = false;
    int32_t poll35510Result = 0;
    bool event26B94ResultKnown = false;
    int32_t event26B94Result = 0;
    bool event26B94OutArgKnown = false;
    int32_t event26B94OutArg = 0;
    bool word800916D0Written = false;
    uint16_t word800916D0 = 0;
    bool call19414ResultKnown = false;
    int32_t call19414Result = 0;
    bool call193F4ResultKnown = false;
    int32_t call193F4Result = 0;
    bool call161A8ResultKnown = false;
    int32_t call161A8Result = 0;
    bool call2776CDoneKnown = false;
    bool call2776CDone = false;
    bool call193B0DoneKnown = false;
    bool call193B0Done = false;
};

struct Call80019414Feedback80015788 {
    bool sub80026784ResultKnown = false;
    int32_t sub80026784Result = 0;
    bool gp716AfterStateMachineKnown = false;
    int32_t gp716AfterStateMachine = 0;
    bool wordA1Plus44Known = false;
    int32_t wordA1Plus44 = 0;
    bool gp720Known = false;
    int32_t gp720 = 0;
    bool call80019284ResultKnown = false;
    int32_t call80019284Result = 0;
    void* call80019284HostArgPtr = nullptr;
};

struct HiScoreCell80019284 {
    bool scoreKnown = false;
    int32_t score = 0;
    bool nameKnown = false;
    char name[4] = {};
    bool formattedAsciiKnown = false;
    char formattedAscii[32] = {};
    bool requested80017FC4Copy = false;
    bool glyphBytesKnown = false;
    size_t glyphByteWriteCount = 0;
    uint8_t glyphBytes[kHiScoreRecordStride80019284] = {};
    uint32_t destinationOffset = 0;
};

struct Call80019284Result80015788 {
    uint32_t sourceFunction = 0;
    bool arg0Known = false;
    int32_t arg0 = 0;
    bool inputRangeKnown = false;
    size_t inputSize = 0;
    bool initialTableKnown = false;
    size_t initialTableSize = 0;
    bool resultKnown = false;
    int32_t result = 0;
    bool tableAsciiKnown = false;
    bool tablePsxGlyphBytesKnown = false;
    bool missingInputRange = false;
    bool missing80017FC4GlyphMap = false;
    bool glyphCopyOverflow80017FC4 = false;
    bool nonPositiveRecordTailCarry80019284 = false;
    bool glyphRecordTailCarry80017FC4 = false;
    uint8_t tableBytes[kHiScoreTableSize80019284] = {};
    HiScoreCell80019284 cells[kHiScoreCellCount80019284] = {};
};

struct Call80019284InputMemory80015788 {
    bool inputMemoryKnown = false;
    bool missingStatusPrefix = false;
    bool missingBankSlot = false;
    size_t statusPrefixSize = 0;
    uint8_t a1Memory[kHiScoreFullA1MemorySize80019284] = {};
    uint8_t initialTableMemory[kHiScoreTableSize80019284] = {};
};

struct Call800191E4Result80015788 {
    uint32_t sourceFunction = 0;
    bool arg0Known = false;
    int32_t arg0 = 0;
    bool modeKnown = false;
    int32_t mode = 0;
    bool called80026784 = false;
    bool sub80026784ResultKnown = false;
    int32_t sub80026784Result = 0;
    bool copied36BytesTo8007CC50 = false;
    bool wroteGp716Zero = false;
    bool wroteGp732Mode = false;
    bool called80017524 = false;
    bool called80018FB0 = false;
    uint32_t callback80018E10 = 0;
    uint32_t callback80019D7C = 0;
    int32_t stateMachineArg3 = 0;
    int32_t stateMachineArg4 = 0;
    bool called80017574 = false;
    bool gp716AfterStateMachineKnown = false;
    int32_t gp716AfterStateMachine = 0;
    bool wordA1Plus44Known = false;
    int32_t wordA1Plus44 = 0;
    bool resultKnown = false;
    int32_t result = 0;
    bool missingGp716AfterStateMachine = false;
    bool missingA1Plus44 = false;
};

struct Call80019414Result80015788 {
    uint32_t sourceFunction = 0;
    bool arg0Known = false;
    int32_t arg0 = 0;
    Call800191E4Result80015788 call800191E4{};
    bool gp720Known = false;
    int32_t gp720 = 0;
    bool called80019284 = false;
    uint32_t call80019284Function = 0;
    bool resultKnown = false;
    int32_t result = 0;
    void* event6HostArgPtr = nullptr;
    bool missingGp720 = false;
    bool missing80019284Result = false;
};

struct HiScoreBankName80016000 {
    bool known = false;
    bool nulTerminatedBeforeScore = false;
    uint8_t bytes[kHiScoreBankNameSize80016000] = {};
};

struct HiScoreBankSlot800164F8 {
    bool valid = false;
    bool scoreKnown = false;
    int32_t score = 0;
    HiScoreBankName80016000 name{};
};

struct HiScoreBankRow800164F8 {
    std::array<HiScoreBankSlot800164F8,
               kHiScoreBankSlotCount800164F8>
        slots{};
    bool wroteScratchSlotBefore80016000 = false;
    uint32_t sortFunction80016000 = 0;
    int32_t sortArg80016000 = 0;
    bool sortConvergenceGap80016000 = false;
};

struct HiScoreBankCarrier800164F8 {
    uint32_t psxAddress = 0x80092F10u;
    bool clearedBy800168DC = false;
    std::array<HiScoreBankRow800164F8, kHiScoreBankRowCount800164F8>
        rows{};
};

struct HiScoreSavePayload800164F8 {
    bool inputRangeKnown = false;
    size_t inputSize = 0;
    bool missingInputRange = false;
    HiScoreBankName80016000 name{};
    bool scoreKnown[kHiScoreBankRowCount800164F8] = {};
    int32_t score[kHiScoreBankRowCount800164F8] = {};
};

struct HiScoreMergeRow800164F8 {
    bool candidateKnown = false;
    bool duplicateInVisibleSlots = false;
    bool wroteScratchSlot = false;
    uint32_t sortFunction80016000 = 0;
    int32_t sortArg80016000 = 0;
    bool sortConvergenceGap80016000 = false;
    bool sortedBy80016000 = false;
};

struct Call800164F8Result80019D7C {
    uint32_t sourceFunction = 0;
    HiScoreSavePayload800164F8 payload{};
    HiScoreBankCarrier800164F8 beforeBank{};
    HiScoreBankCarrier800164F8 afterBank{};
    std::array<HiScoreMergeRow800164F8,
               kHiScoreBankRowCount800164F8>
        rows{};
};

struct Case17CardRow80019D7C {
    bool rowEnabledKnown = false;
    bool rowEnabled = false;
    bool rowNameBuffer8007CBE8Known = false;
    bool liveCase17PayloadViewKnown = false;
    bool successAuthorityKnown800179B4 = false;
    bool success800179B4 = false;
    bool readResultKnown = false;
    bool readSucceeded = false;
    bool eventResult80016EB8Known = false;
    int32_t eventResult80016EB8 = 0;
    bool rowMetadata8007AE14Known = false;
    uint32_t rowMetadata8007AE14 = 0;
    bool readBufferKnown = false;
    uint32_t readBuffer = 0;
    bool readLengthKnown = false;
    size_t readLength = 0;
    bool payloadPointerKnown = false;
    uint32_t payloadPointer = 0;
    bool payloadPassedTo800164F8 = false;
    const uint8_t* payload = nullptr;
    size_t payloadSize = 0;
};

struct Case17Feedback80019D7C {
    bool word8007ABE4Known = false;
    int32_t word8007ABE4 = 0;
    std::array<Case17CardRow80019D7C, kHiScoreCardRowCount80019D7C>
        cardRows{};
};

struct Case17CardRowResult80019D7C {
    bool clearedBlockBuffer80025C44 = false;
    bool rowEnabledKnown = false;
    bool rowEnabled = false;
    bool copiedRowNameTo8007CBE8 = false;
    uint32_t cardReadFunction800179B4 = 0;
    bool readResultKnown = false;
    bool readSucceeded = false;
    bool eventResult80016EB8Known = false;
    int32_t eventResult80016EB8 = 0;
    bool rowMetadata8007AE14Known = false;
    uint32_t rowMetadata8007AE14 = 0;
    bool wroteRowMetadata = false;
    bool readBufferKnown = false;
    uint32_t readBuffer = 0;
    bool readLengthKnown = false;
    size_t readLength = 0;
    bool payloadPointerKnown = false;
    uint32_t payloadPointer = 0;
    bool payloadPassedTo800164F8 = false;
    bool mergeCalled = false;
    Call800164F8Result80019D7C merge{};
};

struct Case17Result80019D7C {
    uint32_t sourceFunction = 0;
    bool arg2Known = false;
    int32_t arg2 = 0;
    bool missingArg2ForCase17Gate = false;
    bool returnedEarlyForArg2Equals3 = false;
    bool called800168DC = false;
    uint32_t clearFunction800168DC = 0;
    bool missingWord8007ABE4 = false;
    bool calledVSyncCallback80017F38 = false;
    bool clearedVSyncCallback = false;
    std::array<Case17CardRowResult80019D7C,
               kHiScoreCardRowCount80019D7C>
        cardRows{};
    HiScoreBankCarrier800164F8 bank{};
    bool gp720Written = false;
    int32_t gp720 = 0;
    bool resultKnown = false;
    int32_t result = 0;
};

struct GenericSwitchStepResult80015788 {
    GenericSwitchState80015788 state{};
    GenericSwitchRequestTrace80015788 trace{};
    bool waitingForFeedback = false;
    bool done = false;
};

SceneEntryKey BuildSceneEntryKeyFromSceneIndex(uint32_t sceneIndex);
uint32_t ComputeSceneEntryRowOffset(uint32_t rowIndex);
SceneEntryRowRole DefaultMovieSegmentRowRole(uint32_t rowIndex);
SceneEntryPathIdentity IdentifySceneEntryPathPtr(uint32_t pathPtr);
SceneEntryRawRow GetSceneEntryStaticRawRow(uint32_t sceneIndex,
                                           uint32_t rowIndex);
SceneEntryRowIdentity IdentifySceneEntryRow(const SceneEntryKey& key,
                                            uint32_t rowIndex);
SceneEntryRowIdentity IdentifySceneEntryRowByOffset(const SceneEntryKey& key,
                                                    uint32_t rowOffset);
SceneCallbackTriplet80048D28 GetSceneCallbackTriplet80048D28(
    uint32_t sceneIndex);

MainSceneState80015D18 InitMainSceneState80015D18();
MainSceneStepResult80015D18
BeginMainSceneLoopIteration80015D18(const MainSceneState80015D18& in);
MainSceneState80015D18
ApplyMainSceneSwitchResult80015D18(const MainSceneState80015D18& in,
                                   int32_t switchResult);
MainSceneStepResult80015D18
PrepareMainSceneCallbacks80015D18(const MainSceneState80015D18& in);
MainSceneStepResult80015D18
CompleteMainSceneLoopIteration80015D18(const MainSceneState80015D18& in,
                                       int32_t sceneFn2Result);
Scene0TitleSelectorResult801C4DC4
ResolveScene0TitleSelectorResult801C4DC4(int selectorResult,
                                         int scene,
                                         int16_t lastWord800916EE);
GenericSwitchState80015788 InitGenericSwitchState80015788(int32_t prevScene);
GenericSwitchStepResult80015788
StepGenericSwitch80015788(const GenericSwitchState80015788& in,
                          const GenericSwitchFeedback80015788& feedback);
Call800191E4Result80015788
PsxCall800191E4_MemcardStateMachine80015788(
    int32_t a1,
    bool a1Known,
    int32_t mode,
    bool modeKnown,
    const Call80019414Feedback80015788& feedback);
Call80019284Result80015788
PsxCall80019284_BuildHiScoreRecords80015788(
    int32_t a1,
    bool a1Known,
    const uint8_t* a1Memory,
    size_t a1MemorySize,
    const uint8_t* initialTableMemory = nullptr,
    size_t initialTableMemorySize = 0);
Call80019284InputMemory80015788
PsxBuild80019284InputMemoryFromStatusAndBank80015788(
    const uint8_t* statusPrefix80092F10,
    size_t statusPrefixSize,
    const HiScoreBankCarrier800164F8& bank);
Call80019414Result80015788
PsxCall80019414_HiScoreEntry80015788(
    int32_t a1,
    bool a1Known,
    const Call80019414Feedback80015788& feedback);
HiScoreBankCarrier800164F8
PsxCall800168DC_ClearHiScoreBank80019D7C();
Call800164F8Result80019D7C
PsxCall800164F8_MergeSavePayloadHiScoreBank80019D7C(
    const HiScoreBankCarrier800164F8& bank,
    const uint8_t* payload,
    size_t payloadSize);
Case17Result80019D7C
PsxCall80019D7C_Case17HiScoreBankCarrier(
    int32_t a2,
    bool a2Known,
    const Case17Feedback80019D7C& feedback);

}  // namespace PrSceneEntryDirect
