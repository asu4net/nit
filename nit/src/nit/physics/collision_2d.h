#pragma once
#include "../entity/entity.h"

namespace nit
{
    struct CollisionInfo
    {
        EntityID source;
        EntityID target;
    };

    using CollisionEvent    = Event<CollisionInfo>;
    using CollisionListener = Listener<CollisionInfo>;

    struct Collisions2D
    {
        CollisionEvent                        enter_event;
        CollisionEvent                        exit_event;
        Array<CollisionInfo>                  collisions_in_last_frame;
        Array<CollisionInfo>                  collisions_enter_events;
        Array<CollisionInfo>                  collisions_exit_events;
        Map<EntityID, UnorderedSet<EntityID>> collisions_map;
    };

    void collisions_2d_set_instance(Collisions2D* instance); 
    bool collisions_2d_has_instance();
    void collisions_2d_init();
    CollisionEvent& collisions_2d_get_enter_event();
    CollisionEvent& collisions_2d_get_exit_event();
}