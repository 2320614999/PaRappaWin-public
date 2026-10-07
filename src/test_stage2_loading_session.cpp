#include "pr/pr_stage2_loading_session.h"
#include "pr/pr_stage2_frame_task.h"
#include "pr/pr_stage2_frame_wait.h"
#include "pr/pr_stage2_disc_file_device.h"
#include "pr/pr_stage2_vram_device.h"
#include "pr/pr_stage2_interrupt_device.h"
#include <iostream>
#include <vector>

namespace {
namespace E = PrStage2SceneEntry;
namespace F = PrStage2FrameTask;
namespace W = PrStage2FrameWait;
void Check(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
template<class Fn> void Reject(Fn&& fn) {
    bool rejected = false;
    try { fn(); } catch (const std::exception&) { rejected = true; }
    Check(rejected, "Expected rejection");
}
struct Missing : std::runtime_error {
    uint32_t address;
    explicit Missing(uint32_t value) : std::runtime_error("Unbound Loading device"), address(value) {}
};
struct IrqReturn {};
struct Session final : PrStage2LoadingSession::Services, W::Progress {
    PrStage2DiscFileDevice::Device disc;
    PrStage2VramDevice::Device vram;
    PrStage2InterruptDevice::Controller irq;
    PrPsxGteDirect::MatrixRegisters gte{};
    F::Task* task = nullptr;
    bool boot = false;
    uint32_t timer = 0, ticks = 0;
    std::vector<uint32_t> external;
    Session(const char* scus, const char* overlay, const char* bin)
        : Services(std::filesystem::u8path(scus), std::filesystem::u8path(overlay)),
          disc(std::filesystem::u8path(bin)) {}
    void Await(F::WaitKind kind, uint32_t target) override {
        Check(task != nullptr, "No retained frame task");
        task->Suspend(kind, 0x80035560u, target);
    }
    int32_t CallLoadingDevice(uint32_t fn, E::Arguments args) override {
        external.push_back(fn);
        const std::vector<uint32_t> a(args);
        // Only the previously source-verified cold BIOS prerequisites are
        // isolated here. No frame, graphics, Loading or audio success input.
        if (boot) {
            if (fn == 0x80047F5Cu && a == std::vector<uint32_t>{0x80055FB0u}) return 0;
            if (fn == 0x80048A30u && a == std::vector<uint32_t>{0x80055FB0u}) return -73;
            if (fn == 0x80048AE0u && a == std::vector<uint32_t>{0}) return -77;
            if (fn == 0x80048AF0u && a == std::vector<uint32_t>{3, 0}) return -11;
            if (fn == 0x80048960u && a.empty()) return -99;
            if (fn == 0x80048A50u && a.empty()) return -17;
            throw Missing(fn);
        }
        if (fn == 0x80035560u && a.size() == 1u)
            return W::VSync80035560(*this, *this, static_cast<int32_t>(a[0]));
        if (fn == 0x800381F8u && a.size() == 2u) return disc.Lookup(*this, a[0], a[1]);
        if (fn == 0x80038FC0u && a.size() == 3u) return disc.StartRead(*this, a[0], a[1], a[2]);
        if (fn == 0x800390C8u && a == std::vector<uint32_t>{1, 0}) return disc.PollRead();
        if (fn == 0x80048A10u && a.empty()) throw IrqReturn{};
        throw Missing(fn);
    }
    void CallLoadingDeviceVoid(uint32_t fn, E::Arguments) override { throw Missing(fn); }
    uint8_t ReadDevice8(uint32_t a) override { throw Missing(a); }
    uint16_t ReadDevice16(uint32_t a) override {
        uint32_t value;
        if (irq.TryRead(a, 2, value)) return static_cast<uint16_t>(value);
        throw Missing(a);
    }
    uint32_t ReadDevice32(uint32_t a) override {
        uint32_t value;
        if (irq.TryRead(a, 4, value)) return value;
        if (a == 0x1F801814u) return 0x04000000u; // Explicit test GPU status.
        if (a == 0x1F801110u) return timer;
        throw Missing(a);
    }
    void WriteDevice8(uint32_t a, uint8_t) override { throw Missing(a); }
    void WriteDevice16(uint32_t a, uint16_t value) override {
        if (!irq.TryWrite(a, 2, value)) throw Missing(a);
    }
    void WriteDevice32(uint32_t a, uint32_t value) override {
        if (irq.TryWrite(a, 4, value)) return;
        if (boot && a == 0x1F801114u) return;
        throw Missing(a);
    }
    PrStage2LifecycleDirect::Words64 Call64(uint32_t fn, E::Arguments) override { throw Missing(fn); }
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32) override {
        throw Missing(0x8003A3DCu);
    }
    int32_t SetCdLocation800367A4(uint32_t location) override { return disc.SetLocation(location); }
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect rect, uint32_t source) override {
        return vram.UploadImageWords(*this, rect, source);
    }
    PrPsxGteDirect::MatrixRegisters& MatrixGte() override { return gte; }
    [[noreturn]] void Exit(uint32_t fn, E::Arguments) override { throw Missing(fn); }
    [[noreturn]] void Break(uint32_t fn, uint32_t) override { throw Missing(fn); }
    void BootFixture() {
        boot = true;
        try { Check(uint32_t(Call(0x80035744u, {})) == 0x80055F78u, "Cold callback prerequisite"); }
        catch (...) { boot = false; throw; }
        boot = false;
        external.clear();
    }
    void Tick() {
        ++ticks;
        timer += 263u; // Explicit deterministic input, not a product wall clock.
        irq.SetLine(PrStage2InterruptDevice::Source::VBlank, true);
        irq.SetLine(PrStage2InterruptDevice::Source::VBlank, false);
        if (!irq.Pending()) return;
        bool returned = false;
        try { Call(0x800359B8u, {}); } catch (const IrqReturn&) { returned = true; }
        Check(returned, "IRQ did not return through its original boundary");
    }
};
void CheckSession(const char* scus, const char* overlay, const char* bin) {
    Session s(scus, overlay, bin);
    s.BootFixture();
    s.InitializeGlobals();
    const uint32_t originalBuffer = s.Read16(0x80096590u);
    const uint32_t packet = s.Read32(0x8006ED50u + 4u * originalBuffer);
    uint32_t starts = 0;
    F::Task task([&] { ++starts; return s.InitializeScene(); });
    s.task = &task;
    Check(task.Start() == F::State::Waiting, "Initializer did not retain its first wait");
    for (uint32_t i = 0; i < 4u; ++i)
        Check(task.Resume(task.Pending().ticket) == F::State::Waiting, "Eventless resume skipped source wait");
    Reject([&] { s.InitializeScene(); });
    Reject([&] { s.RunScene(); });
    Check(starts == 1u && s.disc.LookupRequests() == 7u && s.disc.BytesTransferred() == 8192u,
          "S2 initializer restarted or read past unbound Loading");
    Check(task.Resume(task.Pending().ticket, [&] { s.Tick(); }) == F::State::Failed,
          "Unbound graphics device falsely completed");
    uint32_t missing = 0;
    try { task.RethrowFailure(); } catch (const Missing& error) { missing = error.address; }
    std::cout << "loading-session-next-boundary " << std::hex << missing << std::dec << '\n';
    Check(missing == 0x1F8010E8u, "Unexpected native Loading dependency; inspect current evidence");
    Check(s.Read32(0x8006EDA8u) == originalBuffer && s.Read32(0x8006ED58u) == 1u &&
          s.Read32(0x800901C8u) == packet, "Native prepare did not publish its source-owned work");
    Check(s.Read32(0x8006ECD4u) == 0u && s.GetPhase() == E::Phase::Failed &&
          s.disc.ReadRequests() == 1u && s.disc.BytesTransferred() == 8192u,
          "Failed lower device was acknowledged as a frame or resource load");
    for (uint32_t fn : s.external)
        Check(fn != 0x8001EA74u && fn != 0x80040CC8u && fn != 0x8001EF40u &&
              fn != 0x8001EBF4u && fn != 0x80040420u && fn != 0x80040CA4u && fn != 0x80026ECCu,
              "Native Loading call escaped to a host receipt");
    const uint32_t before = s.Read32(0x8006ED58u);
    Reject([&] { s.Call(0x8001EA74u, {1}); });
    Check(s.Read32(0x8006ED58u) == before, "Wrong arity mutated Loading state");
    std::cout << "loading-session-retained 1-start 7-lookups 8192-bytes no-loading-receipts\n";
}
void CheckAudio(const char* scus, const char* overlay, const char* bin) {
    Session s(scus, overlay, bin);
    uint32_t missing = 0;
    try { (void)s.Call(0x80026ECCu, {}); }
    catch (const Missing& error) { missing = error.address; }
    std::cout << "loading-audio-next-boundary " << std::hex << missing << std::dec << '\n';
    Check(missing >= 0x1F801C00u && missing < 0x1F801E00u,
          "Audio did not reach its actual SPU device boundary");
    Check(s.external.empty(), "Audio flush replaced by a host receipt");
}
}
int main(int argc, char** argv) {
    try {
        if (argc != 4) return 1;
        CheckSession(argv[1], argv[2], argv[3]);
        CheckAudio(argv[1], argv[2], argv[3]);
        std::cout << "loading-session-pass initializer-not-complete\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 2;
    }
}