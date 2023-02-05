#include "animation_system.h"
#include "macros.h"

using namespace Gamma;

void AnimationSystem::initializePlayerRig(GmContext* context, GameState& state) {
  // @todo
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