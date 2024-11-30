﻿#include "primitives_2d.h"
#include "texture.h"
#include "font.h"

namespace nit
{
    void fill_vertex_colors(V4Verts2D& vertex_colors, const Vector4& color)
    {
        vertex_colors = {
            Vector4 {color.x, color.y, color.z, color.w }, // bottom-left
            Vector4 {color.x, color.y, color.z, color.w }, // bottom-right
            Vector4 {color.x, color.y, color.z, color.w }, // top-right
            Vector4 {color.x, color.y, color.z, color.w } // top-left
        };
    }

    void transform_vertex_positions(V4Verts2D& vertex_positions, const Matrix4& transform)
    {
        vertex_positions[0] = transform * vertex_positions[0];
        vertex_positions[1] = transform * vertex_positions[1];
        vertex_positions[2] = transform * vertex_positions[2];
        vertex_positions[3] = transform * vertex_positions[3];
    }

    void fill_quad_vertex_positions(const Vector2& size, V4Verts2D& vertex_positions)
    {
        if (abs(size.x - size.y) <= F32_EPSILON)
        {
            vertex_positions = DEFAULT_VERTEX_POSITIONS_2D;
            return;
        }
        
        Vector2 pos = normalize(size) / 2;;
        
        vertex_positions[0] = { -pos.x, -pos.y, 0.f, 1.f };
        vertex_positions[1] = {  pos.x, -pos.y, 0.f, 1.f };
        vertex_positions[2] = {  pos.x,  pos.y, 0.f, 1.f };
        vertex_positions[3] = { -pos.x,  pos.y, 0.f, 1.f };
    }

    void FlipQuadVertexUVs(V2Verts2D& vertex_uvs, bool flip_x, bool flip_y)
    {
        if (flip_x || flip_y)
        {
            const V2Verts2D uv = vertex_uvs;

            if (flip_x && flip_y)
            {
                vertex_uvs[0] = uv[3];
                vertex_uvs[1] = uv[2];
                vertex_uvs[2] = uv[1];
                vertex_uvs[3] = uv[0];
            }
            else if (flip_x)
            {
                vertex_uvs[0] = uv[1];
                vertex_uvs[1] = uv[0];
                vertex_uvs[2] = uv[3];
                vertex_uvs[3] = uv[2];
            }
            else if (flip_y)
            {
                vertex_uvs[0] = uv[2];
                vertex_uvs[1] = uv[3];
                vertex_uvs[2] = uv[0];
                vertex_uvs[3] = uv[1];
            }
        }
    }
    
    void fill_quad_vertex_u_vs(V2Verts2D& vertex_uvs, bool flip_x, bool flip_y, const Vector2& tiling_factor)
    {
        FlipQuadVertexUVs(vertex_uvs, flip_x, flip_y);

        for (i32 i = 0; i < 4; ++i)
        {
            vertex_uvs[i].x *= tiling_factor.x;
            vertex_uvs[i].y *= tiling_factor.y;
        }
    }

    void fill_quad_vertex_u_vs(
          V2Verts2D&     vertex_uvs
        , const Vector2& texture_size
        , const Vector2& sub_texture_size
        , const Vector2& sub_texture_location
        , bool           flip_x
        , bool           flip_y
        , const Vector2& tiling_factor
    )
    {
        Vector2 top_right, bottom_left;
        
        top_right.x = (sub_texture_location.x + sub_texture_size.x) * (1 / texture_size.x);
        top_right.y = 1 - ((sub_texture_location.y + sub_texture_size.y) * (1 / texture_size.y));

        bottom_left.x = sub_texture_location.x * (1 / texture_size.x);
        bottom_left.y = 1 - (sub_texture_location.y * (1 / texture_size.y));

        vertex_uvs[0] = { bottom_left.x, top_right.y };   // bottom-left
        vertex_uvs[1] = { top_right.x,   top_right.y };   // bottom-right
        vertex_uvs[2] = { top_right.x,   bottom_left.y }; // top-right
        vertex_uvs[3] = { bottom_left.x, bottom_left.y }; // top-left

        fill_quad_vertex_u_vs(vertex_uvs, flip_x, flip_y, tiling_factor);
    }

    void fill_circle_vertex_positions(V4Verts2D& vertex_positions, f32 radius)
    {
        radius *= 2.f;
        const Matrix4 scale_mat = mat_scale(Matrix4(), {radius, radius, 1.f});

        vertex_positions[0] = scale_mat * DEFAULT_VERTEX_POSITIONS_2D[0];
        vertex_positions[1] = scale_mat * DEFAULT_VERTEX_POSITIONS_2D[1];
        vertex_positions[2] = scale_mat * DEFAULT_VERTEX_POSITIONS_2D[2];
        vertex_positions[3] = scale_mat * DEFAULT_VERTEX_POSITIONS_2D[3];
    }

    void fill_line_2d_vertex_positions(V4Verts2D& vertex_positions, const Vector2& start, const Vector2& end, f32 thickness)
    {
        const Vector3 v3_start  = { start.x, start.y };
        const Vector3 v3_end    = { end.x, end.y };
        const Vector3 v3_dir    = normalize(v3_end - v3_start);
        const Vector3 v3_normal = {v3_dir.y, -v3_dir.x, 0}; // Perpendicular clockwise 

        Vector3 v_pos_0 = v3_start + v3_normal *  thickness / 2.f;
        Vector3 v_pos_1 = v3_end   + v3_normal *  thickness / 2.f;
        Vector3 v_pos_2 = v3_end   + v3_normal * -thickness / 2.f;
        Vector3 v_pos_3 = v3_start + v3_normal * -thickness / 2.f;
        
        vertex_positions[0] = {v_pos_0.x, v_pos_0.y, v_pos_0.z, 1 };
        vertex_positions[1] = {v_pos_1.x, v_pos_1.y, v_pos_1.z, 1 };
        vertex_positions[2] = {v_pos_2.x, v_pos_2.y, v_pos_2.z, 1 };
        vertex_positions[3] = {v_pos_3.x, v_pos_3.y, v_pos_3.z, 1 };
    }

    void fill_char_vertex_data(const Matrix4& transform, V4Verts2D& vertex_positions, V2Verts2D& vertex_uvs, const Font* font, f32 size, Vector2& offset, f32 spacing, char c)
    {
        CharData d;
        font_get_char(font, c, d);
        
        static constexpr f32 SCALE = 0.002f;

        Matrix4 transform_offset = mat_translate(transform, { offset.x, offset.y });
        transform_offset *= mat_scale({}, { SCALE * size, -SCALE * size, SCALE });

        vertex_positions[0] = {d.x0, d.y1, 0.f, 1.f};
        vertex_positions[1] = {d.x1, d.y1, 0.f, 1.f};
        vertex_positions[2] = {d.x1, d.y0, 0.f, 1.f};
        vertex_positions[3] = {d.x0, d.y0, 0.f, 1.f};

        vertex_uvs[0] = {d.s0, d.t1};
        vertex_uvs[1] = {d.s1, d.t1};
        vertex_uvs[2] = {d.s1, d.t0};
        vertex_uvs[3] = {d.s0, d.t0};

        transform_vertex_positions(vertex_positions, transform_offset);

        offset.x += d.x_pos * SCALE * spacing * size;
        offset.y += d.y_pos * SCALE * size;
    }
}