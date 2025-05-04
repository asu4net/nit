#include "input_modifiers.h"
#include "input_keycodes.h"
#include "math/math_common.h"
#include "input_registry.h"
#include "editor/editor_utils.h"

namespace nit
{
    // ------------------------------
    // MODIFY INPUTS
    // ------------------------------

    void modify_input(const InputModifierDeadZone* input_modifier, Vector4& input_value, InputType input_type)
    {
        if (input_type == InputType::Digital) return;

        auto dead_zone_lambda = [input_modifier](const f32 axis_val)
        {
            return min(1.f, (max(0.f, abs(axis_val) - input_modifier->lower_threshold) / (input_modifier->upper_threshold - input_modifier->lower_threshold))) * sign(axis_val);
        };

        switch (input_modifier->deadzone_type)
        {
        case DeadZoneType::Axial:
            input_value.x = dead_zone_lambda(input_value.x);
            input_value.y = dead_zone_lambda(input_value.y);
            input_value.z = dead_zone_lambda(input_value.z);
            input_value.w = dead_zone_lambda(input_value.w);
            break;
        case DeadZoneType::Radial:
            if (input_type == InputType::Axis2D)
            {
                Vector2 new_value = Vector2(input_value.x, input_value.y);
                new_value = new_value.x == 0.f && new_value.y == 0.f ? new_value : normalize(new_value);
                new_value = new_value * dead_zone_lambda(magnitude(input_value));
                input_value.x = new_value.x;
                input_value.y = new_value.y;
            }
            else if (input_type == InputType::Axis3D)
            {
                Vector3 new_value = Vector3(input_value.x, input_value.y, input_value.z);
                new_value = new_value.x == 0.f && new_value.y == 0.f && new_value.z == 0.f ? new_value : normalize(new_value);
                new_value *= dead_zone_lambda(magnitude(new_value));
                input_value.x = new_value.x;
                input_value.y = new_value.y;
                input_value.z = new_value.z;
            }
            else if (input_type == InputType::Axis4D)
            {
                input_value = input_value.x == 0.f && input_value.y == 0.f && input_value.z == 0.f && input_value.w == 0.f ? input_value : normalize(input_value) * dead_zone_lambda(magnitude(input_value));
            }
            else
            {
                input_value.x = dead_zone_lambda(input_value.x);
            }
            break;
        }
    }

    void modify_input(const InputModifierScalar* input_modifier, Vector4& input_value, InputType input_type)
    {
        if (input_type == InputType::Digital) return;

        input_value.x = input_value.x * input_modifier->scalar.x;
        input_value.y = input_value.y * input_modifier->scalar.y;
        input_value.z = input_value.z * input_modifier->scalar.z;
        input_value.w = input_value.w * input_modifier->scalar.w;
    }

    void modify_input(const InputModifierAdd* input_modifier, Vector4& input_value, InputType input_type)
    {
        if (input_type == InputType::Digital) return;

        input_value = input_value + input_modifier->amount;
    }

    void modify_input(const InputModifierNegate* input_modifier, Vector4& input_value, InputType input_type)
    {
        if (input_type == InputType::Digital) return;

        input_value = input_value * Vector4(input_modifier->negate_x ? -1.f : 1.f, input_modifier->negate_y ? -1.f : 1.f, input_modifier->negate_z ? -1.f : 1.f, input_modifier->negate_w ? -1.f : 1.f);
    }

    void modify_input(const InputModifierSwizzleAxis* input_modifier, Vector4& input_value, InputType input_type)
    {
        if (input_type == InputType::Digital) return;

        Vector4 aux_value = input_value;
        switch (input_modifier->order)
        {
        case SwizzleAxisType::XZY:
            input_value.y = aux_value.z;
            input_value.z = aux_value.y;

            break;
        case SwizzleAxisType::YXZ:
            input_value.x = aux_value.y;
            input_value.y = aux_value.x;

            break;
        case SwizzleAxisType::YZX:
            input_value.x = aux_value.y;
            input_value.y = aux_value.z;
            input_value.z = aux_value.x;

            break;
        case SwizzleAxisType::ZXY:
            input_value.x = aux_value.z;
            input_value.y = aux_value.x;
            input_value.z = aux_value.y;

            break;
        case SwizzleAxisType::ZYX:
            input_value.x = aux_value.z;
            input_value.z = aux_value.x;

            break;
        }
    }

    // ------------------------------
    // EDITOR AND ASSET THINGIES
    // ------------------------------


    void serialize(const InputModifierDeadZone* input_modifier, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << "type" << YAML::Value << enum_to_string<DeadZoneType>(input_modifier->deadzone_type);
        emitter << YAML::Key << "lower_threshold" << YAML::Value << input_modifier->lower_threshold;
        emitter << YAML::Key << "upper_threshold" << YAML::Value << input_modifier->upper_threshold;
    }

    void deserialize(InputModifierDeadZone* input_modifier, const YAML::Node& node)
    {
        input_modifier->deadzone_type = enum_from_string<DeadZoneType>(node["type"].as<String>());
        input_modifier->lower_threshold = node["lower_threshold"].as<f32>();
        input_modifier->upper_threshold = node["upper_threshold"].as<f32>();
    }

#ifdef NIT_EDITOR_ENABLED
    void draw_editor(InputModifierDeadZone* input_modifier)
    {
        editor_draw_enum_combo("Type", input_modifier->deadzone_type);
        editor_draw_drag_f32("Lower Threshold", input_modifier->lower_threshold);
        editor_draw_drag_f32("Upper Threshold", input_modifier->upper_threshold);
    }
#endif

