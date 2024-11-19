﻿#include "nit_pch.h"
#include "circle_collider.h"
#include "physic_material.h"
#include "entity/entity.h"

#ifdef NIT_EDITOR_ENABLED
#include "editor/editor_utils.h"
#endif

namespace nit
{
    void serialize(const CircleCollider* collider, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << "radius"          << YAML::Value << collider->radius;
        emitter << YAML::Key << "center"          << YAML::Value << collider->center;
        emitter << YAML::Key << "physic_material" << YAML::Value << collider->physic_material;
    }
    
    void deserialize(CircleCollider* collider, const YAML::Node& node)
    {
        collider->radius          = node["radius"]            .as<f32>();
        collider->center          = node["center"]          .as<Vector2>();
        collider->physic_material = node["physic_material"] .as<AssetHandle>();
    }

#ifdef NIT_EDITOR_ENABLED
    void draw_editor(CircleCollider* collider)
    {
        editor_draw_drag_f32("Radius", collider->radius);
        editor_draw_drag_vector2("Center", collider->center);
        editor_draw_asset_combo("Physic Mat", type_get<PhysicMaterial>(), &collider->physic_material);
    }
#endif

    void register_circle_collider_component()
    {
        component_register<CircleCollider>({
            .fn_serialize   = serialize,
            .fn_deserialize = deserialize,
            NIT_IF_EDITOR_ENABLED(.fn_draw_editor = draw_editor)
        });
    }
}