#pragma once

#include "arpg/core/assert.hpp"
#include "arpg/ecs/component_type_id.hpp"
#include "arpg/ecs/entity.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace arpg::ecs {

namespace testing {
struct RegistryTestAccess;
}

template <typename T>
concept Component =
    std::is_object_v<T> && std::same_as<T, std::remove_cv_t<T>> && std::is_nothrow_move_constructible_v<T> &&
    std::is_nothrow_move_assignable_v<T> && std::is_nothrow_destructible_v<T>;

enum class EcsStatus : std::uint8_t {
    success,
    invalid_entity,
    entity_index_exhausted,
    component_already_present,
    component_missing,
    iteration_active,
    deferred_commands_pending,
    deferred_queue_full,
    deferred_payload_full,
    component_type_not_prepared,
    component_capacity_exhausted,
    capacity_limit_exceeded,
    allocation_failed,
};

struct CreateResult {
    Entity entity{};
    EcsStatus status{EcsStatus::success};
};

template <Component T> struct ComponentResult {
    T* component{nullptr};
    EcsStatus status{EcsStatus::success};
};

struct DeferredFlushReport {
    EcsStatus status{EcsStatus::success};
    std::size_t submitted{0U};
    std::size_t applied{0U};
    std::size_t invalid_targets{0U};
    std::size_t duplicate_adds{0U};
    std::size_t missing_removes{0U};
};

