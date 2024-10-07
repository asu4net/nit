﻿#pragma once
#include "nit/render/texture.h"

namespace Nit
{
    struct Sprite
    {
        bool                  visible          = true;
        Texture2D*            texture_data     = nullptr;
        ID                    texture          = 0;
        Vector4               tint             = V4_ONE;
        Vector2               size             = V2_ONE;
        bool                  flip_x           = false;
        bool                  flip_y           = false;
        Vector2               tiling_factor    = V2_ONE;
        bool                  keep_aspect      = true;
    };
}