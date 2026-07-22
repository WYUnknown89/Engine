#include "arpg/memory/fixed_block_pool.hpp"
#include "arpg/memory/linear_arena.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>

namespace {

template <typename Function> [[nodiscard]] auto measure_nanoseconds(Function&& function) noexcept -> std::int64_t {
    const auto started = std::chrono::steady_clock::now();
    function();
    const auto finished = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(finished - started).count();
}

} // namespace

auto main() -> int {
    constexpr std::size_t operation_count = 1000000U;
    constexpr std::size_t allocations_per_reset = 64U;
    arpg::memory::FixedBlockPool pool{64U, 64U, 1U};
    const auto pool_duration = measure_nanoseconds([&pool]() noexcept {
        for (std::size_t index = 0U; index < operation_count; ++index) {
            const auto allocation = pool.try_allocate();
            if (!allocation.succeeded() ||
                pool.release(allocation.address) != arpg::memory::PoolReleaseResult::released) {
                std::terminate();
            }
        }
    });

    arpg::memory::LinearArena arena{4096U, 64U};
    const auto arena_duration = measure_nanoseconds([&arena]() noexcept {
        for (std::size_t index = 0U; index < operation_count; ++index) {
            if (!arena.try_allocate(64U, 64U).succeeded()) {
                std::terminate();
            }
            if ((index + 1U) % allocations_per_reset == 0U) {
                arena.reset();
            }
        }
    });

    std::printf("allocator_benchmark operations=%zu pool_ns=%lld arena_ns=%lld\n", operation_count,
                static_cast<long long>(pool_duration), static_cast<long long>(arena_duration));
    return 0;
}
