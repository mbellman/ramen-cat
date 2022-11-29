#include <cmath>

#include "math/constants.h"
#include "math/orientation.h"
#include "math/Quaternion.h"
#include "math/matrix.h"
#include "math/vector.h"

namespace Gamma {
  void Orientation::operator+=(const Orientation& orientation) {
    roll += orientation.roll;
    pitch += orientation.pitch;
    yaw += orientation.yaw;
  }

  Vec3f Orientation::getDirection() const {
    const static Vec3f forward = Vec3f(0, 0, 1.f);

    float cosr = cosf(-roll * 0.5f);
    float sinr = sinf(-roll * 0.5f);
    float cosy = cosf(yaw * 0.5f);
    float siny = sinf(yaw * 0.5f);
    float cosp = cosf(pitch * 0.5f);
    float sinp = sinf(pitch * 0.5f);

    Quaternion q;

    q.w = cosp * cosy * cosr + sinp * siny * sinr;
    q.x = sinp * cosy * cosr - cosp * siny * sinr;
    q.y = cosp * siny * cosr + sinp * cosy * sinr;
    q.z = cosp * cosy * sinr - sinp * siny * cosr;

    return (q.toMatrix4f() * forward).toVec3f();
  }

  Vec3f Orientation::getLeftDirection() const {
    return Vec3f::cross(getDirection(), getUpDirection()).unit();
  }

  Vec3f Orientation::getRightDirection() const {
    return Vec3f::cross(getUpDirection(), getDirection()).unit();
  }

  Vec3f Orientation::getUpDirection() const {
    return Orientation(roll, (pitch - PI / 2.0f), yaw).getDirection();
  }

  void Orientation::face(const Vec3f& forward, const Vec3f& up) {
    const static Vec3f worldUp = Vec3f(0, 1.f, 0);

    float uDotY = Vec3f::dot(up, worldUp);

    // Calculate yaw as a function of forward z/x
    yaw = -1.0f * (atan2f(forward.z, forward.x) - HALF_PI);

    if (uDotY < 0.0f) {
      // If upside-down, flip the yaw by 180 degrees
      yaw -= PI;
    }

    Vec3f rUp = up;

    // Rotate the up vector back onto the y/z plane,
    // and calculate pitch as a function of y/z
    rUp.z = up.x * sinf(yaw) + up.z * cosf(yaw);

    pitch = -1.0f * (atan2f(rUp.y, rUp.z) - HALF_PI);
  }

  Orientation Orientation::invert() const {
    return Orientation(-roll, -pitch, -yaw);
  }

  // @todo determine why the approach used in getDirection()
  // doesn't work when generating a quaternion used for the
  // camera view matrix
  Quaternion Orientation::toQuaternion() const {
    Quaternion qp = Quaternion::fromAxisAngle(pitch, 1.0f, 0.0f, 0.0f);
    Quaternion qy = Quaternion::fromAxisAngle(yaw, 0.0f, 1.0f, 0.0f);
    Quaternion qr = Quaternion::fromAxisAngle(roll, 0.0f, 0.0f, 1.0f);

    return (qp * qy * qr);
  }

  Vec3f Orientation::toVec3f() const {
    return Vec3f(pitch, yaw, roll);
  }
}