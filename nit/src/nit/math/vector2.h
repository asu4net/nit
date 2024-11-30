﻿#pragma once
#include "nit/math/math_common.h"

namespace nit
{
    struct Vector2
    {
        f32 x = 0;
        f32 y = 0;
    };

    inline constexpr Vector2 V2_ZERO    = { 0,  0, };
    inline constexpr Vector2 V2_ONE     = { 1,  1, };
    inline constexpr Vector2 V2_RIGHT   = { 1,  0, };
    inline constexpr Vector2 V2_LEFT    = {-1,  0, };
    inline constexpr Vector2 V2_UP      = { 0,  1, };
    inline constexpr Vector2 V2_DOWN    = { 0, -1, };
    
    bool      operator==  (const Vector2& a, const Vector2& b);
    bool      operator!=  (const Vector2& a, const Vector2& b);
    Vector2   operator+   (const Vector2& a, const Vector2& b);
    Vector2   operator-   (const Vector2& a, const Vector2& b);
    Vector2   operator*   (const Vector2& vector, f32 num);
    Vector2   operator/   (const Vector2& vector, f32 num);
    Vector2&  operator+=  (Vector2& left, const Vector2& right);
    Vector2&  operator-=  (Vector2& left, const Vector2& right);
    Vector2&  operator*=  (Vector2& left, f32 num);
    Vector2&  operator/=  (Vector2& left, f32 num);
    
    f32     angle(const Vector2& a, const Vector2& b);
    Vector2 rotate_around(Vector2 pivot, f32 angle, Vector2 point);
    Vector2 to_v2(const struct Vector3& value);
    Vector2 random_point_in_square(f32 x_min, f32 y_min, f32 x_max, f32 y_max);
    
    template<>
    Vector2 abs(const Vector2& val);

    template<>
    f32 magnitude(const Vector2& val);

    template<>
    Vector2 normalize(const Vector2& val);

    template<>
    Vector2 multiply(const Vector2& a, const Vector2& b);

    template<>
    Vector2 divide(const Vector2& a, const Vector2& b);

    template<>
    f32 dot(const Vector2& a, const Vector2& b);

    template<>
    f32 distance(const Vector2& a, const Vector2& b);

    
}

template<>
struct YAML::convert<nit::Vector2>
{
    static Node encode(const nit::Vector2& v)
    {
        Node node;
        node.push_back(v.x);
        node.push_back(v.y);
        node.SetStyle(EmitterStyle::Flow);
        return node;
    }

    static bool decode(const Node& node, nit::Vector2& v)
    {
        if (!node.IsSequence() || node.size() != 2)
            return false;
        
        v.x = node[0].as<float>();
        v.y = node[1].as<float>();
        return true;
    }
};

inline YAML::Emitter& operator<<(YAML::Emitter& out, const nit::Vector2& c)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << c.x << c.y << YAML::EndSeq;
    return out;
}