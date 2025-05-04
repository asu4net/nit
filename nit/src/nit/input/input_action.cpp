#include "input_action.h"
#include "nit_pch.h"
#include "core/asset.h"

#ifdef NIT_EDITOR_ENABLED
#include "editor/editor_utils.h"
#endif
#include "input_registry.h"

namespace nit
{
    void serialize(const InputAction* input_action, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << "key" <<                YAML::Value << enum_to_string<GamepadKeys>(input_action->key);
        emitter << YAML::Key << "paired_input_key_x" << YAML::Value << enum_to_string<GamepadKeys>(input_action->paired_input_key_x);
        emitter << YAML::Key << "paired_input_key_y" << YAML::Value << enum_to_string<GamepadKeys>(input_action->paired_input_key_y);
        emitter << YAML::Key << "paired_input_key_z" << YAML::Value << enum_to_string<GamepadKeys>(input_action->paired_input_key_z);
        emitter << YAML::Key << "paired_input_key_w" << YAML::Value << enum_to_string<GamepadKeys>(input_action->paired_input_key_w);
        emitter << YAML::Key << "input_type" <<         YAML::Value << enum_to_string<InputType>(input_action->input_type);
        emitter << YAML::Key << "trigger_type" <<       YAML::Value << enum_to_string<TriggerType>(input_action->trigger_type);

        emitter << YAML::Key << "input_modifiers" << YAML::Value << YAML::BeginMap;

        InputRegistry* input_registry = input_registry_get_instance();
        for (u8 i = 0; i < input_registry->next_input_modifier_type_index - 1; ++i)
        {
            auto& input_modifier_pool = input_registry->input_modifier_pool[i];
            auto& data_pool = input_modifier_pool.data_pool;

            if (!data_pool.type->fn_invoke_deserialize
                || !data_pool.type->fn_invoke_serialize
                || !delegate_invoke(input_modifier_pool.fn_is_in_input_action, input_action))
            {
                continue;
            }

            emitter << YAML::Key << data_pool.type->name << YAML::Value << YAML::BeginMap;

            u32 modifier_id = input_action->input_modifiers[input_modifier_pool.type_index];
            void* raw_data = pool_get_raw_data(&data_pool, modifier_id);
            serialize(data_pool.type, raw_data, emitter);

            emitter << YAML::EndMap;
        }

        emitter << YAML::EndMap;
    }

