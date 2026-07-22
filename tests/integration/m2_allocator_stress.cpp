#include "arpg/memory/fixed_block_pool.hpp"
#include "arpg/memory/linear_arena.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <cstdint>

TEST_CASE("M2 allocator stress preserves bounded fixed storage", "[integration][m2][memory][stress]") {
    constexpr std::size_t allocation_count = 200000U;
    arpg::memory::FixedBlockPool pool{64U, 64U, 1U};

    for (std::size_t index = 0U; index < allocation_count; ++index) {
        const auto allocation = pool.try_allocate();
        REQUIRE(allocation.succeeded());
        REQUIRE(pool.release(allocation.address) == arpg::memory::PoolReleaseResult::released);
    }
    CHECK(pool.statistics().successful_allocations == allocation_count);
    CHECK(pool.statistics().live_blocks == 0U);

    arpg::memory::LinearArena arena{4096U, 64U};
    constexpr std::size_t allocations_per_reset = 64U;
    for (std::size_t index = 0U; index < allocation_count; ++index) {
        REQUIRE(arena.try_allocate(64U, 64U).succeeded());
        if ((index + 1U) % allocations_per_reset == 0U) {
            arena.reset();
        }
    }
    CHECK(arena.statistics().reset_count == allocation_count / allocations_per_reset);
}
