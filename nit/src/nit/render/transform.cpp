#include "transform.h"
#include "entity/entity.h"
#include "entity/entity_utils.h"

#ifdef NIT_EDITOR_ENABLED
    #include "editor/editor_utils.h"
#endif

namespace nit
{
    Matrix4 transform_to_matrix(const Transform& transform, EntityID entity_id)
    {
        const Matrix4 child_matrix = mat_create_transform(transform.position, transform.rotation, transform.scale);

        if (!entity_valid(entity_id))
        {
            return child_matrix;
        }

        EntityID parent = entity_get_parent(entity_id);
        
        if (!entity_valid(parent) || !entity_has<Transform>(parent))
        {
            return child_matrix;
        }

        Vector3 child_pos = transform.position;
        Vector3 child_rot = transform.rotation;
        Vector3 child_scl = transform.scale;
        
        while (entity_valid(entity_get_parent(entity_id)))
        {
            parent = entity_get_parent(entity_id);

            if (!entity_valid(parent))
            {
                break;
            }
            
            const Transform& parent_transform = entity_get<Transform>(parent);
            const Vector3& parent_pos = parent_transform.position;
            const Vector3& parent_rot = parent_transform.rotation;

            child_pos = child_pos + parent_pos;
            child_pos = to_v3(rotate_around(to_v2(parent_pos), parent_rot.z, to_v2(child_pos)));
        
            child_rot = child_rot + parent_rot;
            child_scl = multiply(child_scl, parent_transform.scale);
            entity_id = entity_get_parent(entity_id);
        }
        
        return mat_create_transform(child_pos, child_rot, child_scl);
    }

    Vector3 transform_up(const Transform& transform)
    {
        return look_rotation(transform.rotation, V3_UP);
    }

    Vector3 transform_right(const Transform& transform)
    {
        return look_rotation(transform.rotation, V3_RIGHT);
    }

    Vector3 transform_front(const Transform& transform)
    {
        return look_rotation(transform.rotation, V3_FRONT);
    }
    
    void serialize(const Transform* transform, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << "position"    << YAML::Value << transform->position;
        emitter << YAML::Key << "rotation"    << YAML::Value << transform->rotation;
        emitter << YAML::Key << "scale"       << YAML::Value << transform->scale;
    }

    void deserialize(Transform* transform, const YAML::Node& node)
    {
        transform->position = node["position"].as<Vector3>();
        transform->rotation = node["rotation"].as<Vector3>();
        transform->scale    = node["scale"].as<Vector3>();
    }

#ifdef NIT_EDITOR_ENABLED
    void draw_editor(Transform* transform)
    {
        editor_draw_drag_vector3("Position", transform->position);
        editor_draw_drag_vector3("Rotation", transform->rotation);
        editor_draw_drag_vector3("Scale", transform->scale);
    }
#endif
    
    void register_transform_component()
    {
        component_register<Transform>
        ({
            .fn_serialize   = serialize,
            .fn_deserialize = deserialize,
            NIT_IF_EDITOR_ENABLED(.fn_draw_editor = draw_editor)
        });
    }
}
