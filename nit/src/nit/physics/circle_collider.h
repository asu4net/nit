﻿#pragma once
#include "nit/core/asset.h"

namespace nit
{
    struct CircleCollider
    {
        bool        is_trigger      = false;
        f32         radius          = .5f;
        Vector2     center          = V2_ZERO;
        AssetHandle physic_material = {};
        f32         prev_radius     = .5f;
        void*       fixture_ptr     = nullptr;
    };

    void register_circle_collider_component();
}