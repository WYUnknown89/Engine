#include "arpg/core/assert.hpp"

#include <cstdio>
#include <cstdlib>

namespace arpg::core {

[[noreturn]] void assertion_failed(const AssertionFailure& failure) noexcept {
    std::fprintf(stderr, "Assertion failed: %.*s\nMessage: %.*s\nLocation: %s:%u (%s)\n",
                 static_cast<int>(failure.expression.size()), failure.expression.data(),
                 static_cast<int>(failure.message.size()), failure.message.data(), failure.location.file_name(),
                 failure.location.line(), failure.location.function_name());
    std::abort();
}

} // namespace arpg::core
