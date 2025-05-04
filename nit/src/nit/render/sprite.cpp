#include "nit_pch.h"
#include "sprite.h"
#include "texture.h"
#include "entity/entity.h"

#ifdef NIT_EDITOR_ENABLED
#include "editor/editor_utils.h"
#endif

namespace nit
{
    void serialize(const Sprite* sprite, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << "texture"       << YAML::Value << sprite->texture;
        emitter << YAML::Key << "sub_texture"   << YAML::Value << sprite->sub_texture;
        emitter << YAML::Key << "visible"       << YAML::Value << sprite->visible;
        emitter << YAML::Key << "tint"          << YAML::Value << sprite->tint;
        emitter << YAML::Key << "size"          << YAML::Value << sprite->size;
        emitter << YAML::Key << "flip_x"        << YAML::Value << sprite->flip_x;
        emitter << YAML::Key << "flip_y"        << YAML::Value << sprite->flip_y;
        emitter << YAML::Key << "tiling_factor" << YAML::Value << sprite->tiling_factor;
        emitter << YAML::Key << "keep_aspect"   << YAML::Value << sprite->keep_aspect;
        emitter << YAML::Key << "draw_layer"    << YAML::Value << sprite->draw_layer;
    }

    void deserialize(Sprite* sprite, const YAML::Node& node)
    {
        sprite->texture       = node["texture"]       .as<AssetHandle>();
        sprite->sub_texture   = node["sub_texture"]   .as<String>();
        sprite->visible       = node["visible"]       .as<bool>();
        sprite->tint          = node["tint"]          .as<Vector4>();
        sprite->size          = node["size"]          .as<Vector2>();
        sprite->flip_x        = node["flip_x"]        .as<bool>();
        sprite->flip_y        = node["flip_y"]        .as<bool>();
        sprite->tiling_factor = node["tiling_factor"] .as<Vector2>();
        sprite->keep_aspect   = node["keep_aspect"]   .as<bool>();
        sprite->draw_layer    = node["draw_layer"]    .as<i32>();
    }

#ifdef NIT_EDITOR_ENABLED
    void draw_editor(Sprite* sprite)
    {
        editor_draw_asset_combo("Texture", type_get<Texture2D>(), &sprite->texture);

        if (asset_valid(sprite->texture))
        {
            Texture2D* texture = asset_get_data<Texture2D>(sprite->texture);

            if (texture->sub_texture_count > 0)
            {
                String        sub_texture = sprite->sub_texture;
                Array<String> sub_textures;
                
                sub_textures.emplace_back("None");
                for (u32 i = 0; i < texture->sub_texture_count; ++i) sub_textures.push_back(texture->sub_textures[i].name);
                editor_draw_combo("Sub Texture", sub_texture, sub_textures);
                
                sub_texture = sub_texture == "None" ? "" : sub_texture; 
                sprite->sub_texture = sub_texture;
            }
            
            sprite_set_sub_texture(*sprite, sprite->sub_texture);
        }
        else
        {
            sprite->sub_texture_index = -1;
        }
        
        editor_draw_bool("Visible", sprite->visible);
        editor_draw_color_palette("Tint", sprite->tint);
        editor_draw_drag_vector2("Size", sprite->size);
        editor_draw_bool("Flip X", sprite->flip_x);
        editor_draw_bool("Flip Y", sprite->flip_y);
        editor_draw_drag_vector2("Tiling Factor", sprite->tiling_factor);
        editor_draw_bool("Keep Aspect", sprite->keep_aspect);
        editor_draw_drag_i32("Draw Layer", sprite->draw_layer);
    }
#endif

    void register_sprite_component()
    {
        component_register<Sprite>({
            .fn_serialize   = serialize,
            .fn_deserialize = deserialize,
            NIT_IF_EDITOR_ENABLED(.fn_draw_editor = draw_editor)
        });
    }

    void sprite_set_sub_texture(Sprite& sprite, const String& sub_texture)
    {
        sprite.sub_texture = sub_texture;
        
        if (!asset_valid(sprite.texture))
        {
            sprite.sub_texture_index = -1;
            return;
        }

        i32 index = texture_2d_get_sub_tex_index(asset_get_data<Texture2D>(sprite.texture), sub_texture);
        sprite.sub_texture_index = index;
    }

    void sprite_reset_sub_texture(Sprite& sprite)
    {
        sprite.sub_texture = "";
        sprite.sub_texture_index = -1;
    }
}