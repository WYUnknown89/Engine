#include "arpg/memory/fixed_block_pool.hpp"
#include "arpg/memory/linear_arena.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <new>
#include <stdexcept>
#include <utility>

namespace {

bool allocation_tracking = false;
std::uint64_t tracked_allocations = 0U;

[[nodiscard]] auto is_aligned(const void* const address, const std::size_t alignment) noexcept -> bool {
    return reinterpret_cast<std::uintptr_t>(address) % alignment == 0U;
}

template <typename T> void move_assign(T& destination, T& source) { destination = std::move(source); }

} // namespace

void* operator new(const std::size_t size) {
    if (allocation_tracking) {
        ++tracked_allocations;
    }
    if (void* const memory = std::malloc(size); memory != nullptr) {
        return memory;
    }
    throw std::bad_alloc{};
}

void* operator new[](const std::size_t size) { return ::operator new(size); }

void operator delete(void* const memory) noexcept { std::free(memory); }

void operator delete[](void* const memory) noexcept { std::free(memory); }

void operator delete(void* const memory, const std::size_t) noexcept { std::free(memory); }

void operator delete[](void* const memory, const std::size_t) noexcept { std::free(memory); }

TEST_CASE("M2 LinearArena aligns backing-derived allocations and tracks capacity", "[unit][m2][arena]") {
    arpg::memory::LinearArena arena{{.capacity_bytes = 1024U, .maximum_alignment = 64U}};

    for (const std::size_t alignment : {1U, 2U, 4U, 8U, 16U, 32U, 64U}) {
        const auto allocation = arena.try_allocate(1U, alignment);
        REQUIRE(allocation.succeeded());
        CHECK(is_aligned(allocation.address, alignment));
    }

    const auto statistics = arena.statistics();
    CHECK(statistics.capacity_bytes == 1024U);
    CHECK(statistics.successful_allocations == 7U);
    CHECK(statistics.used_bytes > 0U);
}

TEST_CASE("M2 LinearArena handles exact fit, failures, reset, and reuse", "[unit][m2][arena]") {
    arpg::memory::LinearArena arena{{.capacity_bytes = 16U, .maximum_alignment = 16U}};
    REQUIRE(arena.try_allocate(16U, 1U).succeeded());
    CHECK(arena.try_allocate(1U, 1U).error == arpg::memory::AllocationError::exhausted);
    CHECK(arena.try_allocate(0U).error == arpg::memory::AllocationError::zero_size);
    CHECK(arena.try_allocate(1U, 3U).error == arpg::memory::AllocationError::invalid_alignment);
    CHECK(arena.try_allocate(1U, 32U).error == arpg::memory::AllocationError::invalid_alignment);

    arena.reset();
    CHECK(arena.statistics().used_bytes == 0U);
    CHECK(arena.statistics().reset_count == 1U);
    REQUIRE(arena.try_allocate(16U, 1U).succeeded());
}

TEST_CASE("M2 LinearArena rejects overflow-prone requests", "[unit][m2][arena]") {
    arpg::memory::LinearArena arena{{.capacity_bytes = 64U, .maximum_alignment = 64U}};
    REQUIRE(arena.try_allocate(1U, 1U).succeeded());
    CHECK(arena.try_allocate(std::numeric_limits<std::size_t>::max(), 64U).error ==
          arpg::memory::AllocationError::size_overflow);
}

TEST_CASE("M2 LinearArena moved-from state is safe and non-owning", "[unit][m2][arena][lifetime]") {
    arpg::memory::LinearArena source{{.capacity_bytes = 128U, .maximum_alignment = 32U}};
    REQUIRE(source.try_allocate(8U, 8U).succeeded());
    arpg::memory::LinearArena destination{std::move(source)};

    CHECK(destination.has_backing());
    CHECK(source.has_backing() == false); // NOLINT(bugprone-use-after-move): M2 defines moved-from arena state.
    CHECK(source.try_allocate(1U).error ==
          arpg::memory::AllocationError::allocator_unavailable); // NOLINT(bugprone-use-after-move): M2 defines
                                                                 // moved-from arena state.
    source.reset(); // NOLINT(bugprone-use-after-move): M2 defines moved-from arena state.

    arpg::memory::LinearArena replacement{{.capacity_bytes = 64U, .maximum_alignment = 16U}};
    replacement = std::move(destination);
    CHECK(destination.has_backing() == false); // NOLINT(bugprone-use-after-move): M2 defines moved-from arena state.
    CHECK(destination.try_allocate(1U).error ==
          arpg::memory::AllocationError::allocator_unavailable); // NOLINT(bugprone-use-after-move): M2 defines
                                                                 // moved-from arena state.
    REQUIRE(replacement.try_allocate(1U, 1U).succeeded());
    move_assign(replacement, replacement);
    CHECK(replacement.has_backing());
}

