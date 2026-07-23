#include "m3_component_type_identity_shared.hpp"

auto component_type_id_from_second_translation_unit() noexcept -> arpg::ecs::ComponentTypeId {
    return arpg::ecs::component_type_id<CrossTranslationUnitComponent>();
}
