#pragma once

#include <cmath>

#include "Gamma.h"

#include "macros.h"

internal float easeInOutQuad(float t) {
  return t < 0.5f ? 4.f * t * t * t : 1.f - powf(-2.f * t + 2.f, 3) / 2.f;
}

internal float easeOutQuint(float t) {
  return 1.f - powf(1.f - t, 5);
}

internal float easeOutElastic(float t) {
  const float c4 = (2.f * Gm_PI) / 3.f;

  return (
    t == 0.f ? 0.f :
    t == 1.f ? 1.f :
    powf(2.f, -10.f * t) * sinf((t * 10.f - 0.75f) * c4) + 1.f
  );
}