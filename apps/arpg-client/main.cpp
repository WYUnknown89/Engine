#include "arpg/platform/desktop_platform.hpp"
#include "arpg/runtime/runtime_loop.hpp"

#include <charconv>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <string_view>

namespace {

class Client final : public arpg::runtime::IRuntimeClient {
public:
    explicit Client(const std::uint64_t smoke_tick_limit) noexcept : smoke_tick_limit_(smoke_tick_limit) {}

    auto fixed_update(
        const arpg::runtime::FixedTickContext& context, const arpg::input::InputSnapshot& input) noexcept
        -> arpg::runtime::CallbackControl override {
        for (const auto& transition : input.transitions) {
            if (transition.type == arpg::input::InputTransitionType::key_pressed &&
                transition.key == arpg::input::Key::escape) {
                return arpg::runtime::CallbackControl::request_stop;
            }
        }
        if (smoke_tick_limit_ > 0U && context.tick_index + 1U >= smoke_tick_limit_) {
            return arpg::runtime::CallbackControl::request_stop;
        }
        return arpg::runtime::CallbackControl::continue_running;
    }

    auto render(const arpg::runtime::RenderContext&) noexcept -> arpg::runtime::CallbackControl override {
        return arpg::runtime::CallbackControl::continue_running;
    }

    void on_overload(const arpg::runtime::OverloadEvent& event) noexcept override {
        std::fprintf(
            stderr,
            "Runtime overload: discarded=%llu clamped=%s\n",
            static_cast<unsigned long long>(event.discarded_tick_count),
            event.frame_delta_clamped ? "true" : "false");
    }

private:
    std::uint64_t smoke_tick_limit_{0U};
};

[[nodiscard]] auto parse_smoke_tick_limit(const int argument_count, char** const arguments, std::uint64_t& limit)
    -> bool {
    if (argument_count == 1) {
        return true;
    }
    if (argument_count != 2) {
        return false;
    }
    constexpr std::string_view prefix{"--smoke-ticks="};
    const std::string_view argument{arguments[1]};
    if (!argument.starts_with(prefix)) {
        return false;
    }
    const auto value = argument.substr(prefix.size());
    const auto [position, error] = std::from_chars(value.data(), value.data() + value.size(), limit);
    return error == std::errc{} && position == value.data() + value.size() && limit > 0U;
}

} // namespace

auto main(const int argument_count, char** const arguments) -> int {
    std::uint64_t smoke_tick_limit = 0U;
    if (!parse_smoke_tick_limit(argument_count, arguments, smoke_tick_limit)) {
        std::fprintf(stderr, "Usage: arpg_client [--smoke-ticks=N]\n");
        return 2;
    }

    try {
        arpg::runtime::SteadyClock clock;
        auto platform_result = arpg::platform::create_desktop_platform();
        if (!platform_result.succeeded()) {
            std::fprintf(stderr, "Desktop platform initialization failed: %s\n", platform_result.error.c_str());
            return 1;
        }
        auto platform = std::move(platform_result.platform);
        Client client{smoke_tick_limit};
        arpg::runtime::RunResult result{};
        {
            arpg::runtime::RuntimeLoop loop{clock, *platform, client};
            result = loop.run();
        }
        std::fprintf(
            stderr,
            "Runtime exit: reason=%u ticks=%llu frames=%llu discarded=%llu\n",
            static_cast<unsigned int>(result.reason),
            static_cast<unsigned long long>(result.completed_ticks),
            static_cast<unsigned long long>(result.rendered_frames),
            static_cast<unsigned long long>(result.discarded_ticks));
        return result.reason == arpg::runtime::RunExitReason::client_failure ||
                       result.reason == arpg::runtime::RunExitReason::platform_failure ||
                       result.reason == arpg::runtime::RunExitReason::input_overflow ||
                       result.reason == arpg::runtime::RunExitReason::clock_regression ||
                       result.reason == arpg::runtime::RunExitReason::tick_index_exhausted
                   ? 1
                   : 0;
    } catch (const std::exception& exception) {
        std::fprintf(stderr, "Unhandled runtime exception: %s\n", exception.what());
        return 1;
    }
}
