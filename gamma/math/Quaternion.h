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
    float w = 0.f;
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;

    Quaternion() {};
    Quaternion(float f): w(f), x(f), y(f), z(f) {};
    Quaternion(float w, float x, float y, float z): w(w), x(x), y(y), z(z) {};

    static Quaternion fromAxisAngle(float angle, float x, float y, float z);
    static Quaternion fromEulerAngles(float x, float y, float z);
    static Quaternion slerp(const Quaternion& q1, const Quaternion& q2, float alpha);

    bool operator==(const Quaternion& q2) const;
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