namespace detail {

inline constexpr std::size_t no_dense_index = std::numeric_limits<std::size_t>::max();

template <typename... Ts> struct UniqueTypes : std::true_type {};
template <typename T, typename... Ts>
struct UniqueTypes<T, Ts...> : std::bool_constant<((!std::same_as<T, Ts>) && ...) && UniqueTypes<Ts...>::value> {};

class IComponentPool {
  public:
    virtual ~IComponentPool() = default;
    [[nodiscard]] virtual auto type_id() const noexcept -> ComponentTypeId = 0;
    [[nodiscard]] virtual auto size() const noexcept -> std::size_t = 0;
    [[nodiscard]] virtual auto contains(Entity entity) const noexcept -> bool = 0;
    [[nodiscard]] virtual auto entities() const noexcept -> std::span<const Entity> = 0;
    virtual auto remove(Entity entity) noexcept -> bool = 0;
    virtual void reserve_sparse(std::size_t capacity) = 0;
    virtual void ensure_sparse_for_entity(std::size_t entity_count) = 0;
    virtual void clear() noexcept = 0;
    [[nodiscard]] virtual auto has_payload_capacity() const noexcept -> bool = 0;
    virtual auto stage_payload_from_erased(void* value) noexcept -> std::size_t = 0;
    virtual void discard_payload(std::size_t payload_index) noexcept = 0;
    virtual auto apply_payload(Entity entity, std::size_t payload_index) noexcept -> EcsStatus = 0;
    virtual void prepare_payloads(std::size_t capacity) = 0;
};

template <Component T> class ComponentPool final : public IComponentPool {
  public:
    [[nodiscard]] auto type_id() const noexcept -> ComponentTypeId override { return component_type_id<T>(); }
    [[nodiscard]] auto size() const noexcept -> std::size_t override { return dense_components_.size(); }

    [[nodiscard]] auto contains(const Entity entity) const noexcept -> bool override {
        if (entity.index() >= sparse_.size()) {
            return false;
        }
        const std::size_t position = sparse_[entity.index()];
        return position != no_dense_index && position < dense_entities_.size() && dense_entities_[position] == entity;
    }

    [[nodiscard]] auto entities() const noexcept -> std::span<const Entity> override { return dense_entities_; }

    [[nodiscard]] auto get(const Entity entity) noexcept -> T* {
        if (!contains(entity)) {
            return nullptr;
        }
        return &dense_components_[sparse_[entity.index()]];
    }

    [[nodiscard]] auto get(const Entity entity) const noexcept -> const T* {
        if (!contains(entity)) {
            return nullptr;
        }
        return &dense_components_[sparse_[entity.index()]];
    }

    template <typename... Args>
        requires std::is_nothrow_constructible_v<T, Args...>
    [[nodiscard]] auto emplace(const Entity entity, Args&&... args) -> T* {
        ensure_sparse(entity.index());
        if (contains(entity)) {
            return nullptr;
        }
        dense_components_.emplace_back(std::forward<Args>(args)...);
        try {
            dense_entities_.push_back(entity);
        } catch (...) {
            dense_components_.pop_back();
            throw;
        }
        const std::size_t position = dense_components_.size() - 1U;
        sparse_[entity.index()] = position;
        return &dense_components_.back();
    }

    auto remove(const Entity entity) noexcept -> bool override {
        if (!contains(entity)) {
            return false;
        }
        const std::size_t position = sparse_[entity.index()];
        const std::size_t last = dense_components_.size() - 1U;
        if (position != last) {
            dense_components_[position] = std::move(dense_components_[last]);
            dense_entities_[position] = dense_entities_[last];
            sparse_[dense_entities_[position].index()] = position;
        }
        sparse_[entity.index()] = no_dense_index;
        dense_components_.pop_back();
        dense_entities_.pop_back();
        return true;
    }

    void reserve(const std::size_t capacity) {
        dense_components_.reserve(capacity);
        dense_entities_.reserve(capacity);
    }

    void reserve_sparse(const std::size_t capacity) override { sparse_.reserve(capacity); }
    void ensure_sparse_for_entity(const std::size_t entity_count) override {
        prepare_sparse_for_existing_entities(entity_count);
    }

    void clear() noexcept override {
        while (!dense_components_.empty()) {
            const Entity entity = dense_entities_.back();
            sparse_[entity.index()] = no_dense_index;
            dense_components_.pop_back();
            dense_entities_.pop_back();
        }
    }

    [[nodiscard]] auto has_payload_capacity() const noexcept -> bool override { return !free_payloads_.empty(); }

    [[nodiscard]] auto can_accept_deferred_add(const Entity entity) const noexcept -> bool {
        return entity.index() < sparse_.size() &&
               dense_components_.size() + pending_payloads_ <= dense_components_.capacity() &&
               dense_entities_.size() + pending_payloads_ <= dense_entities_.capacity();
    }

    [[nodiscard]] auto can_stage_deferred_add(const Entity entity) const noexcept -> bool {
        return entity.index() < sparse_.size() &&
               dense_components_.size() + pending_payloads_ < dense_components_.capacity() &&
               dense_entities_.size() + pending_payloads_ < dense_entities_.capacity();
    }

    auto stage_payload_from_erased(void* const value) noexcept -> std::size_t override {
        const std::size_t index = free_payloads_.back();
        free_payloads_.pop_back();
        ARPG_ASSERT(!staged_payloads_[index].has_value(), "A deferred payload slot was not released before reuse.");
        staged_payloads_[index].emplace(std::move(*static_cast<T*>(value)));
        ++pending_payloads_;
        return index;
    }

    void discard_payload(const std::size_t payload_index) noexcept override {
        if (payload_index >= staged_payloads_.size() || !staged_payloads_[payload_index].has_value()) {
            return;
        }
        staged_payloads_[payload_index].reset();
        ARPG_ASSERT(free_payloads_.size() < free_payloads_.capacity(),
                    "Deferred payload free list capacity was not prepared.");
        free_payloads_.push_back(payload_index);
        --pending_payloads_;
    }

    auto apply_payload(const Entity entity, const std::size_t payload_index) noexcept -> EcsStatus override {
        if (payload_index >= staged_payloads_.size() || !staged_payloads_[payload_index].has_value()) {
            return EcsStatus::component_missing;
        }
        if (contains(entity)) {
            discard_payload(payload_index);
            return EcsStatus::component_already_present;
        }
        if (!can_accept_deferred_add(entity)) {
            return EcsStatus::component_capacity_exhausted;
        }
        T* const component = emplace(entity, std::move(*staged_payloads_[payload_index]));
        ARPG_ASSERT(component != nullptr, "Prepared deferred component insertion unexpectedly failed.");
        discard_payload(payload_index);
        return EcsStatus::success;
    }

    void prepare_payloads(const std::size_t capacity) override {
        if (capacity <= staged_payloads_.size()) {
            return;
        }
        const std::size_t previous_size = staged_payloads_.size();
        staged_payloads_.resize(capacity);
        free_payloads_.reserve(capacity);
        for (std::size_t index = capacity; index > previous_size; --index) {
            free_payloads_.push_back(index - 1U);
        }
    }

    void prepare_sparse_for_existing_entities(const std::size_t entity_count) {
        if (sparse_.size() < entity_count) {
            sparse_.resize(entity_count, no_dense_index);
        }
    }

  private:
    void ensure_sparse(const std::uint32_t index) {
        const std::size_t required = static_cast<std::size_t>(index) + 1U;
        if (required > sparse_.size()) {
            sparse_.resize(required, no_dense_index);
        }
    }

    std::vector<Entity> dense_entities_{};
    std::vector<T> dense_components_{};
    std::vector<std::size_t> sparse_{};
    std::vector<std::optional<T>> staged_payloads_{};
    std::vector<std::size_t> free_payloads_{};
    std::size_t pending_payloads_{0U};
};

} // namespace detail

