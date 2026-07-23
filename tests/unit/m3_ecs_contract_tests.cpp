#include "arpg/ecs/registry.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace arpg::ecs::testing {

struct RegistryTestAccess {
    static void force_generation(Registry& registry, const std::uint32_t index, const std::uint32_t generation) {
        registry.slots_[index].generation = generation;
    }

    [[nodiscard]] static auto generation(const Registry& registry, const std::uint32_t index) -> std::uint32_t {
        return registry.slots_[index].generation;
    }

    [[nodiscard]] static auto retired(const Registry& registry, const std::uint32_t index) -> bool {
        return registry.slots_[index].retired;
    }

    [[nodiscard]] static auto registered_pool_count(const Registry& registry) -> std::size_t {
        return registry.pools_.size();
    }
};

} // namespace arpg::ecs::testing

namespace {

using arpg::ecs::EcsStatus;
using arpg::ecs::Entity;
using arpg::ecs::Registry;
using arpg::ecs::testing::RegistryTestAccess;

struct Position {
    int value{0};
};

struct Velocity {
    int value{0};
};

struct Health {
    int value{0};
};

struct MoveOnly {
    explicit MoveOnly(const int initial_value) noexcept : value(std::make_unique<int>(initial_value)) {}
    MoveOnly(MoveOnly&&) noexcept = default;
    auto operator=(MoveOnly&&) noexcept -> MoveOnly& = default;
    MoveOnly(const MoveOnly&) = delete;
    auto operator=(const MoveOnly&) -> MoveOnly& = delete;
    std::unique_ptr<int> value;
};

struct LifetimeComponent {
    static inline int alive = 0;
    static inline int constructions = 0;
    static inline int destructions = 0;
    static inline int moves = 0;

    explicit LifetimeComponent(const int initial_value) noexcept : value(initial_value) {
        ++alive;
        ++constructions;
    }

    LifetimeComponent(LifetimeComponent&& other) noexcept : value(other.value) {
        other.value = -1;
        ++alive;
        ++constructions;
        ++moves;
    }

    auto operator=(LifetimeComponent&& other) noexcept -> LifetimeComponent& {
        value = other.value;
        other.value = -1;
        ++moves;
        return *this;
    }

    LifetimeComponent(const LifetimeComponent&) = delete;
    auto operator=(const LifetimeComponent&) -> LifetimeComponent& = delete;

    ~LifetimeComponent() noexcept {
        --alive;
        ++destructions;
    }

    static void reset() noexcept {
        alive = 0;
        constructions = 0;
        destructions = 0;
        moves = 0;
    }

    int value{0};
};

struct DeferredMoveOnly {
    static inline int moves = 0;

    explicit DeferredMoveOnly(const int initial_value) noexcept : value(std::make_unique<int>(initial_value)) {}

    DeferredMoveOnly(DeferredMoveOnly&& other) noexcept : value(std::move(other.value)) { ++moves; }

    auto operator=(DeferredMoveOnly&& other) noexcept -> DeferredMoveOnly& {
        value = std::move(other.value);
        ++moves;
        return *this;
    }

    DeferredMoveOnly(const DeferredMoveOnly&) = delete;
    auto operator=(const DeferredMoveOnly&) -> DeferredMoveOnly& = delete;

    std::unique_ptr<int> value;
};

template <int Identifier> struct OrderedLifetime {
    static inline std::vector<int>* destruction_order = nullptr;

    explicit OrderedLifetime(const int initial_value) noexcept : value(initial_value) {}

    OrderedLifetime(OrderedLifetime&& other) noexcept : value(other.value), owns_record(other.owns_record) {
        other.owns_record = false;
    }

    auto operator=(OrderedLifetime&& other) noexcept -> OrderedLifetime& {
        value = other.value;
        owns_record = other.owns_record;
        other.owns_record = false;
        return *this;
    }

    OrderedLifetime(const OrderedLifetime&) = delete;
    auto operator=(const OrderedLifetime&) -> OrderedLifetime& = delete;

    ~OrderedLifetime() noexcept {
        if (owns_record && destruction_order != nullptr) {
            destruction_order->push_back(Identifier);
        }
    }

    int value{0};
    bool owns_record{true};
};

using FirstRegistered = OrderedLifetime<1>;
using SecondRegistered = OrderedLifetime<2>;

[[nodiscard]] auto create_entity(Registry& registry) -> Entity {
    const auto result = registry.create();
    REQUIRE(result.status == EcsStatus::success);
    return result.entity;
}

} // namespace

