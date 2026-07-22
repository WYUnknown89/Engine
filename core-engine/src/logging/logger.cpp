#include "arpg/logging/logger.hpp"

#include <chrono>
#include <limits>

namespace arpg::logging {

Logger::Logger(time::IMonotonicClock& clock, const LogSeverity minimum_severity) noexcept
    : clock_(clock), start_time_(clock.now()), minimum_severity_(minimum_severity) {}

auto Logger::add_sink(ILogSink& sink) noexcept -> bool {
    for (std::size_t index = 0U; index < sink_count_; ++index) {
        if (sinks_[index] == &sink) {
            return false;
        }
    }
    if (sink_count_ == sinks_.size()) {
        return false;
    }
    sinks_[sink_count_] = &sink;
    ++sink_count_;
    return true;
}

auto Logger::write(const LogSeverity severity, const std::string_view category, const std::string_view message,
                   const std::source_location location) noexcept -> LogDispatchResult {
    if (dispatch_in_progress_) {
        increment(statistics_.reentrant_rejections);
        return {.status = LogDispatchStatus::reentrant_call_rejected};
    }
    increment(statistics_.attempted_records);
    if (static_cast<unsigned int>(severity) < static_cast<unsigned int>(minimum_severity_)) {
        increment(statistics_.filtered_records);
        return {.status = LogDispatchStatus::filtered};
    }
    if (sink_count_ == 0U) {
        return {.status = LogDispatchStatus::no_sinks};
    }

    dispatch_in_progress_ = true;
    const auto elapsed = clock_.now() - start_time_;
    const auto timestamp = elapsed.count() < 0.0 ? std::chrono::nanoseconds{}
                                                 : std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed);
    const LogRecord record{
        .timestamp = timestamp,
        .sequence = next_sequence_,
        .severity = severity,
        .category = category,
        .message = message,
        .location = location,
    };
    if (next_sequence_ != std::numeric_limits<std::uint64_t>::max()) {
        ++next_sequence_;
    }

    LogDispatchResult result{};
    for (std::size_t index = 0U; index < sink_count_; ++index) {
        if (sinks_[index]->write(record) == SinkWriteResult::written) {
            ++result.delivered_sink_count;
        } else {
            ++result.failed_sink_count;
            increment(statistics_.sink_write_failures);
        }
    }
    dispatch_in_progress_ = false;

    if (result.failed_sink_count == 0U) {
        result.status = LogDispatchStatus::delivered;
        increment(statistics_.delivered_records);
    } else if (result.delivered_sink_count == 0U) {
        result.status = LogDispatchStatus::total_failure;
    } else {
        result.status = LogDispatchStatus::partial_failure;
        increment(statistics_.delivered_records);
    }
    return result;
}

auto Logger::statistics() const noexcept -> LoggerStatistics { return statistics_; }

void Logger::increment(std::uint64_t& value) noexcept {
    if (value != std::numeric_limits<std::uint64_t>::max()) {
        ++value;
    }
}

auto to_string(const LogSeverity severity) noexcept -> std::string_view {
    switch (severity) {
    case LogSeverity::trace:
        return "trace";
    case LogSeverity::debug:
        return "debug";
    case LogSeverity::info:
        return "info";
    case LogSeverity::warning:
        return "warning";
    case LogSeverity::error:
        return "error";
    case LogSeverity::fatal:
        return "fatal";
    }
    return "unknown";
}

} // namespace arpg::logging