TEST_CASE("M2 FixedBlockPool aligns each block and reuses released storage", "[unit][m2][pool]") {
    arpg::memory::FixedBlockPool pool{{.block_size_bytes = 3U, .block_alignment = 32U, .block_count = 4U}};
    std::array<void*, 4U> addresses{};
    for (auto& address : addresses) {
        const auto allocation = pool.try_allocate();
        REQUIRE(allocation.succeeded());
        address = allocation.address;
        CHECK(is_aligned(address, 32U));
    }
    CHECK(pool.try_allocate().error == arpg::memory::AllocationError::exhausted);
    REQUIRE(pool.release(addresses[1]) == arpg::memory::PoolReleaseResult::released);
    const auto reused = pool.try_allocate();
    REQUIRE(reused.succeeded());
    CHECK(reused.address == addresses[1]);
}

TEST_CASE("M2 FixedBlockPool detects invalid release and reset behavior", "[unit][m2][pool]") {
    arpg::memory::FixedBlockPool pool{{.block_size_bytes = 16U, .block_alignment = 16U, .block_count = 2U}};
    const auto allocation = pool.try_allocate();
    REQUIRE(allocation.succeeded());
    CHECK(pool.release(nullptr) == arpg::memory::PoolReleaseResult::null_pointer);
    CHECK(pool.release(static_cast<std::byte*>(allocation.address) + 1) ==
          arpg::memory::PoolReleaseResult::interior_pointer);
    REQUIRE(pool.release(allocation.address) == arpg::memory::PoolReleaseResult::released);
    CHECK(pool.release(allocation.address) == arpg::memory::PoolReleaseResult::double_free);

    pool.reset();
    CHECK(pool.statistics().live_blocks == 0U);
    CHECK(pool.statistics().reset_count == 1U);
}

TEST_CASE("M2 FixedBlockPool validates construction arithmetic", "[unit][m2][pool]") {
    CHECK_THROWS_AS((arpg::memory::FixedBlockPool{{.block_size_bytes = 0U, .block_alignment = 8U, .block_count = 1U}}),
                    std::invalid_argument);
    CHECK_THROWS_AS((arpg::memory::FixedBlockPool{{.block_size_bytes = 1U, .block_alignment = 3U, .block_count = 1U}}),
                    std::invalid_argument);
    CHECK_THROWS_AS(
        (arpg::memory::FixedBlockPool{
            {.block_size_bytes = 64U, .block_alignment = 64U, .block_count = std::numeric_limits<std::size_t>::max()}}),
        std::overflow_error);
}

TEST_CASE("M2 FixedBlockPool moved-from state is safe and non-owning", "[unit][m2][pool][lifetime]") {
    arpg::memory::FixedBlockPool source{{.block_size_bytes = 8U, .block_alignment = 8U, .block_count = 2U}};
    const auto allocation = source.try_allocate();
    REQUIRE(allocation.succeeded());
    arpg::memory::FixedBlockPool destination{std::move(source)};

    CHECK(source.has_backing() == false); // NOLINT(bugprone-use-after-move): M2 defines moved-from pool state.
    CHECK(source.try_allocate().error ==
          arpg::memory::AllocationError::allocator_unavailable); // NOLINT(bugprone-use-after-move): M2 defines
                                                                 // moved-from pool state.
    CHECK(source.release(allocation.address) ==
          arpg::memory::PoolReleaseResult::allocator_unavailable); // NOLINT(bugprone-use-after-move): M2 defines
                                                                   // moved-from pool state.
    source.reset(); // NOLINT(bugprone-use-after-move): M2 defines moved-from pool state.

    arpg::memory::FixedBlockPool replacement{{.block_size_bytes = 4U, .block_alignment = 8U, .block_count = 1U}};
    replacement = std::move(destination);
    CHECK(destination.has_backing() == false); // NOLINT(bugprone-use-after-move): M2 defines moved-from pool state.
    CHECK(destination.release(allocation.address) ==
          arpg::memory::PoolReleaseResult::allocator_unavailable); // NOLINT(bugprone-use-after-move): M2 defines
                                                                   // moved-from pool state.
    REQUIRE(replacement.release(allocation.address) == arpg::memory::PoolReleaseResult::released);
    move_assign(replacement, replacement);
    CHECK(replacement.has_backing());
}

TEST_CASE("M2 allocator steady-state operations do not allocate", "[unit][m2][memory][allocation]") {
    arpg::memory::LinearArena arena{{.capacity_bytes = 128U, .maximum_alignment = 32U}};
    arpg::memory::FixedBlockPool pool{{.block_size_bytes = 16U, .block_alignment = 16U, .block_count = 2U}};

    tracked_allocations = 0U;
    allocation_tracking = true;
    const auto arena_allocation = arena.try_allocate(8U, 8U);
    const auto pool_allocation = pool.try_allocate();
    const auto release_result = pool.release(pool_allocation.address);
    arena.reset();
    pool.reset();
    allocation_tracking = false;

    REQUIRE(arena_allocation.succeeded());
    REQUIRE(pool_allocation.succeeded());
    CHECK(release_result == arpg::memory::PoolReleaseResult::released);
    CHECK(tracked_allocations == 0U);
}