TEST_CASE("M3 entity generations reject zero at every index edge", "[unit][m3][ecs][entity]") {
    Registry registry;

    CHECK_FALSE(registry.valid(Entity::from_parts(0U, 0U)));
    CHECK_FALSE(registry.valid(Entity::from_parts(19U, 0U)));
    CHECK_FALSE(registry.valid(Entity::from_parts(std::numeric_limits<std::uint32_t>::max(), 0U)));
    CHECK_FALSE(registry.valid(Entity::from_parts(1U, 1U)));
    CHECK_FALSE(registry.valid(
        Entity::from_parts(std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max())));

    const Entity entity = create_entity(registry);
    CHECK(entity.index() == 0U);
    CHECK(entity.generation() == 1U);
    CHECK(registry.entity_count() == 1U);
}

TEST_CASE("M3 entity slots reuse in FIFO order while stale handles remain invalid", "[unit][m3][ecs][entity]") {
    Registry registry;
    std::array<Entity, 3U> first_cycle{create_entity(registry), create_entity(registry), create_entity(registry)};
    CHECK(registry.entity_count() == first_cycle.size());

    for (const Entity entity : first_cycle) {
        REQUIRE(registry.destroy(entity) == EcsStatus::success);
    }
    CHECK(registry.entity_count() == 0U);

    std::array<Entity, 3U> second_cycle{create_entity(registry), create_entity(registry), create_entity(registry)};
    for (std::size_t index = 0U; index < first_cycle.size(); ++index) {
        CHECK(second_cycle[index].index() == first_cycle[index].index());
        CHECK(second_cycle[index].generation() == 2U);
        CHECK_FALSE(registry.valid(first_cycle[index]));
    }
    CHECK(registry.entity_count() == second_cycle.size());

    REQUIRE(registry.destroy(second_cycle[1]) == EcsStatus::success);
    REQUIRE(registry.destroy(second_cycle[0]) == EcsStatus::success);
    REQUIRE(registry.destroy(second_cycle[2]) == EcsStatus::success);
    const std::array<Entity, 3U> third_cycle{create_entity(registry), create_entity(registry), create_entity(registry)};
    CHECK(third_cycle[0].index() == second_cycle[1].index());
    CHECK(third_cycle[1].index() == second_cycle[0].index());
    CHECK(third_cycle[2].index() == second_cycle[2].index());
    CHECK(registry.entity_count() == third_cycle.size());
}

TEST_CASE("M3 exhausted entity generations retire permanently without wrapping", "[unit][m3][ecs][entity]") {
    Registry registry;
    const Entity initial = create_entity(registry);
    RegistryTestAccess::force_generation(registry, initial.index(), std::numeric_limits<std::uint32_t>::max());
    const Entity exhausted = Entity::from_parts(initial.index(), std::numeric_limits<std::uint32_t>::max());

    CHECK_FALSE(registry.valid(initial));
    REQUIRE(registry.valid(exhausted));
    REQUIRE(registry.destroy(exhausted) == EcsStatus::success);
    CHECK(registry.entity_count() == 0U);
    CHECK(RegistryTestAccess::retired(registry, initial.index()));
    CHECK(RegistryTestAccess::generation(registry, initial.index()) == std::numeric_limits<std::uint32_t>::max());
    CHECK_FALSE(registry.valid(Entity::from_parts(initial.index(), 0U)));
    CHECK_FALSE(registry.valid(exhausted));

    const Entity replacement = create_entity(registry);
    CHECK(replacement.index() != initial.index());
    CHECK(replacement.index() == 1U);
    CHECK(replacement.generation() == 1U);
    CHECK(registry.entity_count() == 1U);
}

