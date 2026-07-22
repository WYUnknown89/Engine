#include "arpg/diagnostics/timing_metrics.hpp"
#include "arpg/platform/platform.hpp"
#include "arpg/runtime/runtime_loop.hpp"

#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <limits>

namespace {

class StepClock final : public arpg::time::IMonotonicClock {
  public:
    explicit StepClock(const arpg::time::MonotonicTime step) noexcept : step_(step) {}

    [[nodiscard]] auto now() noexcept -> arpg::time::MonotonicTime override {
        const auto result = current_;
        current_ += step_;
        return result;
    }

  private:
    arpg::time::MonotonicTime current_{0.0};
    arpg::time::MonotonicTime step_;
};

class MetricsPlatform final : public arpg::platform::IPlatform {
  public:
    void poll_events() noexcept override {}
    void wait_events() noexcept override {}

    [[nodiscard]] auto state() const noexcept -> arpg::platform::PlatformState override {
        return {.framebuffer_extent = {.width = 1, .height = 1}};
    }

    [[nodiscard]] auto input() noexcept -> arpg::input::InputBuffer& override { return input_; }
    [[nodiscard]] auto failed() const noexcept -> bool override { return false; }
    [[nodiscard]] auto error_message() const noexcept -> std::string_view override { return {}; }

  private:
    arpg::input::InputBuffer input_{};
};

class MetricsClient final : public arpg::runtime::IRuntimeClient {
  public:
    auto fixed_update(const arpg::runtime::FixedTickContext&,
                      const arpg::input::InputSnapshot&) noexcept -> arpg::runtime::CallbackControl override {
        ++fixed_calls;
        return arpg::runtime::CallbackControl::continue_running;
    }

    auto render(const arpg::runtime::RenderContext&) noexcept -> arpg::runtime::CallbackControl override {
        ++render_calls;
        return arpg::runtime::CallbackControl::request_stop;
    }

    void on_overload(const arpg::runtime::OverloadEvent&) noexcept override {}

    std::uint32_t fixed_calls{0U};
    std::uint32_t render_calls{0U};
};

} // namespace

TEST_CASE("M2 timing metrics aggregate and reset deterministically", "[unit][m2][metrics]") {
    arpg::diagnostics::TimingAccumulator metrics;
    CHECK(metrics.record(std::chrono::nanoseconds{10}));
    CHECK(metrics.record(std::chrono::nanoseconds{30}));
    CHECK(metrics.record(std::chrono::nanoseconds{20}));

    const auto summary = metrics.summary();
    CHECK(summary.sample_count == 3U);
    CHECK(summary.current == std::chrono::nanoseconds{20});
    CHECK(summary.minimum == std::chrono::nanoseconds{10});
    CHECK(summary.maximum == std::chrono::nanoseconds{30});
    CHECK(summary.total == std::chrono::nanoseconds{60});
    CHECK(summary.average == std::chrono::nanoseconds{20});

    CHECK(metrics.record(std::chrono::nanoseconds{-1}) == false);
    CHECK(metrics.summary().rejected_sample_count == 1U);
    metrics.reset();
    CHECK(metrics.summary().sample_count == 0U);
}

TEST_CASE("M2 timing metrics saturate total duration", "[unit][m2][metrics]") {
    arpg::diagnostics::TimingAccumulator metrics;
    REQUIRE(metrics.record(std::chrono::nanoseconds{std::numeric_limits<std::int64_t>::max()}));
    REQUIRE(metrics.record(std::chrono::nanoseconds{1}));
    CHECK(metrics.summary().total_overflowed);
    CHECK(metrics.summary().total.count() == std::numeric_limits<std::int64_t>::max());
}

TEST_CASE("M2 optional runtime diagnostics record timing without changing scheduling", "[unit][m2][metrics]") {
    StepClock scheduling_clock{arpg::runtime::fixed_tick_duration};
    StepClock diagnostic_clock{arpg::time::MonotonicTime{0.000001}};
    MetricsPlatform platform;
    MetricsClient client;
    arpg::diagnostics::TimingAccumulator frame_times;
    arpg::diagnostics::TimingAccumulator fixed_tick_times;

    arpg::runtime::RuntimeLoop loop{
        scheduling_clock,
        platform,
        client,
        {},
        {.clock = &diagnostic_clock, .frame_times = &frame_times, .fixed_tick_times = &fixed_tick_times},
    };
    const auto result = loop.run();

    CHECK(result.reason == arpg::runtime::RunExitReason::requested_stop);
    CHECK(result.completed_ticks == 1U);
    CHECK(result.rendered_frames == 1U);
    CHECK(client.fixed_calls == 1U);
    CHECK(client.render_calls == 1U);
    CHECK(frame_times.summary().sample_count == 1U);
    CHECK(fixed_tick_times.summary().sample_count == 1U);
}
