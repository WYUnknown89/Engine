#include "arpg/runtime/fixed_step.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace arpg::runtime {

FixedStepScheduler::FixedStepScheduler(const FixedStepConfig config) : config_(config) {
    if (config_.maximum_catch_up_ticks == 0U) {
        config_.maximum_catch_up_ticks = 1U;
    }
}

auto FixedStepScheduler::schedule(const std::chrono::duration<double> elapsed) noexcept -> FrameSchedule {
    FrameSchedule schedule{};
    if (elapsed.count() < 0.0) {
        schedule.clock_regressed = true;
        return schedule;
    }

    const auto maximum_delta = std::chrono::duration<double>{config_.maximum_frame_delta}.count();
    const auto accepted_delta = std::min(elapsed.count(), maximum_delta);
    schedule.frame_delta_clamped = accepted_delta != elapsed.count();
    accumulator_seconds_ += accepted_delta;

    const auto available_ticks =
        static_cast<std::uint64_t>(std::floor(accumulator_seconds_ / fixed_tick_duration.count()));
    const auto allowed_ticks = static_cast<std::uint64_t>(config_.maximum_catch_up_ticks);
    const auto executed_ticks = std::min(available_ticks, allowed_ticks);
    schedule.tick_count = static_cast<std::uint32_t>(executed_ticks);
    schedule.discarded_tick_count = available_ticks - executed_ticks;

    const auto accounted_ticks = available_ticks;
    accumulator_seconds_ -= static_cast<double>(accounted_ticks) * fixed_tick_duration.count();
    accumulator_seconds_ = std::clamp(accumulator_seconds_, 0.0, fixed_tick_duration.count());
    schedule.interpolation_alpha = accumulator_seconds_ / fixed_tick_duration.count();
    return schedule;
}

auto FixedStepScheduler::next_tick() const noexcept -> FixedTickContext { return {.tick_index = next_tick_index_}; }

auto FixedStepScheduler::complete_tick() noexcept -> bool {
    if (next_tick_index_ == std::numeric_limits<TickIndex>::max()) {
        return false;
    }
    ++next_tick_index_;
    return true;
}

void FixedStepScheduler::reset_timing() noexcept { accumulator_seconds_ = 0.0; }

} // namespace arpg::runtime
