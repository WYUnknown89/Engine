#pragma once

#include <cstddef>
#include <limits>

namespace arpg::memory::detail {

[[nodiscard]] constexpr auto is_power_of_two(const std::size_t value) noexcept -> bool {
    return value != 0U && (value & (value - 1U)) == 0U;
}

[[nodiscard]] constexpr auto checked_add(const std::size_t left, const std::size_t right,
                                         std::size_t& result) noexcept -> bool {
    if (right > std::numeric_limits<std::size_t>::max() - left) {
        return false;
    }
    result = left + right;
    return true;
}

[[nodiscard]] constexpr auto checked_multiply(const std::size_t left, const std::size_t right,
                                              std::size_t& result) noexcept -> bool {
    if (left != 0U && right > std::numeric_limits<std::size_t>::max() / left) {
        return false;
    }
    result = left * right;
    return true;
}

[[nodiscard]] constexpr auto checked_align_up(const std::size_t value, const std::size_t alignment,
                                              std::size_t& result) noexcept -> bool {
    if (!is_power_of_two(alignment)) {
        return false;
    }
    const auto padding = alignment - 1U;
    if (value > std::numeric_limits<std::size_t>::max() - padding) {
        return false;
    }
    result = (value + padding) & ~padding;
    return true;
}

template <typename T> constexpr void saturating_increment(T& value) noexcept {
    if (value != std::numeric_limits<T>::max()) {
        ++value;
    }
}

} // namespace arpg::memory::detail