class Registry {
  public:
    Registry() = default;
    ~Registry() noexcept;
    Registry(const Registry&) = delete;
    auto operator=(const Registry&) -> Registry& = delete;
    Registry(Registry&&) = delete;
    auto operator=(Registry&&) -> Registry& = delete;

    [[nodiscard]] auto create() -> CreateResult;
    [[nodiscard]] auto destroy(Entity entity) noexcept -> EcsStatus;
    [[nodiscard]] auto valid(Entity entity) const noexcept -> bool;

    [[nodiscard]] auto prepare_entities(std::size_t capacity) -> EcsStatus;
    [[nodiscard]] auto prepare_deferred_commands(std::size_t capacity) -> EcsStatus;
    [[nodiscard]] auto defer_destroy(Entity entity) noexcept -> EcsStatus;
    [[nodiscard]] auto flush_deferred() noexcept -> DeferredFlushReport;
    [[nodiscard]] auto discard_deferred() noexcept -> std::size_t;
    [[nodiscard]] auto entity_count() const noexcept -> std::size_t { return live_entities_; }
    [[nodiscard]] auto entity_capacity() const noexcept -> std::size_t { return slots_.capacity(); }
    [[nodiscard]] auto deferred_count() const noexcept -> std::size_t { return commands_.size(); }

    template <Component T> [[nodiscard]] auto prepare_components(const std::size_t capacity) -> EcsStatus {
        if (iteration_depth_ != 0U) {
            return EcsStatus::iteration_active;
        }
        if (!commands_.empty()) {
            return EcsStatus::deferred_commands_pending;
        }
        try {
            auto& pool = ensure_pool<T>();
            pool.reserve(capacity);
            pool.reserve_sparse(slots_.capacity());
        } catch (...) {
            return EcsStatus::allocation_failed;
        }
        return EcsStatus::success;
    }

    template <Component T> [[nodiscard]] auto prepare_deferred_adds(const std::size_t capacity) -> EcsStatus {
        if (iteration_depth_ != 0U) {
            return EcsStatus::iteration_active;
        }
        if (!commands_.empty()) {
            return EcsStatus::deferred_commands_pending;
        }
        try {
            auto& pool = ensure_pool<T>();
            pool.reserve(pool.size() + capacity);
            pool.reserve_sparse(slots_.capacity());
            pool.prepare_sparse_for_existing_entities(slots_.size());
            pool.prepare_payloads(capacity);
        } catch (...) {
            return EcsStatus::allocation_failed;
        }
        return EcsStatus::success;
    }

