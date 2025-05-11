#include "circle.h"
#include "flipbook.h"
#include "line_2d.h"
#include "sprite.h"
#include "text.h"
#include "primitives_2d.h"
#include "renderer_2d.h"
#include "entity/entity_utils.h"
#include "nit/core/engine.h"
#include "nit/entity/entity.h"
#include "nit/render/texture.h"
#include "nit/render/font.h"
#include "physics/box_collider_2d.h"
#include "physics/circle_collider.h"
#include "physics/rigidbody_2d.h"
#include "render/camera.h"
#include "render/transform.h"
#include "particles/particles.h"

namespace nit
{
    V4Verts2D vertex_positions = DEFAULT_VERTEX_POSITIONS_2D;
    V2Verts2D vertex_uvs       = DEFAULT_VERTEX_U_VS_2D;
    V4Verts2D vertex_colors    = DEFAULT_VERTEX_COLORS_2D;
    
    ListenerAction start();
    ListenerAction end();
    ListenerAction update();
    ListenerAction draw();
    
    static ListenerAction on_asset_destroyed(const AssetDestroyedArgs& args);
    static ListenerAction on_component_added(const ComponentAddedArgs& args);
    static ListenerAction on_component_removed(const ComponentRemovedArgs& args);

    void register_draw_system()
    {
        engine_event(Stage::Start)  += EngineListener::create(start);
        engine_event(Stage::End)    += EngineListener::create(end);
        engine_event(Stage::Update) += EngineListener::create(update);
        engine_event(Stage::Draw)   += EngineListener::create(draw);
        
        entity_create_group<Sprite, Transform>();
        entity_create_group<Camera, Transform>();
        entity_create_group<Circle, Transform>();
        entity_create_group<Line2D, Transform>();
        entity_create_group<Text,   Transform>();
        entity_create_group<Sprite, Transform, FlipBookAnimation>();
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

    ListenerAction update()
    {
        for (EntityID entity : entity_get_group<Sprite, Transform, FlipBookAnimation>().entities)
        {
            if (!entity_global_enabled(entity))
            {
                continue;
            }
            
            auto& animation = entity_get<FlipBookAnimation>(entity);

            if (!asset_valid(animation.flipbook) || !animation.playing)
            {
                continue;
            }

            FlipBook* flipbook = asset_get_data<FlipBook>(animation.flipbook);

            if (flipbook->key_count == 0)
            {
                continue;
            }

            animation.time += delta_seconds();

            const FlipBook::Key& current_key = flipbook->keys[animation.current_key];

            if (animation.time >= current_key.time)
            {
                auto& sprite = entity_get<Sprite>(entity);
                sprite.sub_texture = current_key.name;
                sprite.sub_texture_index = animation.current_key;
                ++animation.current_key;

                if (animation.current_key == flipbook->key_count)
                {
                    animation.time = 0.f;
                    animation.current_key = 0u;

                    if (!animation.loop)
                    {
                        animation.playing = false;
                    }
                }
            }
        }
        return ListenerAction::StayListening;
    }
    
    static ListenerAction on_asset_destroyed(const AssetDestroyedArgs& args)
    {
        if (args.asset_handle.type == type_get<Texture2D>())
        {
            
        }
        else if (args.asset_handle.type == type_get<Font>())
        {
            
        }
        
        return ListenerAction::StayListening;
    }

    static ListenerAction on_component_added(const ComponentAddedArgs& args)
    {
        if (args.type == type_get<Sprite>())
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
        else if (args.type == type_get<Text>())
        {
            auto& asset = entity_get<Text>(args.entity).font;
            asset_retarget_handle(asset);
            if (asset_valid(asset) && !asset_loaded(asset))
            {
                asset_retain(asset);
            }
        }
        else if (args.type == type_get<FlipBookAnimation>())
        {
            auto& asset = entity_get<FlipBookAnimation>(args.entity).flipbook;
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
        if (args.type == type_get<Sprite>())
        {
            auto& sprite = entity_get<Sprite>(args.entity); 
            auto& asset = sprite.texture;
            asset_retarget_handle(asset);
            
            if (asset_valid(asset) && asset_loaded(asset))
            {
                asset_release(asset);
            }
        }
        else if (args.type == type_get<Text>())
        {
            auto& asset = entity_get<Text>(args.entity).font;
            asset_retarget_handle(asset);
            if (asset_valid(asset) && asset_loaded(asset))
            {
                asset_release(asset);
            }
        }
        else if (args.type == type_get<FlipBookAnimation>())
        {
            auto& asset = entity_get<FlipBookAnimation>(args.entity).flipbook;
            asset_retarget_handle(asset);
            if (asset_valid(asset) && !asset_loaded(asset))
            {
                asset_retain(asset);
            }
        }
        return ListenerAction::StayListening;
    }
    
    static void draw_sprite(const Transform& transform, const Sprite& sprite, EntityID entity = NULL_ENTITY)
    {
        if (!sprite.visible || sprite.tint.w <= F32_EPSILON)
        {
            return;
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
                && (u32)sprite.sub_texture_index < texture_data->sub_texture_count)
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
                fill_quad_vertex_positions(size, vertex_positions);
            }
            else
            {
                vertex_positions = DEFAULT_VERTEX_POSITIONS_2D;
            }

            transform_vertex_positions(vertex_positions, transform_to_matrix(transform, entity));
        }
        else
        {
            vertex_positions = DEFAULT_VERTEX_POSITIONS_2D;
            transform_vertex_positions(vertex_positions, transform_to_matrix(transform, entity));
        }

        fill_vertex_colors(vertex_colors, sprite.tint);
        draw_quad(texture_data, vertex_positions, vertex_uvs, vertex_colors, (i32)entity);
    }

