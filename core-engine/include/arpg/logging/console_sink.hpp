#pragma once

#include "arpg/logging/logger.hpp"

#include <cstdio>

namespace arpg::logging {

class ConsoleSink final : public ILogSink {
  public:
    ConsoleSink(std::FILE* standard_output = stdout, std::FILE* standard_error = stderr) noexcept;

    [[nodiscard]] auto write(const LogRecord& record) noexcept -> SinkWriteResult override;

  private:
    std::FILE* standard_output_;
    std::FILE* standard_error_;
};

} // namespace arpg::logging
