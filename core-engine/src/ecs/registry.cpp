#include "arpg/ecs/registry.hpp"

#include <limits>

namespace arpg::ecs {

Registry::~Registry() noexcept {
    static_cast<void>(discard_deferred());
    for (auto& pool : pools_) {
        pool->clear();
        pool.reset();
    }
}

auto Registry::valid(const Entity entity) const noexcept -> bool {
    if (!entity.has_valid_generation() || entity.index() >= slots_.size()) {
        return false;
    }
    const Slot& slot = slots_[entity.index()];
    return slot.alive && !slot.retired && slot.generation == entity.generation();
}

auto Registry::create() -> CreateResult {
    if (iteration_depth_ != 0U) {
        return {{}, EcsStatus::iteration_active};
    }
    if (!commands_.empty()) {
        return {{}, EcsStatus::deferred_commands_pending};
    }
    if (free_head_ < free_slots_.size()) {
        const std::uint32_t index = free_slots_[free_head_++];
        Slot& slot = slots_[index];
        slot.alive = true;
        ++live_entities_;
        if (free_head_ == free_slots_.size()) {
            free_slots_.clear();
            free_head_ = 0U;
        }
        return {Entity::from_parts(index, slot.generation), EcsStatus::success};
    }
    if (slots_.size() > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())) {
        return {{}, EcsStatus::entity_index_exhausted};
    }
    try {
        slots_.reserve(slots_.size() + 1U);
        free_slots_.reserve(slots_.size() + 1U);
        slots_.push_back({});
    } catch (...) {
        return {{}, EcsStatus::allocation_failed};
    }
    const std::uint32_t index = static_cast<std::uint32_t>(slots_.size() - 1U);
    slots_.back().alive = true;
    for (auto& pool : pools_) {
        pool->ensure_sparse_for_entity(slots_.size());
    }
    ++live_entities_;
    return {Entity::from_parts(index, 1U), EcsStatus::success};
}

auto Registry::destroy(const Entity entity) noexcept -> EcsStatus {
    if (iteration_depth_ != 0U) {
        return EcsStatus::iteration_active;
    }
    if (!commands_.empty()) {
        return EcsStatus::deferred_commands_pending;
    }
    return destroy_impl(entity);
}

auto Registry::destroy_impl(const Entity entity) noexcept -> EcsStatus {
    if (!valid(entity)) {
        return EcsStatus::invalid_entity;
    }
    for (const auto& pool : pools_) {
        static_cast<void>(pool->remove(entity));
    }
    Slot& slot = slots_[entity.index()];
    slot.alive = false;
    --live_entities_;
    if (slot.generation == std::numeric_limits<std::uint32_t>::max()) {
        slot.retired = true;
        return EcsStatus::success;
    }
    ++slot.generation;
    ARPG_ASSERT(free_slots_.size() < free_slots_.capacity(),
                "Entity creation did not reserve reusable-slot capacity for destruction.");
    free_slots_.push_back(entity.index());
    return EcsStatus::success;
}

auto Registry::prepare_entities(const std::size_t capacity) -> EcsStatus {
    if (iteration_depth_ != 0U) {
        return EcsStatus::iteration_active;
    }
    if (!commands_.empty()) {
        return EcsStatus::deferred_commands_pending;
    }
    if (capacity > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()) + 1U) {
        return EcsStatus::entity_index_exhausted;
    }
    try {
        slots_.reserve(capacity);
        free_slots_.reserve(capacity);
        for (auto& pool : pools_) {
            pool->reserve_sparse(capacity);
        }
    } catch (...) {
        return EcsStatus::allocation_failed;
    }
    return EcsStatus::success;
}

auto Registry::prepare_deferred_commands(const std::size_t capacity) -> EcsStatus {
    if (iteration_depth_ != 0U) {
        return EcsStatus::iteration_active;
    }
    if (!commands_.empty()) {
        return EcsStatus::deferred_commands_pending;
    }
    try {
        commands_.reserve(capacity);
    } catch (...) {
        return EcsStatus::allocation_failed;
    }
    command_limit_ = capacity;
    return EcsStatus::success;
}

auto Registry::append_command(const DeferredCommand command) noexcept -> EcsStatus {
    if (commands_.size() >= command_limit_) {
        return EcsStatus::deferred_queue_full;
    }
    commands_.push_back(command);
    return EcsStatus::success;
}

auto Registry::defer_destroy(const Entity entity) noexcept -> EcsStatus {
    if (!valid(entity)) {
        return EcsStatus::invalid_entity;
    }
    return append_command({DeferredCommand::Kind::destroy, entity, {}, 0U});
}

auto Registry::find_pool(const ComponentTypeId type) noexcept -> detail::IComponentPool* {
    for (const auto& pool : pools_) {
        if (pool->type_id() == type) {
            return pool.get();
        }
    }
    return nullptr;
}

auto Registry::find_pool(const ComponentTypeId type) const noexcept -> const detail::IComponentPool* {
    for (const auto& pool : pools_) {
        if (pool->type_id() == type) {
            return pool.get();
        }
    }
    return nullptr;
}

auto Registry::discard_deferred() noexcept -> std::size_t {
    const std::size_t count = commands_.size();
    for (const DeferredCommand& command : commands_) {
        if (command.kind == DeferredCommand::Kind::add) {
            if (auto* const pool = find_pool(command.type); pool != nullptr) {
                pool->discard_payload(command.payload);
            }
        }
    }
    commands_.clear();
    return count;
}

auto Registry::flush_deferred() noexcept -> DeferredFlushReport {
    DeferredFlushReport report{};
    report.submitted = commands_.size();
    if (iteration_depth_ != 0U) {
        report.status = EcsStatus::iteration_active;
        return report;
    }
    for (const DeferredCommand& command : commands_) {
        if (!valid(command.entity)) {
            ++report.invalid_targets;
            if (command.kind == DeferredCommand::Kind::add) {
                if (auto* const pool = find_pool(command.type); pool != nullptr) {
                    pool->discard_payload(command.payload);
                }
            }
            continue;
        }
        if (command.kind == DeferredCommand::Kind::destroy) {
            if (destroy_impl(command.entity) == EcsStatus::success) {
                ++report.applied;
            }
            continue;
        }
        auto* const pool = find_pool(command.type);
        if (command.kind == DeferredCommand::Kind::remove) {
            if (pool != nullptr && pool->remove(command.entity)) {
                ++report.applied;
            } else {
                ++report.missing_removes;
            }
            continue;
        }
        if (pool == nullptr) {
            ++report.missing_removes;
            continue;
        }
        const EcsStatus result = pool->apply_payload(command.entity, command.payload);
        if (result == EcsStatus::success) {
            ++report.applied;
        } else if (result == EcsStatus::component_already_present) {
            ++report.duplicate_adds;
        } else {
            ++report.missing_removes;
        }
    }
    commands_.clear();
    return report;
}

} // namespace arpg::ecs
