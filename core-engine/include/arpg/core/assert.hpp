#pragma once

#include <source_location>
#include <string_view>

namespace arpg::core {

struct AssertionFailure {
    std::string_view expression;
    std::string_view message;
    std::source_location location;
};

[[noreturn]] void assertion_failed(const AssertionFailure& failure) noexcept;

namespace detail {

[[nodiscard]] constexpr auto release_assertion_type_check(const bool, const std::string_view) noexcept -> int {
    return 0;
}

} // namespace detail

} // namespace arpg::core

#if ARPG_ENABLE_ASSERTIONS
#define ARPG_ASSERT(expression, message)                                                                               \
    do {                                                                                                               \
        if (!(expression)) {                                                                                           \
            ::arpg::core::assertion_failed({#expression, (message), std::source_location::current()});                 \
        }                                                                                                              \
    } while (false)
#else
#define ARPG_ASSERT(expression, message)                                                                               \
    do {                                                                                                               \
        static_cast<void>(sizeof(::arpg::core::detail::release_assertion_type_check(static_cast<bool>(expression),     \
                                                                                    std::string_view{message})));      \
    } while (false)
#endif
