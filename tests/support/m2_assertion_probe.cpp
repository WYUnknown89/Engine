#include "arpg/core/assert.hpp"

auto main() -> int {
    ARPG_ASSERT(false, "M2 assertion probe");
    return 0;
}
