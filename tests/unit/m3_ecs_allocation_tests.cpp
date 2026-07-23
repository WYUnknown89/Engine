#include "arpg/ecs/registry.hpp"

#include <atomic>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <new>

namespace {

std::atomic_bool tracking{false};
std::atomic_size_t allocation_count{0U};
std::atomic_size_t fail_on_allocation{std::numeric_limits<std::size_t>::max()};

struct Position {
    int value{0};
};

struct Velocity {
    int value{0};
};

[[nodiscard]] auto allocate(const std::size_t size) -> void* {
    if (tracking.load(std::memory_order_relaxed)) {
        const std::size_t allocation = allocation_count.fetch_add(1U, std::memory_order_relaxed);
        if (allocation == fail_on_allocation.load(std::memory_order_relaxed)) {
            throw std::bad_alloc{};
        }
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

namespace {

[[nodiscard]] auto transactional_creation_survives_sparse_growth_failure() -> bool {
    arpg::ecs::Registry registry;
    const auto existing = registry.create();
    if (existing.status != arpg::ecs::EcsStatus::success ||
        registry.emplace<Position>(existing.entity, 7).status != arpg::ecs::EcsStatus::success) {
        return false;
    }

    allocation_count.store(0U, std::memory_order_relaxed);
    fail_on_allocation.store(2U, std::memory_order_relaxed);
    tracking.store(true, std::memory_order_relaxed);
    const auto failed = registry.create();
    tracking.store(false, std::memory_order_relaxed);
    fail_on_allocation.store(std::numeric_limits<std::size_t>::max(), std::memory_order_relaxed);

    const Position* const position = registry.get<Position>(existing.entity);
    if (failed.status != arpg::ecs::EcsStatus::allocation_failed || failed.entity != arpg::ecs::Entity{} ||
        registry.entity_count() != 1U || !registry.valid(existing.entity) || position == nullptr ||
        position->value != 7) {
        return false;
    }

    const auto recovered = registry.create();
    return recovered.status == arpg::ecs::EcsStatus::success && recovered.entity.index() == 1U &&
           recovered.entity.generation() == 1U && registry.entity_count() == 2U;
}

[[nodiscard]] auto prepared_hot_path_allocates_no_storage() -> bool {
    arpg::ecs::Registry registry;
    if (registry.prepare_entities(2U) != arpg::ecs::EcsStatus::success ||
        registry.prepare_components<Position>(2U) != arpg::ecs::EcsStatus::success ||
        registry.prepare_components<Velocity>(2U) != arpg::ecs::EcsStatus::success ||
        registry.prepare_deferred_commands(2U) != arpg::ecs::EcsStatus::success ||
        registry.prepare_deferred_adds<Position>(1U) != arpg::ecs::EcsStatus::success) {
        return false;
    }
    const auto first = registry.create().entity;
    const auto second = registry.create().entity;
    if (registry.emplace<Position>(first, 1).status != arpg::ecs::EcsStatus::success ||
        registry.emplace<Velocity>(first, 2).status != arpg::ecs::EcsStatus::success) {
        return false;
    }

    allocation_count.store(0U, std::memory_order_relaxed);
    fail_on_allocation.store(std::numeric_limits<std::size_t>::max(), std::memory_order_relaxed);
    tracking.store(true, std::memory_order_relaxed);
    static_cast<void>(registry.for_each<Position, Velocity>(
        [](auto, Position& position, Velocity& velocity) { position.value += velocity.value; }));
    const auto queued = registry.defer_add<Position>(second, Position{4});
    const auto flushed = registry.flush_deferred();
    tracking.store(false, std::memory_order_relaxed);

    return queued == arpg::ecs::EcsStatus::success && flushed.applied == 1U &&
           allocation_count.load(std::memory_order_relaxed) == 0U;
}

} // namespace

auto main() -> int {
    const bool transactional = transactional_creation_survives_sparse_growth_failure();
    const bool zero_allocation = prepared_hot_path_allocates_no_storage();
    std::printf("m3_ecs_allocation_tests transactional_sparse_growth_failure=%s prepared_hot_path_allocations=%zu "
                "zero_allocations_verified=%s\n",
                transactional ? "true" : "false", allocation_count.load(std::memory_order_relaxed),
                zero_allocation ? "true" : "false");
    return transactional && zero_allocation ? 0 : 1;
}
