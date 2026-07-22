#include "arpg/platform/detail/glfw_error_latch.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>

TEST_CASE("M1 GLFW error latch propagates a copied runtime error", "[unit][platform][m1]") {
    arpg::platform::detail::GlfwErrorLatch latch;
    latch.record("window system failure");

    CHECK(latch.failed());
    CHECK(latch.message() == "window system failure");
}

TEST_CASE("M1 GLFW error latch bounds callback diagnostic storage", "[unit][platform][m1]") {
    arpg::platform::detail::GlfwErrorLatch latch;
    const std::string long_message(512U, 'x');
    latch.record(long_message);

    CHECK(latch.failed());
    CHECK(latch.message().size() == 255U);
}
