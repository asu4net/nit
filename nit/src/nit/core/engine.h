#pragma once
#include "nit/core/window.h"
#include "nit/core/asset.h"
#include "nit/render/renderer_2d.h"
#include "nit/render/imgui_renderer.h"
#include "nit/entity/entity.h"
#include "nit/input/input_registry.h"
#include "nit/render/render_objects.h"
#include "nit/audio/audio.h"
#include "nit/physics/physics_2d.h"
#include "nit/physics/collision_2d.h"
#include "nit/particles/particles.h"

#ifdef NIT_EDITOR_ENABLED
#include "nit/editor/editor.h"
#endif

namespace nit
{
    enum class Stage : u8
    {
        Init
      , Start
      , Update
      , FixedUpdate
      , LateUpdate
      , PreDraw
      , Draw
      , PostDraw
      , End
      , Count
    };

    using EngineEvent    = Event<>;
    using EngineListener = Listener<>;
    
    struct Engine
    {
        EngineEvent    events[(u8) Stage::Count];
        
        Window           window;
        TypeRegistry     type_registry;
        RenderObjects    render_objects;
        Renderer2D       renderer_2d;
        AssetRegistry    asset_registry;
        EntityRegistry   entity_registry;
        ParticleRegistry particle_registry;
        InputRegistry    input_registry;
        AudioRegistry    audio_registry;
        Physics2D        physics_2d;
        Collisions2D     collisions_2d;
        
        NIT_IF_EDITOR_ENABLED(ImGuiRenderer  im_gui_renderer);
        NIT_IF_EDITOR_ENABLED(Editor editor);
        
        f32 delta_seconds    = 0;
        f64 seconds          = 0;
        u32 frame_count      = 0;
        f64 acc_fixed_delta  = 0;
        f64 last_time        = 0;

        f64 max_delta_time      = 1.f / 15.f;
        f64 fixed_delta_seconds = 0.0166;
    };

    void         engine_set_instance(Engine* new_engine_instance);
    Engine*      engine_get_instance();
    bool         engine_has_instance();
    f32          delta_seconds();
    f32          fixed_delta_seconds();
    EngineEvent& engine_event(Stage stage);
    void         engine_init(VoidFunc on_init = nullptr);
    Vector2      engine_window_size();
}
