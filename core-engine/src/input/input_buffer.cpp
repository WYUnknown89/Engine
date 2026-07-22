#include "arpg/input/input_buffer.hpp"

namespace arpg::input {

auto InputBuffer::push_key(const Key key, const bool pressed, const bool repeated) noexcept -> bool {
    const auto index = static_cast<std::size_t>(key);
    if (index >= held_keys_.size()) {
        return false;
    }
    held_keys_[index] = pressed;
    if (repeated) {
        return true;
    }
    return append({
        .type = pressed ? InputTransitionType::key_pressed : InputTransitionType::key_released,
        .key = key,
    });
}

auto InputBuffer::push_mouse_button(const MouseButton button, const bool pressed) noexcept -> bool {
    const auto index = static_cast<std::size_t>(button);
    if (index >= held_mouse_buttons_.size()) {
        return false;
    }
    held_mouse_buttons_[index] = pressed;
    return append({
        .type = pressed ? InputTransitionType::mouse_pressed : InputTransitionType::mouse_released,
        .mouse_button = button,
    });
}

auto InputBuffer::push_scroll(const double horizontal, const double vertical) noexcept -> bool {
    return append({
        .type = InputTransitionType::scroll,
        .horizontal_scroll = horizontal,
        .vertical_scroll = vertical,
    });
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): Cartesian cursor coordinates are always x then y.
void InputBuffer::set_cursor_position(const double x, const double y) noexcept {
    cursor_x_ = x;
    cursor_y_ = y;
}

void InputBuffer::set_gamepad_state(const std::size_t index, const GamepadState& state) noexcept {
    if (index < gamepads_.size()) {
        gamepads_[index] = state;
    }
}

void InputBuffer::clear_held_state() noexcept {
    held_keys_.fill(false);
    held_mouse_buttons_.fill(false);
}

void InputBuffer::discard_transitions() noexcept { transition_count_ = 0U; }

auto InputBuffer::snapshot(const runtime::TickIndex tick_index) const noexcept -> InputSnapshot {
    return {
        .tick_index = tick_index,
        .transitions = std::span<const InputTransition>{transitions_.data(), transition_count_},
        .held_keys = held_keys_,
        .held_mouse_buttons = held_mouse_buttons_,
        .gamepads = gamepads_,
        .cursor_x = cursor_x_,
        .cursor_y = cursor_y_,
    };
}

auto InputBuffer::overflowed() const noexcept -> bool { return overflowed_; }

auto InputBuffer::append(InputTransition transition) noexcept -> bool {
    if (transition_count_ == transitions_.size()) {
        overflowed_ = true;
        return false;
    }
    transition.sequence = next_sequence_;
    ++next_sequence_;
    transitions_[transition_count_] = transition;
    ++transition_count_;
    return true;
}

} // namespace arpg::input
