#pragma once

#include <cstddef>
#include <cstdint>

namespace arpg::memory {

enum class AllocationError : std::uint8_t {
    none,
    zero_size,
    invalid_alignment,
    size_overflow,
    exhausted,
    allocator_unavailable,
};

struct AllocationResult {
    void* address{nullptr};
    AllocationError error{AllocationError::none};

    [[nodiscard]] auto succeeded() const noexcept -> bool {
        return address != nullptr && error == AllocationError::none;
    }
};

struct ArenaStatistics {
    std::size_t capacity_bytes{0U};
    std::size_t used_bytes{0U};
    std::size_t peak_used_bytes{0U};
    std::uint64_t successful_allocations{0U};
    std::uint64_t failed_allocations{0U};
    std::uint64_t reset_count{0U};
};

enum class PoolReleaseResult : std::uint8_t {
    released,
    allocator_unavailable,
    null_pointer,
    foreign_pointer,
    interior_pointer,
    double_free,
};

struct PoolStatistics {
    std::size_t block_size{0U};
    std::size_t block_alignment{0U};
    std::size_t block_capacity{0U};
    std::size_t live_blocks{0U};
    std::size_t peak_live_blocks{0U};
    std::uint64_t successful_allocations{0U};
    std::uint64_t failed_allocations{0U};
    std::uint64_t successful_releases{0U};
    std::uint64_t invalid_releases{0U};
    std::uint64_t reset_count{0U};
};

} // namespace arpg::memory
