#pragma once

#include <cstdint>

namespace arpg::ecs {

class Entity {
  public:
    constexpr Entity() noexcept = default;

    [[nodiscard]] static constexpr auto from_parts(const std::uint32_t index,
                                                   const std::uint32_t generation) noexcept -> Entity {
        return Entity{index, generation};
    }

    [[nodiscard]] constexpr auto index() const noexcept -> std::uint32_t { return index_; }
    [[nodiscard]] constexpr auto generation() const noexcept -> std::uint32_t { return generation_; }
    [[nodiscard]] constexpr auto has_valid_generation() const noexcept -> bool { return generation_ != 0U; }

    friend constexpr auto operator==(const Entity&, const Entity&) noexcept -> bool = default;

  private:
    constexpr Entity(const std::uint32_t index, const std::uint32_t generation) noexcept
        : index_(index), generation_(generation) {}

    std::uint32_t index_{0U};
    std::uint32_t generation_{0U};
};

static_assert(sizeof(Entity) == sizeof(std::uint64_t));

} // namespace arpg::ecs
