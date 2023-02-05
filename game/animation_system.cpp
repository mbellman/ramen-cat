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
      .position = Vec3f(0, 0.323f, -0.451f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Neck [1]
    rig.joints.push_back({
      .position = Vec3f(0, 0.115f, -0.312f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Torso [2]
    rig.joints.push_back({
      .position = Vec3f(0, -0.115f, -0.312f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Right leg top [3]
    rig.joints.push_back({
      .position = Vec3f(-0.225f, -0.323f, -0.334f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Right leg knee [4]
    rig.joints.push_back({
      .position = Vec3f(-0.241f, -0.628f, -0.349f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Right leg bottom [5]
    rig.joints.push_back({
      .position = Vec3f(-0.237f, -0.904f, -0.345f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Left leg top [6]
    rig.joints.push_back({
      .position = Vec3f(0.267f, -0.329f, -0.349f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Left leg knee [7]
    rig.joints.push_back({
      .position = Vec3f(0.297f, -0.626f, -0.353f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Left leg bottom [8]
    rig.joints.push_back({
      .position = Vec3f(0.3f, -0.908f, -0.351f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Tailbone [9]
    rig.joints.push_back({
      .position = Vec3f(0, -0.115f, -0.312f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Rear right leg top [10]
    rig.joints.push_back({
      .position = Vec3f(-0.247f, -0.103f, 0.408f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Rear right leg knee [11]
    rig.joints.push_back({
      .position = Vec3f(-0.276f, -0.631f, 0.41f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Rear right leg bottom [12]
    rig.joints.push_back({
      .position = Vec3f(-0.28f, -0.88f, 0.394f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Rear left leg top [13]
    rig.joints.push_back({
      .position = Vec3f(0.271f, -0.354f, 0.36f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Rear left leg knee [14]
    rig.joints.push_back({
      .position = Vec3f(0.304f, -0.62f, 0.356f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Rear left leg bottom [15]
    rig.joints.push_back({
      .position = Vec3f(0.303f, -0.898f, 0.358f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Created animated vertices, weighted by animated joints
    for (auto& vertex : mesh("player")->vertices) {
      AnimatedVertex a_vertex;

      // @todo use vertex index instead
      a_vertex.vertex = vertex;

      for (auto& joint : rig.joints) {
        float distance = (vertex.position - joint.position).magnitude();

        if (distance < 0.5f) {
          float weight = 2.f * (0.5f - distance);

          WeightedAnimationJoint w_joint;

          w_joint.joint = &joint;
          w_joint.weight = weight;

          a_vertex.joints.push_back(w_joint);
        }
      }

      rig.vertices.push_back(a_vertex);
    }

    for (auto& vertex : mesh("player")->vertices) {
      mesh("player")->transformedVertices.push_back(vertex);
    }
  }
}

void AnimationSystem::handleAnimations(GmContext* context, GameState& state, float dt) {
  START_TIMING("handleAnimations");

  // Player character animation
  {
    auto& mesh = mesh("player");
    auto& player = get_player();
    float rotation = atan2f(state.direction.x, state.direction.z) + Gm_PI;

    player.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1, 0), rotation);

    // @temporary
    state.animations.player.joints[0].offset = Vec3f(0, 0.3f, 0) * sinf(get_running_time());
    state.animations.player.joints[5].offset = Vec3f(0, 0, 0.5f) * cosf(get_running_time() * 2.f);
    state.animations.player.joints[8].offset = Vec3f(0, 0, 0.5f) * sinf(get_running_time() * 2.f);
    state.animations.player.joints[12].offset = Vec3f(0, 0, 0.5f) * cosf(get_running_time() * 2.f);
    state.animations.player.joints[15].offset = Vec3f(0, 0, 0.5f) * sinf(get_running_time() * 2.f);

    // @temporary
    for (u32 i = 0; i < state.animations.player.vertices.size(); i++) {
      auto& a_vertex = state.animations.player.vertices[i];

      a_vertex.vertex = mesh->vertices[i];

      for (auto& w_joint : a_vertex.joints) {
        // @todo factor in rotation
        Vec3f offset = a_vertex.vertex.position + w_joint.joint->offset;

        a_vertex.vertex.position = Vec3f::lerp(a_vertex.vertex.position, offset, w_joint.weight);
      }

      mesh->transformedVertices[i] = a_vertex.vertex;
    }
  }

  LOG_TIME();
}