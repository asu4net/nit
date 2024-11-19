﻿#include "circle.h"
#include "line_2d.h"
#include "sprite.h"
#include "text.h"
#include "primitives_2d.h"
#include "renderer_2d.h"
#include "render_api.h"
#include "nit/core/engine.h"
#include "nit/entity/entity.h"
#include "nit/render/texture.h"
#include "nit/render/font.h"
#include "render/camera.h"
#include "render/transform.h"

namespace nit
{
    V4Verts2D vertex_positions = DEFAULT_VERTEX_POSITIONS_2D;
    V2Verts2D vertex_uvs       = DEFAULT_VERTEX_U_VS_2D;
    V4Verts2D vertex_colors    = DEFAULT_VERTEX_COLORS_2D;
    
    ListenerAction start();
    ListenerAction end();
    ListenerAction draw();
    
    static ListenerAction on_asset_destroyed(const AssetDestroyedArgs& args);
    static ListenerAction on_component_added(const ComponentAddedArgs& args);
    static ListenerAction on_component_removed(const ComponentRemovedArgs& args);

    Entity get_main_camera()
    {
        auto& camera_group = entity_get_group<Camera, Transform>();
        
        if (camera_group.entities.empty())
        {
            return NULL_ENTITY;
        }

#ifdef NIT_EDITOR_ENABLED

        if (!engine_get_instance()->editor.enabled)
        {
            for (Entity entity : camera_group.entities)
            {
                if (!entity_has<EditorCameraController>(entity))
                {
                    return entity;
                }
            }
        }

        return engine_get_instance()->editor.editor_camera_entity;
        
#else
        return *camera_group.entities.begin();
#endif
    }

    void register_draw_system()
    {
        engine_event(Stage::Start)  += EngineListener::create(start);
        engine_event(Stage::End)    += EngineListener::create(end);
        engine_event(Stage::Draw)   += EngineListener::create(draw);
        
        entity_create_group<Sprite, Transform>();
        entity_create_group<Camera, Transform>();
        entity_create_group<Circle, Transform>();
        entity_create_group<Line2D, Transform>();
        entity_create_group<Text,   Transform>();
    }
    
    ListenerAction start()
    {
        engine_get_instance()->asset_registry.asset_destroyed_event    += AssetDestroyedListener::create(on_asset_destroyed);
        engine_get_instance()->entity_registry.component_added_event   += ComponentAddedListener::create(on_component_added);
        engine_get_instance()->entity_registry.component_removed_event += ComponentRemovedListener::create(on_component_removed);
        return ListenerAction::StayListening;
    }

    ListenerAction end()
    {
        engine_get_instance()->asset_registry.asset_destroyed_event    -= AssetDestroyedListener::create(on_asset_destroyed);
        engine_get_instance()->entity_registry.component_added_event   -= ComponentAddedListener::create(on_component_added);
        engine_get_instance()->entity_registry.component_removed_event -= ComponentRemovedListener::create(on_component_removed);
        return ListenerAction::StayListening;
    }
    
    static ListenerAction on_asset_destroyed(const AssetDestroyedArgs& args)
    {
        if (args.asset_handle.type == GetType<Texture2D>())
        {
            
        }
        else if (args.asset_handle.type == GetType<Font>())
        {
            
        }
        
        return ListenerAction::StayListening;
    }

    static ListenerAction on_component_added(const ComponentAddedArgs& args)
    {
        if (args.type == GetType<Sprite>())
        {
            auto& sprite = entity_get<Sprite>(args.entity); 
            auto& asset = sprite.texture;

            asset_retarget_handle(asset);

            bool is_valid = asset_valid(asset);
            
            if (is_valid && !asset_loaded(asset))
            {
                asset_retain(asset);
            }

            if (is_valid)
            {
                sprite.sub_texture_index = texture_2d_get_sub_tex_index(asset_get_data<Texture2D>(asset), sprite.sub_texture);
            }
            else
            {
                sprite.sub_texture_index = -1;
            }
        }
        else if (args.type == GetType<Text>())
        {
            auto& asset = entity_get<Text>(args.entity).font;
            asset_retarget_handle(asset);
            if (asset_valid(asset) && !asset_loaded(asset))
            {
                asset_retain(asset);
            }
        }
        return ListenerAction::StayListening;
    }

    static ListenerAction on_component_removed(const ComponentRemovedArgs& args)
    {
        if (args.type == GetType<Sprite>())
        {
            auto& sprite = entity_get<Sprite>(args.entity); 
            auto& asset = sprite.texture;
            asset_retarget_handle(asset);
            
            if (asset_valid(asset) && asset_loaded(asset))
            {
                asset_release(asset);
            }
        }
        else if (args.type == GetType<Text>())
        {
            auto& asset = entity_get<Text>(args.entity).font;
            asset_retarget_handle(asset);
            if (asset_valid(asset) && asset_loaded(asset))
            {
                asset_release(asset);
            }
        }
        return ListenerAction::StayListening;
    }
    
