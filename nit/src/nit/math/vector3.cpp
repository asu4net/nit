#include "vector3.h"

namespace nit
{
    bool operator==(const Vector3& a, const Vector3& b)
    {
        return (abs(a.x - b.x) <= F32_EPSILON) && (abs(a.y - b.y) <= F32_EPSILON) && (abs(a.z - b.z) <= F32_EPSILON);
    }

    bool operator!=(const Vector3& a, const Vector3& b)
    {
        return !(a == b);
    }

    Vector3 operator+(const Vector3& a, const Vector3& b)
    {
        return { a.x + b.x, a.y + b.y, a.z + b.z };
    }

    Vector3 operator-(const Vector3& a, const Vector3& b)
    {
        return { a.x - b.x, a.y - b.y, a.z - b.z };
    }

    Vector3 operator*(const Vector3& vector, f32 num)
    {
        return { vector.x * num, vector.y * num, vector.z * num };
    }

    Vector3 operator*(const Vector3& a, const Vector3& b)
    {
        return { a.x * b.x, a.y * b.y, a.z * b.z };
    }

    Vector3 operator/(const Vector3& vector, f32 num)
    {
        return { vector.x / num, vector.y / num, vector.z / num };
    }

    Vector3& operator+=(Vector3& left, const Vector3& right)
    {
        left.x += right.x;
        left.y += right.y;
        left.z += right.z;
        return left;
    }

    Vector3& operator-=(Vector3& left, const Vector3& right)
    {
        left.x -= right.x;
        left.y -= right.y;
        left.z -= right.z;
        return left;
    }

    Vector3& operator*=(Vector3& left, f32 num)
    {
        left.x *= num;
        left.y *= num;
        left.z *= num;
        return left;
    }

    Vector3& operator/=(Vector3& left, f32 num)
    {
        left.x /= num;
        left.y /= num;
        left.z /= num;
        return left;
    }
    
    Vector3 look_rotation(const Vector3& rotation, const Vector3& dir)
    {
        Matrix4 rotation_mat;
        rotation_mat = mat_rotate(rotation_mat, rotation);
        Vector4 v4_dir = { dir.x, dir.y, dir.z, 1.f };
        Vector4 look_rot = rotation_mat *  v4_dir; 
        return { look_rot.x, look_rot.y, look_rot.z };
    }

    f32 length(const Vector3& val)
    {
        return std::sqrt(val.x * val.x + val.y * val.y + val.z * val.z);
    }

    template <>
    f32 magnitude<Vector3>(const Vector3& val)
    {
        return std::sqrt(std::powf(val.x, 2) + std::powf(val.y, 2) + std::powf(val.z, 2));
    }
    template <>
    Vector3 normalize<Vector3>(const Vector3& val)
    {
        f32 mag = magnitude(val);
        return { val.x / mag, val.y / mag, val.z / mag };
    }
    

    template <>
    Vector3 multiply<Vector3>(const Vector3& a, const Vector3& b)
    {
        Vector3 r;
        r.x = a.x * b.x;
        r.y = a.y * b.y;
        r.z = a.z * b.z;
        return r;
    }

    template <>
    Vector3 divide<Vector3>(const Vector3& a, const Vector3& b)
    {
        Vector3 r;
        r.x = a.x / b.x;
        r.y = a.y / b.y;
        r.z = a.z / b.z;
        return r;
    }

    template <>
    f32 dot<Vector3>(const Vector3& a, const Vector3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    template <>
    f32 distance<Vector3>(const Vector3& a, const Vector3& b)
    {
        return std::sqrtf(std::powf(a.x - b.x, 2) + std::powf(a.y - b.y, 2) + std::powf(a.z - b.z, 2));
    }

    template<>
    f32 sqrd_distance<Vector3>(const Vector3& a, const Vector3& b)
    {
        return std::powf(a.x - b.x, 2) + std::powf(a.y - b.y, 2) + std::powf(a.z - b.z, 2);
    }

    Vector3 to_v3(const Vector2& value)
    {
        return { value.x, value.y, 0.f };
    }
}
