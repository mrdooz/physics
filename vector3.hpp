#pragma once

namespace physics
{
  #pragma pack(push, 1)
  struct Vector3
  {
    Vector3() {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    float x, y, z;

    Vector3& operator *= (float s) { x *= s; y *= s; z *= s; return *this; }
    float& operator[](u32 idx) { return ((float*)&x)[idx]; }
    float operator[](u32 idx) const { return ((float*)&x)[idx]; }
  };

  #pragma pack(pop)

  float Dot(const Vector3& lhs, const Vector3& rhs)
  {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
  }

  Vector3 Cross(const Vector3& lhs, const Vector3& rhs)
  {
    return Vector3(
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x);
  }

  float Length(const Vector3& v)
  {
    float dx = v.x * v.x;
    float dy = v.y * v.y;
    float dz = v.z * v.z;
    return sqrtf(dx+dy+dz);
  }

  Vector3 operator+(const Vector3& lhs, const Vector3& rhs)
  {
    return Vector3(lhs.x + rhs.y, lhs.y + rhs.y, lhs.z + rhs.z);
  }

  Vector3 operator-(const Vector3& lhs, const Vector3& rhs)
  {
    return Vector3(lhs.x - rhs.y, lhs.y - rhs.y, lhs.z - rhs.z);
  }

  Vector3 operator*(float s, const Vector3& v)
  {
    return Vector3(s*v.x, s*v.y, s*v.z);
  }

  Vector3 operator*(const Vector3& v, float s)
  {
    return Vector3(s*v.x, s*v.y, s*v.z);
  }
}