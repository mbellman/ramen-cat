#pragma once

namespace Gamma {
  constexpr float Gm_PI = 3.141592f;
  constexpr float Gm_TAU = Gm_PI * 2.f;
  constexpr float Gm_HALF_PI = Gm_PI / 2.f;

  inline float Gm_Absf(float value) {
    return value < 0.f ? -value : value;
  }

  inline float Gm_Clampf(float value, float min, float max) {
    return value > max ? max : value < min ? min : value;
  }

  inline float Gm_Lerpf(float a, float b, float alpha) {
    return a + (b - a) * alpha;
  }

  /**
   * @todo description
   */
  inline float Gm_LerpCircularf(float a, float b, float alpha, float maxRange) {
    float range = b - a;

    if (range > maxRange) {
      a += maxRange * 2.f;
    } else if (range < -maxRange) {
      a -= maxRange * 2.f;
    }

    return a + (b - a) * alpha;
  }
}