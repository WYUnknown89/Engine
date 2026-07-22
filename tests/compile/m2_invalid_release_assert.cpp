#define ARPG_ENABLE_ASSERTIONS 0

#include "../../core-engine/include/arpg/core/assert.hpp"

namespace {

struct NotBoolean {};

} // namespace

auto main() -> int {
    ARPG_ASSERT(NotBoolean{}, "Release assertions must type-check their expressions");
    return 0;
}
