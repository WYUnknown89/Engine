#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>

namespace arpg::ecs {

class ComponentTypeId {
  public:
    constexpr ComponentTypeId() noexcept = default;

    friend constexpr auto operator==(const ComponentTypeId&, const ComponentTypeId&) noexcept -> bool = default;

  private:
    explicit constexpr ComponentTypeId(const void* const token) noexcept : token_(token) {}

    const void* token_{nullptr};

    template <typename T> friend auto component_type_id() noexcept -> ComponentTypeId;
    friend class Registry;
};

namespace detail {

template <typename T> struct ComponentTypeToken {
    inline static const std::byte value{};
};

} // namespace detail

template <typename T> [[nodiscard]] auto component_type_id() noexcept -> ComponentTypeId {
    using CanonicalType = std::remove_cv_t<T>;
    return ComponentTypeId{std::addressof(detail::ComponentTypeToken<CanonicalType>::value)};
}

} // namespace arpg::ecs
