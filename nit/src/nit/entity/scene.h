#pragma once
#include "entity.h"

namespace nit
{
    enum class SceneStage : u8
    {
          Load
        , Unload
    };

    using SceneEvent = Event<const Array<EntityID>&>;
    using SceneListener = Listener<const Array<EntityID>&>;

    struct Scene
    {
        String cached_scene;
        Array<EntityID> entities;
    };
    
    void register_scene_asset();
    void scene_serialize(const Scene* scene, YAML::Emitter& emitter);
    void scene_deserialize(Scene* scene, const YAML::Node& node);
    void scene_load(Scene* scene);
    void scene_free(Scene* scene);
    bool scene_entities_loaded(const Scene* scene);
    void scene_save_entities(Scene* scene);
    void scene_free_entities(Scene* scene);
    void scene_load_entities(Scene* scene);
    SceneEvent& scene_event(SceneStage stage);
}
