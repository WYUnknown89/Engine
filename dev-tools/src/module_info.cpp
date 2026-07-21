#include "arpg/tools/module_info.hpp"

#include "arpg/core/build_info.hpp"

namespace arpg::tools {

auto module_info() noexcept -> ModuleInfo {
    const auto engine = core::build_info();
    return {
        .name = "arpg-tools",
        .required_specification_version = engine.specification_version,
    };
}

} // namespace arpg::tools
