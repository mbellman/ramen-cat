#pragma once

namespace Gamma {
  struct Matrix4f;
  struct Orientation;
  struct Vec3f;

  /**
   * Quaternion
   * ----------
   */
  struct Quaternion {
    float w;
    float x;
    float y;
    float z;

    static Quaternion fromAxisAngle(float angle, float x, float y, float z);
    static Quaternion slerp(const Quaternion& q1, const Quaternion& q2, float alpha);

    Quaternion operator*(const Quaternion& q2) const;
    void operator*=(const Quaternion& q2);

    void debug() const;
    Vec3f getDirection() const;
    Vec3f getLeftDirection() const;
    Vec3f getUpDirection() const;
    void rotateAboutAxis(const Vec3f& axis, float angle);
    Matrix4f toMatrix4f() const;
    Quaternion unit() const;
  };
}