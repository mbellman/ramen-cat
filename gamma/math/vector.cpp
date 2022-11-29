#include <cstdio>
#include <math.h>

#include "math/vector.h"
#include "math/utilities.h"

namespace Gamma {
  /**
   * Vec3f
   * -----
   */
  bool Vec3f::operator==(const Vec3f& vector) const {
    return x == vector.x && y == vector.y && z == vector.z;
  }

  Vec3f Vec3f::operator+(const Vec3f& vector) const {
    return {
      x + vector.x,
      y + vector.y,
      z + vector.z
    };
  }

  void Vec3f::operator+=(const Vec3f& vector) {
    x += vector.x;
    y += vector.y;
    z += vector.z;
  }

  Vec3f Vec3f::operator-(const Vec3f& vector) const {
    return {
      x - vector.x,
      y - vector.y,
      z - vector.z
    };
  }

  void Vec3f::operator-=(const Vec3f& vector) {
    x -= vector.x;
    y -= vector.y;
    z -= vector.z;
  }

  Vec3f Vec3f::operator*(float scalar) const {
    return {
      x * scalar,
      y * scalar,
      z * scalar
    };
  }

  Vec3f Vec3f::operator*(const Vec3f& vector) const {
    return {
      x * vector.x,
      y * vector.y,
      z * vector.z
    };
  }

  void Vec3f::operator*=(float scalar) {
    *this = *this * scalar;
  }

  void Vec3f::operator*=(const Vec3f& vector) {
    *this = *this * vector;
  }

  Vec3f Vec3f::operator/(float divisor) const {
    return {
      x / divisor,
      y / divisor,
      z / divisor
    };
  }

  void Vec3f::operator/=(float divisor) {
    x /= divisor;
    y /= divisor;
    z /= divisor;
  }

  Vec3f Vec3f::cross(const Vec3f& v1, const Vec3f& v2) {
    return {
      v1.y * v2.z - v1.z * v2.y,
      v1.z * v2.x - v1.x * v2.z,
      v1.x * v2.y - v1.y * v2.x
    };
  }

  void Vec3f::debug() const {
    printf("{ %f, %f, %f }\n", x, y, z);
  }

  float Vec3f::dot(const Vec3f& v1, const Vec3f& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  }

  Vec3f Vec3f::gl() const {
    return *this * Vec3f(1.0f, 1.0f, -1.0f);
  }

  Vec3f Vec3f::invert() const {
    return *this * -1.0f;
  }

  Vec3f Vec3f::lerp(const Vec3f& v1, const Vec3f& v2, float alpha) {
    return Vec3f(
      Gm_Lerpf(v1.x, v2.x, alpha),
      Gm_Lerpf(v1.y, v2.y, alpha),
      Gm_Lerpf(v1.z, v2.z, alpha)
    );
  }

  float Vec3f::magnitude() const {
    return sqrtf(x * x + y * y + z * z);
  }

  Vec3f Vec3f::unit() const {
    float m = magnitude();

    return {
      x / m,
      y / m,
      z / m
    };
  }

  Vec3f Vec3f::xz() const {
    return *this * Vec3f(1.0f, 0.0f, 1.0f);
  }

  /**
   * Vec4f
   * -----
   */
  Vec3f Vec4f::homogenize() const {
    return Vec3f(x / w, y / w, z / w);
  }

  // @todo rename xyz()
  Vec3f Vec4f::toVec3f() const {
    return Vec3f(x, y, z);
  }
}