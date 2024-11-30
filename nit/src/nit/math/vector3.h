﻿#pragma once
#include "nit/math/math_common.h"

namespace nit
{
    struct Vector3
    {
        f32 x = 0;
        f32 y = 0;
        f32 z = 0;
    };

    inline constexpr Vector3 V3_ZERO    = { 0,  0,  0 };
    inline constexpr Vector3 V3_ONE     = { 1,  1,  1 };
    inline constexpr Vector3 V3_RIGHT   = { 1,  0,  0 };
    inline constexpr Vector3 V3_LEFT    = {-1,  0,  0 };
    inline constexpr Vector3 V3_UP      = { 0,  1,  0 };
    inline constexpr Vector3 V3_DOWN    = { 0, -1,  0 };
    inline constexpr Vector3 V3_BACK    = { 0,  0, -1 };
    inline constexpr Vector3 V3_FRONT   = { 0,  0,  1 };
    
    bool operator==(const Vector3& a, const Vector3& b);
    
    bool operator!=(const Vector3& a, const Vector3& b);
    
    Vector3 operator+(const Vector3& a, const Vector3& b);
    
    Vector3 operator-(const Vector3& a, const Vector3& b);
    
    Vector3 operator*(const Vector3& vector, f32 num);
    
    Vector3 operator/(const Vector3& vector, f32 num);

    Vector3& operator+=(Vector3& left, const Vector3& right);
    
    Vector3& operator-=(Vector3& left, const Vector3& right);

    Vector3& operator*=(Vector3& left, f32 num);

    Vector3& operator/=(Vector3& left, f32 num);

    Vector3 LookRotation(const Vector3& rotation, const Vector3& dir);
    
    inline Vector3 to_degrees(const Vector3& radians) { return radians * RADIANS_TO_DEGREES_FACTOR; }
    
    inline Vector3 to_radians(const Vector3& degrees) { return degrees * DEGREES_TO_RADIANS_FACTOR; }

    f32 Lenght(const Vector3& val);
    
    template<>
    inline Vector3 Abs(const Vector3& val) { return { Abs(val.x), Abs(val.y), Abs(val.z)  }; }

    template<>
    f32 Magnitude(const Vector3& val);

    template<>
    Vector3 Normalize(const Vector3& val);

    template<>
    Vector3 Multiply(const Vector3& a, const Vector3& b);

    template<>
    Vector3 Divide(const Vector3& a, const Vector3& b);

    template<>
    f32 Dot(const Vector3& a, const Vector3& b);

    template<>
    f32 Distance(const Vector3& a, const Vector3& b);

    Vector3 ToVector3(const struct Vector2& value);
}

template<>
struct YAML::convert<nit::Vector3>
{
    static Node encode(const nit::Vector3& v)
    {
        Node node;
        node.push_back(v.x);
        node.push_back(v.y);
        node.push_back(v.z);
        node.SetStyle(EmitterStyle::Flow);
        return node;
    }

    static bool decode(const Node& node, nit::Vector3& v)
    {
        if (!node.IsSequence() || node.size() != 3)
            return false;
        
        v.x = node[0].as<float>();
        v.y = node[1].as<float>();
        v.z = node[2].as<float>();
        return true;
    }
};

inline YAML::Emitter& operator<<(YAML::Emitter& out, const nit::Vector3& c)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << c.x << c.y << c.z << YAML::EndSeq;
    return out;
}