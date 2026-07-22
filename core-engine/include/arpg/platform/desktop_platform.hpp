#pragma once

#include "arpg/platform/platform.hpp"

#include <memory>
#include <string>

namespace arpg::platform {

struct DesktopPlatformCreateResult {
    std::unique_ptr<IPlatform> platform{};
    std::string error{};

    [[nodiscard]] auto succeeded() const noexcept -> bool {
        return platform != nullptr;
    }
};

[[nodiscard]] auto create_desktop_platform() -> DesktopPlatformCreateResult;

} // namespace arpg::platform
