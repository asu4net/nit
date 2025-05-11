#include "collision_2d.h"
#include "circle_collider.h"
#include "collision_category.h"

#ifdef NIT_EDITOR_ENABLED
#include "editor/editor_utils.h"
#endif
#include "collision_flags.h"
#include "render/transform.h"
#include "math/vector3.h"
#include "core/engine.h"
#include "entity/entity.h"

namespace nit
{
#define COLLISION_GROUP_SIGNATURE CircleCollider, CollisionCategory

    Collisions2D* collisions_2d_instance = nullptr;

    static ListenerAction collisions_2d_end();
    static ListenerAction on_component_added(const ComponentAddedArgs& args);
    static ListenerAction on_component_removed(const ComponentRemovedArgs& args);
    static ListenerAction query_2d_collisions();
    static void handle_collision_enter(EntityID entity_A, EntityID entity_B);
    static bool circle_collides(EntityID entity_A, EntityID entity_B);

    void collisions_2d_set_instance(Collisions2D* instance)
    {
        NIT_CHECK(instance);
        collisions_2d_instance = instance;
    }

    bool collisions_2d_has_instance()
    {
        return collisions_2d_instance != nullptr;
    }

    void collisions_2d_init()
    {
        if (!collisions_2d_has_instance())
        {
            static Collisions2D instance;
            collisions_2d_set_instance(&instance);
        }
        entity_create_group<COLLISION_GROUP_SIGNATURE>();

        engine_event(Stage::LateUpdate)                                += EngineListener::create(query_2d_collisions);
        engine_event(Stage::End)                                       += EngineListener::create(collisions_2d_end);
        engine_get_instance()->entity_registry.component_added_event   += ComponentAddedListener::create(on_component_added);
        engine_get_instance()->entity_registry.component_removed_event += ComponentRemovedListener::create(on_component_removed);
    }

    static ListenerAction collisions_2d_end()
    {
        engine_event(Stage::LateUpdate)                                -= EngineListener::create(query_2d_collisions);
        engine_get_instance()->entity_registry.component_added_event   -= ComponentAddedListener::create(on_component_added);
        engine_get_instance()->entity_registry.component_removed_event -= ComponentRemovedListener::create(on_component_removed);

        return ListenerAction::StopListening;
    }

    CollisionEvent& collisions_2d_get_enter_event()
    {
        return collisions_2d_instance->enter_event;
    }

    CollisionEvent& collisions_2d_get_exit_event()
    {
        return collisions_2d_instance->exit_event;
    }

    static ListenerAction on_component_added(const ComponentAddedArgs& args)
    {
        if (args.type == type_get<CollisionCategory>())
        {
            CollisionCategory& collision_category = entity_get<CollisionCategory>(args.entity);
            AssetHandle&       asset              = collision_category.collision_flags;
            asset_retarget_handle(asset);
            if (asset_valid(asset))
            {
                if(!asset_loaded(asset))
                {
                    asset_retain(asset);
                }

                CollisionFlags* flags = asset_get_data<CollisionFlags>(asset);

                auto data = collision_flags_get_category_data(flags, collision_category.name);
                collision_category.category = data.category;
                collision_category.mask = data.mask;
            }
        }
        return ListenerAction::StayListening;
    }

    static ListenerAction on_component_removed(const ComponentRemovedArgs& args)
    {
        if (args.type == type_get<CollisionCategory>())
        {
            auto& asset = entity_get<CollisionCategory>(args.entity).collision_flags;
            asset_retarget_handle(asset);
            if (asset_valid(asset) && !asset_loaded(asset))
            {
                asset_retain(asset);
            }
        }
        return ListenerAction::StayListening;
    }


    static ListenerAction query_2d_collisions()
    {
        collisions_2d_instance->collisions_enter_events.clear();
        collisions_2d_instance->collisions_exit_events.clear();

        Array<CollisionInfo> prev_collisions = Array<CollisionInfo>(collisions_2d_instance->collisions_in_last_frame.size());

        std::copy(collisions_2d_instance->collisions_in_last_frame.begin(), collisions_2d_instance->collisions_in_last_frame.end(), back_inserter(prev_collisions));
        collisions_2d_instance->collisions_in_last_frame.clear();

        Set<EntityID>& circle_set = entity_get_group<COLLISION_GROUP_SIGNATURE>().entities;
        for (auto entity_A_it = circle_set.begin(); entity_A_it != circle_set.end(); ++entity_A_it)
        {
            if(!entity_enabled(*entity_A_it)) continue;

            for(auto entity_B_it = std::next(entity_A_it); entity_B_it != circle_set.end(); ++entity_B_it)
            {
                if (!entity_enabled(*entity_B_it)) continue;

                handle_collision_enter(*entity_A_it, *entity_B_it);
                handle_collision_enter(*entity_B_it, *entity_A_it);
            }
        }

        // Collision exit handle
        for (const CollisionInfo& prev_collision : prev_collisions)
        {
            bool found = false;
            for (const CollisionInfo& curr_collision : collisions_2d_instance->collisions_in_last_frame)
            {
                if (prev_collision.source == curr_collision.source && prev_collision.target == curr_collision.target)
                {
                    found = true;
                    break;
                }
            }

            if (found) continue;

            event_broadcast(collisions_2d_instance->exit_event, { prev_collision.source, prev_collision.target });
            collisions_2d_instance->collisions_exit_events.push_back(prev_collision);
            collisions_2d_instance->collisions_map[prev_collision.source].erase(prev_collision.target);
        }

        return ListenerAction::StayListening;
    }

    static void handle_collision_enter(EntityID entity_A, EntityID entity_B) 
    {
        const CollisionCategory& category_A = entity_get<CollisionCategory>(entity_A);
        const CollisionCategory& category_B = entity_get<CollisionCategory>(entity_B);

        if (!circle_collides(entity_A, entity_B)) return;
        if (category_A.collision_flags.id != category_B.collision_flags.id || !asset_valid(category_A.collision_flags)) return;

        if (category_A.category & category_B.mask)
        {
            if(!collisions_2d_instance->collisions_map.contains(entity_A))
            {
                collisions_2d_instance->collisions_map.insert({entity_A, UnorderedSet<EntityID>()});
            }

            collisions_2d_instance->collisions_in_last_frame.push_back({ entity_A, entity_B });

            if (!collisions_2d_instance->collisions_map[entity_A].contains(entity_B))
            {
                event_broadcast(collisions_2d_instance->enter_event, {entity_A, entity_B});
                collisions_2d_instance->collisions_enter_events.push_back({ entity_A, entity_B });
                collisions_2d_instance->collisions_map[entity_A].insert(entity_B);
            }
        }
    }

    static bool circle_collides(EntityID entity_A, EntityID entity_B)
    {
        const Transform&      transform_A       = entity_get<Transform>(entity_A);
        const Transform&      transform_B       = entity_get<Transform>(entity_B);
        const CircleCollider& collider_A        = entity_get<CircleCollider>(entity_A);
        const CircleCollider& collider_B        = entity_get<CircleCollider>(entity_B);
        Vector3               collider_center_A = { collider_A.center.x, collider_A.center.y, 0 };
        Vector3               collider_center_B = { collider_B.center.x, collider_B.center.y, 0 };

        float radius_sum = collider_A.radius + collider_B.radius;
        return sqrd_distance((transform_A.position + collider_center_A), (transform_B.position + collider_center_B)) <= radius_sum * radius_sum;
    }

}