    template <Component T, typename... Args>
        requires std::is_nothrow_constructible_v<T, Args...>
    [[nodiscard]] auto emplace(const Entity entity, Args&&... args) -> ComponentResult<T> {
        if (iteration_depth_ != 0U) {
            return {nullptr, EcsStatus::iteration_active};
        }
        if (!commands_.empty()) {
            return {nullptr, EcsStatus::deferred_commands_pending};
        }
        if (!valid(entity)) {
            return {nullptr, EcsStatus::invalid_entity};
        }
        detail::ComponentPool<T>* pool = nullptr;
        try {
            pool = &ensure_pool<T>();
        } catch (...) {
            return {nullptr, EcsStatus::allocation_failed};
        }
        if (pool->contains(entity)) {
            return {nullptr, EcsStatus::component_already_present};
        }
        try {
            return {pool->emplace(entity, std::forward<Args>(args)...), EcsStatus::success};
        } catch (...) {
            return {nullptr, EcsStatus::allocation_failed};
        }
    }

    template <Component T> [[nodiscard]] auto remove(const Entity entity) noexcept -> EcsStatus {
        if (iteration_depth_ != 0U) {
            return EcsStatus::iteration_active;
        }
        if (!commands_.empty()) {
            return EcsStatus::deferred_commands_pending;
        }
        if (!valid(entity)) {
            return EcsStatus::invalid_entity;
        }
        auto* pool = find_pool<T>();
        return pool != nullptr && pool->remove(entity) ? EcsStatus::success : EcsStatus::component_missing;
    }

    template <Component T> [[nodiscard]] auto get(const Entity entity) noexcept -> T* {
        auto* pool = find_pool<T>();
        return valid(entity) && pool != nullptr ? pool->get(entity) : nullptr;
    }

    template <Component T> [[nodiscard]] auto get(const Entity entity) const noexcept -> const T* {
        const auto* pool = find_pool<T>();
        return valid(entity) && pool != nullptr ? pool->get(entity) : nullptr;
    }

    template <Component T> [[nodiscard]] auto contains(const Entity entity) const noexcept -> bool {
        return get<T>(entity) != nullptr;
    }

    template <Component T> [[nodiscard]] auto component_count() const noexcept -> std::size_t {
        const auto* pool = find_pool<T>();
        return pool != nullptr ? pool->size() : 0U;
    }

    template <Component T> [[nodiscard]] auto defer_remove(const Entity entity) noexcept -> EcsStatus {
        if (!valid(entity)) {
            return EcsStatus::invalid_entity;
        }
        return append_command({DeferredCommand::Kind::remove, entity, component_type_id<T>(), 0U});
    }

    template <Component T> [[nodiscard]] auto defer_add(const Entity entity, T component) noexcept -> EcsStatus {
        if (!valid(entity)) {
            return EcsStatus::invalid_entity;
        }
        auto* pool = find_pool<T>();
        if (pool == nullptr) {
            return EcsStatus::component_type_not_prepared;
        }
        if (commands_.size() >= command_limit_) {
            return EcsStatus::deferred_queue_full;
        }
        if (!pool->has_payload_capacity()) {
            return EcsStatus::deferred_payload_full;
        }
        if (!pool->can_stage_deferred_add(entity)) {
            return EcsStatus::component_capacity_exhausted;
        }
        const std::size_t payload = pool->stage_payload_from_erased(&component);
        commands_.push_back({DeferredCommand::Kind::add, entity, component_type_id<T>(), payload});
        return EcsStatus::success;
    }

    template <Component... Ts, typename Function> [[nodiscard]] auto for_each(Function&& function) -> std::size_t {
        return for_each_impl<Ts...>(std::forward<Function>(function));
    }

    template <Component... Ts, typename Function>
    [[nodiscard]] auto for_each(Function&& function) const -> std::size_t {
        static_assert(sizeof...(Ts) > 0U);
        static_assert(detail::UniqueTypes<Ts...>::value);
        const auto pools = std::tuple<const detail::ComponentPool<Ts>*...>{find_pool<Ts>()...};
        if (std::apply([](const auto*... pool) { return ((pool == nullptr) || ...); }, pools)) {
            return 0U;
        }
        const detail::IComponentPool* driver = nullptr;
        for (const auto& candidate : pools_) {
            const bool requested = ((candidate->type_id() == component_type_id<Ts>()) || ...);
            if (requested && (driver == nullptr || candidate->size() < driver->size())) {
                driver = candidate.get();
            }
        }
        IterationGuard guard{*this};
        std::size_t visited = 0U;
        for (const Entity entity : driver->entities()) {
            std::apply(
                [&](const auto*... pool) {
                    if ((pool->contains(entity) && ...)) {
                        function(entity, *pool->get(entity)...);
                        ++visited;
                    }
                },
                pools);
        }
        return visited;
    }

