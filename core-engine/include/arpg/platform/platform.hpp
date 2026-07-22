#pragma once

#include "arpg/input/input_buffer.hpp"

#include <cstdint>
#include <string_view>

namespace arpg::platform {

struct FramebufferExtent {
    std::int32_t width{0};
    std::int32_t height{0};
};

struct PlatformState {
    bool close_requested{false};
    bool focused{true};
    bool iconified{false};
    FramebufferExtent framebuffer_extent{};

    [[nodiscard]] auto suspended() const noexcept -> bool {
        return iconified || framebuffer_extent.width <= 0 || framebuffer_extent.height <= 0;
    }
};

class IPlatform {
  public:
    virtual ~IPlatform() = default;

    virtual void poll_events() noexcept = 0;
    virtual void wait_events() noexcept = 0;
    [[nodiscard]] virtual auto state() const noexcept -> PlatformState = 0;
    [[nodiscard]] virtual auto input() noexcept -> input::InputBuffer& = 0;
    [[nodiscard]] virtual auto failed() const noexcept -> bool = 0;
    [[nodiscard]] virtual auto error_message() const noexcept -> std::string_view = 0;
};

} // namespace arpg::platform
