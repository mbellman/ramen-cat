#include <cmath>
#include <cstdio>

#include "math/constants.h"
#include "math/matrix.h"
#include "math/orientation.h"
#include "math/Quaternion.h"

namespace Gamma {
  /**
   * Matrix4f
   * -------
   */
  Matrix4f Matrix4f::glPerspective(const Area<u32>& area, float fov, float near, float far) {
    float f = 1.0f / tanf(fov / 2.0f * DEGREES_TO_RADIANS);
    float aspectRatio = (float)area.width / (float)area.height;

    return {
      f / aspectRatio, 0.0f, 0.0f, 0.0f,
      0.0f, f, 0.0f, 0.0f,
      0.0f, 0.0f, (far + near) / (near - far), (2 * far * near) / (near - far),
      0.0f, 0.0f, -1.0f, 0.0f
    };
  }

  Matrix4f Matrix4f::identity() {
    return {
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
  }

  Matrix4f Matrix4f::lookAt(const Vec3f& eye, const Vec3f& direction, const Vec3f& top) {
    Vec3f forward = direction.unit();
    Vec3f right = Vec3f::cross(top, forward).unit();
    Vec3f up = Vec3f::cross(forward, right).unit();
    Matrix4f translation = Matrix4f::translation(eye.invert());

    Matrix4f rotation = {
      right.x, right.y, right.z, 0.0f,
      up.x, up.y, up.z, 0.0f,
      forward.x, forward.y, forward.z, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    return rotation * translation;
  }

  Matrix4f Matrix4f::inverse() const {
    float A2323 = m[10] * m[15] - m[11] * m[14];
    float A1323 = m[9] * m[15] - m[11] * m[13];
    float A1223 = m[9] * m[14] - m[10] * m[13];
    float A0323 = m[8] * m[15] - m[11] * m[12];
    float A0223 = m[8] * m[14] - m[10] * m[12];
    float A0123 = m[8] * m[13] - m[9] * m[12];
    float A2313 = m[6] * m[15] - m[7] * m[14];
    float A1313 = m[5] * m[15] - m[7] * m[13];
    float A1213 = m[5] * m[14] - m[6] * m[13];
    float A2312 = m[6] * m[11] - m[7] * m[10];
    float A1312 = m[5] * m[11] - m[7] * m[9];
    float A1212 = m[5] * m[10] - m[6] * m[9];
    float A0313 = m[4] * m[15] - m[7] * m[12];
    float A0213 = m[4] * m[14] - m[6] * m[12];
    float A0312 = m[4] * m[11] - m[7] * m[8];
    float A0212 = m[4] * m[10] - m[6] * m[8];
    float A0113 = m[4] * m[13] - m[5] * m[12];
    float A0112 = m[4] * m[9] - m[5] * m[8];

    float determinant = 1.0f / (
      m[0] * (m[5] * A2323 - m[6] * A1323 + m[7] * A1223) -
      m[1] * (m[4] * A2323 - m[6] * A0323 + m[7] * A0223) +
      m[2] * (m[4] * A1323 - m[5] * A0323 + m[7] * A0123) -
      m[3] * (m[4] * A1223 - m[5] * A0223 + m[6] * A0123)
    );

    Matrix4f inverse;

    inverse.m[0] = determinant *  (m[5] * A2323 - m[6] * A1323 + m[7] * A1223);
    inverse.m[1] = determinant * -(m[1] * A2323 - m[2] * A1323 + m[3] * A1223);
    inverse.m[2] = determinant *  (m[1] * A2313 - m[2] * A1313 + m[3] * A1213);
    inverse.m[3] = determinant * -(m[1] * A2312 - m[2] * A1312 + m[3] * A1212);
    inverse.m[4] = determinant * -(m[4] * A2323 - m[6] * A0323 + m[7] * A0223);
    inverse.m[5] = determinant *  (m[0] * A2323 - m[2] * A0323 + m[3] * A0223);
    inverse.m[6] = determinant * -(m[0] * A2313 - m[2] * A0313 + m[3] * A0213);
    inverse.m[7] = determinant *  (m[0] * A2312 - m[2] * A0312 + m[3] * A0212);
    inverse.m[8] = determinant *  (m[4] * A1323 - m[5] * A0323 + m[7] * A0123);
    inverse.m[9] = determinant * -(m[0] * A1323 - m[1] * A0323 + m[3] * A0123);
    inverse.m[10] = determinant *  (m[0] * A1313 - m[1] * A0313 + m[3] * A0113);
    inverse.m[11] = determinant * -(m[0] * A1312 - m[1] * A0312 + m[3] * A0112);
    inverse.m[12] = determinant * -(m[4] * A1223 - m[5] * A0223 + m[6] * A0123);
    inverse.m[13] = determinant *  (m[0] * A1223 - m[1] * A0223 + m[2] * A0123);
    inverse.m[14] = determinant * -(m[0] * A1213 - m[1] * A0213 + m[2] * A0113);
    inverse.m[15] = determinant *  (m[0] * A1212 - m[1] * A0212 + m[2] * A0112);

    return inverse;
  }

  Matrix4f Matrix4f::orthographic(float top, float bottom, float left, float right, float near, float far) {
    return {
      2.0f / (right - left), 0.0f, 0.0f, -(right + left) / (right - left),
      0.0f, 2.0f / (top - bottom), 0.0f, -(top + bottom) / (top - bottom),
      0.0f, 0.0f, -2.0f / (far - near), -(far + near) / (far - near),
      0.0f, 0.0f, 0.0f, 1.0f
    };
  }

  Matrix4f Matrix4f::rotation(const Vec3f& rotation) {
    Quaternion pitch = Quaternion::fromAxisAngle(rotation.x, 1.0f, 0.0f, 0.0f);
    Quaternion yaw = Quaternion::fromAxisAngle(rotation.y, 0.0f, 1.0f, 0.0f);
    Quaternion roll = Quaternion::fromAxisAngle(rotation.z, 0.0f, 0.0f, 1.0f);

    return (roll * pitch * yaw).toMatrix4f();
  }

  Matrix4f Matrix4f::rotation(const Orientation& orientation) {
    Quaternion pitch = Quaternion::fromAxisAngle(orientation.pitch, 1.0f, 0.0f, 0.0f);
    Quaternion yaw = Quaternion::fromAxisAngle(orientation.yaw, 0.0f, 1.0f, 0.0f);
    Quaternion roll = Quaternion::fromAxisAngle(orientation.roll, 0.0f, 0.0f, 1.0f);

    return (pitch * yaw * roll).toMatrix4f();
  }

  Matrix4f Matrix4f::scale(const Vec3f& scale) {
    return {
      scale.x, 0.0f, 0.0f, 0.0f,
      0.0f, scale.y, 0.0f, 0.0f,
      0.0f, 0.0f, scale.z, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
  }

  Matrix4f Matrix4f::transformation(const Vec3f& translation, const Vec3f& scale, const Vec3f& rotation) {
    Matrix4f m_transform;
    Matrix4f m_scale = Matrix4f::scale(scale);
    Matrix4f m_rotation = Matrix4f::rotation(rotation);

    // Declares a small float buffer which helps reduce the number
    // of cache misses in the scale * rotation loop. Scale terms
    // can be written in sequentially, followed by rotation terms,
    // followed by a sequential read when multiplying the buffered
    // terms. Confers a ~5-10% speedup, which is appreciable once
    // the number of transforms per frame reaches into the thousands.
    float v[6];

    // Accumulate rotation * scale
    for (u32 r = 0; r < 3; r++) {
      // Store rotation terms
      v[0] = m_rotation.m[r * 4];
      v[2] = m_rotation.m[r * 4 + 1];
      v[4] = m_rotation.m[r * 4 + 2];

      for (u32 c = 0; c < 3; c++) {
        // Store scale terms
        v[1] = m_scale.m[c];
        v[3] = m_scale.m[4 + c];
        v[5] = m_scale.m[8 + c];

        // rotation * scale
        m_transform.m[r * 4 + c] = (
          v[0] * v[1] +
          v[2] * v[3] +
          v[4] * v[5]
        );
      }
    }

    // Apply translation directly
    m_transform.m[3] = translation.x;
    m_transform.m[7] = translation.y;
    m_transform.m[11] = translation.z;
    m_transform.m[15] = 1.0f;

    return m_transform;
  }

  Matrix4f Matrix4f::translation(const Vec3f& translation) {
    return {
      1.0f, 0.0f, 0.0f, translation.x,
      0.0f, 1.0f, 0.0f, translation.y,
      0.0f, 0.0f, 1.0f, translation.z,
      0.0f, 0.0f, 0.0f, 1.0f
    };
  }

  Matrix4f Matrix4f::transpose() const {
    return {
      m[0], m[4], m[8], m[12],
      m[1], m[5], m[9], m[13],
      m[2], m[6], m[10], m[14],
      m[3], m[7], m[11], m[15]
    };
  }

  void Matrix4f::debug() const {
    for (u32 i = 0; i < 4; i++) {
      printf("[ %f, %f, %f, %f ]\n", m[i * 4], m[i * 4 + 1], m[i * 4 + 2], m[i * 4 + 3]);
    }

    printf("\n");
  }

  Matrix4f Matrix4f::operator*(const Matrix4f& matrix) const {
    Matrix4f product;

    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        float& value = product.m[r * 4 + c] = 0;

        for (int n = 0; n < 4; n++) {
          value += m[r * 4 + n] * matrix.m[n * 4 + c];
        }
      }
    }

    return product;
  }

  Vec4f Matrix4f::operator*(const Vec3f& vector) const {
    float x = vector.x;
    float y = vector.y;
    float z = vector.z;
    float w = 1.0f;

    return Vec4f(
      x * m[0] + y * m[1] + z * m[2] + w * m[3],
      x * m[4] + y * m[5] + z * m[6] + w * m[7],
      x * m[8] + y * m[9] + z * m[10] + w * m[11],
      x * m[12] + y * m[13] + z * m[14] + w * m[15]
    );
  }
}