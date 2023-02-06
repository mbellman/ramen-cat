#pragma once

#include "Gamma.h"

struct GameState;

struct AnimationJoint {
  Gamma::Vec3f position;
  Gamma::Vec3f offset;
  Gamma::Quaternion rotation;
  Gamma::Matrix4f r_matrix;
};

struct WeightedAnimationJoint {
  AnimationJoint* joint = nullptr;
  float weight = 0.f;
};

struct AnimatedVertex {
  Gamma::Vertex vertex;
  std::vector<WeightedAnimationJoint> joints;
};

struct AnimationRig {
  std::vector<AnimationJoint> joints;
  std::vector<AnimatedVertex> vertices;
};

namespace AnimationSystem {
  void initializeAnimations(GmContext* context, GameState& state);
  void handleAnimations(GmContext* context, GameState& state, float dt);
}