#include "arpg/platform/desktop_platform.hpp"

#include <GLFW/glfw3.h>
#include <array>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>

namespace arpg::platform {
namespace {

[[nodiscard]] auto translate_key(const int key) noexcept -> input::Key {
    if (key == GLFW_KEY_ESCAPE) {
        return input::Key::escape;
    }
    if (key == GLFW_KEY_SPACE) {
        return input::Key::space;
    }
    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
        return static_cast<input::Key>(static_cast<int>(input::Key::a) + key - GLFW_KEY_A);
    }
    return input::Key::unknown;
}

[[nodiscard]] auto translate_mouse_button(const int button) noexcept -> input::MouseButton {
    if (button >= GLFW_MOUSE_BUTTON_1 && button <= GLFW_MOUSE_BUTTON_8) {
        return static_cast<input::MouseButton>(button - GLFW_MOUSE_BUTTON_1);
    }
    return input::MouseButton::left;
}

void glfw_error_callback(const int error_code, const char* const description) noexcept {
    std::fprintf(stderr, "GLFW error %d: %s\n", error_code, description == nullptr ? "unknown" : description);
}

class GlfwPlatform final : public IPlatform {
  public:
    GlfwPlatform() = default;
    ~GlfwPlatform() override {
        if (window_ != nullptr) {
            glfwSetWindowUserPointer(window_, nullptr);
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
        if (initialized_) {
            glfwTerminate();
            initialized_ = false;
        }
    }

    GlfwPlatform(const GlfwPlatform&) = delete;
    auto operator=(const GlfwPlatform&) -> GlfwPlatform& = delete;

    [[nodiscard]] auto initialize() -> std::string {
        glfwSetErrorCallback(glfw_error_callback);
        if (glfwInit() != GLFW_TRUE) {
            return "glfwInit failed";
        }
        initialized_ = true;
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window_ = glfwCreateWindow(1280, 720, "ARPG Engine", nullptr, nullptr);
        if (window_ == nullptr) {
            return "glfwCreateWindow failed";
        }

        glfwSetWindowUserPointer(window_, this);
        glfwSetKeyCallback(window_, key_callback);
        glfwSetMouseButtonCallback(window_, mouse_button_callback);
        glfwSetCursorPosCallback(window_, cursor_position_callback);
        glfwSetScrollCallback(window_, scroll_callback);
        glfwSetWindowFocusCallback(window_, focus_callback);
        glfwSetWindowIconifyCallback(window_, iconify_callback);
        glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);
        glfwSetWindowCloseCallback(window_, close_callback);
        glfwGetFramebufferSize(window_, &state_.framebuffer_extent.width, &state_.framebuffer_extent.height);
        glfwGetCursorPos(window_, &cursor_x_, &cursor_y_);
        input_.set_cursor_position(cursor_x_, cursor_y_);
        glfwShowWindow(window_);
        return {};
    }

    void poll_events() noexcept override {
        glfwPollEvents();
        sample_gamepads();
        update_input_failure();
    }

    void wait_events() noexcept override {
        glfwWaitEvents();
        sample_gamepads();
        update_input_failure();
    }

    [[nodiscard]] auto state() const noexcept -> PlatformState override { return state_; }

    [[nodiscard]] auto input() noexcept -> input::InputBuffer& override { return input_; }

    [[nodiscard]] auto failed() const noexcept -> bool override { return failed_; }

    [[nodiscard]] auto error_message() const noexcept -> std::string_view override { return error_message_; }

  private:
    [[nodiscard]] static auto from(GLFWwindow* const window) noexcept -> GlfwPlatform* {
        return static_cast<GlfwPlatform*>(glfwGetWindowUserPointer(window));
    }

    static void key_callback(GLFWwindow* const window, const int key, int, const int action, int) noexcept {
        auto* const platform = from(window);
        if (platform == nullptr) {
            return;
        }
        const auto translated_key = translate_key(key);
        if (translated_key == input::Key::unknown) {
            return;
        }
        const auto repeated = action == GLFW_REPEAT;
        const auto pressed = action != GLFW_RELEASE;
        if (!platform->input_.push_key(translated_key, pressed, repeated)) {
            platform->set_failure("Input transition capacity exhausted");
        }
    }

