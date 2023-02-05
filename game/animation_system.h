#pragma once

#include "Gamma.h"

struct GameState;

struct AnimationJoint {
  Gamma::Vec3f position;
  Gamma::Quaternion rotation;
};

struct AnimationRig {
  std::vector<AnimationJoint> joints;
};

namespace AnimationSystem {
  void initializeAnimations(GmContext* context, GameState& state);
  void handleAnimations(GmContext* context, GameState& state, float dt);
}