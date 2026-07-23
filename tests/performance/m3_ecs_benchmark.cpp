#include "arpg/ecs/registry.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace {

struct Position {
    float value{0.0F};
};

struct Velocity {
    float value{0.0F};
};

template <typename Function> [[nodiscard]] auto measure_ns(Function&& function) -> std::int64_t {
    const auto started = std::chrono::steady_clock::now();
    function();
    const auto finished = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(finished - started).count();
}

} // namespace

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
    const auto tick = [&registry]() {
        static_cast<void>(registry.for_each<Position, Velocity>(
            [](auto, Position& position, const Velocity& velocity) { position.value += velocity.value; }));
    };
    for (std::size_t tick_index = 0U; tick_index < warmup_ticks; ++tick_index) {
        tick();
    }
    std::array<std::int64_t, samples> timings{};
    for (std::size_t sample = 0U; sample < samples; ++sample) {
        timings[sample] = measure_ns([&]() {
            for (std::size_t tick_index = 0U; tick_index < measured_ticks; ++tick_index) {
                tick();
            }
        });
    }
    std::sort(timings.begin(), timings.end());
    float checksum = 0.0F;
    static_cast<void>(registry.for_each<Position>([&](auto, const Position& position) { checksum += position.value; }));
    std::printf("m3_ecs_benchmark positions=%zu moving=%zu warmup_ticks=%zu measured_ticks=%zu samples=%zu "
                "median_ns=%lld p95_ns=%lld checksum=%.3f\n",
                position_count, moving_count, warmup_ticks, measured_ticks, samples,
                static_cast<long long>(timings[timings.size() / 2U]),
                static_cast<long long>(timings[(timings.size() * 95U) / 100U]), static_cast<double>(checksum));
    return checksum > 0.0F ? 0 : 1;
}