    static void mouse_button_callback(GLFWwindow* const window, const int button, const int action, int) noexcept {
        auto* const platform = from(window);
        if (platform == nullptr || (button < GLFW_MOUSE_BUTTON_1 || button > GLFW_MOUSE_BUTTON_8)) {
            return;
        }
        if (!platform->input_.push_mouse_button(translate_mouse_button(button), action == GLFW_PRESS)) {
            platform->set_failure("Input transition capacity exhausted");
        }
    }

    static void cursor_position_callback(GLFWwindow* const window, const double x, const double y) noexcept {
        auto* const platform = from(window);
        if (platform != nullptr) {
            platform->input_.set_cursor_position(x, y);
        }
    }

    static void scroll_callback(GLFWwindow* const window, const double x, const double y) noexcept {
        auto* const platform = from(window);
        if (platform != nullptr && !platform->input_.push_scroll(x, y)) {
            platform->set_failure("Input transition capacity exhausted");
        }
    }

    static void focus_callback(GLFWwindow* const window, const int focused) noexcept {
        auto* const platform = from(window);
        if (platform != nullptr) {
            platform->state_.focused = focused == GLFW_TRUE;
            if (!platform->state_.focused) {
                platform->input_.clear_held_state();
            }
        }
    }

    static void iconify_callback(GLFWwindow* const window, const int iconified) noexcept {
        auto* const platform = from(window);
        if (platform != nullptr) {
            platform->state_.iconified = iconified == GLFW_TRUE;
        }
    }

    static void framebuffer_size_callback(GLFWwindow* const window, const int width, const int height) noexcept {
        auto* const platform = from(window);
        if (platform != nullptr) {
            platform->state_.framebuffer_extent = {.width = width, .height = height};
        }
    }

    static void close_callback(GLFWwindow* const window) noexcept {
        auto* const platform = from(window);
        if (platform != nullptr) {
            platform->state_.close_requested = true;
        }
    }

    void sample_gamepads() noexcept {
        for (int joystick = GLFW_JOYSTICK_1; joystick <= GLFW_JOYSTICK_LAST; ++joystick) {
            input::GamepadState gamepad{};
            gamepad.connected = glfwJoystickIsGamepad(joystick) == GLFW_TRUE;
            if (gamepad.connected) {
                GLFWgamepadstate glfw_state{};
                if (glfwGetGamepadState(joystick, &glfw_state) == GLFW_TRUE) {
                    for (std::size_t button = 0U; button < gamepad.buttons.size(); ++button) {
                        gamepad.buttons[button] = glfw_state.buttons[button] == GLFW_PRESS;
                    }
                    for (std::size_t axis = 0U; axis < gamepad.axes.size(); ++axis) {
                        gamepad.axes[axis] = glfw_state.axes[axis];
                    }
                }
            }
            input_.set_gamepad_state(static_cast<std::size_t>(joystick - GLFW_JOYSTICK_1), gamepad);
        }
    }

    void update_input_failure() noexcept {
        if (input_.overflowed()) {
            set_failure("Input transition capacity exhausted");
        }
    }

    void set_failure(const char* const message) noexcept {
        failed_ = true;
        error_message_ = message;
    }

    bool initialized_{false};
    GLFWwindow* window_{nullptr};
    PlatformState state_{};
    input::InputBuffer input_{};
    double cursor_x_{0.0};
    double cursor_y_{0.0};
    bool failed_{false};
    std::string_view error_message_{};
};

} // namespace

auto create_desktop_platform() -> DesktopPlatformCreateResult {
    auto platform = std::make_unique<GlfwPlatform>();
    auto error = platform->initialize();
    if (!error.empty()) {
        return {.error = std::move(error)};
    }
    return {.platform = std::move(platform)};
}

} // namespace arpg::platform
