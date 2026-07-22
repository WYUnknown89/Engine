#pragma once

#include <array>
#include <cstddef>
#include <string_view>

namespace arpg::platform::detail {

class GlfwErrorLatch {
  public:
    void record(std::string_view message) noexcept;
    [[nodiscard]] auto failed() const noexcept -> bool;
    [[nodiscard]] auto message() const noexcept -> std::string_view;

  private:
    std::array<char, 256U> message_{};
    std::size_t length_{0U};
    bool failed_{false};
};

} // namespace arpg::platform::detail
