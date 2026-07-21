#include "arpg/core/build_info.hpp"

namespace arpg::core {

auto build_info() noexcept -> BuildInfo {
    return {
        .project_name = "ARPG Engine",
        .project_version = "0.1.0",
        .specification_version = "1.1",
        .pointer_size = sizeof(void*),
        .cpp_standard = __cplusplus,
    };
}

} // namespace arpg::core
