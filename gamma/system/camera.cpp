#include <cmath>

#include "math/utilities.h"
#include "system/camera.h"

namespace Gamma {
  /**
   * ThirdPersonCamera::calculatePosition()
   * --------------------------------------
   *
   * Computes a Vec3f position as spherical coordinates
   * derived from the third-person camera's altitude,
   * azimuth, and radius.
   */
  Vec3f ThirdPersonCamera::calculatePosition() const {
    return Vec3f(
      cosf(altitude) * cosf(azimuth) * radius,
      sinf(altitude) * radius,
      cosf(altitude) * sinf(azimuth) * radius
    );
  }

  /**
   * ThirdPersonCamera::isUpsideDown()
   * ---------------------------------
   *
   * Determines whether the third-person camera is oriented
   * upside-down, as determined by its altitude value.
   */
  bool ThirdPersonCamera::isUpsideDown() const {
    return cosf(altitude) < 0.0f;
  }

  /**
   * ThirdPersonCamera::limitAltitude()
   * ----------------------------------
   *
   * Restricts the third-person camera's altitude based on
   * a provided factor, where a factor of exactly 1 allows
   * the altitude to reach the poles and no further.
   */
  void ThirdPersonCamera::limitAltitude(float factor) {
    constexpr float BASE_ALTITUDE_LIMIT = Gm_HALF_PI * 0.999f;
    float altitudeLimit = BASE_ALTITUDE_LIMIT * factor;

    altitude = Gm_Clampf(altitude, -altitudeLimit, altitudeLimit);
  }
}