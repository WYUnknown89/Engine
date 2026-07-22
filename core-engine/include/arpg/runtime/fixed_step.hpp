#pragma once

#include <chrono>
#include <cstdint>

namespace arpg::runtime {

using TickIndex = std::uint64_t;

inline constexpr std::uint32_t authoritative_tick_rate_hz = 60U;
inline constexpr auto fixed_tick_duration = std::chrono::duration<double>{1.0 / 60.0};

struct FixedTickContext {
    TickIndex tick_index;

    [[nodiscard]] static constexpr auto rate_hz() noexcept -> std::uint32_t {
        return authoritative_tick_rate_hz;
    }
};

struct FixedStepConfig {
    std::uint32_t maximum_catch_up_ticks{8U};
    std::chrono::milliseconds maximum_frame_delta{250};
};

struct FrameSchedule {
    std::uint32_t tick_count{0U};
    std::uint64_t discarded_tick_count{0U};
    bool frame_delta_clamped{false};
    bool clock_regressed{false};
    double interpolation_alpha{0.0};
};

class FixedStepScheduler {
public:
    explicit FixedStepScheduler(FixedStepConfig config = {});

    [[nodiscard]] auto schedule(std::chrono::duration<double> elapsed) noexcept -> FrameSchedule;
    [[nodiscard]] auto next_tick() const noexcept -> FixedTickContext;
    [[nodiscard]] auto complete_tick() noexcept -> bool;
    void reset_timing() noexcept;

private:
    FixedStepConfig config_;
    TickIndex next_tick_index_{0U};
    double accumulator_seconds_{0.0};
};

} // namespace arpg::runtime
