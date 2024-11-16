﻿#include "entity.h"

namespace nit
{
#define NIT_CHECK_ENTITY_REGISTRY_CREATED NIT_CHECK(entity_registry)
    
    EntityRegistry* entity_registry = nullptr;

    void entity_registry_set_instance(EntityRegistry* entity_registry_instance)
    {
        NIT_CHECK(entity_registry_instance);
        entity_registry = entity_registry_instance;
    }

    EntityRegistry* entity_registry_get_instance()
    {
        NIT_CHECK_ENTITY_REGISTRY_CREATED
        return entity_registry;
    }

    ComponentPool* entity_find_component_pool(const Type* type)
    {
        NIT_CHECK_ENTITY_REGISTRY_CREATED
        for (u32 i = 0; i < entity_registry->next_component_type_index; ++i)
        {
            ComponentPool& component_pool = entity_registry->component_pool[i];
            if (component_pool.data_pool.type == type)
            {
                return &component_pool;
            }
        }
        return nullptr;
    }

    Entity entity_clone(Entity entity)
    {
        if (!entity_valid(entity))
        {
            return NULL_ENTITY;
        }

        Entity cloned_entity = entity_create();

        for (u32 i = 0; i < entity_registry->next_component_type_index - 1; ++i)
        {
            ComponentPool* pool = &entity_registry->component_pool[i];
            if (!delegate_invoke(pool->fn_is_in_entity, entity))
            {
                continue;
            }
            void* component_data = delegate_invoke(pool->fn_get_from_entity, entity);
            delegate_invoke(pool->fn_add_to_entity, cloned_entity, component_data);
        }

        return cloned_entity;
    }

    EntitySignature entity_get_signature(Entity entity)
    {
        NIT_CHECK_ENTITY_REGISTRY_CREATED
        if (entity_valid(entity))
        {
            NIT_CHECK_MSG(false, "Trying to get signature from non existent entity!");
            return {};
        }
        return entity_registry->signatures[entity];
    }

    void entity_registry_init()
    {
        NIT_CHECK_ENTITY_REGISTRY_CREATED

        entity_registry->signatures = new EntitySignature[entity_registry->max_entities];
        entity_registry->component_pool = new ComponentPool[NIT_MAX_COMPONENT_TYPES];
        
        for (u32 i = 0; i < entity_registry->max_entities; ++i)
        {
            entity_registry->available_entities.push(i);
        }
    }

    void entity_registry_finish()
    {
        NIT_CHECK_ENTITY_REGISTRY_CREATED
        for (u32 i = 0; i < entity_registry->next_component_type_index; ++i)
        {
            ComponentPool& data = entity_registry->component_pool[i];
            pool_free(&data.data_pool);
        }
    }

    Entity entity_create()
    {
        NIT_CHECK_ENTITY_REGISTRY_CREATED
        NIT_CHECK_MSG(entity_registry->entity_count < entity_registry->max_entities, "Entity limit reached!");
        Entity entity = entity_registry->available_entities.front();
        entity_registry->available_entities.pop();
        ++entity_registry->entity_count;
        entity_registry->signatures[entity].set(0, true);
        return entity;
    }

    void entity_destroy(Entity entity)
    {
        NIT_CHECK_ENTITY_REGISTRY_CREATED
        NIT_CHECK_MSG(entity_valid(entity), "Entity is not valid!");

        for (u32 i = 0; i < entity_registry->next_component_type_index; ++i)
        {
            ComponentPool& component_pool = entity_registry->component_pool[i];

            if (!entity_registry->signatures[entity].test(i + 1))
            {
                continue;
            }

            event_broadcast<const ComponentRemovedArgs&>(entity_registry->component_removed_event, {entity, component_pool.data_pool.type});
            pool_delete_data(&component_pool.data_pool, entity);
        }

        entity_registry->signatures[entity].reset();
        entity_registry->available_entities.push(entity);
        
        --entity_registry->entity_count;
        entity_registry->signatures[entity].set(0, false);

        for (auto& [signature, group] : entity_registry->entity_groups)
        {
            group.entities.erase(entity);
        }
    }

    bool entity_valid(const Entity entity)
    {
        NIT_CHECK_ENTITY_REGISTRY_CREATED
        return entity < entity_registry->max_entities && entity_registry->signatures[entity].test(0);
    }

    void entity_signature_changed(Entity entity, EntitySignature new_entity_signature)
    {
        NIT_CHECK_ENTITY_REGISTRY_CREATED
        for (auto& [signature, group] : entity_registry->entity_groups)
        {
            if ((signature | new_entity_signature) == new_entity_signature)
            {
                group.entities.insert(entity);
                continue;
            }

            group.entities.erase(entity);
        }
    }

    EntitySignature entity_create_group(const Array<u64>& type_hashes)
    {
        NIT_CHECK_ENTITY_REGISTRY_CREATED
        NIT_CHECK_MSG(!entity_registry->entity_count, "Create the group before any entity gets created!");
        EntitySignature group_signature = entity_build_signature(type_hashes);

        if (entity_registry->entity_groups.count(group_signature) != 0)
        {
            return group_signature;
        }
        
        EntityGroup* group = &entity_registry->entity_groups[group_signature];
        group->signature = group_signature;
        return group_signature;
    }

    EntitySignature entity_build_signature(const Array<u64>& type_hashes)
    {
        NIT_CHECK_ENTITY_REGISTRY_CREATED
        EntitySignature group_signature;
        group_signature.set(0, true);
        for (u64 type_hash : type_hashes)
        {
            if (ComponentPool* pool = entity_find_component_pool(GetType(type_hash)))
            {
                group_signature.set(pool->type_index, true);
            }
        }
        return group_signature;
    }

    EntityGroup& entity_get_group(EntitySignature signature)
    {
        NIT_CHECK_ENTITY_REGISTRY_CREATED
        return entity_registry->entity_groups[signature];
    }

    void entity_serialize(Entity entity, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << "Entity" << YAML::Value << YAML::BeginMap;
        
        for (u8 i = 0; i < entity_registry->next_component_type_index - 1; ++i)
        {
            auto& component_pool = entity_registry->component_pool[i];
            auto& data_pool = component_pool.data_pool;
            
            if (!data_pool.type->fn_invoke_deserialize
                || !data_pool.type->fn_invoke_serialize
                || !delegate_invoke(component_pool.fn_is_in_entity, entity))
            {
                continue;
            }

            emitter << YAML::Key << data_pool.type->name << YAML::Value << YAML::BeginMap;
                
            void* raw_data = pool_get_raw_data(&data_pool, entity);
            serialize(data_pool.type, raw_data, emitter);
                
            emitter << YAML::EndMap;
        }

        emitter << YAML::EndMap;
    }

    Entity entity_deserialize(const YAML::Node& node)
    {
        if (!node)
        {
            return 0;
        }

        Entity entity = entity_create();
        
        for (const auto& entity_node_child : node)
        {
            const YAML::Node& component_node = entity_node_child.second;
            String type_name = entity_node_child.first.as<String>();
            auto* component_pool = entity_find_component_pool(GetType(type_name));
            auto& data_pool = component_pool->data_pool;
            void* null_data = nullptr;
            delegate_invoke(component_pool->fn_add_to_entity, entity, null_data);
            void* component_data = delegate_invoke(component_pool->fn_get_from_entity, entity);
            deserialize(data_pool.type, component_data, component_node);
            ComponentAddedArgs args;
            args.entity = entity;
            args.type = component_pool->data_pool.type;
            event_broadcast<const ComponentAddedArgs&>(entity_registry_get_instance()->component_added_event, args);
        }
        
        return entity;
    }
}
