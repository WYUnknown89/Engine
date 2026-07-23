#pragma once

#include "arpg/ecs/component_type_id.hpp"

struct CrossTranslationUnitComponent {
    int value{0};
};

struct DistinctTranslationUnitComponent {
    int value{0};
};

[[nodiscard]] auto component_type_id_from_second_translation_unit() noexcept -> arpg::ecs::ComponentTypeId;