    ListenerAction draw()
    {
        // Sprite sorting

        auto& sprite_group = entity_get_group<Sprite, Transform>().entities;
        Array<EntityID> sorted_sprite_group(sprite_group.begin(), sprite_group.end());
        std::ranges::sort(sorted_sprite_group, [](EntityID a, EntityID b) ->bool {
            return entity_get<Sprite>(a).draw_layer < entity_get<Sprite>(b).draw_layer; 
        });
        
        Scene2D scene_2d
        {
            .camera           = entity_get<Camera>(get_main_camera()),
            .camera_transform = entity_get<Transform>(get_main_camera()),
            .window_size      = engine_window_size()
        };
        
        begin_scene_2d(scene_2d);
        {
            // Particles
            const ParticleRegistry* particle_registry = particle_registry_get_instance();
            const ParticleData* particle_pool = (const ParticleData*)(particle_registry->particles.elements);
            for (u32 i = 0; i < particle_registry->particles.sparse_set.count; ++i)
            {
                if (!&particle_pool[i] || !particle_pool[i].enabled) continue;

                draw_sprite(particle_pool[i].transform, particle_pool[i].sprite);
            }

            for (EntityID entity : sorted_sprite_group)
            {
                if (!entity_global_enabled(entity))
                {
                    continue;
                }
                
                auto& transform = entity_get<Transform>(entity);
                auto& sprite = entity_get<Sprite>(entity);

                draw_sprite(transform, sprite);
            }

            for (EntityID entity : entity_get_group<Line2D, Transform>().entities)
            {
                if (!entity_global_enabled(entity))
                {
                    continue;
                }
                
                auto& transform = entity_get<Transform>(entity);
                auto& line = entity_get<Line2D>(entity);

                if (!line.visible || line.tint.w <= F32_EPSILON )
                {
                    continue;
                }
                
                fill_line_2d_vertex_positions(vertex_positions, line.start, line.end, line.thickness);
                transform_vertex_positions(vertex_positions, transform_to_matrix(transform, entity));
                fill_vertex_colors(vertex_colors, line.tint);
                draw_line_2d(vertex_positions, vertex_colors, (i32) entity);
            }
            
            for (EntityID entity : entity_get_group<Text, Transform>().entities)
            {
                if (!entity_global_enabled(entity))
                {
                    continue;
                }
                
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
                    , transform_to_matrix(transform, entity)
                    , text.tint
                    , text.spacing
                    , text.size
                    , (i32) entity
                );
            }

            for (EntityID entity : entity_get_group<Circle, Transform>().entities)
            {
                if (!entity_global_enabled(entity))
                {
                    continue;
                }
                
                auto& transform = entity_get<Transform>(entity);
                auto& circle = entity_get<Circle>(entity);

                if (!circle.visible || circle.tint.w <= F32_EPSILON)
                {
                    continue;
                }
                
                fill_circle_vertex_positions(vertex_positions, circle.radius);
                transform_vertex_positions(vertex_positions, transform_to_matrix(transform, entity));
                fill_vertex_colors(vertex_colors, circle.tint);
                draw_circle(vertex_positions, vertex_colors, circle.thickness, circle.fade, (i32) entity);
            }

            // Colliders
#ifdef NIT_EDITOR_ENABLED
            if (engine_get_instance()->editor.enabled)
            {
                for (EntityID entity : entity_get_group<Transform, Rigidbody2D, BoxCollider2D>().entities)
                {
                    if (!entity_global_enabled(entity))
                    {
                        continue;
                    }
                    
                    auto& transform  = entity_get<Transform>(entity);
                    auto& collider   = entity_get<BoxCollider2D>(entity);
                    auto collider_color = collider.is_trigger ? V4_COLOR_CYAN : V4_COLOR_LIGHT_GREEN;

                    Vector3 position = transform.position + to_v3(collider.center); 
                    Vector3 rotation = transform.rotation;
                
                    draw_line_2d(position, rotation, V3_ONE,  { -0.5f * collider.size.x, -0.5f * collider.size.y }, { 0.5f * collider.size.x, -0.5f * collider.size.y }, collider_color, 0.05f / 2.f, entity);
                    draw_line_2d(position, rotation, V3_ONE,  {  0.5f * collider.size.x, -0.5f * collider.size.y }, { 0.5f * collider.size.x,  0.5f * collider.size.y }, collider_color, 0.05f / 2.f, entity);
                    draw_line_2d(position, rotation, V3_ONE,  {  0.5f * collider.size.x,  0.5f * collider.size.y }, {-0.5f * collider.size.x,  0.5f * collider.size.y }, collider_color, 0.05f / 2.f, entity);
                    draw_line_2d(position, rotation, V3_ONE,  { -0.5f * collider.size.x,  0.5f * collider.size.y }, {-0.5f * collider.size.x, -0.5f * collider.size.y }, collider_color, 0.05f / 2.f, entity);
                }
        
                for (EntityID entity : entity_get_group<Transform, Rigidbody2D, CircleCollider>().entities)
                {
                    if (!entity_global_enabled(entity))
                    {
                        continue;
                    }
                    
                    auto& transform  = entity_get<Transform>(entity);
                    auto& collider   = entity_get<CircleCollider>(entity);
                    auto collider_color = collider.is_trigger ? V4_COLOR_CYAN : V4_COLOR_LIGHT_GREEN;
                    
                    draw_circle(
                         transform.position + to_v3(collider.center),
                         transform.rotation,
                         {collider.radius * 2.f, collider.radius * 2.f, 1.f},
                         collider_color,
                         .05f,
                         .01f,
                         entity
                    );
                }
            }
#endif
        }
        end_scene_2d();

        return ListenerAction::StayListening;
    }
}