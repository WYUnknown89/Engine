#include "arpg/logging/logger.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <utility>

static_assert(noexcept(std::declval<arpg::logging::Logger&>().write(arpg::logging::LogSeverity::info, "test", "msg")));
static_assert(
    noexcept(std::declval<arpg::logging::ILogSink&>().write(std::declval<const arpg::logging::LogRecord&>())));

namespace {

class TestClock final : public arpg::time::IMonotonicClock {
  public:
    [[nodiscard]] auto now() noexcept -> arpg::time::MonotonicTime override {
        const auto value = current_;
        current_ += arpg::time::MonotonicTime{1.0};
        return value;
    }

  private:
    arpg::time::MonotonicTime current_{0.0};
};

class CaptureSink final : public arpg::logging::ILogSink {
  public:
    [[nodiscard]] auto
    write(const arpg::logging::LogRecord& record) noexcept -> arpg::logging::SinkWriteResult override {
        ++calls;
        last = record;
        return result;
    }

    arpg::logging::LogRecord last{};
    std::uint32_t calls{0U};
    arpg::logging::SinkWriteResult result{arpg::logging::SinkWriteResult::written};
};

class ReentrantSink final : public arpg::logging::ILogSink {
  public:
    explicit ReentrantSink(arpg::logging::Logger& logger) noexcept : logger_(logger) {}

    [[nodiscard]] auto write(const arpg::logging::LogRecord&) noexcept -> arpg::logging::SinkWriteResult override {
        nested_result = logger_.write(arpg::logging::LogSeverity::error, "nested", "nested");
        return arpg::logging::SinkWriteResult::written;
    }

    arpg::logging::LogDispatchResult nested_result{};

  private:
    arpg::logging::Logger& logger_;
};

} // namespace

TEST_CASE("M2 Logger filters records and captures structured contents", "[unit][m2][logging]") {
    TestClock clock;
    arpg::logging::Logger logger{clock, arpg::logging::LogSeverity::info};
    CaptureSink sink;
    REQUIRE(logger.add_sink(sink));
    CHECK(logger.add_sink(sink) == false);

    CHECK(logger.write(arpg::logging::LogSeverity::debug, "test", "filtered").status ==
          arpg::logging::LogDispatchStatus::filtered);
    const auto delivered = logger.write(arpg::logging::LogSeverity::info, "test", "message");
    CHECK(delivered.status == arpg::logging::LogDispatchStatus::delivered);
    CHECK(sink.calls == 1U);
    CHECK(sink.last.category == "test");
    CHECK(sink.last.message == "message");
    CHECK(sink.last.sequence == 0U);
    CHECK(sink.last.timestamp.count() > 0);
    CHECK(logger.statistics().filtered_records == 1U);
}

TEST_CASE("M2 Logger reports sink failures without suppressing healthy sinks", "[unit][m2][logging]") {
    TestClock clock;
    arpg::logging::Logger logger{clock};
    CaptureSink failing;
    failing.result = arpg::logging::SinkWriteResult::output_failure;
    CaptureSink healthy;
    REQUIRE(logger.add_sink(failing));
    REQUIRE(logger.add_sink(healthy));

    const auto result = logger.write(arpg::logging::LogSeverity::fatal, "test", "long message remains a view");
    CHECK(result.status == arpg::logging::LogDispatchStatus::partial_failure);
    CHECK(result.delivered_sink_count == 1U);
    CHECK(result.failed_sink_count == 1U);
    CHECK(healthy.calls == 1U);
    CHECK(logger.statistics().sink_write_failures == 1U);

    failing.result = arpg::logging::SinkWriteResult::written;
    CHECK(logger.write(arpg::logging::LogSeverity::fatal, "test", "still running").status ==
          arpg::logging::LogDispatchStatus::delivered);
}

TEST_CASE("M2 Logger rejects reentrant dispatch and remains usable", "[unit][m2][logging]") {
    TestClock clock;
    arpg::logging::Logger logger{clock};
    ReentrantSink sink{logger};
    REQUIRE(logger.add_sink(sink));

    CHECK(logger.write(arpg::logging::LogSeverity::error, "outer", "outer").status ==
          arpg::logging::LogDispatchStatus::delivered);
    CHECK(sink.nested_result.status == arpg::logging::LogDispatchStatus::reentrant_call_rejected);
    CHECK(logger.statistics().reentrant_rejections == 1U);
    CHECK(logger.write(arpg::logging::LogSeverity::info, "outer", "later").status ==
          arpg::logging::LogDispatchStatus::delivered);
}
