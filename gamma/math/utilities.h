#pragma once

#include <cmath>

constexpr float Gm_PI = 3.141592f;
constexpr float Gm_TAU = Gm_PI * 2.f;
constexpr float Gm_HALF_PI = Gm_PI / 2.f;
constexpr float Gm_FLOAT_MAX = 3.402823466e+38F;

inline float Gm_Absf(float value) {
  return value < 0.f ? -value : value;
}

inline float Gm_Minf(float value, float min) {
  return value < min ? value : min;
}

inline float Gm_Maxf(float value, float max) {
  return value > max ? value : max;
}

inline float Gm_Clampf(float value, float min, float max) {
  return value > max ? max : value < min ? min : value;
}

inline float Gm_Clampf(float value) {
  return Gm_Clampf(value, 0.f, 1.f);
}

inline float Gm_Lerpf(float a, float b, float alpha) {
  return a + (b - a) * alpha;
}

inline float Gm_Modf(float value, float m) {
  return value - m * floorf(value / m);
}

inline float Gm_Signf(float value) {
  return value < 0.f ? -1.f : 1.f;
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

inline float Gm_EaseInOut(float t) {
  return -(std::cosf(Gm_PI * t) - 1.f) / 2.f;
}