TEST_CASE("M3 component lookup mutation removal and re-add obey entity lifetime", "[unit][m3][ecs][pool]") {
    Registry registry;
    const Entity entity = create_entity(registry);
    const Entity fabricated = Entity::from_parts(100U, 1U);

    CHECK_FALSE(registry.contains<Position>(entity));
    CHECK(registry.get<Position>(entity) == nullptr);
    CHECK(registry.emplace<Position>(fabricated, 3).status == EcsStatus::invalid_entity);
    CHECK(registry.remove<Position>(fabricated) == EcsStatus::invalid_entity);

    const auto added = registry.emplace<Position>(entity, 4);
    REQUIRE(added.status == EcsStatus::success);
    REQUIRE(added.component != nullptr);
    added.component->value = 7;
    CHECK(registry.contains<Position>(entity));
    CHECK(registry.get<Position>(entity)->value == 7);

    const Registry& const_registry = registry;
    static_assert(std::is_same_v<decltype(const_registry.get<Position>(entity)), const Position*>);
    REQUIRE(const_registry.get<Position>(entity) != nullptr);
    CHECK(const_registry.get<Position>(entity)->value == 7);

    REQUIRE(registry.remove<Position>(entity) == EcsStatus::success);
    CHECK_FALSE(registry.contains<Position>(entity));
    REQUIRE(registry.emplace<Position>(entity, 11).status == EcsStatus::success);
    CHECK(registry.get<Position>(entity)->value == 11);

    REQUIRE(registry.destroy(entity) == EcsStatus::success);
    CHECK(registry.get<Position>(entity) == nullptr);
    CHECK(registry.emplace<Position>(entity, 12).status == EcsStatus::invalid_entity);
    CHECK(registry.remove<Position>(entity) == EcsStatus::invalid_entity);
    const Entity reused = create_entity(registry);
    CHECK(reused.index() == entity.index());
    CHECK_FALSE(registry.contains<Position>(reused));
}

TEST_CASE("M3 duplicate emplace preserves the live component without constructing another", "[unit][m3][ecs][pool]") {
    LifetimeComponent::reset();
    {
        Registry registry;
        const Entity entity = create_entity(registry);
        REQUIRE(registry.prepare_components<LifetimeComponent>(1U) == EcsStatus::success);
        REQUIRE(registry.emplace<LifetimeComponent>(entity, 21).status == EcsStatus::success);
        const int constructions_before_duplicate = LifetimeComponent::constructions;

        CHECK(registry.emplace<LifetimeComponent>(entity, 99).status == EcsStatus::component_already_present);
        CHECK(LifetimeComponent::constructions == constructions_before_duplicate);
        REQUIRE(registry.get<LifetimeComponent>(entity) != nullptr);
        CHECK(registry.get<LifetimeComponent>(entity)->value == 21);
    }
    CHECK(LifetimeComponent::alive == 0);
}

TEST_CASE("M3 sparse-set swap removal repairs sole first middle and last entries", "[unit][m3][ecs][pool]") {
    SECTION("sole") {
        Registry registry;
        const Entity entity = create_entity(registry);
        REQUIRE(registry.emplace<Position>(entity, 1).status == EcsStatus::success);
        REQUIRE(registry.remove<Position>(entity) == EcsStatus::success);
        CHECK(registry.component_count<Position>() == 0U);
        CHECK_FALSE(registry.contains<Position>(entity));
    }

    const auto verify_removal = [](const std::size_t removed_index, const std::array<std::size_t, 3U> expected_order) {
        Registry registry;
        std::array<Entity, 4U> entities{};
        for (std::size_t index = 0U; index < entities.size(); ++index) {
            entities[index] = create_entity(registry);
            REQUIRE(registry.emplace<Position>(entities[index], static_cast<int>(index + 1U)).status ==
                    EcsStatus::success);
        }

        REQUIRE(registry.remove<Position>(entities[removed_index]) == EcsStatus::success);
        CHECK_FALSE(registry.contains<Position>(entities[removed_index]));
        std::vector<std::size_t> visited;
        REQUIRE(registry.for_each<Position>([&](const Entity entity, Position&) {
            visited.push_back(entity.index());
        }) == expected_order.size());
        REQUIRE(visited.size() == expected_order.size());
        for (std::size_t index = 0U; index < expected_order.size(); ++index) {
            CHECK(visited[index] == entities[expected_order[index]].index());
            REQUIRE(registry.get<Position>(entities[expected_order[index]]) != nullptr);
            CHECK(registry.get<Position>(entities[expected_order[index]])->value ==
                  static_cast<int>(expected_order[index] + 1U));
        }
    };

    SECTION("first") { verify_removal(0U, {3U, 1U, 2U}); }
    SECTION("middle") { verify_removal(1U, {0U, 3U, 2U}); }
    SECTION("last") { verify_removal(3U, {0U, 1U, 2U}); }
}

