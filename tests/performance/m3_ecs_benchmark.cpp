#include "arpg/ecs/registry.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <vector>

namespace {

struct Position {
    float value{0.0F};
};

struct Velocity {
    float value{0.0F};
};

std::atomic_bool allocation_tracking{false};
std::atomic_size_t measured_allocations{0U};

[[nodiscard]] auto allocate(const std::size_t size) -> void* {
    if (allocation_tracking.load(std::memory_order_relaxed)) {
        measured_allocations.fetch_add(1U, std::memory_order_relaxed);
    }
    if (void* const memory = std::malloc(size); memory != nullptr) {
        return memory;
    }
    throw std::bad_alloc{};
}

template <typename Function> [[nodiscard]] auto measure_ns(Function&& function) -> std::int64_t {
    const auto started = std::chrono::steady_clock::now();
    function();
    const auto finished = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(finished - started).count();
}

} // namespace

void* operator new(const std::size_t size) { return allocate(size); }
void* operator new[](const std::size_t size) { return allocate(size); }
void operator delete(void* const memory) noexcept { std::free(memory); }
void operator delete[](void* const memory) noexcept { std::free(memory); }
void operator delete(void* const memory, const std::size_t) noexcept { std::free(memory); }
void operator delete[](void* const memory, const std::size_t) noexcept { std::free(memory); }

auto main() -> int {
    constexpr std::size_t position_count = 1200U;
    constexpr std::size_t moving_count = 1000U;
    constexpr std::size_t warmup_ticks = 120U;
    constexpr std::size_t measured_ticks = 600U;
    constexpr std::size_t samples = 9U;

    arpg::ecs::Registry registry;
    if (registry.prepare_entities(position_count) != arpg::ecs::EcsStatus::success ||
        registry.prepare_components<Position>(position_count) != arpg::ecs::EcsStatus::success ||
        registry.prepare_components<Velocity>(moving_count) != arpg::ecs::EcsStatus::success) {
        return 1;
    }
    for (std::size_t index = 0U; index < position_count; ++index) {
        const auto entity = registry.create().entity;
        if (registry.emplace<Position>(entity, static_cast<float>(index)).status != arpg::ecs::EcsStatus::success) {
            return 1;
        }
        if (index < moving_count && registry.emplace<Velocity>(entity, 0.25F + static_cast<float>(index % 7U)).status !=
                                        arpg::ecs::EcsStatus::success) {
            return 1;
        }
    }
    const auto tick = [&registry]() -> std::size_t {
        return registry.for_each<Position, Velocity>(
            [](auto, Position& position, const Velocity& velocity) { position.value += velocity.value; });
    };
    for (std::size_t tick_index = 0U; tick_index < warmup_ticks; ++tick_index) {
        if (tick() != moving_count) {
            return 1;
        }
    }
    std::array<std::int64_t, samples> timings{};
    std::size_t total_visits = 0U;
    measured_allocations.store(0U, std::memory_order_relaxed);
    allocation_tracking.store(true, std::memory_order_relaxed);
    for (std::size_t sample = 0U; sample < samples; ++sample) {
        timings[sample] = measure_ns([&]() {
            for (std::size_t tick_index = 0U; tick_index < measured_ticks; ++tick_index) {
                total_visits += tick();
            }
        });
    }
    allocation_tracking.store(false, std::memory_order_relaxed);
    std::sort(timings.begin(), timings.end());
    float checksum = 0.0F;
    static_cast<void>(registry.for_each<Position>([&](auto, const Position& position) { checksum += position.value; }));
    float expected_checksum = 0.0F;
    constexpr std::size_t total_ticks = warmup_ticks + (samples * measured_ticks);
    for (std::size_t index = 0U; index < position_count; ++index) {
        float expected_position = static_cast<float>(index);
        if (index < moving_count) {
            const float velocity = 0.25F + static_cast<float>(index % 7U);
            for (std::size_t tick_index = 0U; tick_index < total_ticks; ++tick_index) {
                expected_position += velocity;
            }
        }
        expected_checksum += expected_position;
    }
    constexpr std::size_t expected_visits = moving_count * measured_ticks * samples;
    const std::int64_t median_ns = timings[timings.size() / 2U];
    const std::int64_t p95_ns = timings[(timings.size() * 95U) / 100U];
    const double median_ns_per_tick = static_cast<double>(median_ns) / static_cast<double>(measured_ticks);
    const double visits_per_second =
        (static_cast<double>(moving_count) * static_cast<double>(measured_ticks) * 1000000000.0) /
        static_cast<double>(median_ns);
    const float checksum_tolerance = std::fmax(0.5F, std::fabs(expected_checksum) * 0.00001F);
    const bool visits_verified = total_visits == expected_visits;
    const bool checksum_verified = std::fabs(checksum - expected_checksum) <= checksum_tolerance;
    const bool allocations_verified = measured_allocations.load(std::memory_order_relaxed) == 0U;
    std::printf("m3_ecs_benchmark compiler=%s build=%s positions=%zu moving=%zu warmup_ticks=%zu measured_ticks=%zu "
                "samples=%zu total_visits=%zu expected_visits=%zu visits_verified=%s median_batch_ns=%lld "
                "p95_batch_ns=%lld median_ns_per_tick=%.3f visits_per_second=%.3f measured_allocations=%zu "
                "checksum=%.3f expected_checksum=%.3f checksum_tolerance=%.3f checksum_verified=%s\n",
#if defined(__clang__)
                __clang_version__,
#elif defined(__GNUC__)
                __VERSION__,
#else
                "unknown",
#endif
#if defined(NDEBUG)
                "Release",
#else
                "Debug",
#endif
                position_count, moving_count, warmup_ticks, measured_ticks, samples, total_visits, expected_visits,
                visits_verified ? "true" : "false", static_cast<long long>(median_ns), static_cast<long long>(p95_ns),
                median_ns_per_tick, visits_per_second, measured_allocations.load(std::memory_order_relaxed),
                static_cast<double>(checksum), static_cast<double>(expected_checksum),
                static_cast<double>(checksum_tolerance), checksum_verified ? "true" : "false");
    return visits_verified && checksum_verified && allocations_verified ? 0 : 1;
}
