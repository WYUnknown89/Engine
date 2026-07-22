#include "arpg/diagnostics/timing_metrics.hpp"
#include "arpg/logging/console_sink.hpp"
#include "arpg/logging/logger.hpp"
#include "arpg/platform/desktop_platform.hpp"
#include "arpg/runtime/runtime_loop.hpp"

#include <array>
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <string_view>

namespace {

class Client final : public arpg::runtime::IRuntimeClient {
  public:
    Client(const std::uint64_t smoke_tick_limit, arpg::logging::Logger& logger) noexcept
        : smoke_tick_limit_(smoke_tick_limit), logger_(logger) {}

    auto fixed_update(const arpg::runtime::FixedTickContext& context,
                      const arpg::input::InputSnapshot& input) noexcept -> arpg::runtime::CallbackControl override {
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
        std::array<char, 128U> message{};
        const auto length = std::snprintf(message.data(), message.size(), "discarded=%llu clamped=%s",
                                          static_cast<unsigned long long>(event.discarded_tick_count),
                                          event.frame_delta_clamped ? "true" : "false");
        const auto message_length =
            length > 0 ? (static_cast<std::size_t>(length) < message.size() ? static_cast<std::size_t>(length)
                                                                            : message.size() - 1U)
                       : 0U;
        if (message_length > 0U) {
            static_cast<void>(logger_.write(arpg::logging::LogSeverity::warning, "runtime",
                                            std::string_view{message.data(), message_length}));
        }
    }

  private:
    std::uint64_t smoke_tick_limit_{0U};
    arpg::logging::Logger& logger_;
};

[[nodiscard]] auto parse_smoke_tick_limit(const int argument_count, char** const arguments,
                                          std::uint64_t& limit) -> bool {
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

template <std::size_t Size>
[[nodiscard]] auto bounded_message_length(const int length, const std::array<char, Size>&) noexcept -> std::size_t {
    if (length <= 0) {
        return 0U;
    }
    const auto converted_length = static_cast<std::size_t>(length);
    return converted_length < Size ? converted_length : Size - 1U;
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
        arpg::logging::ConsoleSink console_sink;
        arpg::logging::Logger logger{clock};
        static_cast<void>(logger.add_sink(console_sink));
        arpg::diagnostics::TimingAccumulator frame_times;
        arpg::diagnostics::TimingAccumulator fixed_tick_times;
        auto platform_result = arpg::platform::create_desktop_platform();
        if (!platform_result.succeeded()) {
            static_cast<void>(logger.write(arpg::logging::LogSeverity::error, "platform", platform_result.error));
            return 1;
        }
        auto platform = std::move(platform_result.platform);
        Client client{smoke_tick_limit, logger};
        arpg::runtime::RunResult result{};
        {
            arpg::runtime::RuntimeLoop loop{
                clock,
                *platform,
                client,
                {},
                {.clock = &clock, .frame_times = &frame_times, .fixed_tick_times = &fixed_tick_times},
            };
            result = loop.run();
        }
        const auto frame_summary = frame_times.summary();
        const auto fixed_tick_summary = fixed_tick_times.summary();
        std::array<char, 256U> message{};
        const auto length = std::snprintf(
            message.data(), message.size(),
            "exit=%u ticks=%llu frames=%llu discarded=%llu frame_samples=%llu tick_samples=%llu",
            static_cast<unsigned int>(result.reason), static_cast<unsigned long long>(result.completed_ticks),
            static_cast<unsigned long long>(result.rendered_frames),
            static_cast<unsigned long long>(result.discarded_ticks),
            static_cast<unsigned long long>(frame_summary.sample_count),
            static_cast<unsigned long long>(fixed_tick_summary.sample_count));
        const auto message_length = bounded_message_length(length, message);
        if (message_length > 0U) {
            static_cast<void>(logger.write(arpg::logging::LogSeverity::info, "runtime",
                                           std::string_view{message.data(), message_length}));
        }
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