TEST_CASE("M3 component lifetimes balance for removal registry teardown and move-only insertion",
          "[unit][m3][ecs][pool]") {
    LifetimeComponent::reset();
    {
        Registry registry;
        REQUIRE(registry.prepare_entities(2U) == EcsStatus::success);
        REQUIRE(registry.prepare_components<LifetimeComponent>(2U) == EcsStatus::success);
        const Entity first = create_entity(registry);
        const Entity second = create_entity(registry);
        REQUIRE(registry.emplace<LifetimeComponent>(first, 1).status == EcsStatus::success);
        REQUIRE(registry.emplace<LifetimeComponent>(second, 2).status == EcsStatus::success);
        CHECK(LifetimeComponent::alive == 2);
        REQUIRE(registry.remove<LifetimeComponent>(first) == EcsStatus::success);
        CHECK(LifetimeComponent::alive == 1);
        CHECK(LifetimeComponent::destructions == 1);

        REQUIRE(registry.emplace<MoveOnly>(first, 7).status == EcsStatus::success);
        REQUIRE(registry.get<MoveOnly>(first) != nullptr);
        CHECK(*registry.get<MoveOnly>(first)->value == 7);
    }
    CHECK(LifetimeComponent::alive == 0);
    CHECK(LifetimeComponent::destructions == LifetimeComponent::constructions);
}

TEST_CASE("M3 component pools destroy in deterministic registration order", "[unit][m3][ecs][pool]") {
    std::vector<int> destruction_order;
    destruction_order.reserve(2U);
    FirstRegistered::destruction_order = &destruction_order;
    SecondRegistered::destruction_order = &destruction_order;
    {
        Registry registry;
        const Entity entity = create_entity(registry);
        REQUIRE(registry.prepare_components<FirstRegistered>(1U) == EcsStatus::success);
        REQUIRE(registry.prepare_components<SecondRegistered>(1U) == EcsStatus::success);
        REQUIRE(registry.emplace<FirstRegistered>(entity, 1).status == EcsStatus::success);
        REQUIRE(registry.emplace<SecondRegistered>(entity, 2).status == EcsStatus::success);
    }
    FirstRegistered::destruction_order = nullptr;
    SecondRegistered::destruction_order = nullptr;

    REQUIRE(destruction_order.size() == 2U);
    CHECK(destruction_order[0] == 1);
    CHECK(destruction_order[1] == 2);
}

TEST_CASE("M3 queries preserve dense order intersect pools and mutate values", "[unit][m3][ecs][query]") {
    Registry registry;
    const std::array<Entity, 3U> entities{create_entity(registry), create_entity(registry), create_entity(registry)};
    REQUIRE(registry.emplace<Position>(entities[2], 3).status == EcsStatus::success);
    REQUIRE(registry.emplace<Position>(entities[0], 1).status == EcsStatus::success);
    REQUIRE(registry.emplace<Position>(entities[1], 2).status == EcsStatus::success);
    REQUIRE(registry.emplace<Velocity>(entities[1], 20).status == EcsStatus::success);
    REQUIRE(registry.emplace<Velocity>(entities[2], 30).status == EcsStatus::success);

    std::vector<std::uint32_t> single_order;
    CHECK(registry.for_each<Position>([&](const Entity entity, Position& position) {
        single_order.push_back(entity.index());
        position.value += 10;
    }) == 3U);
    REQUIRE(single_order.size() == 3U);
    CHECK(single_order[0] == entities[2].index());
    CHECK(single_order[1] == entities[0].index());
    CHECK(single_order[2] == entities[1].index());

    std::vector<std::uint32_t> intersection_order;
    CHECK(registry.for_each<Position, Velocity>([&](const Entity entity, Position& position, Velocity& velocity) {
        intersection_order.push_back(entity.index());
        position.value += velocity.value;
    }) == 2U);
    REQUIRE(intersection_order.size() == 2U);
    CHECK(intersection_order[0] == entities[1].index());
    CHECK(intersection_order[1] == entities[2].index());
    CHECK(registry.get<Position>(entities[1])->value == 32);
    CHECK(registry.get<Position>(entities[2])->value == 43);
}

