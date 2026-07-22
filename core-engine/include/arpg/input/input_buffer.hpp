#pragma once

#include "arpg/runtime/fixed_step.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace arpg::input {

enum class Key : std::uint8_t {
    unknown = 0U,
    escape,
    space,
    a,
    b,
    c,
    d,
    e,
    f,
    g,
    h,
    i,
    j,
    k,
    l,
    m,
    n,
    o,
    p,
    q,
    r,
    s,
    t,
    u,
    v,
    w,
    x,
    y,
    z,
    count,
};

enum class MouseButton : std::uint8_t {
    left = 0U,
    right,
    middle,
    button_4,
    button_5,
    button_6,
    button_7,
    button_8,
    count,
};

enum class InputTransitionType : std::uint8_t {
    key_pressed,
    key_released,
    mouse_pressed,
    mouse_released,
    scroll,
};

struct InputTransition {
    std::uint64_t sequence{0U};
    InputTransitionType type{InputTransitionType::key_pressed};
    Key key{Key::unknown};
    MouseButton mouse_button{MouseButton::left};
    double horizontal_scroll{0.0};
    double vertical_scroll{0.0};
};

struct GamepadState {
    std::array<bool, 15U> buttons{};
    std::array<float, 6U> axes{};
    bool connected{false};
};

inline constexpr std::size_t maximum_input_transitions = 4096U;
inline constexpr std::size_t maximum_gamepads = 16U;

struct InputSnapshot {
    runtime::TickIndex tick_index{0U};
    std::span<const InputTransition> transitions{};
    std::array<bool, static_cast<std::size_t>(Key::count)> held_keys{};
    std::array<bool, static_cast<std::size_t>(MouseButton::count)> held_mouse_buttons{};
    std::array<GamepadState, maximum_gamepads> gamepads{};
    double cursor_x{0.0};
    double cursor_y{0.0};
};

class InputBuffer {
public:
    [[nodiscard]] auto push_key(Key key, bool pressed, bool repeated = false) noexcept -> bool;
    [[nodiscard]] auto push_mouse_button(MouseButton button, bool pressed) noexcept -> bool;
    [[nodiscard]] auto push_scroll(double horizontal, double vertical) noexcept -> bool;
    void set_cursor_position(double x, double y) noexcept;
    void set_gamepad_state(std::size_t index, const GamepadState& state) noexcept;
    void clear_held_state() noexcept;
    void discard_transitions() noexcept;

    [[nodiscard]] auto snapshot(runtime::TickIndex tick_index) const noexcept -> InputSnapshot;
    [[nodiscard]] auto overflowed() const noexcept -> bool;

private:
    [[nodiscard]] auto append(InputTransition transition) noexcept -> bool;

    std::array<InputTransition, maximum_input_transitions> transitions_{};
    std::array<bool, static_cast<std::size_t>(Key::count)> held_keys_{};
    std::array<bool, static_cast<std::size_t>(MouseButton::count)> held_mouse_buttons_{};
    std::array<GamepadState, maximum_gamepads> gamepads_{};
    std::size_t transition_count_{0U};
    std::uint64_t next_sequence_{0U};
    double cursor_x_{0.0};
    double cursor_y_{0.0};
    bool overflowed_{false};
};

} // namespace arpg::input
