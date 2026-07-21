#pragma once

#include <cstddef>
#include <string_view>

namespace arpg::core {

struct BuildInfo {
    std::string_view project_name;
    std::string_view project_version;
    std::string_view specification_version;
    std::size_t pointer_size;
    long cpp_standard;
};

[[nodiscard]] auto build_info() noexcept -> BuildInfo;

} // namespace arpg::core