TEST_CASE("M3 equal-size query pools use registration order as the driver tie-break", "[unit][m3][ecs][query]") {
    Registry registry;
    REQUIRE(registry.prepare_components<Velocity>(2U) == EcsStatus::success);
    REQUIRE(registry.prepare_components<Position>(2U) == EcsStatus::success);
    const Entity first = create_entity(registry);
    const Entity second = create_entity(registry);
    REQUIRE(registry.emplace<Velocity>(second, 2).status == EcsStatus::success);
    REQUIRE(registry.emplace<Velocity>(first, 1).status == EcsStatus::success);
    REQUIRE(registry.emplace<Position>(first, 1).status == EcsStatus::success);
    REQUIRE(registry.emplace<Position>(second, 2).status == EcsStatus::success);

    std::vector<std::uint32_t> visited;
    CHECK(registry.for_each<Position, Velocity>(
              [&](const Entity entity, Position&, Velocity&) { visited.push_back(entity.index()); }) == 2U);
    REQUIRE(visited.size() == 2U);
    CHECK(visited[0] == second.index());
    CHECK(visited[1] == first.index());
}

TEST_CASE("M3 an absent query pool produces no visits and does not register a pool", "[unit][m3][ecs][query]") {
    Registry registry;
    const Entity entity = create_entity(registry);
    REQUIRE(registry.emplace<Position>(entity, 1).status == EcsStatus::success);
    const std::size_t pools_before = RegistryTestAccess::registered_pool_count(registry);

    CHECK(registry.for_each<Position, Health>([](Entity, Position&, Health&) {}) == 0U);
    const Registry& const_registry = registry;
    CHECK(const_registry.for_each<Health>([](Entity, const Health&) {}) == 0U);
    CHECK(RegistryTestAccess::registered_pool_count(registry) == pools_before);
    CHECK(registry.component_count<Health>() == 0U);
}

TEST_CASE("M3 nested mutable const and same-pool queries hold structural protection", "[unit][m3][ecs][query]") {
    Registry registry;
    const Entity entity = create_entity(registry);
    REQUIRE(registry.emplace<Position>(entity, 1).status == EcsStatus::success);
    REQUIRE(registry.emplace<Velocity>(entity, 2).status == EcsStatus::success);
    const Registry& const_registry = registry;
    std::size_t same_pool_visits = 0U;
    std::size_t mixed_visits = 0U;

    CHECK(registry.for_each<Position>([&](const Entity outer_entity, Position&) {
        CHECK(registry.destroy(outer_entity) == EcsStatus::iteration_active);
        same_pool_visits += registry.for_each<Position>([&](const Entity inner_entity, Position&) {
            CHECK(registry.remove<Velocity>(inner_entity) == EcsStatus::iteration_active);
            mixed_visits += const_registry.for_each<Velocity>([&](const Entity nested_entity, const Velocity&) {
                CHECK(registry.emplace<Health>(nested_entity, 5).status == EcsStatus::iteration_active);
                CHECK(registry.prepare_entities(4U) == EcsStatus::iteration_active);
            });
        });
    }) == 1U);
    CHECK(same_pool_visits == 1U);
    CHECK(mixed_visits == 1U);
    CHECK(registry.destroy(entity) == EcsStatus::success);
}

