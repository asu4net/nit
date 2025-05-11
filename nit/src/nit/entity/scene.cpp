#include "scene.h"
#include "core/engine.h"
#include "nit/core/asset.h"

namespace nit
{
    void register_scene_asset()
    {
        asset_register_type<Scene>({
              scene_load
            , scene_free
            , scene_serialize
            , scene_deserialize
        });
    }

    static void serialize_entities(const Scene* scene, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << "Entities" << YAML::Value << YAML::BeginMap;
        for (EntityID entity : scene->entities)
        {
            if (entity_valid(entity_get_parent(entity)))
            {
                continue;
            }
            entity_serialize(entity, emitter);
        }
        emitter << YAML::EndMap;
    }
    
    void scene_serialize(const Scene* scene, YAML::Emitter& emitter)
    {
        // More items to serialize
        serialize_entities(scene, emitter);
    }
    
    void scene_deserialize(Scene* scene, const YAML::Node& node)
    {
        StringStream ss;
        ss << node;
        scene->cached_scene = ss.str();
    }
    
    void scene_load(Scene* scene)
    {
        scene_load_entities(scene);

        event_broadcast<const Array<EntityID>&>(scene_event(SceneStage::Load), scene->entities);
    }

    void scene_free(Scene* scene)
    {
        // uuuh maybe should specify which scene was loaded/unloaded
        event_broadcast<const Array<EntityID>&>(scene_event(SceneStage::Unload), scene->entities);

        scene_free_entities(scene);
    }

    bool scene_entities_loaded(const Scene* scene)
    {
        return !scene->entities.empty();
    }

    void scene_save_entities(Scene* scene)
    {
        NIT_CHECK(scene);
        YAML::Emitter emitter;
        serialize_entities(scene, emitter);
        scene->cached_scene = emitter.c_str();
    }

    void scene_free_entities(Scene* scene)
    {
        NIT_CHECK(scene);
        if (!scene->entities.empty())
        {
            for (EntityID entity : scene->entities)
            {
                entity_destroy(entity);
            }
        }
        scene->entities.clear();
    }

    void scene_load_entities(Scene* scene)
    {
        NIT_CHECK(scene);
        const YAML::Node node = YAML::Load(scene->cached_scene);
        const YAML::Node& entities_node = node["Entities"];

        for (const auto& entity_node : entities_node)
        {
            const YAML::Node& entity_node_value = entity_node.second;
            EntityID entity = entity_deserialize(entity_node_value);
            scene->entities.push_back(entity);
        }
    }

    // me la suda
    static SceneEvent scene_events[2];

    SceneEvent& scene_event(SceneStage stage)
    {
        return scene_events[(u8)stage];
    }
}
