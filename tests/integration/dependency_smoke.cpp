#include <GLFW/glfw3.h>
#include <catch2/catch_test_macros.hpp>
#include <glm/glm.hpp>
#include <imgui.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>

TEST_CASE("approved M0 dependency headers and libraries are compatible", "[integration][dependencies]") {
    int glfw_major = 0;
    int glfw_minor = 0;
    int glfw_revision = 0;
    glfwGetVersion(&glfw_major, &glfw_minor, &glfw_revision);

    CHECK(glfw_major == 3);
    CHECK(glfw_minor >= 4);
    CHECK(glfw_revision >= 0);
    CHECK(GLM_VERSION_MAJOR == 1);
    CHECK(GLM_VERSION_MINOR == 0);
    CHECK(GLM_VERSION_PATCH >= 3);
    CHECK(IMGUI_VERSION_NUM >= 19208);
    CHECK(VK_HEADER_VERSION >= 350);

    const glm::vec3 ground_position{1.0F, 0.0F, 2.0F};
    CHECK(ground_position.y == 0.0F);

    const PFN_vkCreateInstance create_instance = nullptr;
    CHECK(create_instance == nullptr);
}
