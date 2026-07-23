#include "arpg/ecs/registry.hpp"

#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <new>

namespace {

std::atomic_bool tracking{false};
std::atomic_size_t allocation_count{0U};

struct Position {
    int value{0};
};

struct Velocity {
    int value{0};
};

[[nodiscard]] auto allocate(const std::size_t size) -> void* {
    if (tracking.load(std::memory_order_relaxed)) {
        allocation_count.fetch_add(1U, std::memory_order_relaxed);
    }
    if (void* const memory = std::malloc(size); memory != nullptr) {
        return memory;
    }
    throw std::bad_alloc{};
}

} // namespace

void* operator new(const std::size_t size) { return allocate(size); }
void* operator new[](const std::size_t size) { return allocate(size); }
void operator delete(void* const memory) noexcept { std::free(memory); }
void operator delete[](void* const memory) noexcept { std::free(memory); }
void operator delete(void* const memory, const std::size_t) noexcept { std::free(memory); }
void operator delete[](void* const memory, const std::size_t) noexcept { std::free(memory); }

auto main() -> int {
    arpg::ecs::Registry registry;
    if (registry.prepare_entities(2U) != arpg::ecs::EcsStatus::success ||
        registry.prepare_components<Position>(2U) != arpg::ecs::EcsStatus::success ||
        registry.prepare_components<Velocity>(2U) != arpg::ecs::EcsStatus::success ||
        registry.prepare_deferred_commands(2U) != arpg::ecs::EcsStatus::success ||
        registry.prepare_deferred_adds<Position>(1U) != arpg::ecs::EcsStatus::success) {
        return 1;
    }
    const auto first = registry.create().entity;
    const auto second = registry.create().entity;
    if (registry.emplace<Position>(first, 1).status != arpg::ecs::EcsStatus::success ||
        registry.emplace<Velocity>(first, 2).status != arpg::ecs::EcsStatus::success) {
        return 1;
    }

    allocation_count.store(0U, std::memory_order_relaxed);
    tracking.store(true, std::memory_order_relaxed);
    static_cast<void>(registry.for_each<Position, Velocity>(
        [](auto, Position& position, Velocity& velocity) { position.value += velocity.value; }));
    const auto queued = registry.defer_add<Position>(second, Position{4});
    const auto flushed = registry.flush_deferred();
    tracking.store(false, std::memory_order_relaxed);

    return queued == arpg::ecs::EcsStatus::success && flushed.applied == 1U &&
                   allocation_count.load(std::memory_order_relaxed) == 0U
               ? 0
               : 1;
}
