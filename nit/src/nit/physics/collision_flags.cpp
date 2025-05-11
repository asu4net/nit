#include "collision_flags.h"

#include "physics_2d.h"
#include "core/asset.h"
#include "editor/editor_utils.h"

using namespace nit;

String get_category_name(const CollisionFlags* flags, u32 index)
{
    String category_name = flags->names[index];
    
    if (category_name.empty())
    {
        category_name = "Category " + std::to_string(index);
    }

    return category_name;
}

void serialize(const CollisionFlags* flags, YAML::Emitter& emitter)
{
    emitter << YAML::Key << "categories" << YAML::Value << YAML::BeginMap;
    
    for (u32 i = 0; i < flags->count; ++i)
    {
        emitter << YAML::Key << "category" << YAML::Value << YAML::BeginMap;
        emitter << YAML::Key << "name" << YAML::Value << get_category_name(flags, i);
        emitter << YAML::Key << "collides_with" << YAML::Flow << YAML::BeginSeq;
        
        for (u32 j = 0; j < flags->count; ++j)
        {
            if (flags->masks[i] & flags->categories[j])
            {
                emitter << get_category_name(flags, j);
            }
        }

        emitter << YAML::EndSeq << YAML::EndMap;
    }

    emitter << YAML::EndMap;
}   

void deserialize(CollisionFlags* flags, const YAML::Node& node)
{
    memset(flags->categories, 0, MAX_COLLISION_FLAGS);
    memset(flags->masks, 0, MAX_COLLISION_FLAGS);
    
    u32 index = 0;
    
    for (auto& category_node : node["categories"])
    {
        flags->names[index] = category_node.second["name"].as<String>();
        flags->categories[index] |= 1 << index;
        ++index;
    }
    
    flags->count = index;
    
    index = 0;

    for (auto& category_node : node["categories"])
    {
        for (auto& name_node : category_node.second["collides_with"])
        {
            String collides_name = name_node.as<String>();
            
            for (u32 i = 0; i < flags->count; ++i)
            {
                if (collides_name == flags->names[i])
                {
                    flags->masks[index] |= 1 << i;          
                }
            }
        }
        ++index;
    }
}

#ifdef NIT_EDITOR_ENABLED
void draw_editor(CollisionFlags* flags)
{
    if (ImGui::Button("+"))
    {
        if (flags->count < MAX_COLLISION_FLAGS) { flags->categories[flags->count] = 1 << flags->count; }
        flags->count = clamp(flags->count + 1, 0u, MAX_COLLISION_FLAGS);
    }
    ImGui::SameLine();
    if (ImGui::Button("-"))
    {
        flags->count = clamp(flags->count - 1, 0u, MAX_COLLISION_FLAGS);
    }

    for (u32 i = 0; i < flags->count; ++i)
    {
        ImGui::PushID(i);
        
        String category_title = "Category " + std::to_string(i);

        if (ImGui::TreeNodeEx(category_title.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            editor_draw_input_text("Name", flags->names[i]);

            if (ImGui::TreeNodeEx("Collides with", ImGuiTreeNodeFlags_DefaultOpen))
            {
                for (u32 w = 0; w < flags->count; ++w)
                {
                    if (flags->categories[w] & flags->masks[i])
                    {
                        String category_to_add_name = get_category_name(flags, w);
                    
                        if (ImGui::TreeNodeEx(category_to_add_name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf))
                        {
                            ImGui::TreePop();
                        }
                    }
                }
                ImGui::TreePop();
            }
            
            if (editor_draw_centered_button("Toggle Category"))
            {
                ImGui::OpenPopup("Toggle Category");
            }

            if (ImGui::BeginPopup("Toggle Category"))
            {
                for (u32 j = 0; j < flags->count; ++j)
                {
                    String category_to_add_name = get_category_name(flags, j);

                    if (ImGui::MenuItem(category_to_add_name.c_str()))
                    {
                        flags->masks[i] ^= 1 << j;
                    }
                }
                ImGui::EndPopup();
            }
            
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}
#endif

CollisionCategoryData collision_flags_get_category_data(const CollisionFlags* flags, const String& mask_name)
{
    NIT_CHECK(flags);
    for (u32 i = 0; i < flags->count; ++i)
    {
        if (flags->names[i] == mask_name)
        {
            return { flags->categories[i], flags->masks[i] };
        }
    }
    NIT_CHECK_MSG(false, "Collision mask not exists! :p");
    return {};
}

void register_collision_flags_asset()
{
    asset_register_type<CollisionFlags>({
        .fn_serialize   = serialize,
        .fn_deserialize = deserialize,
#ifdef  NIT_EDITOR_ENABLED
        .fn_draw_editor = draw_editor,
#endif
        .max_elements   = 1
    });
}