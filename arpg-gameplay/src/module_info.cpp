#include "arpg/gameplay/module_info.hpp"

#include "arpg/core/build_info.hpp"

namespace arpg::gameplay {

auto module_info() noexcept -> ModuleInfo {
    const auto engine = core::build_info();
    return {
        .name = "arpg-gameplay",
        .required_specification_version = engine.specification_version,
    };
}

} // namespace arpg::gameplay
