#pragma once

#include <cmath>

#include "Gamma.h"

#include "macros.h"

internal float easeInOutQuad(float t) {
  return t < 0.5f ? 4.f * t * t * t : 1.f - powf(-2.f * t + 2.f, 3) / 2.f;
}

internal float easeInOutQuart(float t) {
  return t < 0.5f ? 8.f * t * t * t * t : 1.f - powf(-2.f * t + 2.f, 4) / 2.f;
}

internal float easeOutQuint(float t) {
  return 1.f - powf(1.f - t, 5);
}

internal float easeOutElastic(float t) {
  const float c4 = (2.f * Gm_PI) / 3.f;

  return (
    t == 0.f ? 0.f :
    t == 1.f ? 1.f :
    powf(1.7f, -10.f * t) * sinf((t * 5.f - 1.f) * c4) + 1.f
  );
}

internal float easeOutBack(float t, float overshot) {
  float x = overshot + 1.f;

  return 1.f + x * powf(t - 1.f, 3) + overshot * powf(t - 1.f, 2);
}