  private:
    friend struct testing::RegistryTestAccess;

    struct Slot {
        std::uint32_t generation{1U};
        bool alive{false};
        bool retired{false};
    };

    struct DeferredCommand {
        enum class Kind : std::uint8_t { destroy, remove, add };
        Kind kind;
        Entity entity;
        ComponentTypeId type;
        std::size_t payload;
    };

    class IterationGuard {
      public:
        explicit IterationGuard(const Registry& registry) noexcept : registry_(registry) {
            ++registry_.iteration_depth_;
        }
        ~IterationGuard() { --registry_.iteration_depth_; }
        IterationGuard(const IterationGuard&) = delete;
        auto operator=(const IterationGuard&) -> IterationGuard& = delete;

      private:
        const Registry& registry_;
    };

    [[nodiscard]] auto append_command(DeferredCommand command) noexcept -> EcsStatus;
    [[nodiscard]] auto destroy_impl(Entity entity) noexcept -> EcsStatus;
    [[nodiscard]] auto find_pool(ComponentTypeId type) noexcept -> detail::IComponentPool*;
    [[nodiscard]] auto find_pool(ComponentTypeId type) const noexcept -> const detail::IComponentPool*;

    template <Component T> [[nodiscard]] auto find_pool() noexcept -> detail::ComponentPool<T>* {
        auto* pool = find_pool(component_type_id<T>());
        return static_cast<detail::ComponentPool<T>*>(pool);
    }

    template <Component T> [[nodiscard]] auto find_pool() const noexcept -> const detail::ComponentPool<T>* {
        const auto* pool = find_pool(component_type_id<T>());
        return static_cast<const detail::ComponentPool<T>*>(pool);
    }

    template <Component T> [[nodiscard]] auto ensure_pool() -> detail::ComponentPool<T>& {
        if (auto* const existing = find_pool<T>(); existing != nullptr) {
            return *existing;
        }
        auto pool = std::make_unique<detail::ComponentPool<T>>();
        pool->reserve_sparse(slots_.capacity());
        auto* const result = pool.get();
        pools_.push_back(std::move(pool));
        return *result;
    }

    template <Component... Ts, typename Function> [[nodiscard]] auto for_each_impl(Function&& function) -> std::size_t {
        static_assert(sizeof...(Ts) > 0U);
        static_assert(detail::UniqueTypes<Ts...>::value);
        auto pools = std::tuple<detail::ComponentPool<Ts>*...>{find_pool<Ts>()...};
        if (std::apply([](const auto*... pool) { return ((pool == nullptr) || ...); }, pools)) {
            return 0U;
        }
        detail::IComponentPool* driver = nullptr;
        for (const auto& candidate : pools_) {
            const bool requested = ((candidate->type_id() == component_type_id<Ts>()) || ...);
            if (requested && (driver == nullptr || candidate->size() < driver->size())) {
                driver = candidate.get();
            }
        }
        if (driver == nullptr) {
            return 0U;
        }
        IterationGuard guard{*this};
        std::size_t visited = 0U;
        for (const Entity entity : driver->entities()) {
            std::apply(
                [&](auto*... pool) {
                    if ((pool->contains(entity) && ...)) {
                        function(entity, *pool->get(entity)...);
                        ++visited;
                    }
                },
                pools);
        }
        return visited;
    }

    std::vector<Slot> slots_{};
    std::vector<std::uint32_t> free_slots_{};
    std::size_t free_head_{0U};
    std::size_t live_entities_{0U};
    std::vector<std::unique_ptr<detail::IComponentPool>> pools_{};
    std::vector<DeferredCommand> commands_{};
    std::size_t command_limit_{0U};
    mutable std::size_t iteration_depth_{0U};
};

} // namespace arpg::ecs
