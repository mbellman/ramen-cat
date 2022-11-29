#include "system/random.h"

static std::random_device randomDevice;
static std::default_random_engine randomEngine(randomDevice());
static std::uniform_real_distribution<float> randomRange(0.f, 1.f);

float Gm_Random(float low, float high) {
  return low + randomRange(randomEngine) * (high - low);
}