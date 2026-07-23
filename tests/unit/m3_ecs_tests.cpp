#include "arpg/ecs/registry.hpp"
#include "m3_component_type_identity_shared.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <limits>
#include <memory>
#include <utility>

namespace {

struct Position {
    int value{0};
};

struct Velocity {
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

} // namespace

TEST_CASE("M3 ComponentTypeId is process-wide across translation units", "[unit][m3][ecs][identity]") {
    const auto local = arpg::ecs::component_type_id<CrossTranslationUnitComponent>();
    const auto remote = component_type_id_from_second_translation_unit();
    const auto distinct = arpg::ecs::component_type_id<DistinctTranslationUnitComponent>();

    CHECK(local == remote);
    CHECK(local != distinct);
}

TEST_CASE("M3 entities reject generation zero and reuse FIFO slots", "[unit][m3][ecs][entity]") {
    arpg::ecs::Registry registry;
    CHECK_FALSE(registry.valid({}));
    CHECK_FALSE(registry.valid(arpg::ecs::Entity::from_parts(17U, 0U)));

    const auto first = registry.create();
    const auto second = registry.create();
    REQUIRE(first.status == arpg::ecs::EcsStatus::success);
    REQUIRE(second.status == arpg::ecs::EcsStatus::success);
    CHECK(first.entity.generation() == 1U);
    REQUIRE(registry.destroy(first.entity) == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.destroy(second.entity) == arpg::ecs::EcsStatus::success);
    CHECK_FALSE(registry.valid(first.entity));

    const auto reused_first = registry.create();
    const auto reused_second = registry.create();
    CHECK(reused_first.entity.index() == first.entity.index());
    CHECK(reused_first.entity.generation() == 2U);
    CHECK(reused_second.entity.index() == second.entity.index());
    CHECK_FALSE(registry.valid(first.entity));
}

TEST_CASE("M3 component operations preserve dense sparse membership", "[unit][m3][ecs][pool]") {
    arpg::ecs::Registry registry;
    const auto first = registry.create().entity;
    const auto second = registry.create().entity;
    const auto third = registry.create().entity;

    REQUIRE(registry.emplace<Position>(first, 1).status == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.emplace<Position>(second, 2).status == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.emplace<Position>(third, 3).status == arpg::ecs::EcsStatus::success);
    CHECK(registry.emplace<Position>(first, 9).status == arpg::ecs::EcsStatus::component_already_present);
    REQUIRE(registry.remove<Position>(second) == arpg::ecs::EcsStatus::success);
    CHECK_FALSE(registry.contains<Position>(second));
    REQUIRE(registry.get<Position>(third) != nullptr);
    CHECK(registry.get<Position>(third)->value == 3);
    CHECK(registry.remove<Position>(second) == arpg::ecs::EcsStatus::component_missing);
}

TEST_CASE("M3 supports move-only immediate and deferred components", "[unit][m3][ecs][deferred]") {
    arpg::ecs::Registry registry;
    REQUIRE(registry.prepare_entities(2U) == arpg::ecs::EcsStatus::success);
    const auto entity = registry.create().entity;
    REQUIRE(registry.emplace<MoveOnly>(entity, 7).status == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.remove<MoveOnly>(entity) == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.prepare_deferred_commands(2U) == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.prepare_deferred_adds<MoveOnly>(1U) == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.defer_add<MoveOnly>(entity, MoveOnly{42}) == arpg::ecs::EcsStatus::success);
    CHECK(registry.get<MoveOnly>(entity) == nullptr);
    const auto report = registry.flush_deferred();
    CHECK(report.applied == 1U);
    REQUIRE(registry.get<MoveOnly>(entity) != nullptr);
    CHECK(*registry.get<MoveOnly>(entity)->value == 42);
}

TEST_CASE("M3 queries select the smallest pool and defer structural changes", "[unit][m3][ecs][query]") {
    arpg::ecs::Registry registry;
    REQUIRE(registry.prepare_entities(4U) == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.prepare_components<Position>(4U) == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.prepare_components<Velocity>(2U) == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.prepare_deferred_commands(2U) == arpg::ecs::EcsStatus::success);
    const auto first = registry.create().entity;
    const auto second = registry.create().entity;
    const auto third = registry.create().entity;
    REQUIRE(registry.emplace<Position>(first, 1).status == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.emplace<Position>(second, 2).status == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.emplace<Position>(third, 3).status == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.emplace<Velocity>(third, 30).status == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.emplace<Velocity>(first, 10).status == arpg::ecs::EcsStatus::success);

    std::uint32_t first_seen = std::numeric_limits<std::uint32_t>::max();
    const std::size_t visited = registry.for_each<Position, Velocity>(
        [&](const arpg::ecs::Entity entity, Position& position, Velocity& velocity) {
            if (first_seen == std::numeric_limits<std::uint32_t>::max()) {
                first_seen = entity.index();
            }
            position.value += velocity.value;
            CHECK(registry.destroy(entity) == arpg::ecs::EcsStatus::iteration_active);
            CHECK(registry.defer_remove<Velocity>(entity) == arpg::ecs::EcsStatus::success);
        });
    CHECK(visited == 2U);
    CHECK(first_seen == third.index());
    CHECK(registry.deferred_count() == 2U);
    CHECK(registry.flush_deferred().applied == 2U);
    CHECK(registry.for_each<Position, Velocity>([](auto, auto&, auto&) {}) == 0U);
}

TEST_CASE("M3 deferred destruction dominates later commands and flush is explicit", "[unit][m3][ecs][deferred]") {
    arpg::ecs::Registry registry;
    REQUIRE(registry.prepare_entities(1U) == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.prepare_deferred_commands(2U) == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.prepare_deferred_adds<Position>(1U) == arpg::ecs::EcsStatus::success);
    const auto entity = registry.create().entity;
    REQUIRE(registry.defer_destroy(entity) == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.defer_add<Position>(entity, Position{1}) == arpg::ecs::EcsStatus::success);
    CHECK(registry.valid(entity));
    const auto report = registry.flush_deferred();
    CHECK_FALSE(registry.valid(entity));
    CHECK(report.invalid_targets == 1U);
    CHECK(registry.get<Position>(entity) == nullptr);
}

TEST_CASE("M3 prepared deferred additions enforce live-component capacity", "[unit][m3][ecs][deferred]") {
    arpg::ecs::Registry registry;
    REQUIRE(registry.prepare_entities(3U) == arpg::ecs::EcsStatus::success);
    const auto first = registry.create().entity;
    const auto second = registry.create().entity;
    REQUIRE(registry.prepare_components<Position>(2U) == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.emplace<Position>(first, 1).status == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.prepare_deferred_commands(2U) == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.prepare_deferred_adds<Position>(1U) == arpg::ecs::EcsStatus::success);
    REQUIRE(registry.emplace<Position>(second, 2).status == arpg::ecs::EcsStatus::success);
    CHECK(registry.defer_add<Position>(first, Position{3}) == arpg::ecs::EcsStatus::component_capacity_exhausted);

    REQUIRE(registry.remove<Position>(second) == arpg::ecs::EcsStatus::success);
    const auto queued = registry.defer_add<Position>(second, Position{4});
    const auto report = registry.flush_deferred();
    static_cast<void>(registry.for_each<Position>([](auto, Position& position) { ++position.value; }));
    CHECK(queued == arpg::ecs::EcsStatus::success);
    CHECK(report.applied == 1U);
}

TEST_CASE("M3 const registry query exposes const component references", "[unit][m3][ecs][query]") {
    arpg::ecs::Registry mutable_registry;
    const auto entity = mutable_registry.create().entity;
    REQUIRE(mutable_registry.emplace<Position>(entity, 9).status == arpg::ecs::EcsStatus::success);
    const arpg::ecs::Registry& registry = mutable_registry;
    int observed = 0;
    CHECK(registry.for_each<Position>(
              [&](const arpg::ecs::Entity, const Position& position) { observed = position.value; }) == 1U);
    CHECK(observed == 9);
}

TEST_CASE("M3 const iteration protects a captured mutable registry", "[unit][m3][ecs][query]") {
    arpg::ecs::Registry mutable_registry;
    const auto entity = mutable_registry.create().entity;
    REQUIRE(mutable_registry.emplace<Position>(entity, 9).status == arpg::ecs::EcsStatus::success);
    REQUIRE(mutable_registry.prepare_deferred_commands(1U) == arpg::ecs::EcsStatus::success);
    const arpg::ecs::Registry& registry = mutable_registry;
    CHECK(registry.for_each<Position>([&](const arpg::ecs::Entity traversed, const Position&) {
        CHECK(mutable_registry.destroy(traversed) == arpg::ecs::EcsStatus::iteration_active);
        CHECK(mutable_registry.defer_remove<Position>(traversed) == arpg::ecs::EcsStatus::success);
        CHECK(registry.for_each<Position>([](const auto, const Position&) {}) == 1U);
    }) == 1U);
    CHECK(mutable_registry.flush_deferred().applied == 1U);
    CHECK_FALSE(mutable_registry.contains<Position>(entity));
}