    void deserialize(InputAction* input_action, const YAML::Node& node)
    {
        InputRegistry* input_registry = input_registry_get_instance();

        input_action->id = input_registry->available_input_actions.front(); 
        input_registry->available_input_actions.pop();
        input_action->key =             enum_from_string<GamepadKeys>(node["key"].as<String>());
        input_action->paired_input_key_x = enum_from_string<GamepadKeys>(node["paired_input_key_x"].as<String>());
        input_action->paired_input_key_y = enum_from_string<GamepadKeys>(node["paired_input_key_y"].as<String>());
        input_action->paired_input_key_z = enum_from_string<GamepadKeys>(node["paired_input_key_z"].as<String>());
        input_action->paired_input_key_w = enum_from_string<GamepadKeys>(node["paired_input_key_w"].as<String>());
        input_action->input_type =       enum_from_string<InputType>(node["input_type"].as<String>());
        input_action->trigger_type =     enum_from_string<TriggerType>(node["trigger_type"].as<String>());

        const YAML::Node& modifiers_node = node["input_modifiers"];

        const InputAction* const_input_action = input_action;
        for (const auto& action_node_child : modifiers_node)
        {
            const YAML::Node& modifier_node = action_node_child.second;
            String type_name = action_node_child.first.as<String>();
            auto* input_modifier_pool = input_find_modifier_pool(type_get(type_name));
            auto& data_pool = input_modifier_pool->data_pool;
            void* null_data = nullptr;
            delegate_invoke(input_modifier_pool->fn_add_to_input_action, input_action, null_data, false);
            void* modifier_data = delegate_invoke(input_modifier_pool->fn_get_from_input_action, const_input_action);
            deserialize(data_pool.type, modifier_data, modifier_node);
            InputModifierAddedArgs args;
            args.input_action = input_action;
            args.type = input_modifier_pool->data_pool.type;
            event_broadcast<const InputModifierAddedArgs&>(input_registry->input_modifier_added_event, args);
        }
    }

#ifdef NIT_EDITOR_ENABLED
    void draw_editor(InputAction* input_action)
    {

        editor_draw_enum_combo("Key", input_action->key);
        editor_draw_enum_combo("Paired Key X", input_action->paired_input_key_x);
        editor_draw_enum_combo("Paired Key Y", input_action->paired_input_key_y);
        editor_draw_enum_combo("Paired Key Z", input_action->paired_input_key_z);
        editor_draw_enum_combo("Paired Key W", input_action->paired_input_key_w);
        editor_draw_enum_combo("Input Type", input_action->input_type);
        editor_draw_enum_combo("Trigger Type", input_action->trigger_type);

        InputRegistry* input_registry = input_registry_get_instance();

        // if delegate needs a const InputAction* using a non const doesnt compile
        const InputAction* const_input_action = (const InputAction*)input_action;
        for (u32 i = 0; i < input_registry->next_input_modifier_type_index - 1; ++i)
        {
            InputModifierPool* modifier_pool = &input_registry->input_modifier_pool[i];
            if (!delegate_invoke(modifier_pool->fn_is_in_input_action, const_input_action))
            {
                continue;
            }

            Type* modifier_type = modifier_pool->data_pool.type;

            bool is_expanded = ImGui::TreeNodeEx(modifier_type->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

            ImGui::SameLine();

            ImGui::PushID(i);
            if (ImGui::Button("+"))
            {
                ImGui::OpenPopup("Input Modifier Settings");
            }

            bool remove_input_modifier = false;

            if (ImGui::BeginPopup("Input Modifier Settings"))
            {
                if (ImGui::MenuItem("Remove Input Modifier"))
                {
                    remove_input_modifier = true;
                }

                ImGui::EndPopup();
            }
            ImGui::PopID();

            if (is_expanded)
            {
                ImGui::Spacing();

                if (modifier_pool->data_pool.type->fn_invoke_draw_editor)
                {
                    u32 modifier_id = input_action->input_modifiers[modifier_pool->type_index];
                    void* data = pool_get_raw_data(&modifier_pool->data_pool, modifier_id);
                    NIT_CHECK(data);
                    type_draw_editor(modifier_type, data);
                }

                ImGui::Separator();
                ImGui::Spacing();
                ImGui::TreePop();
            }

            if (remove_input_modifier)
            {
                delegate_invoke(modifier_pool->fn_remove_from_input_action, input_action);
            }
        }
        if (editor_draw_centered_button("Add Input Modifier"))
        {
            ImGui::OpenPopup("Add Input Modifier");
        }

        if (ImGui::BeginPopup("Add Input Modifier"))
        {
            for (u32 i = 0; i < input_registry_get_instance()->next_input_modifier_type_index - 1; ++i)
            {
                InputModifierPool* pool = &input_registry_get_instance()->input_modifier_pool[i];
                if (delegate_invoke(pool->fn_is_in_input_action, const_input_action))
                {
                    continue;
                }

                if (ImGui::MenuItem(pool->data_pool.type->name.c_str()))
                {
                    void* null_data = nullptr;
                    delegate_invoke(pool->fn_add_to_input_action, input_action, null_data, true);
                }
            }
            ImGui::EndPopup();
        }
    }
#endif

    void print_key(InputAction* input_action)
    {
        NIT_LOG_TRACE("%s", enum_to_string<GamepadKeys>(input_action->key).c_str());
    }

    void register_input_action_asset()
    {
        enum_register<GamepadKeys>();
        enum_register_value<GamepadKeys>("GamepadButtonNONE", GamepadKeys::GamepadButtonNONE);
        enum_register_value<GamepadKeys>("GamepadButtonSouth", GamepadKeys::GamepadButtonSouth);
        enum_register_value<GamepadKeys>("GamepadButtonEast", GamepadKeys::GamepadButtonEast);
        enum_register_value<GamepadKeys>("GamepadButtonWest", GamepadKeys::GamepadButtonWest);
        enum_register_value<GamepadKeys>("GamepadButtonNorth", GamepadKeys::GamepadButtonNorth);
        enum_register_value<GamepadKeys>("GamepadButtonLeftBumper", GamepadKeys::GamepadButtonLeftBumper);
        enum_register_value<GamepadKeys>("GamepadButtonRightBumper", GamepadKeys::GamepadButtonRightBumper);
        enum_register_value<GamepadKeys>("GamepadButtonSelect", GamepadKeys::GamepadButtonSelect);
        enum_register_value<GamepadKeys>("GamepadButtonStart", GamepadKeys::GamepadButtonStart);
        enum_register_value<GamepadKeys>("GamepadButtonHome", GamepadKeys::GamepadButtonHome);
        enum_register_value<GamepadKeys>("GamepadButtonLeftThumb", GamepadKeys::GamepadButtonLeftThumb);
        enum_register_value<GamepadKeys>("GamepadButtonRightThumb", GamepadKeys::GamepadButtonRightThumb);
        enum_register_value<GamepadKeys>("GamepadButtonDpadUp", GamepadKeys::GamepadButtonDpadUp);
        enum_register_value<GamepadKeys>("GamepadButtonDpadRight", GamepadKeys::GamepadButtonDpadRight);
        enum_register_value<GamepadKeys>("GamepadButtonDpadDown", GamepadKeys::GamepadButtonDpadDown);
        enum_register_value<GamepadKeys>("GamepadButtonDpadLeft", GamepadKeys::GamepadButtonDpadLeft);
        enum_register_value<GamepadKeys>("GamepadAxisLeftTrigger", GamepadKeys::GamepadAxisLeftTrigger);
        enum_register_value<GamepadKeys>("GamepadAxisRightTrigger", GamepadKeys::GamepadAxisRightTrigger);
        enum_register_value<GamepadKeys>("GamepadAxisLeftX", GamepadKeys::GamepadAxisLeftX);
        enum_register_value<GamepadKeys>("GamepadAxisLeftY", GamepadKeys::GamepadAxisLeftY);
        enum_register_value<GamepadKeys>("GamepadAxisLeft2D", GamepadKeys::GamepadAxisLeft2D);
        enum_register_value<GamepadKeys>("GamepadAxisRightX", GamepadKeys::GamepadAxisRightX);
        enum_register_value<GamepadKeys>("GamepadAxisRightY", GamepadKeys::GamepadAxisRightY);
        enum_register_value<GamepadKeys>("GamepadAxisRight2D", GamepadKeys::GamepadAxisRight2D);
        enum_register_value<GamepadKeys>("GamepadAxisGyroX", GamepadKeys::GamepadAxisGyroX);
        enum_register_value<GamepadKeys>("GamepadAxisGyroY", GamepadKeys::GamepadAxisGyroY);
        enum_register_value<GamepadKeys>("GamepadAxisGyroZ", GamepadKeys::GamepadAxisGyroZ);
        enum_register_value<GamepadKeys>("GamepadAxisGyro3D", GamepadKeys::GamepadAxisGyro3D);
        enum_register_value<GamepadKeys>("GamepadAxisTotalAccelX", GamepadKeys::GamepadAxisTotalAccelX);
        enum_register_value<GamepadKeys>("GamepadAxisTotalAccelY", GamepadKeys::GamepadAxisTotalAccelY);
        enum_register_value<GamepadKeys>("GamepadAxisTotalAccelZ", GamepadKeys::GamepadAxisTotalAccelZ);
        enum_register_value<GamepadKeys>("GamepadAxisTotalAccel3D", GamepadKeys::GamepadAxisTotalAccel3D);
        enum_register_value<GamepadKeys>("GamepadAxisAccelX", GamepadKeys::GamepadAxisAccelX);
        enum_register_value<GamepadKeys>("GamepadAxisAccelY", GamepadKeys::GamepadAxisAccelY);
        enum_register_value<GamepadKeys>("GamepadAxisAccelZ", GamepadKeys::GamepadAxisAccelZ);
        enum_register_value<GamepadKeys>("GamepadAxisAccel3D", GamepadKeys::GamepadAxisAccel3D);
        enum_register_value<GamepadKeys>("GamepadAxisGravX", GamepadKeys::GamepadAxisGravX);
        enum_register_value<GamepadKeys>("GamepadAxisGravY", GamepadKeys::GamepadAxisGravY);
        enum_register_value<GamepadKeys>("GamepadAxisGravZ", GamepadKeys::GamepadAxisGravZ);
        enum_register_value<GamepadKeys>("GamepadAxisGrav3D", GamepadKeys::GamepadAxisGrav3D);
        enum_register_value<GamepadKeys>("GamepadAxisQuatX", GamepadKeys::GamepadAxisQuatX);
        enum_register_value<GamepadKeys>("GamepadAxisQuatY", GamepadKeys::GamepadAxisQuatY);
        enum_register_value<GamepadKeys>("GamepadAxisQuatZ", GamepadKeys::GamepadAxisQuatZ);
        enum_register_value<GamepadKeys>("GamepadAxisQuatW", GamepadKeys::GamepadAxisQuatW);
        enum_register_value<GamepadKeys>("GamepadAxisQuat4D", GamepadKeys::GamepadAxisQuat4D);
        enum_register_value<GamepadKeys>("SwitchButtonCapture", GamepadKeys::SwitchButtonCapture);
        enum_register_value<GamepadKeys>("SwitchButtonLeftSL", GamepadKeys::SwitchButtonLeftSL);
        enum_register_value<GamepadKeys>("SwitchButtonLeftSR", GamepadKeys::SwitchButtonLeftSR);
        enum_register_value<GamepadKeys>("SwitchButtonRightSL", GamepadKeys::SwitchButtonRightSL);
        enum_register_value<GamepadKeys>("SwitchButtonRightSR", GamepadKeys::SwitchButtonRightSR);
        enum_register_value<GamepadKeys>("SwitchButtonSL", GamepadKeys::SwitchButtonSL);
        enum_register_value<GamepadKeys>("SwitchButtonSR", GamepadKeys::SwitchButtonSR);
        enum_register_value<GamepadKeys>("SwitchAxisLeftGyroX", GamepadKeys::SwitchAxisLeftGyroX);
        enum_register_value<GamepadKeys>("SwitchAxisLeftGyroY", GamepadKeys::SwitchAxisLeftGyroY);
        enum_register_value<GamepadKeys>("SwitchAxisLeftGyroZ", GamepadKeys::SwitchAxisLeftGyroZ);
        enum_register_value<GamepadKeys>("SwitchAxisLeftGyro3D", GamepadKeys::SwitchAxisLeftGyro3D);
        enum_register_value<GamepadKeys>("SwitchAxisLeftAccelX", GamepadKeys::SwitchAxisLeftAccelX);
        enum_register_value<GamepadKeys>("SwitchAxisLeftAccelY", GamepadKeys::SwitchAxisLeftAccelY);
        enum_register_value<GamepadKeys>("SwitchAxisLeftAccelZ", GamepadKeys::SwitchAxisLeftAccelZ);
        enum_register_value<GamepadKeys>("SwitchAxisLeftAccel3D", GamepadKeys::SwitchAxisLeftAccel3D);
        enum_register_value<GamepadKeys>("SwitchAxisLeftGravX", GamepadKeys::SwitchAxisLeftGravX);
        enum_register_value<GamepadKeys>("SwitchAxisLeftGravY", GamepadKeys::SwitchAxisLeftGravY);
        enum_register_value<GamepadKeys>("SwitchAxisLeftGravZ", GamepadKeys::SwitchAxisLeftGravZ);
        enum_register_value<GamepadKeys>("SwitchAxisLeftGrav3D", GamepadKeys::SwitchAxisLeftGrav3D);
        enum_register_value<GamepadKeys>("SwitchAxisLeftQuatX", GamepadKeys::SwitchAxisLeftQuatX);
        enum_register_value<GamepadKeys>("SwitchAxisLeftQuatY", GamepadKeys::SwitchAxisLeftQuatY);
        enum_register_value<GamepadKeys>("SwitchAxisLeftQuatZ", GamepadKeys::SwitchAxisLeftQuatZ);
        enum_register_value<GamepadKeys>("SwitchAxisLeftQuatW", GamepadKeys::SwitchAxisLeftQuatW);
        enum_register_value<GamepadKeys>("SwitchAxisLeftQuat4D", GamepadKeys::SwitchAxisLeftQuat4D);
        enum_register_value<GamepadKeys>("DualShockButtonCreate", GamepadKeys::DualShockButtonCreate);
        enum_register_value<GamepadKeys>("DualShockButtonMic", GamepadKeys::DualShockButtonMic);
        enum_register_value<GamepadKeys>("DualShockButtonTouchpadDown", GamepadKeys::DualShockButtonTouchpadDown);
        enum_register_value<GamepadKeys>("DualShockButtonFirstTouchpad", GamepadKeys::DualShockButtonFirstTouchpad);
        enum_register_value<GamepadKeys>("DualShockButtonSecondTouchpad", GamepadKeys::DualShockButtonSecondTouchpad);
        enum_register_value<GamepadKeys>("DualShockAxisFirstTouchpadX", GamepadKeys::DualShockAxisFirstTouchpadX);
        enum_register_value<GamepadKeys>("DualShockAxisFirstTouchpadY", GamepadKeys::DualShockAxisFirstTouchpadY);
        enum_register_value<GamepadKeys>("DualShockAxisFirstTouchpad2D", GamepadKeys::DualShockAxisFirstTouchpad2D);
        enum_register_value<GamepadKeys>("DualShockAxisSecondTouchpadX", GamepadKeys::DualShockAxisSecondTouchpadX);
        enum_register_value<GamepadKeys>("DualShockAxisSecondTouchpadY", GamepadKeys::DualShockAxisSecondTouchpadY);
        enum_register_value<GamepadKeys>("DualShockAxisSecondTouchpad2D", GamepadKeys::DualShockAxisSecondTouchpad2D);
        enum_register_value<GamepadKeys>("DualShockAxisDpadUp", GamepadKeys::DualShockAxisDpadUp);
        enum_register_value<GamepadKeys>("DualShockAxisDpadRight", GamepadKeys::DualShockAxisDpadRight);
        enum_register_value<GamepadKeys>("DualShockAxisDpadDown", GamepadKeys::DualShockAxisDpadDown);
        enum_register_value<GamepadKeys>("DualShockAxisDpadLeft", GamepadKeys::DualShockAxisDpadLeft);
        enum_register_value<GamepadKeys>("DualShockAxisSouth", GamepadKeys::DualShockAxisSouth);
        enum_register_value<GamepadKeys>("DualShockAxisEast", GamepadKeys::DualShockAxisEast);
        enum_register_value<GamepadKeys>("DualShockAxisWest", GamepadKeys::DualShockAxisWest);
        enum_register_value<GamepadKeys>("DualShockAxisNorth", GamepadKeys::DualShockAxisNorth);
        enum_register_value<GamepadKeys>("DualShockAxisLeftBumper", GamepadKeys::DualShockAxisLeftBumper);
        enum_register_value<GamepadKeys>("DualShockAxisRightBumper", GamepadKeys::DualShockAxisRightBumper);
        enum_register_value<GamepadKeys>("KeySpace", GamepadKeys::KeySpace);
        enum_register_value<GamepadKeys>("KeyApostrophe", GamepadKeys::KeyApostrophe);
        enum_register_value<GamepadKeys>("KeyComma", GamepadKeys::KeyComma);
        enum_register_value<GamepadKeys>("KeyMinus", GamepadKeys::KeyMinus);
        enum_register_value<GamepadKeys>("KeyPeriod", GamepadKeys::KeyPeriod);
        enum_register_value<GamepadKeys>("KeySlash", GamepadKeys::KeySlash);
        enum_register_value<GamepadKeys>("Key0", GamepadKeys::Key0);
        enum_register_value<GamepadKeys>("Key1", GamepadKeys::Key1);
        enum_register_value<GamepadKeys>("Key2", GamepadKeys::Key2);
        enum_register_value<GamepadKeys>("Key3", GamepadKeys::Key3);
        enum_register_value<GamepadKeys>("Key4", GamepadKeys::Key4);
        enum_register_value<GamepadKeys>("Key5", GamepadKeys::Key5);
        enum_register_value<GamepadKeys>("Key6", GamepadKeys::Key6);
        enum_register_value<GamepadKeys>("Key7", GamepadKeys::Key7);
        enum_register_value<GamepadKeys>("Key8", GamepadKeys::Key8);
        enum_register_value<GamepadKeys>("Key9", GamepadKeys::Key9);
        enum_register_value<GamepadKeys>("KeySemicolon", GamepadKeys::KeySemicolon);
        enum_register_value<GamepadKeys>("KeyEqual", GamepadKeys::KeyEqual);
        enum_register_value<GamepadKeys>("KeyA", GamepadKeys::KeyA);
        enum_register_value<GamepadKeys>("KeyB", GamepadKeys::KeyB);
        enum_register_value<GamepadKeys>("KeyC", GamepadKeys::KeyC);
        enum_register_value<GamepadKeys>("KeyD", GamepadKeys::KeyD);
        enum_register_value<GamepadKeys>("KeyE", GamepadKeys::KeyE);
        enum_register_value<GamepadKeys>("KeyF", GamepadKeys::KeyF);
        enum_register_value<GamepadKeys>("KeyG", GamepadKeys::KeyG);
        enum_register_value<GamepadKeys>("KeyH", GamepadKeys::KeyH);
        enum_register_value<GamepadKeys>("KeyI", GamepadKeys::KeyI);
        enum_register_value<GamepadKeys>("KeyJ", GamepadKeys::KeyJ);
        enum_register_value<GamepadKeys>("KeyK", GamepadKeys::KeyK);
        enum_register_value<GamepadKeys>("KeyL", GamepadKeys::KeyL);
        enum_register_value<GamepadKeys>("KeyM", GamepadKeys::KeyM);
        enum_register_value<GamepadKeys>("KeyN", GamepadKeys::KeyN);
        enum_register_value<GamepadKeys>("KeyO", GamepadKeys::KeyO);
        enum_register_value<GamepadKeys>("KeyP", GamepadKeys::KeyP);
        enum_register_value<GamepadKeys>("KeyQ", GamepadKeys::KeyQ);
        enum_register_value<GamepadKeys>("KeyR", GamepadKeys::KeyR);
        enum_register_value<GamepadKeys>("KeyS", GamepadKeys::KeyS);
        enum_register_value<GamepadKeys>("KeyT", GamepadKeys::KeyT);
        enum_register_value<GamepadKeys>("KeyU", GamepadKeys::KeyU);
        enum_register_value<GamepadKeys>("KeyV", GamepadKeys::KeyV);
        enum_register_value<GamepadKeys>("KeyW", GamepadKeys::KeyW);
        enum_register_value<GamepadKeys>("KeyX", GamepadKeys::KeyX);
        enum_register_value<GamepadKeys>("KeyY", GamepadKeys::KeyY);
        enum_register_value<GamepadKeys>("KeyZ", GamepadKeys::KeyZ);
        enum_register_value<GamepadKeys>("KeyLeftBracket", GamepadKeys::KeyLeftBracket);
        enum_register_value<GamepadKeys>("KeyBackSlash", GamepadKeys::KeyBackSlash);
        enum_register_value<GamepadKeys>("KeyRightBracket", GamepadKeys::KeyRightBracket);
        enum_register_value<GamepadKeys>("KeyGraveAccent", GamepadKeys::KeyGraveAccent);
        enum_register_value<GamepadKeys>("KeyWorld1", GamepadKeys::KeyWorld1);
        enum_register_value<GamepadKeys>("KeyWorld2", GamepadKeys::KeyWorld2);
        enum_register_value<GamepadKeys>("KeyEscape", GamepadKeys::KeyEscape);
        enum_register_value<GamepadKeys>("KeyEnter", GamepadKeys::KeyEnter);
        enum_register_value<GamepadKeys>("KeyTab", GamepadKeys::KeyTab);
        enum_register_value<GamepadKeys>("KeyBackSpace", GamepadKeys::KeyBackSpace);
        enum_register_value<GamepadKeys>("KeyInsert", GamepadKeys::KeyInsert);
        enum_register_value<GamepadKeys>("KeyDelete", GamepadKeys::KeyDelete);
        enum_register_value<GamepadKeys>("KeyRight", GamepadKeys::KeyRight);
        enum_register_value<GamepadKeys>("KeyLeft", GamepadKeys::KeyLeft);
        enum_register_value<GamepadKeys>("KeyDown", GamepadKeys::KeyDown);
        enum_register_value<GamepadKeys>("KeyUp", GamepadKeys::KeyUp);
        enum_register_value<GamepadKeys>("KeyPageUp", GamepadKeys::KeyPageUp);
        enum_register_value<GamepadKeys>("KeyPageDown", GamepadKeys::KeyPageDown);
        enum_register_value<GamepadKeys>("KeyHome", GamepadKeys::KeyHome);
        enum_register_value<GamepadKeys>("KeyEnd", GamepadKeys::KeyEnd);
        enum_register_value<GamepadKeys>("KeyCapsLock", GamepadKeys::KeyCapsLock);
        enum_register_value<GamepadKeys>("KeyScrollLock", GamepadKeys::KeyScrollLock);
        enum_register_value<GamepadKeys>("KeyNumLock", GamepadKeys::KeyNumLock);
        enum_register_value<GamepadKeys>("KeyPrintScreen", GamepadKeys::KeyPrintScreen);
        enum_register_value<GamepadKeys>("KeyPause", GamepadKeys::KeyPause);
        enum_register_value<GamepadKeys>("KeyF1", GamepadKeys::KeyF1);
        enum_register_value<GamepadKeys>("KeyF2", GamepadKeys::KeyF2);
        enum_register_value<GamepadKeys>("KeyF3", GamepadKeys::KeyF3);
        enum_register_value<GamepadKeys>("KeyF4", GamepadKeys::KeyF4);
        enum_register_value<GamepadKeys>("KeyF5", GamepadKeys::KeyF5);
        enum_register_value<GamepadKeys>("KeyF6", GamepadKeys::KeyF6);
        enum_register_value<GamepadKeys>("KeyF7", GamepadKeys::KeyF7);
        enum_register_value<GamepadKeys>("KeyF8", GamepadKeys::KeyF8);
        enum_register_value<GamepadKeys>("KeyF9", GamepadKeys::KeyF9);
        enum_register_value<GamepadKeys>("KeyF10", GamepadKeys::KeyF10);
        enum_register_value<GamepadKeys>("KeyF11", GamepadKeys::KeyF11);
        enum_register_value<GamepadKeys>("KeyF12", GamepadKeys::KeyF12);
        enum_register_value<GamepadKeys>("KeyF13", GamepadKeys::KeyF13);
        enum_register_value<GamepadKeys>("KeyF14", GamepadKeys::KeyF14);
        enum_register_value<GamepadKeys>("KeyF15", GamepadKeys::KeyF15);
        enum_register_value<GamepadKeys>("KeyF16", GamepadKeys::KeyF16);
        enum_register_value<GamepadKeys>("KeyF17", GamepadKeys::KeyF17);
        enum_register_value<GamepadKeys>("KeyF18", GamepadKeys::KeyF18);
        enum_register_value<GamepadKeys>("KeyF19", GamepadKeys::KeyF19);
        enum_register_value<GamepadKeys>("KeyF20", GamepadKeys::KeyF20);
        enum_register_value<GamepadKeys>("KeyF21", GamepadKeys::KeyF21);
        enum_register_value<GamepadKeys>("KeyF22", GamepadKeys::KeyF22);
        enum_register_value<GamepadKeys>("KeyF23", GamepadKeys::KeyF23);
        enum_register_value<GamepadKeys>("KeyF24", GamepadKeys::KeyF24);
        enum_register_value<GamepadKeys>("KeyF25", GamepadKeys::KeyF25);
        enum_register_value<GamepadKeys>("KeyKp0", GamepadKeys::KeyKp0);
        enum_register_value<GamepadKeys>("KeyKp1", GamepadKeys::KeyKp1);
        enum_register_value<GamepadKeys>("KeyKp2", GamepadKeys::KeyKp2);
        enum_register_value<GamepadKeys>("KeyKp3", GamepadKeys::KeyKp3);
        enum_register_value<GamepadKeys>("KeyKp4", GamepadKeys::KeyKp4);
        enum_register_value<GamepadKeys>("KeyKp5", GamepadKeys::KeyKp5);
        enum_register_value<GamepadKeys>("KeyKp6", GamepadKeys::KeyKp6);
        enum_register_value<GamepadKeys>("KeyKp7", GamepadKeys::KeyKp7);
        enum_register_value<GamepadKeys>("KeyKp8", GamepadKeys::KeyKp8);
        enum_register_value<GamepadKeys>("KeyKp9", GamepadKeys::KeyKp9);
        enum_register_value<GamepadKeys>("KeyKpDecimal", GamepadKeys::KeyKpDecimal);
        enum_register_value<GamepadKeys>("KeyKpDivide", GamepadKeys::KeyKpDivide);
        enum_register_value<GamepadKeys>("KeyKpMultiply", GamepadKeys::KeyKpMultiply);
        enum_register_value<GamepadKeys>("KeyKpSubstract", GamepadKeys::KeyKpSubstract);
        enum_register_value<GamepadKeys>("KeyKpAdd", GamepadKeys::KeyKpAdd);
        enum_register_value<GamepadKeys>("KeyKpEnter", GamepadKeys::KeyKpEnter);
        enum_register_value<GamepadKeys>("KeyKpEqual", GamepadKeys::KeyKpEqual);
        enum_register_value<GamepadKeys>("KeyLeftShift", GamepadKeys::KeyLeftShift);
        enum_register_value<GamepadKeys>("KeyLeftControl", GamepadKeys::KeyLeftControl);
        enum_register_value<GamepadKeys>("KeyLeftAlt", GamepadKeys::KeyLeftAlt);
        enum_register_value<GamepadKeys>("KeyLeftSuper", GamepadKeys::KeyLeftSuper);
        enum_register_value<GamepadKeys>("KeyRightShift", GamepadKeys::KeyRightShift);
        enum_register_value<GamepadKeys>("KeyRightControl", GamepadKeys::KeyRightControl);
        enum_register_value<GamepadKeys>("KeyRightAlt", GamepadKeys::KeyRightAlt);
        enum_register_value<GamepadKeys>("KeyRightSuper", GamepadKeys::KeyRightSuper);
        enum_register_value<GamepadKeys>("KeyMenu", GamepadKeys::KeyMenu);

        enum_register<InputType>();
        enum_register_value<InputType>("Digital", InputType::Digital);
        enum_register_value<InputType>("Axis1D", InputType::Axis1D);
        enum_register_value<InputType>("Axis2D", InputType::Axis2D);
        enum_register_value<InputType>("Axis3D", InputType::Axis3D);
        enum_register_value<InputType>("Axis4D", InputType::Axis4D);

        enum_register<TriggerType>();
        enum_register_value<TriggerType>("Down", TriggerType::Down);
        enum_register_value<TriggerType>("Pressed", TriggerType::Pressed);
        enum_register_value<TriggerType>("Released", TriggerType::Released);

        u32 max_input_actions = input_registry_get_instance()->max_input_actions;
        
        AssetTypeArgs<InputAction> args;
        args.fn_serialize = serialize;
        args.fn_deserialize = deserialize;
        NIT_IF_EDITOR_ENABLED(args.fn_draw_editor = draw_editor);
        args.max_elements = max_input_actions;
        asset_register_type<InputAction>(args);
    }

    
}

