#include "animation_system.h"
#include "game.h"
#include "macros.h"

using namespace Gamma;

void AnimationSystem::initializeAnimations(GmContext* context, GameState& state) {
  // Initialize  player rig
  // @todo store joints in a file
  {
    auto& rig = state.animations.player;

    // Head [0]
    rig.joints.push_back({
      .position = Vec3f(0, -0.451f, 0.323f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Neck [1]
    rig.joints.push_back({
      .position = Vec3f(0, -0.312f, 0.115f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Torso [2]
    rig.joints.push_back({
      .position = Vec3f(0, -0.312f, -0.115f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Right leg top [3]
    rig.joints.push_back({
      .position = Vec3f(-0.225f, -0.334f, -0.323f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Right leg knee [4]
    rig.joints.push_back({
      .position = Vec3f(-0.241f, -0.349f, -0.628f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Right leg bottom [5]
    rig.joints.push_back({
      .position = Vec3f(-0.237f, -0.345f, -0.904f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Left leg top [6]
    rig.joints.push_back({
      .position = Vec3f(0.267f, -0.349f, -0.329f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Left leg knee [7]
    rig.joints.push_back({
      .position = Vec3f(0.297f, -0.353f, -0.626f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Left leg bottom [8]
    rig.joints.push_back({
      .position = Vec3f(0.3f, -0.351f, -0.908f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Tailbone [9]
    rig.joints.push_back({
      .position = Vec3f(0, -0.312f, -0.115f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // @todo finish adding joints
  }
}

void AnimationSystem::handleAnimations(GmContext* context, GameState& state, float dt) {
  // Player character animation
  {
    auto& player = get_player();
    float rotation = atan2f(state.direction.x, state.direction.z) + Gm_PI;

    player.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1, 0), rotation);

    // @todo proper mesh animation
  }
}