TEST_CASE("M3 query exceptions restore iteration depth and deferred commands flush only explicitly",
          "[unit][m3][ecs][query]") {
    Registry registry;
    const Entity first = create_entity(registry);
    const Entity second = create_entity(registry);
    REQUIRE(registry.emplace<Position>(first, 1).status == EcsStatus::success);
    REQUIRE(registry.emplace<Position>(second, 2).status == EcsStatus::success);

    REQUIRE_THROWS_AS(registry.for_each<Position>([](Entity, Position&) { throw std::runtime_error{"query failure"}; }),
                      std::runtime_error);
    CHECK(registry.remove<Position>(second) == EcsStatus::success);

    REQUIRE(registry.prepare_deferred_commands(1U) == EcsStatus::success);
    CHECK(registry.for_each<Position>([&](const Entity entity, Position&) {
        CHECK(registry.defer_remove<Position>(entity) == EcsStatus::success);
    }) == 1U);
    CHECK(registry.contains<Position>(first));
    CHECK(registry.deferred_count() == 1U);
    CHECK(registry.remove<Position>(first) == EcsStatus::deferred_commands_pending);
    const auto report = registry.flush_deferred();
    CHECK(report.applied == 1U);
    CHECK_FALSE(registry.contains<Position>(first));
}

TEST_CASE("M3 deferred capacity failures are isolated and staged storage is reusable", "[unit][m3][ecs][deferred]") {
    Registry registry;
    REQUIRE(registry.prepare_entities(2U) == EcsStatus::success);
    const Entity first = create_entity(registry);
    const Entity second = create_entity(registry);
    REQUIRE(registry.prepare_deferred_commands(1U) == EcsStatus::success);
    REQUIRE(registry.prepare_deferred_adds<Position>(1U) == EcsStatus::success);

    CHECK(registry.defer_remove<Position>(first) == EcsStatus::success);
    CHECK(registry.defer_add<Position>(first, Position{1}) == EcsStatus::deferred_queue_full);
    CHECK(registry.discard_deferred() == 1U);
    CHECK(registry.defer_add<Position>(first, Position{2}) == EcsStatus::success);
    CHECK(registry.defer_add<Position>(second, Position{3}) == EcsStatus::deferred_queue_full);
    CHECK(registry.flush_deferred().applied == 1U);

    REQUIRE(registry.prepare_deferred_commands(2U) == EcsStatus::success);
    REQUIRE(registry.remove<Position>(first) == EcsStatus::success);
    CHECK(registry.defer_add<Position>(first, Position{4}) == EcsStatus::success);
    CHECK(registry.defer_add<Position>(second, Position{5}) == EcsStatus::deferred_payload_full);
    CHECK(registry.discard_deferred() == 1U);
    CHECK(registry.defer_add<Health>(second, Health{6}) == EcsStatus::component_type_not_prepared);
    CHECK(registry.defer_add<Position>(second, Position{7}) == EcsStatus::success);
    CHECK(registry.flush_deferred().applied == 1U);
    CHECK(registry.get<Position>(second)->value == 7);
}

TEST_CASE("M3 discard and registry teardown destroy staged values without applying them", "[unit][m3][ecs][deferred]") {
    LifetimeComponent::reset();
    {
        Registry registry;
        const Entity entity = create_entity(registry);
        REQUIRE(registry.prepare_deferred_commands(1U) == EcsStatus::success);
        REQUIRE(registry.prepare_deferred_adds<LifetimeComponent>(1U) == EcsStatus::success);
        REQUIRE(registry.defer_add<LifetimeComponent>(entity, LifetimeComponent{1}) == EcsStatus::success);
        CHECK(LifetimeComponent::alive == 1);
        CHECK(registry.discard_deferred() == 1U);
        CHECK(LifetimeComponent::alive == 0);
        CHECK_FALSE(registry.contains<LifetimeComponent>(entity));

        REQUIRE(registry.defer_add<LifetimeComponent>(entity, LifetimeComponent{2}) == EcsStatus::success);
        CHECK(LifetimeComponent::alive == 1);
        CHECK_FALSE(registry.contains<LifetimeComponent>(entity));
    }
    CHECK(LifetimeComponent::alive == 0);
    CHECK(LifetimeComponent::destructions == LifetimeComponent::constructions);
}

