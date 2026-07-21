#pragma once

#include <string_view>

namespace arpg::tools {

struct ModuleInfo {
    std::string_view name;
    std::string_view required_specification_version;
};

[[nodiscard]] auto module_info() noexcept -> ModuleInfo;

} // namespace arpg::tools
