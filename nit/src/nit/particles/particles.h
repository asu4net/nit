#pragma once
#include "nit/render/transform.h"
#include "nit/render/sprite.h"

namespace nit
{
    /////////////////////////////
    //:Particle
    /////////////////////////////

    inline constexpr u32 NULL_PARTICLE = U32_MAX;
    inline constexpr f32 PARTICLE_LIFETIME = 1.f;

    using ParticleID = u32;

    struct ParticleData
    {
        ParticleID id = NULL_PARTICLE;
        bool       enabled = true;
        Transform  transform;
        Sprite     sprite;
        Vector3    velocity = V3_ONE;
        Vector4    color = V4_COLOR_LIGHT_GREEN;
        f32        life = PARTICLE_LIFETIME;
        f32        total_life = PARTICLE_LIFETIME;
    };

    /////////////////////////////
    //:Particle Emitter
    /////////////////////////////

    inline constexpr u8  MAX_SPRITES_PER_EMITTER = 5;
    inline constexpr f32 SPAWN_TIME = 0.01f;

    using EmitterID = u32;

    struct ParticleEmitter
    {
        //u8          num_textures     = 0;
        AssetHandle texture;//s[MAX_SPRITES_PER_EMITTER];
        Vector3     pos_amplitude    = {0.1f, 0.1f, 0.f};
        Vector3     scale            = V3_ONE;
        Vector3     velocity         = V3_ONE;
        Vector3     vel_amplitude    = V3_ONE;
        Vector4     tint             = V4_COLOR_LIGHT_GREEN;
        Vector4     tint_amplitude   = {0.5f, 0.5f, 0.5f, 0.f};
        f32         starting_life    = PARTICLE_LIFETIME;
        f32         cooldown         = SPAWN_TIME;
        f32         current_cooldown = SPAWN_TIME;
        bool        enabled           = true;
    };

    void register_particle_emitter_component();

    /////////////////////////////
    //:Particle Registry
    /////////////////////////////

    struct ParticleRegistry
    {
        Pool                              particles;
        u32                               particle_count = 0;
        u32                               max_particles= 100000;
    };

    void              particle_registry_set_instance(ParticleRegistry* particle_registry_instance);
    ParticleRegistry* particle_registry_get_instance();

    void       particle_registry_init();
    void       particle_registry_finish();
    ParticleID particle_create();

    struct ParticleDestroyResult { Array<ParticleID> particles; u32 count = 0; };

    void       particle_destroy(ParticleID particle, ParticleDestroyResult* result = nullptr);
    bool       particle_valid(ParticleID particle);

    bool particle_enabled(ParticleID particle);
    void particle_set_enabled(ParticleID particle, bool enabled = true);

    struct ParticleArray { ParticleData* particles = nullptr; u32 count = 0; };

    ParticleArray particle_get_alive_particles();
}