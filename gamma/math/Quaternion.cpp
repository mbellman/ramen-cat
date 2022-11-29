#include <cmath>
#include <cstdio>

#include "math/matrix.h"
#include "math/orientation.h"
#include "math/Quaternion.h"
#include "math/utilities.h"
#include "math/vector.h"

namespace Gamma {
  Quaternion Quaternion::fromAxisAngle(float angle, float x, float y, float z) {
    float sa = sinf(angle / 2.0f);

    return {
      cosf(angle / 2.0f),
      x * sa,
      y * sa,
      z * sa
    };
  }

  /**
   * @source: https://wrf.ecse.rpi.edu/wiki/ComputerGraphicsFall2013/guha/Code/quaternionAnimation.cpp
   */
  Quaternion Quaternion::slerp(const Quaternion& q1, const Quaternion& q2, float alpha) {
    float w1, x1, y1, z1, w2, x2, y2, z2, w3, x3, y3, z3;
    float theta, mult1, mult2;

    w1 = q1.w;
    x1 = q1.x;
    y1 = q1.y;
    z1 = q1.z; 

    w2 = q2.w;
    x2 = q2.x;
    y2 = q2.y;
    z2 = q2.z;

    if (w1*w2 + x1*x2 + y1*y2 + z1*z2 < 0.f) {
      w2 = -w2;
      x2 = -x2;
      y2 = -y2;
      z2 = -z2;
    }

    theta = acosf(w1*w2 + x1*x2 + y1*y2 + z1*z2);

    if (theta > 0.000001f) {
      mult1 = sinf( (1.f-alpha)*theta ) / sinf( theta );
      mult2 = sinf( alpha*theta ) / sinf( theta );
    } else {
      mult1 = 1.f - alpha;
      mult2 = alpha;
    }

    w3 =  mult1*w1 + mult2*w2;
    x3 =  mult1*x1 + mult2*x2;
    y3 =  mult1*y1 + mult2*y2;
    z3 =  mult1*z1 + mult2*z2;

    Quaternion r;

    r.w = w3;
    r.x = x3;
    r.y = y3;
    r.z = z3;

    return r.unit();
  }

  Quaternion Quaternion::operator*(const Quaternion& q2) const {
    return {
      w * q2.w - x * q2.x - y * q2.y - z * q2.z,
      w * q2.x + x * q2.w + y * q2.z - z * q2.y,
      w * q2.y - x * q2.z + y * q2.w + z * q2.x,
      w * q2.z + x * q2.y - y * q2.x + z * q2.w
    };
  }

  void Quaternion::operator*=(const Quaternion& q2) {
    *this = *this * q2;
  }

  void Quaternion::debug() const {
    printf("{ %f, %f, %f, %f }\n", w, x, y, z);
  }

  Vec3f Quaternion::getDirection() const {
    const static Vec3f forward = Vec3f(0, 0, 1.f);

    return (toMatrix4f() * forward).toVec3f();
  }

  Vec3f Quaternion::getUpDirection() const {
    const static Vec3f up = Vec3f(0, 1.f, 0);

    return (toMatrix4f() * up).toVec3f();
  }

  Vec3f Quaternion::getLeftDirection() const {
    const static Vec3f left = Vec3f(-1.f, 0, 0);

    return (toMatrix4f() * left).toVec3f();
  }

  void Quaternion::rotateAboutAxis(const Vec3f& axis, float angle) {
    Quaternion rotation = Quaternion::fromAxisAngle(angle, axis.x, axis.y, axis.z);

    *this *= rotation;
  }

  Matrix4f Quaternion::toMatrix4f() const {
    return {
      1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * z * w, 2 * x * z + 2 * y * w, 0.0f,
      2 * x * y + 2 * z * w, 1 - 2 * x * x - 2 * z * z, 2 * y * z - 2 * x * w, 0.0f,
      2 * x * z - 2 * y * w, 2 * y * z + 2 * x * w, 1 - 2 * x * x - 2 * y * y, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
  }

  Quaternion Quaternion::unit() const {
    auto magnitude = sqrtf(w*w + x*x + y*y + z*z);

    return {
      w / magnitude,
      x / magnitude,
      y / magnitude,
      z / magnitude
    };
  }
}