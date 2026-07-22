#include "arpg/input/input_buffer.hpp"
#include "arpg/platform/platform.hpp"
#include "arpg/runtime/fixed_step.hpp"
#include "arpg/runtime/runtime_loop.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>

namespace {

bool allocation_tracking = false;
std::uint64_t tracked_allocations = 0U;

class TestClock final : public arpg::runtime::IClock {
public:
    explicit TestClock(const double increment = 1.0 / 60.0) noexcept : increment_(increment) {}

    auto now() noexcept -> arpg::runtime::MonotonicTime override {
        const auto result = now_;
        now_ += arpg::runtime::MonotonicTime{increment_};
        return result;
    }

private:
    arpg::runtime::MonotonicTime now_{0.0};
    double increment_;
};

class AllocationPlatform final : public arpg::platform::IPlatform {
public:
    void poll_events() noexcept override {
        ++poll_count_;
        if (poll_count_ > 1000U) {
            state_.close_requested = true;
        }
    }

    void wait_events() noexcept override {}

    [[nodiscard]] auto state() const noexcept -> arpg::platform::PlatformState override {
        return state_;
    }

    [[nodiscard]] auto input() noexcept -> arpg::input::InputBuffer& override {
        return input_;
    }

    [[nodiscard]] auto failed() const noexcept -> bool override {
        return false;
    }

    [[nodiscard]] auto error_message() const noexcept -> std::string_view override {
        return {};
    }

private:
    arpg::platform::PlatformState state_{.framebuffer_extent = {.width = 1280, .height = 720}};
    arpg::input::InputBuffer input_{};
    std::uint32_t poll_count_{0U};
};

class AllocationClient final : public arpg::runtime::IRuntimeClient {
public:
    auto fixed_update(const arpg::runtime::FixedTickContext&, const arpg::input::InputSnapshot&) noexcept
        -> arpg::runtime::CallbackControl override {
        ++fixed_update_count;
        return arpg::runtime::CallbackControl::continue_running;
    }

    auto render(const arpg::runtime::RenderContext&) noexcept -> arpg::runtime::CallbackControl override {
        ++render_count;
        return arpg::runtime::CallbackControl::continue_running;
    }

    void on_overload(const arpg::runtime::OverloadEvent&) noexcept override {}

    std::uint32_t fixed_update_count{0U};
    std::uint32_t render_count{0U};
};

} // namespace

void* operator new(const std::size_t size) {
    if (allocation_tracking) {
        ++tracked_allocations;
    }
    if (void* const memory = std::malloc(size); memory != nullptr) {
        return memory;
    }
    throw std::bad_alloc{};
}

void* operator new[](const std::size_t size) {
    return ::operator new(size);
}

void operator delete(void* const memory) noexcept {
    std::free(memory);
}

void operator delete[](void* const memory) noexcept {
    std::free(memory);
}

void operator delete(void* const memory, const std::size_t) noexcept {
    std::free(memory);
}

void operator delete[](void* const memory, const std::size_t) noexcept {
    std::free(memory);
}

TEST_CASE("M1 permanently fixes authoritative simulation at 60 Hz", "[unit][m1][timing]") {
    static_assert(arpg::runtime::authoritative_tick_rate_hz == 60U);
    CHECK(arpg::runtime::authoritative_tick_rate_hz == 60U);
    CHECK(arpg::runtime::FixedTickContext::rate_hz() == 60U);

    arpg::runtime::FixedStepScheduler scheduler{};
    std::uint64_t ticks = 0U;
    for (std::uint32_t frame = 0U; frame < 600U; ++frame) {
        const auto schedule = scheduler.schedule(arpg::runtime::fixed_tick_duration);
        REQUIRE(schedule.tick_count == 1U);
        REQUIRE(scheduler.complete_tick());
        ++ticks;
    }
    CHECK(ticks == 600U);
    CHECK(scheduler.next_tick().tick_index == 600U);
}

TEST_CASE("M1 accumulator bounds catch-up and preserves interpolation remainder", "[unit][m1][timing]") {
    arpg::runtime::FixedStepScheduler scheduler{};
    const auto schedule = scheduler.schedule(std::chrono::milliseconds{250});

    CHECK(schedule.frame_delta_clamped == false);
    CHECK(schedule.tick_count == 8U);
    CHECK(schedule.discarded_tick_count == 7U);
    CHECK(schedule.interpolation_alpha == 0.0);

    const auto clamped = scheduler.schedule(std::chrono::seconds{1});
    CHECK(clamped.frame_delta_clamped);
    CHECK(clamped.tick_count == 8U);
    CHECK(clamped.discarded_tick_count == 7U);
    CHECK(clamped.interpolation_alpha >= 0.0);
    CHECK(clamped.interpolation_alpha < 1.0);
}

TEST_CASE("M1 input retains a short press and release until one fixed tick", "[unit][m1][input]") {
    arpg::input::InputBuffer input;
    REQUIRE(input.push_key(arpg::input::Key::a, true));
    REQUIRE(input.push_key(arpg::input::Key::a, false));

    const auto snapshot = input.snapshot(42U);
    REQUIRE(snapshot.transitions.size() == 2U);
    CHECK(snapshot.tick_index == 42U);
    CHECK(snapshot.transitions[0].type == arpg::input::InputTransitionType::key_pressed);
    CHECK(snapshot.transitions[1].type == arpg::input::InputTransitionType::key_released);
    CHECK(snapshot.transitions[0].sequence < snapshot.transitions[1].sequence);
    CHECK(snapshot.held_keys[static_cast<std::size_t>(arpg::input::Key::a)] == false);

    input.discard_transitions();
    CHECK(input.snapshot(43U).transitions.empty());
}

TEST_CASE("M1 fixed-loop infrastructure allocates nothing after initialization", "[unit][m1][allocation]") {
    arpg::runtime::FixedStepScheduler scheduler{};
    arpg::input::InputBuffer input;
    REQUIRE(input.push_key(arpg::input::Key::space, true));
    input.discard_transitions();

    tracked_allocations = 0U;
    allocation_tracking = true;
    for (std::uint32_t frame = 0U; frame < 1000U; ++frame) {
        const auto schedule = scheduler.schedule(arpg::runtime::fixed_tick_duration);
        for (std::uint32_t tick = 0U; tick < schedule.tick_count; ++tick) {
            const auto snapshot = input.snapshot(scheduler.next_tick().tick_index);
            static_cast<void>(snapshot);
            input.discard_transitions();
            REQUIRE(scheduler.complete_tick());
        }
    }
    allocation_tracking = false;
    CHECK(tracked_allocations == 0U);
}

TEST_CASE("M1 runtime loop allocates nothing during steady-state fixed updates", "[unit][m1][allocation]") {
    TestClock clock;
    AllocationPlatform platform;
    AllocationClient client;
    arpg::runtime::RuntimeLoop loop{clock, platform, client};

    tracked_allocations = 0U;
    allocation_tracking = true;
    const auto result = loop.run();
    allocation_tracking = false;

    CHECK(result.reason == arpg::runtime::RunExitReason::normal_close);
    CHECK(client.fixed_update_count == 1000U);
    CHECK(client.render_count == 1000U);
    CHECK(tracked_allocations == 0U);
}

TEST_CASE("M1 input capacity fails explicitly without growing storage", "[unit][m1][input][allocation]") {
    arpg::input::InputBuffer input;
    for (std::size_t index = 0U; index < arpg::input::maximum_input_transitions; ++index) {
        REQUIRE(input.push_scroll(0.0, 1.0));
    }
    CHECK(input.push_scroll(0.0, 1.0) == false);
    CHECK(input.overflowed());
}
