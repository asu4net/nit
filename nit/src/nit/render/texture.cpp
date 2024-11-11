#include "texture.h"

#ifdef NIT_GRAPHICS_API_OPENGL

#include <glad/glad.h>

#ifdef NIT_EDITOR_ENABLED
#include "editor/editor_utils.h"
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>
#include "nit/core/asset.h"

namespace nit
{
    void SetMagFilter(const u32 texture_id, const MagFilter mag_filter)
    {
        switch (mag_filter)
        {
        case MagFilter::Linear:
            glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            return;
        case MagFilter::Nearest:
            glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
    }

    void SetMinFilter(const u32 texture_id, const MinFilter mag_filter)
    {
        switch (mag_filter)
        {
        case MinFilter::Linear:
            glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            return;
        case MinFilter::Nearest:
            glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }
    }

    void SetWrapMode(const u32 texture_id, const TextureCoordinate texture_coordinate, const WrapMode wrap_mode)
    {
        const GLenum coord = texture_coordinate == TextureCoordinate::U ? GL_TEXTURE_WRAP_S : GL_TEXTURE_WRAP_T;

        switch (wrap_mode)
        {
        case WrapMode::Repeat:
            glTextureParameteri(texture_id, coord, GL_REPEAT);
            return;
        case WrapMode::ClampToEdge:
            glTextureParameteri(texture_id, coord, GL_CLAMP_TO_EDGE);
        }
    }
    
    void DeserializeSubTexture2D(SubTexture2D* sub_texture, const YAML::Node& node)
    {
        NIT_CHECK(sub_texture);
        sub_texture->name     = node["name"].as<String>();
        sub_texture->size     = node["size"].as<Vector2>();
        sub_texture->location = node["location"].as<Vector2>();
    }

    void SerializeSubTexture2D(const SubTexture2D* sub_texture, YAML::Emitter& emitter)
    {
        NIT_CHECK(sub_texture);
        emitter << YAML::Key << "name"     << YAML::Value << sub_texture->name;
        emitter << YAML::Key << "size"     << YAML::Value << sub_texture->size;
        emitter << YAML::Key << "location" << YAML::Value << sub_texture->location;
    }

    void RegisterTexture2DAsset()
    {
        RegisterEnumType<MinFilter>();
        RegisterEnumValue<MinFilter>("Linear", MinFilter::Linear);
        RegisterEnumValue<MinFilter>("Nearest",MinFilter::Nearest);
        
        RegisterEnumType<MagFilter>();
        RegisterEnumValue<MagFilter>("Linear", MagFilter::Linear);
        RegisterEnumValue<MagFilter>("Nearest",MagFilter::Nearest);

        RegisterEnumType<WrapMode>();
        RegisterEnumValue<WrapMode>("Repeat",     WrapMode::Repeat);
        RegisterEnumValue<WrapMode>("ClampToEdge",WrapMode::ClampToEdge);
        
        RegisterEnumType<TextureCoordinate>();
        RegisterEnumValue<TextureCoordinate>("U",TextureCoordinate::U);
        RegisterEnumValue<TextureCoordinate>("V",TextureCoordinate::V);
        
        RegisterAssetType<Texture2D>({
              LoadTexture2D
            , FreeTexture2D
            , SerializeTexture2D
            , DeserializeTexture2D
#ifdef NIT_EDITOR_ENABLED
            , DrawEditorTexture2D
#endif
        });
    }

    i32 FindIndexOfSubTexture2D(const Texture2D* texture, const String& sub_texture_name)
    {
        NIT_CHECK(texture);
        
        for (u32 i = 0; i < texture->sub_texture_count; ++i)
        {
            if (texture->sub_textures[i].name == sub_texture_name)
            {
                return i;
            }
        }
        return -1;
    }

    void SerializeTexture2D(const Texture2D* texture, YAML::Emitter& emitter)
    {
        using namespace YAML;
        
        emitter << Key << "image_path"        << Value << texture->image_path;
        emitter << Key << "mag_filter"        << Value << GetStringFromEnumValue<MagFilter> (texture->mag_filter);
        emitter << Key << "min_filter"        << Value << GetStringFromEnumValue<MinFilter> (texture->min_filter);
        emitter << Key << "wrap_mode_u"       << Value << GetStringFromEnumValue<WrapMode>  (texture->wrap_mode_u);
        emitter << Key << "wrap_mode_u"       << Value << GetStringFromEnumValue<WrapMode>  (texture->wrap_mode_v);
        emitter << Key << "sub_texture_count" << Value << texture->sub_texture_count;
        
        if (texture->sub_textures)
        {
            emitter << Key << "sub_textures" << Value << BeginMap;
            for (u32 i = 0; i < texture->sub_texture_count; ++i)
            {
                SubTexture2D* sub_texture = &texture->sub_textures[i];
                emitter << Key << "sub_texture" << Value << BeginMap;
                SerializeSubTexture2D(sub_texture, emitter);
                emitter << EndMap;
            }
            emitter << EndMap;
        }
    }

    void DeserializeTexture2D(Texture2D* texture, const YAML::Node& node)
    {
        texture->image_path        = node["image_path"]                                     .as<String>();
        texture->mag_filter        = GetEnumValueFromString<MagFilter> (node["mag_filter"]  .as<String>());
        texture->min_filter        = GetEnumValueFromString<MinFilter> (node["min_filter"]  .as<String>());
        texture->wrap_mode_u       = GetEnumValueFromString<WrapMode>  (node["wrap_mode_u"] .as<String>());
        texture->wrap_mode_u       = GetEnumValueFromString<WrapMode>  (node["wrap_mode_u"] .as<String>());
        texture->sub_texture_count = node["sub_texture_count"]                              .as<u32>();
        
        const YAML::Node& sub_textures_node = node["sub_textures"];
        if (sub_textures_node && texture->sub_texture_count > 0)
        {
            texture->sub_textures = new SubTexture2D[texture->sub_texture_count];
            u32 sub_texture_index = 0;
            for (const auto& sub_texture_node_child : sub_textures_node)
            {
                const YAML::Node& sub_texture_node = sub_texture_node_child.second;
                SubTexture2D* sub_texture = &texture->sub_textures[sub_texture_index]; 
                DeserializeSubTexture2D(sub_texture, sub_texture_node);
                ++sub_texture_index;
            }
        }
    }

#ifdef NIT_EDITOR_ENABLED
    void DrawEditorTexture2D(Texture2D* texture)
    {
        editor::DrawResourceCombo("resource", {".jpg", ".png"},  texture->image_path);
        editor::DrawEnumCombo("mag filter", texture->mag_filter);
        editor::DrawEnumCombo("min filter", texture->min_filter);
        editor::DrawEnumCombo("wrap u", texture->wrap_mode_u);
        editor::DrawEnumCombo("wrap v", texture->wrap_mode_v);

        if (texture->sub_texture_count > 0 && ImGui::TreeNodeEx("subtextures", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool expanded = ImGui::BeginPopupContextItem();

            for (u32 i = 0; i < texture->sub_texture_count; ++i)
            {
                ImGui::MenuItem(texture->sub_textures[i].name.c_str());
            }
            
            if (expanded)
            {
                ImGui::EndPopup();
            }
            
            ImGui::TreePop();
        }

    }
#endif

    void FreeTextureImage(Texture2D* texture)
    {
        if (!texture->pixel_data)
        {
            return;
        }
        
        free(texture->pixel_data);
        texture->pixel_data = nullptr;
    }

    void LoadTexture2D(Texture2D* texture)
    {
        if (!texture->image_path.empty())
        {
            if (texture->pixel_data)
            {
                FreeTextureImage(texture);    
            }
            
            stbi_set_flip_vertically_on_load(1);
            i32 width, height, channels;
            
            String image_path = "assets/";
            image_path.append(texture->image_path);
            texture->pixel_data = stbi_load(image_path.c_str(), &width, &height, &channels, 0);
            
            texture->size = {(f32) width, (f32) height }; 
            texture->channels = static_cast<u32>(channels);
        }

        UploadToGPU(texture);
    }

    void UploadToGPU(Texture2D* texture)
    {
        NIT_CHECK(texture->id == 0);
        
        GLenum internal_format = 0, data_format = 0;
        
        if (texture->channels == 4)
        {
            internal_format = GL_RGBA8;
            data_format = GL_RGBA;
        }
        else if (texture->channels == 3)
        {
            internal_format = GL_RGB8;
            data_format = GL_RGB;
        }

        u32 width  = (u32) texture->size.x;
        u32 height = (u32) texture->size.y;
        
        glCreateTextures(GL_TEXTURE_2D, 1, &texture->id);
        glTextureStorage2D(texture->id, 1, internal_format, width, height);

        SetMinFilter(texture->id, texture->min_filter);
        SetMagFilter(texture->id, texture->mag_filter);

        SetWrapMode(texture->id, TextureCoordinate::U, texture->wrap_mode_u);
        SetWrapMode(texture->id, TextureCoordinate::V, texture->wrap_mode_v);
        
        glTextureSubImage2D(texture->id, 0, 0, 0, width, height, data_format,
            GL_UNSIGNED_BYTE, texture->pixel_data);
    }

    void FreeTexture2D(Texture2D* texture)
    {
        glDeleteTextures(1, &texture->id);
        texture->id = 0;
        FreeTextureImage(texture);
    }

    bool IsTexture2DValid(const Texture2D* texture)
    {
        return texture != nullptr && texture->id != 0;
    }
    
    void BindTexture2D(const Texture2D* texture, u32 slot)
    {
        NIT_CHECK(IsTexture2DValid(texture));
        glBindTextureUnit(slot, texture->id);
    }

    struct Image
    {
        u8* data;
        i32 width, height, channels;
        String filename;
    };
    
    void LoadTexture2DAsSpriteSheet(Texture2D* texture, const String& sprite_sheet_name, const String& source_path, const String& dest_path, i32 max_width)
    {
        NIT_CHECK(texture);
        Array<Image> images;

        const Path src_path = source_path;

        if (!exists(src_path))
        {
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(src_path))
        {
            String path = entry.path().string();
            String filename = entry.path().filename().stem().string();
            i32 width, height, channels;
            if (u8* data = stbi_load(path.c_str(), &width, &height, &channels, 4))
            {
                images.push_back({data, width, height, channels, filename});
            }
        }

        if (images.empty())
        {
            return;
        }

        const u32 num_of_images = (u32)images.size();
        texture->sub_textures = new SubTexture2D[num_of_images];
        texture->sub_texture_count = num_of_images;
        
        i32 total_width = 0;
            
        for (const auto& image : images)
        {
            total_width += image.width;
        }

        i32 sprite_sheet_width = std::min(total_width, max_width);

        i32 current_x_offset = 0;
        i32 current_y_offset = 0;
        i32 max_row_height = 0;

        for (const auto& image : images)
        {
            if (current_x_offset + image.width > sprite_sheet_width)
            {
                current_x_offset = 0;
                current_y_offset += max_row_height;
                max_row_height = 0;
            }

            max_row_height = std::max(max_row_height, image.height);
            current_x_offset += image.width;
        }

        i32 sprite_sheet_height = current_y_offset + max_row_height;

        const u32 pixel_data_count = sprite_sheet_width * sprite_sheet_height * 4;
        texture->pixel_data = new u8[pixel_data_count];
        memset(texture->pixel_data, 0, pixel_data_count);

        current_x_offset = 0;
        current_y_offset = 0;
        max_row_height = 0;

        for (u32 i = 0; i < num_of_images; ++i)
        {
            auto& image = images[i];

            if (current_x_offset + image.width > sprite_sheet_width)
            {
                current_x_offset = 0;
                current_y_offset += max_row_height;
                max_row_height = 0;
            }

            for (i32 y = 0; y < image.height; ++y)
            {
                for (i32 x = 0; x < image.width; ++x)
                {
                    u32 sprite_idx = (u32) ((y + current_y_offset) * sprite_sheet_width + (x + current_x_offset)) * 4;
                    i32 img_idx = (y * image.width + x) * 4;

                    if (sprite_idx < pixel_data_count && img_idx < image.width * image.height * 4)
                    {
                        texture->pixel_data[sprite_idx] = image.data[img_idx];         // R
                        texture->pixel_data[sprite_idx + 1] = image.data[img_idx + 1]; // G
                        texture->pixel_data[sprite_idx + 2] = image.data[img_idx + 2]; // B
                        texture->pixel_data[sprite_idx + 3] = image.data[img_idx + 3]; // A
                    }
                }
            }

            SubTexture2D& sub_texture_2d = texture->sub_textures[i];
            sub_texture_2d.name = image.filename;
            sub_texture_2d.size = {(f32)image.width, (f32)image.height};
            sub_texture_2d.location = {(f32)current_x_offset, (f32)current_y_offset};

            current_x_offset += image.width;
            max_row_height = std::max(max_row_height, image.height);
        }

        String final_path = dest_path;
        final_path.append("\\").append(sprite_sheet_name).append(".png");

        stbi_write_png(final_path.c_str(), sprite_sheet_width, sprite_sheet_height, 4, texture->pixel_data, sprite_sheet_width * 4);

        delete[] texture->pixel_data;
        texture->pixel_data = nullptr;

        texture->image_path = final_path;
        LoadTexture2D(texture);
    }
}

#endif