    ListenerAction draw()
    {

        // Sprite sorting

        auto& sprite_group = entity_get_group<Sprite, Transform>().entities;
        Array<Entity> sorted_sprite_group(sprite_group.begin(), sprite_group.end());
        std::ranges::sort(sorted_sprite_group, [](Entity a, Entity b) ->bool {
            return entity_get<Sprite>(a).draw_layer < entity_get<Sprite>(b).draw_layer; 
        });
        
        clear_screen();
        
        Entity main_camera = get_main_camera();

        Camera& camera = entity_get<Camera>(main_camera);

        i32 width, height;
        window_retrieve_size(&width, &height);
        
#ifdef NIT_EDITOR_ENABLED
        if (engine_get_instance()->editor.enabled && engine_get_instance()->editor.show_viewport)
        {
            clear_attachment(&engine_get_instance()->editor.frame_buffer, 1, -1);
            width  = engine_get_instance()->editor.frame_buffer.width;
            height = engine_get_instance()->editor.frame_buffer.height;
        }
#endif

        set_viewport(0, 0, width, height);
        
        camera.aspect = (f32) width / (f32) height;   
        
        if (isnan(camera.aspect))
        {
            return ListenerAction::StayListening;;
        }

        set_depth_test_enabled(camera.projection == CameraProjection::Perspective);
        
        begin_scene_2d(camera_proj_view(camera, entity_get<Transform>(main_camera)));
        {
            for (Entity entity : sorted_sprite_group)
            {
                auto& transform = entity_get<Transform>(entity);
                auto& sprite = entity_get<Sprite>(entity);

                if (!sprite.visible || sprite.tint.w <= F32_EPSILON)
                {
                    continue;
                }

                bool has_texture = asset_valid(sprite.texture); 

                Texture2D* texture_data = has_texture ? asset_get_data<Texture2D>(sprite.texture) : nullptr; 
                
                if (has_texture)
                {
                    if (!asset_loaded(sprite.texture))
                    {
                        asset_retain(sprite.texture);
                    }
                    
                    Vector2 size = texture_data->size;

                    if (texture_data->sub_textures
                        && sprite.sub_texture_index >= 0
                        && (u32) sprite.sub_texture_index < texture_data->sub_texture_count)
                    {
                        const SubTexture2D& sub_texture = texture_data->sub_textures[sprite.sub_texture_index];
                        size = sub_texture.size;
                        
                        fill_quad_vertex_u_vs(
                              vertex_uvs
                            , texture_data->size
                            , sub_texture.size
                            , sub_texture.location
                            , sprite.flip_x
                            , sprite.flip_y
                            , sprite.tiling_factor);
                    }
                    else
                    {
                        vertex_uvs = DEFAULT_VERTEX_U_VS_2D;
                        
                        fill_quad_vertex_u_vs(
                          vertex_uvs
                        , sprite.flip_x
                        , sprite.flip_y
                        , sprite.tiling_factor);
                    }

                    if (sprite.keep_aspect)
                    {
                        fill_quad_vertex_positions(size , vertex_positions);
                    }
                    else
                    {
                        vertex_positions = DEFAULT_VERTEX_POSITIONS_2D;
                    }
                    
                    transform_vertex_positions(vertex_positions, transform_to_matrix(transform));
                }
                else
                {
                    vertex_positions = DEFAULT_VERTEX_POSITIONS_2D;
                    transform_vertex_positions(vertex_positions, transform_to_matrix(transform));
                }
                
                fill_vertex_colors(vertex_colors, sprite.tint);
                draw_quad(texture_data, vertex_positions, vertex_uvs, vertex_colors, (i32) entity);
            }

            for (Entity entity : entity_get_group<Line2D, Transform>().entities)
            {
                auto& transform = entity_get<Transform>(entity);
                auto& line = entity_get<Line2D>(entity);

                if (!line.visible || line.tint.w <= F32_EPSILON )
                {
                    continue;
                }
                
                fill_line_2d_vertex_positions(vertex_positions, line.start, line.end, line.thickness);
                transform_vertex_positions(vertex_positions, transform_to_matrix(transform));
                fill_vertex_colors(vertex_colors, line.tint);
                draw_line_2d(vertex_positions, vertex_colors, (i32) entity);
            }

            
            for (Entity entity : entity_get_group<Text, Transform>().entities)
            {
                auto& transform = entity_get<Transform>(entity);
                auto& text = entity_get<Text>(entity);
                Font* font_data = asset_valid(text.font) ? asset_get_data<Font>(text.font) : nullptr;

                if (font_data && !asset_loaded(text.font))
                {
                    asset_retain(text.font);
                }
                
                if (!text.visible || text.text.empty() || !font_data)
                {
                    continue;
                }
                
                draw_text(
                      font_data
                    , text.text
                    , transform_to_matrix(transform)
                    , text.tint
                    , text.spacing
                    , text.size
                    , (i32) entity
                );
            }

            for (Entity entity : entity_get_group<Circle, Transform>().entities)
            {
                auto& transform = entity_get<Transform>(entity);
                auto& circle = entity_get<Circle>(entity);

                if (!circle.visible || circle.tint.w <= F32_EPSILON)
                {
                    continue;
                }
                
                fill_circle_vertex_positions(vertex_positions, circle.radius);
                transform_vertex_positions(vertex_positions, transform_to_matrix(transform));
                fill_vertex_colors(vertex_colors, circle.tint);
                draw_circle(vertex_positions, vertex_colors, circle.thickness, circle.fade, (i32) entity);
            }
        }
        end_scene_2d();

        return ListenerAction::StayListening;
    }
}