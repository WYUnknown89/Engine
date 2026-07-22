#include "arpg/platform/platform.hpp"
#include "arpg/runtime/runtime_loop.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <string_view>

namespace {

class ScriptedClock final : public arpg::runtime::IClock {
  public:
    auto now() noexcept -> arpg::runtime::MonotonicTime override {
        const auto result = now_;
        now_ += arpg::runtime::fixed_tick_duration;
        return result;
    }

  private:
    arpg::runtime::MonotonicTime now_{0.0};
};

class FakePlatform final : public arpg::platform::IPlatform {
  public:
    void poll_events() noexcept override {
        ++poll_count;
        if (minimize_on_first_poll && poll_count == 1U) {
            state_.iconified = true;
        }
    }

    void wait_events() noexcept override {
        ++wait_count;
        state_.iconified = false;
        state_.framebuffer_extent = {.width = 1280, .height = 720};
    }

    [[nodiscard]] auto state() const noexcept -> arpg::platform::PlatformState override { return state_; }

    [[nodiscard]] auto input() noexcept -> arpg::input::InputBuffer& override { return input_; }

    [[nodiscard]] auto failed() const noexcept -> bool override { return fail_after_poll && poll_count >= 1U; }

    [[nodiscard]] auto error_message() const noexcept -> std::string_view override { return {}; }

    arpg::platform::PlatformState state_{.framebuffer_extent = {.width = 1280, .height = 720}};
    arpg::input::InputBuffer input_{};
    std::uint32_t poll_count{0U};
    std::uint32_t wait_count{0U};
    bool minimize_on_first_poll{false};
    bool fail_after_poll{false};
};

class FakeClient final : public arpg::runtime::IRuntimeClient {
  public:
    auto fixed_update(const arpg::runtime::FixedTickContext&,
                      const arpg::input::InputSnapshot& input) noexcept -> arpg::runtime::CallbackControl override {
        transition_counts[fixed_calls] = input.transitions.size();
        ++fixed_calls;
        if (fail_on_first_tick) {
            return arpg::runtime::CallbackControl::failure;
        }
        return fixed_calls >= stop_after_ticks ? arpg::runtime::CallbackControl::request_stop
                                               : arpg::runtime::CallbackControl::continue_running;
    }

    auto render(const arpg::runtime::RenderContext&) noexcept -> arpg::runtime::CallbackControl override {
        ++render_calls;
        return arpg::runtime::CallbackControl::continue_running;
    }

    void on_overload(const arpg::runtime::OverloadEvent&) noexcept override { ++overload_calls; }

    std::array<std::size_t, 8U> transition_counts{};
    std::uint32_t fixed_calls{0U};
    std::uint32_t render_calls{0U};
    std::uint32_t overload_calls{0U};
    std::uint32_t stop_after_ticks{1U};
    bool fail_on_first_tick{false};
};

} // namespace

TEST_CASE("M1 runtime consumes edges once across catch-up ticks", "[integration][m1][runtime]") {
    ScriptedClock clock;
    FakePlatform platform;
    FakeClient client;
    client.stop_after_ticks = 3U;
    REQUIRE(platform.input().push_key(arpg::input::Key::a, true));

    arpg::runtime::RuntimeLoop loop{clock, platform, client};
    const auto result = loop.run();

    CHECK(result.reason == arpg::runtime::RunExitReason::requested_stop);
    CHECK(client.fixed_calls == 3U);
    CHECK(client.transition_counts[0] == 1U);
    CHECK(client.transition_counts[1] == 0U);
    CHECK(client.transition_counts[2] == 0U);
}

TEST_CASE("M1 minimized runtime waits and resets scheduling before restore", "[integration][m1][runtime]") {
    ScriptedClock clock;
    FakePlatform platform;
    platform.minimize_on_first_poll = true;
    FakeClient client;

    arpg::runtime::RuntimeLoop loop{clock, platform, client};
    const auto result = loop.run();

    CHECK(result.reason == arpg::runtime::RunExitReason::requested_stop);
    CHECK(platform.wait_count == 1U);
    CHECK(client.fixed_calls == 1U);
    CHECK(client.render_calls == 0U);
}

TEST_CASE("M1 runtime quiesces after a callback failure", "[integration][m1][lifetime]") {
    ScriptedClock clock;
    FakePlatform platform;
    FakeClient client;
    client.fail_on_first_tick = true;

    arpg::runtime::RuntimeLoop loop{clock, platform, client};
    const auto first = loop.run();
    const auto fixed_calls_after_failure = client.fixed_calls;
    const auto second = loop.run();

    CHECK(first.reason == arpg::runtime::RunExitReason::client_failure);
    CHECK(second.reason == arpg::runtime::RunExitReason::requested_stop);
    CHECK(client.fixed_calls == fixed_calls_after_failure);
}

TEST_CASE("M1 platform failure prevents client callbacks", "[integration][m1][lifetime]") {
    ScriptedClock clock;
    FakePlatform platform;
    platform.fail_after_poll = true;
    FakeClient client;

    arpg::runtime::RuntimeLoop loop{clock, platform, client};
    const auto result = loop.run();

    CHECK(result.reason == arpg::runtime::RunExitReason::platform_failure);
    CHECK(client.fixed_calls == 0U);
    CHECK(client.render_calls == 0U);
}
