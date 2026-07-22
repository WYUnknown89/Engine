#include "arpg/platform/detail/glfw_error_latch.hpp"

#include <algorithm>

namespace arpg::platform::detail {

void GlfwErrorLatch::record(const std::string_view message) noexcept {
    const auto copied_length = std::min(message.size(), message_.size() - 1U);
    std::copy_n(message.data(), copied_length, message_.data());
    message_[copied_length] = '\0';
    length_ = copied_length;
    failed_ = true;
}

auto GlfwErrorLatch::failed() const noexcept -> bool { return failed_; }

auto GlfwErrorLatch::message() const noexcept -> std::string_view { return {message_.data(), length_}; }

} // namespace arpg::platform::detail