TEST_CASE("M3 deferred move-only payload transfers ownership once into live storage", "[unit][m3][ecs][deferred]") {
    DeferredMoveOnly::moves = 0;
    Registry registry;
    const Entity entity = create_entity(registry);
    REQUIRE(registry.prepare_deferred_commands(1U) == EcsStatus::success);
    REQUIRE(registry.prepare_deferred_adds<DeferredMoveOnly>(1U) == EcsStatus::success);
    REQUIRE(registry.defer_add<DeferredMoveOnly>(entity, DeferredMoveOnly{42}) == EcsStatus::success);
    const int moves_after_enqueue = DeferredMoveOnly::moves;
    CHECK(moves_after_enqueue == 1);
    CHECK(registry.get<DeferredMoveOnly>(entity) == nullptr);

    const auto report = registry.flush_deferred();
    CHECK(report.applied == 1U);
    CHECK(DeferredMoveOnly::moves == moves_after_enqueue + 1);
    REQUIRE(registry.get<DeferredMoveOnly>(entity) != nullptr);
    REQUIRE(registry.get<DeferredMoveOnly>(entity)->value != nullptr);
    CHECK(*registry.get<DeferredMoveOnly>(entity)->value == 42);
}

TEST_CASE("M3 deferred add and remove conflicts apply in exact submission order", "[unit][m3][ecs][deferred]") {
    SECTION("duplicate add") {
        Registry registry;
        const Entity entity = create_entity(registry);
        REQUIRE(registry.prepare_deferred_commands(1U) == EcsStatus::success);
        REQUIRE(registry.emplace<Position>(entity, 1).status == EcsStatus::success);
        REQUIRE(registry.prepare_deferred_adds<Position>(1U) == EcsStatus::success);
        REQUIRE(registry.defer_add<Position>(entity, Position{2}) == EcsStatus::success);
        const auto report = registry.flush_deferred();
        CHECK(report.duplicate_adds == 1U);
        CHECK(registry.get<Position>(entity)->value == 1);
    }

    SECTION("missing and repeated remove") {
        Registry registry;
        const Entity entity = create_entity(registry);
        REQUIRE(registry.prepare_deferred_commands(2U) == EcsStatus::success);
        REQUIRE(registry.emplace<Position>(entity, 1).status == EcsStatus::success);
        REQUIRE(registry.defer_remove<Position>(entity) == EcsStatus::success);
        REQUIRE(registry.defer_remove<Position>(entity) == EcsStatus::success);
        const auto report = registry.flush_deferred();
        CHECK(report.applied == 1U);
        CHECK(report.missing_removes == 1U);
        CHECK_FALSE(registry.contains<Position>(entity));
    }

    SECTION("add then remove") {
        Registry registry;
        const Entity entity = create_entity(registry);
        REQUIRE(registry.prepare_deferred_commands(2U) == EcsStatus::success);
        REQUIRE(registry.prepare_deferred_adds<Position>(1U) == EcsStatus::success);
        REQUIRE(registry.defer_add<Position>(entity, Position{1}) == EcsStatus::success);
        REQUIRE(registry.defer_remove<Position>(entity) == EcsStatus::success);
        const auto report = registry.flush_deferred();
        CHECK(report.applied == 2U);
        CHECK_FALSE(registry.contains<Position>(entity));
    }

    SECTION("remove then add") {
        Registry registry;
        const Entity entity = create_entity(registry);
        REQUIRE(registry.prepare_deferred_commands(2U) == EcsStatus::success);
        REQUIRE(registry.emplace<Position>(entity, 1).status == EcsStatus::success);
        REQUIRE(registry.prepare_deferred_adds<Position>(1U) == EcsStatus::success);
        REQUIRE(registry.defer_remove<Position>(entity) == EcsStatus::success);
        REQUIRE(registry.defer_add<Position>(entity, Position{2}) == EcsStatus::success);
        const auto report = registry.flush_deferred();
        CHECK(report.applied == 2U);
        REQUIRE(registry.get<Position>(entity) != nullptr);
        CHECK(registry.get<Position>(entity)->value == 2);
    }

    SECTION("repeated add") {
        Registry registry;
        const Entity entity = create_entity(registry);
        REQUIRE(registry.prepare_deferred_commands(2U) == EcsStatus::success);
        REQUIRE(registry.prepare_deferred_adds<Position>(2U) == EcsStatus::success);
        REQUIRE(registry.defer_add<Position>(entity, Position{1}) == EcsStatus::success);
        REQUIRE(registry.defer_add<Position>(entity, Position{2}) == EcsStatus::success);
        const auto report = registry.flush_deferred();
        CHECK(report.applied == 1U);
        CHECK(report.duplicate_adds == 1U);
        REQUIRE(registry.get<Position>(entity) != nullptr);
        CHECK(registry.get<Position>(entity)->value == 1);
    }
}

