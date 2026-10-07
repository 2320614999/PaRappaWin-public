#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>

class D3D11Renderer;
struct PrGameContext;

namespace PrStage2ProductRuntime {

// Product-owned S2 session. S2_NATIVE_DATA.BIN and COMOD2 provide assets for
// the translated routines. The Windows program owns the loop and devices;
// the original PSX executable is not a runtime dependency.
class Runtime final {
public:
    Runtime(const std::filesystem::path& dataRoot, D3D11Renderer& renderer,
            PrGameContext& context,
            std::function<uint32_t()> debugPadSource = {});
    ~Runtime();

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    // Returns the original RunScene successor while the retained source is
    // running. A value of 2 means that the S2 scene remains active.
    int Tick();
    // Advance a retained device wait from a host render step.
    void Pump();
    bool OwnsPresentation() const noexcept;
    bool Running() const noexcept;
    int16_t ExitReason() const;
    bool BeginResidentDirectory(PrGameContext& ctx, int previousScene);
    void RethrowFailure() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace PrStage2ProductRuntime
