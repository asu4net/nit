#include "particles.h"
#include "editor/editor_utils.h"
#include "core/engine.h"

namespace nit
{

    /////////////////////////////
    //:Particle Emitter
    /////////////////////////////

    void
    serialize(const ParticleEmitter* particle_emitter, YAML::Emitter& emitter)
    {
        //emitter << YAML::Key << "num_textures"   << YAML::Value << particle_emitter->num_textures;
        emitter << YAML::Key << "texture"        << YAML::Value << particle_emitter->texture;
        emitter << YAML::Key << "pos_amplitude"  << YAML::Value << particle_emitter->pos_amplitude;
        emitter << YAML::Key << "scale"          << YAML::Value << particle_emitter->scale;
        emitter << YAML::Key << "velocity"       << YAML::Value << particle_emitter->velocity;
        emitter << YAML::Key << "vel_amplitude"  << YAML::Value << particle_emitter->vel_amplitude;
        emitter << YAML::Key << "tint"           << YAML::Value << particle_emitter->tint;
        emitter << YAML::Key << "tint_amplitude" << YAML::Value << particle_emitter->tint_amplitude;
        emitter << YAML::Key << "starting_life"  << YAML::Value << particle_emitter->starting_life;
        emitter << YAML::Key << "cooldown"       << YAML::Value << particle_emitter->cooldown;
    }

    void
    deserialize(ParticleEmitter* particle_emitter, const YAML::Node& node)
    {
        //particle_emitter->num_textures   = node["num_textures"].as<u8>();
        particle_emitter->texture        = node["texture"].as<AssetHandle>();
        particle_emitter->pos_amplitude  = node["pos_amplitude"].as<Vector3>();
        particle_emitter->velocity       = node["velocity"].as<Vector3>();
        particle_emitter->scale          = node["scale"].as<Vector3>();
        particle_emitter->vel_amplitude  = node["vel_amplitude"].as<Vector3>();
        particle_emitter->tint           = node["tint"].as<Vector4>();
        particle_emitter->tint_amplitude = node["tint_amplitude"].as<Vector4>();
        particle_emitter->starting_life  = node["starting_life"].as<f32>();
        particle_emitter->cooldown       = node["cooldown"].as<f32>();
    }

    #ifdef NIT_EDITOR_ENABLED
    void
    draw_editor(ParticleEmitter* particle_emitter)
    {
        editor_draw_bool("Enabled", particle_emitter->enabled);
        // editor_draw_drag_u32("Num Textures", particle_emitter->num_textures);
        editor_draw_asset_combo("Texture", type_get<Texture2D>(), &particle_emitter->texture);
        editor_draw_drag_vector3("Pos Amplitude", particle_emitter->pos_amplitude);
        editor_draw_drag_vector3("Scale", particle_emitter->scale);
        editor_draw_drag_vector3("Velocity", particle_emitter->velocity);
        editor_draw_drag_vector3("Vel Amplitude", particle_emitter->vel_amplitude);
        editor_draw_color_palette("Tint", particle_emitter->tint);
        editor_draw_color_palette("Tint Amplitude", particle_emitter->tint_amplitude);
        editor_draw_drag_f32("Starting Life", particle_emitter->starting_life);
        editor_draw_drag_f32("Cooldown", particle_emitter->cooldown);
    }
    #endif

    void
    register_particle_emitter_component()
    {
        TypeArgs<ParticleEmitter> args;
        args.fn_serialize = serialize;
        args.fn_deserialize = deserialize;
        NIT_IF_EDITOR_ENABLED(args.fn_draw_editor = draw_editor);
        component_register<ParticleEmitter>(args);
        entity_create_group<ParticleEmitter>();
    }

    /////////////////////////////
    //:Particle Registry
    /////////////////////////////

    void spawn_particle(EntityID emitter);

#define NIT_CHECK_PARTICLE_REGISTRY_CREATED NIT_CHECK(particle_registry)
    ParticleRegistry* particle_registry = nullptr;
    static ListenerAction particle_registry_update();

    void
    particle_registry_set_instance(ParticleRegistry* particle_registry_instance)
    {
        NIT_CHECK(particle_registry_instance);
        particle_registry = particle_registry_instance;
    }

    ParticleRegistry*
    particle_registry_get_instance()
    {
        NIT_CHECK_PARTICLE_REGISTRY_CREATED
        return particle_registry;
    }

    void
    particle_registry_init()
    {
        NIT_CHECK_PARTICLE_REGISTRY_CREATED

        pool_load<ParticleData>(&particle_registry->particles, particle_registry->max_particles);
        engine_event(Stage::Update) += EngineListener::create(particle_registry_update);
    }

    void
    particle_registry_finish()
    {
        NIT_CHECK_PARTICLE_REGISTRY_CREATED
    }

