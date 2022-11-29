#pragma once

#include "math/matrix.h"
#include "math/orientation.h"
#include "math/Quaternion.h"
#include "math/vector.h"

namespace Gamma {
  struct Camera {
    Vec3f position;
    Orientation orientation;
    // @todo @bug higher fov values produce faulty SSAO
    // near the edges of geometry against the sky
    float fov = 45.f;
    Quaternion rotation = Orientation(0, 0, 0).toQuaternion();
  };

  struct ThirdPersonCamera {
    float azimuth = 0.0f;
    float altitude = 0.0f;
    float radius = 100.0f;

    Vec3f calculatePosition() const;
    bool isUpsideDown() const;
    void limitAltitude(float factor);
  };
}