    void register_deadzone_input_modifier()
    {
        enum_register<DeadZoneType>();
        enum_register_value<DeadZoneType>("Axial", DeadZoneType::Axial);
        enum_register_value<DeadZoneType>("Radial", DeadZoneType::Radial);

        input_modifier_register<InputModifierDeadZone>
            ({
                .fn_serialize = serialize,
                .fn_deserialize = deserialize,
                .fn_modify = modify_input,
                NIT_IF_EDITOR_ENABLED(.fn_draw_editor = draw_editor)
                });
    }

    void serialize(const InputModifierScalar* input_modifier, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << "scalar" << YAML::Value << input_modifier->scalar;
    }

    void deserialize(InputModifierScalar* input_modifier, const YAML::Node& node)
    {
        input_modifier->scalar = node["scalar"].as<Vector4>();
    }

#ifdef NIT_EDITOR_ENABLED
    void draw_editor(InputModifierScalar* input_modifier)
    {
        editor_draw_drag_vector4("Scalar", input_modifier->scalar);
    }
#endif

    void register_scalar_input_modifier()
    {
        input_modifier_register<InputModifierScalar>
            ({
                .fn_serialize = serialize,
                .fn_deserialize = deserialize,
                .fn_modify = modify_input,
                NIT_IF_EDITOR_ENABLED(.fn_draw_editor = draw_editor)
                });
    }


    void serialize(const InputModifierAdd* input_modifier, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << "amount" << YAML::Value << input_modifier->amount;
    }

    void deserialize(InputModifierAdd* input_modifier, const YAML::Node& node)
    {
        input_modifier->amount = node["amount"].as<Vector4>();
    }

#ifdef NIT_EDITOR_ENABLED
    void draw_editor(InputModifierAdd* input_modifier)
    {
        editor_draw_drag_vector4("Amount", input_modifier->amount);
    }
#endif

    void register_add_input_modifier()
    {
        input_modifier_register<InputModifierAdd>
            ({
                .fn_serialize = serialize,
                .fn_deserialize = deserialize,
                .fn_modify = modify_input,
                NIT_IF_EDITOR_ENABLED(.fn_draw_editor = draw_editor)
                });
    }

    void serialize(const InputModifierNegate* input_modifier, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << "negate_x" << YAML::Value << input_modifier->negate_x;
        emitter << YAML::Key << "negate_y" << YAML::Value << input_modifier->negate_y;
        emitter << YAML::Key << "negate_z" << YAML::Value << input_modifier->negate_z;
        emitter << YAML::Key << "negate_w" << YAML::Value << input_modifier->negate_w;
    }

    void deserialize(InputModifierNegate* input_modifier, const YAML::Node& node)
    {
        input_modifier->negate_x = node["negate_x"].as<bool>();
        input_modifier->negate_y = node["negate_y"].as<bool>();
        input_modifier->negate_z = node["negate_z"].as<bool>();
        input_modifier->negate_w = node["negate_w"].as<bool>();
    }

#ifdef NIT_EDITOR_ENABLED
    void draw_editor(InputModifierNegate* input_modifier)
    {
        editor_draw_bool("Negate X", input_modifier->negate_x);
        editor_draw_bool("Negate Y", input_modifier->negate_y);
        editor_draw_bool("Negate Z", input_modifier->negate_z);
        editor_draw_bool("Negate W", input_modifier->negate_w);
    }
#endif

    void register_negate_input_modifier()
    {
        input_modifier_register<InputModifierNegate>
            ({
                .fn_serialize = serialize,
                .fn_deserialize = deserialize,
                .fn_modify = modify_input,
                NIT_IF_EDITOR_ENABLED(.fn_draw_editor = draw_editor)
                });
    }

    void serialize(const InputModifierSwizzleAxis* input_modifier, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << "order" << YAML::Value << enum_to_string<SwizzleAxisType>(input_modifier->order);
    }

    void deserialize(InputModifierSwizzleAxis* input_modifier, const YAML::Node& node)
    {
        input_modifier->order = enum_from_string<SwizzleAxisType>(node["order"].as<String>());
    }

#ifdef NIT_EDITOR_ENABLED
    void draw_editor(InputModifierSwizzleAxis* input_modifier)
    {
        editor_draw_enum_combo("Order", input_modifier->order);
    }
#endif

    void register_swizzle_input_modifier()
    {
        enum_register<SwizzleAxisType>();
        enum_register_value<SwizzleAxisType>("YXZ", SwizzleAxisType::YXZ);
        enum_register_value<SwizzleAxisType>("ZYX", SwizzleAxisType::ZYX);
        enum_register_value<SwizzleAxisType>("XZY", SwizzleAxisType::XZY);
        enum_register_value<SwizzleAxisType>("YZX", SwizzleAxisType::YZX);
        enum_register_value<SwizzleAxisType>("ZXY", SwizzleAxisType::ZXY);

        input_modifier_register<InputModifierSwizzleAxis>
            ({
                .fn_serialize = serialize,
                .fn_deserialize = deserialize,
                .fn_modify = modify_input,
                NIT_IF_EDITOR_ENABLED(.fn_draw_editor = draw_editor)
                });
    }

}
