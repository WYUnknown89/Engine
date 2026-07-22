#include "arpg/runtime/runtime_loop.hpp"

#include <chrono>

namespace arpg::runtime {

auto SteadyClock::now() noexcept -> MonotonicTime {
    static_assert(std::chrono::steady_clock::is_steady);
    return std::chrono::duration_cast<MonotonicTime>(std::chrono::steady_clock::now().time_since_epoch());
}

RuntimeLoop::RuntimeLoop(IClock& clock, platform::IPlatform& platform, IRuntimeClient& client,
                         const FixedStepConfig config)
    : clock_(clock), platform_(platform), client_(client), scheduler_(config) {}

RuntimeLoop::~RuntimeLoop() { shutdown(); }

auto RuntimeLoop::run() noexcept -> RunResult {
    if (quiescent_) {
        return {.reason = RunExitReason::requested_stop};
    }

    auto previous_time = clock_.now();
    std::uint64_t frame_index = 0U;
    RunResult result{};

    while (true) {
        platform_.poll_events();
        if (platform_.failed()) {
            result.reason = RunExitReason::platform_failure;
            result.error_message = platform_.error_message();
            break;
        }

        auto state = platform_.state();
        if (state.close_requested) {
            result.reason = RunExitReason::normal_close;
            break;
        }

        if (state.suspended()) {
            scheduler_.reset_timing();
            platform_.input().discard_transitions();
            do {
                platform_.wait_events();
                if (platform_.failed()) {
                    result.reason = RunExitReason::platform_failure;
                    result.error_message = platform_.error_message();
                    break;
                }
                state = platform_.state();
                platform_.input().discard_transitions();
            } while (!state.close_requested && state.suspended());

            if (result.reason == RunExitReason::platform_failure) {
                break;
            }
            if (state.close_requested) {
                result.reason = RunExitReason::normal_close;
                break;
            }
            scheduler_.reset_timing();
            previous_time = clock_.now();
            continue;
        }

        const auto current_time = clock_.now();
        const auto schedule = scheduler_.schedule(current_time - previous_time);
        previous_time = current_time;
        if (schedule.clock_regressed) {
            result.reason = RunExitReason::clock_regression;
            break;
        }
        if (platform_.input().overflowed()) {
            result.reason = RunExitReason::input_overflow;
            result.error_message = "Input transition capacity exhausted";
            break;
        }
        if (schedule.discarded_tick_count > 0U || schedule.frame_delta_clamped) {
            result.discarded_ticks += schedule.discarded_tick_count;
            client_.on_overload({
                .discarded_tick_count = schedule.discarded_tick_count,
                .frame_delta_clamped = schedule.frame_delta_clamped,
            });
        }

        for (std::uint32_t tick = 0U; tick < schedule.tick_count; ++tick) {
            const auto tick_context = scheduler_.next_tick();
            const auto snapshot = platform_.input().snapshot(tick_context.tick_index);
            const auto control = client_.fixed_update(tick_context, snapshot);
            platform_.input().discard_transitions();
            if (control == CallbackControl::failure) {
                result.reason = RunExitReason::client_failure;
                goto finish;
            }
            if (!scheduler_.complete_tick()) {
                result.reason = RunExitReason::tick_index_exhausted;
                goto finish;
            }
            ++result.completed_ticks;
            if (control == CallbackControl::request_stop) {
                result.reason = RunExitReason::requested_stop;
                goto finish;
            }
        }

        if (client_.render({.frame_index = frame_index, .interpolation_alpha = schedule.interpolation_alpha}) ==
            CallbackControl::failure) {
            result.reason = RunExitReason::client_failure;
            break;
        }
        ++result.rendered_frames;
        ++frame_index;
    }

finish:
    shutdown();
    return result;
}

void RuntimeLoop::shutdown() noexcept { quiescent_ = true; }

} // namespace arpg::runtime
