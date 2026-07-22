#include "arpg/logging/console_sink.hpp"

#include <cinttypes>

namespace arpg::logging {

ConsoleSink::ConsoleSink(std::FILE* const standard_output, std::FILE* const standard_error) noexcept
    : standard_output_(standard_output), standard_error_(standard_error) {}

auto ConsoleSink::write(const LogRecord& record) noexcept -> SinkWriteResult {
    std::FILE* const stream =
        static_cast<unsigned int>(record.severity) >= static_cast<unsigned int>(LogSeverity::warning)
            ? standard_error_
            : standard_output_;
    if (stream == nullptr) {
        return SinkWriteResult::output_failure;
    }
    const auto prefix = std::fprintf(stream, "[+%" PRId64 "ns][%.*s][%.*s][%s:%u] ",
                                     static_cast<std::int64_t>(record.timestamp.count()),
                                     static_cast<int>(to_string(record.severity).size()),
                                     to_string(record.severity).data(), static_cast<int>(record.category.size()),
                                     record.category.data(), record.location.file_name(), record.location.line());
    const auto body = std::fwrite(record.message.data(), 1U, record.message.size(), stream);
    const auto suffix = std::fputc('\n', stream);
    if (prefix < 0 || body != record.message.size() || suffix == EOF || std::ferror(stream) != 0) {
        return SinkWriteResult::output_failure;
    }
    return SinkWriteResult::written;
}

} // namespace arpg::logging
