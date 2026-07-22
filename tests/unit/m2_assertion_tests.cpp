#include "arpg/core/assert.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string_view>

namespace {

int expression_evaluations = 0;
int message_evaluations = 0;

[[nodiscard]] auto expression_with_side_effect() noexcept -> bool {
    ++expression_evaluations;
    return true;
}

[[nodiscard]] auto message_with_side_effect() noexcept -> std::string_view {
    ++message_evaluations;
    return "message";
}

struct NotBoolean {};

template <typename T>
concept BooleanCastable = requires(T value) { static_cast<bool>(value); };

static_assert(!BooleanCastable<NotBoolean>);

} // namespace

TEST_CASE("M2 assertion evaluates according to build contract", "[unit][m2][assert]") {
    expression_evaluations = 0;
    message_evaluations = 0;

    ARPG_ASSERT(expression_with_side_effect(), message_with_side_effect());

#if ARPG_ENABLE_ASSERTIONS
    CHECK(expression_evaluations == 1);
    CHECK(message_evaluations == 0);
#else
    CHECK(expression_evaluations == 0);
    CHECK(message_evaluations == 0);
#endif
}
