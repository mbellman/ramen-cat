#pragma once

namespace Gamma {
  struct Vec2f {
    Vec2f() {};
    Vec2f(float f): x(f), y(f) {};
    Vec2f(float x, float y) : x(x), y(y) {};

    float x = 0.0f;
    float y = 0.0f;
  };

  struct Vec3f : Vec2f {
    Vec3f() {};
    Vec3f(float f) : Vec2f(f, f), z(f) {};
    Vec3f(float x, float y, float z) : Vec2f(x, y), z(z) {};

    float z = 0.0f;

    static Vec3f cross(const Vec3f& v1, const Vec3f& v2);
    static float dot(const Vec3f& v1, const Vec3f& v2);
    static Vec3f lerp(const Vec3f& v1, const Vec3f& v2, float alpha);

    bool operator==(const Vec3f& vector) const;
    Vec3f operator+(const Vec3f& vector) const;
    void operator+=(const Vec3f& vector);
    Vec3f operator-(const Vec3f& vector) const;
    void operator-=(const Vec3f& vector);
    Vec3f operator*(float scalar) const;
    Vec3f operator*(const Vec3f& vector) const;
    void operator*=(float scalar);
    void operator*=(const Vec3f& vector);
    Vec3f operator/(float divisor) const;
    void operator/=(float divisor);

    void debug() const;
    Vec3f gl() const;
    Vec3f invert() const;
    float magnitude() const;
    Vec3f unit() const;
    Vec3f xz() const;
  };

  struct Vec4f {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    Vec4f() {};
    Vec4f(float f) : x(f), y(f), z(f), w(f) {};
    Vec4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {};

    Vec3f homogenize() const;
    Vec3f toVec3f() const;
  };
}