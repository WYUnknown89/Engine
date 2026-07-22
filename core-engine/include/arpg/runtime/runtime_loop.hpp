#pragma once

#include "arpg/input/input_buffer.hpp"
#include "arpg/platform/platform.hpp"
#include "arpg/runtime/fixed_step.hpp"

#include <chrono>
#include <cstdint>
#include <string_view>

namespace arpg::runtime {

using MonotonicTime = std::chrono::duration<double>;

class IClock {
  public:
    virtual ~IClock() = default;

    [[nodiscard]] virtual auto now() noexcept -> MonotonicTime = 0;
};

class SteadyClock final : public IClock {
  public:
    [[nodiscard]] auto now() noexcept -> MonotonicTime override;
};

enum class CallbackControl : std::uint8_t {
    continue_running,
    request_stop,
    failure,
};

struct RenderContext {
    std::uint64_t frame_index{0U};
    double interpolation_alpha{0.0};
};

struct OverloadEvent {
    std::uint64_t discarded_tick_count{0U};
    bool frame_delta_clamped{false};
};

class IRuntimeClient {
  public:
    virtual ~IRuntimeClient() = default;

    virtual auto fixed_update(const FixedTickContext& context,
                              const input::InputSnapshot& input) noexcept -> CallbackControl = 0;
    virtual auto render(const RenderContext& context) noexcept -> CallbackControl = 0;
    virtual void on_overload(const OverloadEvent& event) noexcept = 0;
};

enum class RunExitReason : std::uint8_t {
    normal_close,
    requested_stop,
    platform_failure,
    input_overflow,
    clock_regression,
    tick_index_exhausted,
    client_failure,
};

struct RunResult {
    RunExitReason reason{RunExitReason::normal_close};
    std::uint64_t completed_ticks{0U};
    std::uint64_t rendered_frames{0U};
    std::uint64_t discarded_ticks{0U};
    std::string_view error_message{};
};

class RuntimeLoop {
  public:
    RuntimeLoop(IClock& clock, platform::IPlatform& platform, IRuntimeClient& client, FixedStepConfig config = {});
    ~RuntimeLoop();

    RuntimeLoop(const RuntimeLoop&) = delete;
    auto operator=(const RuntimeLoop&) -> RuntimeLoop& = delete;

    [[nodiscard]] auto run() noexcept -> RunResult;
    void shutdown() noexcept;

  private:
    IClock& clock_;
    platform::IPlatform& platform_;
    IRuntimeClient& client_;
    FixedStepScheduler scheduler_;
    bool quiescent_{false};
};

} // namespace arpg::runtime