TEST_CASE("M3 deferred destroy dominates later commands after applying earlier commands", "[unit][m3][ecs][deferred]") {
    Registry registry;
    const Entity entity = create_entity(registry);
    REQUIRE(registry.prepare_deferred_commands(3U) == EcsStatus::success);
    REQUIRE(registry.prepare_deferred_adds<Position>(2U) == EcsStatus::success);
    REQUIRE(registry.defer_add<Position>(entity, Position{1}) == EcsStatus::success);
    REQUIRE(registry.defer_destroy(entity) == EcsStatus::success);
    REQUIRE(registry.defer_add<Position>(entity, Position{2}) == EcsStatus::success);

    const auto report = registry.flush_deferred();
    CHECK(report.submitted == 3U);
    CHECK(report.applied == 2U);
    CHECK(report.invalid_targets == 1U);
    CHECK_FALSE(registry.valid(entity));
    CHECK(registry.get<Position>(entity) == nullptr);
}

TEST_CASE("M3 active iteration leaves deferred batches intact and pending batches block structure",
          "[unit][m3][ecs][deferred]") {
    Registry registry;
    REQUIRE(registry.prepare_entities(2U) == EcsStatus::success);
    const Entity first = create_entity(registry);
    REQUIRE(registry.emplace<Position>(first, 1).status == EcsStatus::success);
    REQUIRE(registry.prepare_deferred_commands(1U) == EcsStatus::success);
    REQUIRE(registry.defer_remove<Position>(first) == EcsStatus::success);

    CHECK(registry.for_each<Position>([&](Entity, Position&) {
        const auto report = registry.flush_deferred();
        CHECK(report.status == EcsStatus::iteration_active);
        CHECK(report.submitted == 1U);
        CHECK(registry.deferred_count() == 1U);
    }) == 1U);
    CHECK(registry.contains<Position>(first));
    CHECK(registry.create().status == EcsStatus::deferred_commands_pending);
    CHECK(registry.destroy(first) == EcsStatus::deferred_commands_pending);
    CHECK(registry.emplace<Velocity>(first, 2).status == EcsStatus::deferred_commands_pending);
    CHECK(registry.remove<Position>(first) == EcsStatus::deferred_commands_pending);

    CHECK(registry.discard_deferred() == 1U);
    CHECK(registry.remove<Position>(first) == EcsStatus::success);
    const Entity second = create_entity(registry);
    CHECK(second.index() == 1U);
}

TEST_CASE("M3 deferred command and payload storage are reusable across batches", "[unit][m3][ecs][deferred]") {
    Registry registry;
    const Entity entity = create_entity(registry);
    REQUIRE(registry.prepare_deferred_commands(1U) == EcsStatus::success);
    REQUIRE(registry.prepare_deferred_adds<Position>(1U) == EcsStatus::success);

    for (int value = 1; value <= 3; ++value) {
        REQUIRE(registry.defer_add<Position>(entity, Position{value}) == EcsStatus::success);
        const auto add_report = registry.flush_deferred();
        CHECK(add_report.applied == 1U);
        REQUIRE(registry.get<Position>(entity) != nullptr);
        CHECK(registry.get<Position>(entity)->value == value);

        REQUIRE(registry.defer_remove<Position>(entity) == EcsStatus::success);
        const auto remove_report = registry.flush_deferred();
        CHECK(remove_report.applied == 1U);
        CHECK_FALSE(registry.contains<Position>(entity));
    }
}
