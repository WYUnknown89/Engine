#pragma once

#include "arpg/memory/allocation.hpp"

#include <cstddef>
#include <vector>

namespace arpg::memory {

struct FixedBlockPoolConfig {
    std::size_t block_size_bytes{0U};
    std::size_t block_alignment{0U};
    std::size_t block_count{0U};
};

class FixedBlockPool {
  public:
    explicit FixedBlockPool(FixedBlockPoolConfig config);
    ~FixedBlockPool();

    FixedBlockPool(const FixedBlockPool&) = delete;
    auto operator=(const FixedBlockPool&) -> FixedBlockPool& = delete;
    FixedBlockPool(FixedBlockPool&& other) noexcept;
    auto operator=(FixedBlockPool&& other) noexcept -> FixedBlockPool&;

    [[nodiscard]] auto try_allocate() noexcept -> AllocationResult;
    [[nodiscard]] auto release(void* address) noexcept -> PoolReleaseResult;
    void reset() noexcept;

    [[nodiscard]] auto has_backing() const noexcept -> bool;
    [[nodiscard]] auto statistics() const noexcept -> PoolStatistics;

  private:
    struct Slot {
        std::size_t next{0U};
        bool allocated{false};
    };

    static constexpr std::size_t no_slot = static_cast<std::size_t>(-1);

    void release_backing() noexcept;
    void move_from(FixedBlockPool& other) noexcept;
    void initialize_free_list() noexcept;
    void record_invalid_release() noexcept;
    void record_failed_allocation() noexcept;

    std::byte* backing_{nullptr};
    std::vector<Slot> slots_{};
    std::size_t backing_bytes_{0U};
    std::size_t block_size_{0U};
    std::size_t block_alignment_{0U};
    std::size_t stride_{0U};
    std::size_t block_count_{0U};
    std::size_t free_head_{no_slot};
    PoolStatistics statistics_{};
};

} // namespace arpg::memory