    ParticleID
    particle_create()
    {
        NIT_CHECK_PARTICLE_REGISTRY_CREATED
        NIT_CHECK_MSG(particle_registry->particle_count < particle_registry->max_particles, "Particle limit reached!");
        ParticleID particle; pool_insert_data(&particle_registry->particles, particle);
        ++particle_registry->particle_count;
        ParticleData* data = pool_get_data<ParticleData>(&particle_registry->particles, particle);
        data->id           = particle;
        return particle;
    }

    void
    particle_destroy(ParticleID particle, ParticleDestroyResult* result /*= nullptr*/)
    {
        NIT_CHECK_PARTICLE_REGISTRY_CREATED
        NIT_CHECK_MSG(particle_valid(particle), "Particle is not valid!");

        if (result)
        {
            result->particles.push_back(particle);
            ++result->count;
        }

        pool_delete_data(&particle_registry->particles, particle);

        --particle_registry->particle_count;
    }

    bool
    particle_valid(ParticleID particle)
    {
        NIT_CHECK_PARTICLE_REGISTRY_CREATED
        return particle < particle_registry->max_particles && pool_is_valid(&particle_registry->particles, particle);
    }

    bool
    particle_enabled(ParticleID particle)
    {
        NIT_CHECK_PARTICLE_REGISTRY_CREATED
        ParticleData* data = pool_get_data<ParticleData>(&particle_registry->particles, particle);
        return data->enabled;
    }

    void
    particle_set_enabled(ParticleID particle, bool enabled /*= true*/)
    {
        NIT_CHECK_PARTICLE_REGISTRY_CREATED
        ParticleData* data = pool_get_data<ParticleData>(&particle_registry->particles, particle);
        data->enabled = enabled;
    }

    ParticleArray
    particle_get_alive_particles()
    {
        NIT_CHECK_PARTICLE_REGISTRY_CREATED
        return { (ParticleData*) particle_registry->particles.elements, particle_registry->particle_count };
    }

    void
    spawn_particle(EntityID emitter)
    {
        NIT_CHECK_PARTICLE_REGISTRY_CREATED
        NIT_CHECK_MSG(particle_registry->particle_count < particle_registry->max_particles, "Particle limit reached!");
        
        ParticleID particle;
        pool_insert_data(&particle_registry->particles, particle);
        ++particle_registry->particle_count;

        // Generate numbers between -1.0 and 1.0
        Vector3 random        = {random_value(-1.f, 1.f), random_value(-1.f, 1.f), 0};
        Vector4 random_color  = {random_value(-1.f, 1.f), random_value(-1.f, 1.f), random_value(-1.f, 1.f), 0};
        //u8      random_sprite = random_value<u8>(0, particle_emitter.num_textures);
        
        ParticleData*          data              = pool_get_data<ParticleData>(&particle_registry->particles, particle);
        const ParticleEmitter& particle_emitter  = entity_get<ParticleEmitter>(emitter);
        const Transform&       emitter_transform = entity_get<Transform>(emitter);

        data->id                 = particle;
        data->enabled            = true;
        data->transform.position = emitter_transform.position + random * particle_emitter.pos_amplitude;
        data->transform.scale    = emitter_transform.scale * particle_emitter.scale;
        data->velocity           = particle_emitter.velocity + random * particle_emitter.vel_amplitude;
        data->life               = particle_emitter.starting_life;
        data->total_life         = particle_emitter.starting_life;
        data->sprite             = {
            .texture = particle_emitter.texture,
            .visible = true,
            .tint = particle_emitter.tint + random_color * particle_emitter.tint_amplitude
        };
    }

    ListenerAction
    particle_registry_update()
    {
        NIT_CHECK_PARTICLE_REGISTRY_CREATED
        
        // Emitter
        auto& emitter_group = entity_get_group<ParticleEmitter>().entities;
        for(EntityID emitter : emitter_group)
        {
            if (!entity_global_enabled(emitter))
            {
                continue;
            }

            ParticleEmitter& emitter_data = entity_get<ParticleEmitter>(emitter);
            if(!emitter_data.enabled)
            {
                continue;
            }

            emitter_data.current_cooldown -= delta_seconds();
            while (emitter_data.current_cooldown <= 0)
            {
                emitter_data.current_cooldown += emitter_data.cooldown == 0 ? 0.1f : emitter_data.cooldown; // You will believe what caused this
                spawn_particle(emitter);
            }
        }

        // Particles
        ParticleData* particle_pool = (ParticleData*)(particle_registry->particles.elements);
        for (u32 i = 0; i < particle_registry->particles.sparse_set.count; ++i)
        {
            if (!&particle_pool[i] || !particle_pool[i].enabled) continue;

            particle_pool[i].life -= delta_seconds();
            if(particle_pool[i].life >= 0)
            {
                particle_pool[i].transform.position += particle_pool[i].velocity * delta_seconds();
                f32 alpha                            = particle_pool[i].life / particle_pool[i].total_life;
                particle_pool[i].sprite.tint.w       = alpha;
            }
            else
            {
                particle_destroy(particle_pool[i].id);
            }
        }

        return ListenerAction::StayListening;
    }
}