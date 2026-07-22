#pragma once

#include "arpg/time/monotonic_clock.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <source_location>
#include <string_view>

namespace arpg::logging {

enum class LogSeverity : std::uint8_t { trace, debug, info, warning, error, fatal };

enum class SinkWriteResult : std::uint8_t { written, output_failure };

enum class LogDispatchStatus : std::uint8_t {
    filtered,
    delivered,
    partial_failure,
    total_failure,
    no_sinks,
    reentrant_call_rejected,
};

struct LogRecord {
    std::chrono::nanoseconds timestamp{};
    std::uint64_t sequence{0U};
    LogSeverity severity{LogSeverity::info};
    std::string_view category{};
    std::string_view message{};
    std::source_location location{};
};

struct LogDispatchResult {
    LogDispatchStatus status{LogDispatchStatus::no_sinks};
    std::size_t delivered_sink_count{0U};
    std::size_t failed_sink_count{0U};
};

struct LoggerStatistics {
    std::uint64_t attempted_records{0U};
    std::uint64_t filtered_records{0U};
    std::uint64_t delivered_records{0U};
    std::uint64_t sink_write_failures{0U};
    std::uint64_t reentrant_rejections{0U};
};

class ILogSink {
  public:
    virtual ~ILogSink() = default;
    [[nodiscard]] virtual auto write(const LogRecord& record) noexcept -> SinkWriteResult = 0;
};

class Logger {
  public:
    static constexpr std::size_t maximum_sinks = 8U;

    explicit Logger(time::IMonotonicClock& clock, LogSeverity minimum_severity = LogSeverity::info) noexcept;

    [[nodiscard]] auto add_sink(ILogSink& sink) noexcept -> bool;
    [[nodiscard]] auto
    write(LogSeverity severity, std::string_view category, std::string_view message,
          std::source_location location = std::source_location::current()) noexcept -> LogDispatchResult;

    [[nodiscard]] auto statistics() const noexcept -> LoggerStatistics;

  private:
    void increment(std::uint64_t& value) noexcept;

    time::IMonotonicClock& clock_;
    time::MonotonicTime start_time_{};
    std::array<ILogSink*, maximum_sinks> sinks_{};
    std::size_t sink_count_{0U};
    std::uint64_t next_sequence_{0U};
    LogSeverity minimum_severity_{LogSeverity::info};
    LoggerStatistics statistics_{};
    bool dispatch_in_progress_{false};
};

[[nodiscard]] auto to_string(LogSeverity severity) noexcept -> std::string_view;

} // namespace arpg::logging
