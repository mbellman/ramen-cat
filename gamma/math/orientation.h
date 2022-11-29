#pragma once

#include "math/vector.h"

namespace Gamma {
  struct Quaternion;

  struct Orientation {
    float roll = 0.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;

    Orientation() {};
    // @todo pitch, yaw, roll
    Orientation(float roll, float pitch, float yaw): roll(roll), pitch(pitch), yaw(yaw) {};

    void operator+=(const Orientation& orientation);

    Vec3f getDirection() const;
    Vec3f getLeftDirection() const;
    Vec3f getRightDirection() const;
    Vec3f getUpDirection() const;
    void face(const Vec3f& forward, const Vec3f& up);
    Orientation invert() const;
    Quaternion toQuaternion() const;
    Vec3f toVec3f() const;
  };
}