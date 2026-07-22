#pragma once

#include "arpg/memory/allocation.hpp"

#include <cstddef>

namespace arpg::memory {

class LinearArena {
  public:
    explicit LinearArena(std::size_t capacity_bytes, std::size_t maximum_alignment = alignof(std::max_align_t));
    ~LinearArena();

    LinearArena(const LinearArena&) = delete;
    auto operator=(const LinearArena&) -> LinearArena& = delete;
    LinearArena(LinearArena&& other) noexcept;
    auto operator=(LinearArena&& other) noexcept -> LinearArena&;

    [[nodiscard]] auto try_allocate(std::size_t size,
                                    std::size_t alignment = alignof(std::max_align_t)) noexcept -> AllocationResult;
    void reset() noexcept;

    [[nodiscard]] auto has_backing() const noexcept -> bool;
    [[nodiscard]] auto statistics() const noexcept -> ArenaStatistics;

  private:
    void release_backing() noexcept;
    void move_from(LinearArena& other) noexcept;
    void record_failure() noexcept;

    std::byte* backing_{nullptr};
    std::size_t capacity_bytes_{0U};
    std::size_t maximum_alignment_{0U};
    std::size_t used_bytes_{0U};
    ArenaStatistics statistics_{};
};

} // namespace arpg::memory
