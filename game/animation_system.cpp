#include "animation_system.h"
#include "game.h"
#include "game_constants.h"
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

    // Spine [3]
    rig.joints.push_back({
      .position = Vec3f(0, -0.115f, -0.05f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Tailbone [4]
    rig.joints.push_back({
      .position = Vec3f(0, -0.115f, 0.2f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Right leg top [5]
    rig.joints.push_back({
      .position = Vec3f(-0.225f, -0.323f, -0.334f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Right leg knee [6]
    rig.joints.push_back({
      .position = Vec3f(-0.241f, -0.628f, -0.349f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Right leg bottom [7]
    rig.joints.push_back({
      .position = Vec3f(-0.237f, -0.904f, -0.345f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Left leg top [8]
    rig.joints.push_back({
      .position = Vec3f(0.267f, -0.329f, -0.349f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Left leg knee [9]
    rig.joints.push_back({
      .position = Vec3f(0.297f, -0.626f, -0.353f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Left leg bottom [10]
    rig.joints.push_back({
      .position = Vec3f(0.3f, -0.908f, -0.351f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Rear right leg top [11]
    rig.joints.push_back({
      .position = Vec3f(-0.247f, -0.103f, 0.408f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Rear right leg knee [12]
    rig.joints.push_back({
      .position = Vec3f(-0.276f, -0.631f, 0.41f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Rear right leg bottom [13]
    rig.joints.push_back({
      .position = Vec3f(-0.28f, -0.88f, 0.394f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Rear left leg top [14]
    rig.joints.push_back({
      .position = Vec3f(0.271f, -0.354f, 0.36f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Rear left leg knee [15]
    rig.joints.push_back({
      .position = Vec3f(0.304f, -0.62f, 0.356f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Rear left leg bottom [16]
    rig.joints.push_back({
      .position = Vec3f(0.303f, -0.898f, 0.358f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Created animated vertices, weighted by animated joints
    for (auto& vertex : mesh("player")->vertices) {
      AnimatedVertex a_vertex;

      // @todo use vertex index instead
      a_vertex.vertex = vertex;

      // @todo this does not take into account joint connections
      float closest = Gm_FLOAT_MAX;
      u32 closestIndex = 0;
      float closest2 = Gm_FLOAT_MAX;
      u32 closest2Index = 0;
      float closest3 = Gm_FLOAT_MAX;
      u32 closest3Index = 0;

      for (u32 i = 0; i < rig.joints.size(); i++) {
        auto& joint = rig.joints[i];
        float distance = (vertex.position - joint.position).magnitude();

        if (distance < closest) {
          closest = distance;
          closestIndex = i;
        }
      }


      for (u32 i = 0; i < rig.joints.size(); i++) {
        auto& joint = rig.joints[i];
        float distance = (vertex.position - joint.position).magnitude();

        if (distance < closest2 && distance > closest) {
          closest2 = distance;
          closest2Index = i;
        }
      }


      for (u32 i = 0; i < rig.joints.size(); i++) {
        auto& joint = rig.joints[i];
        float distance = (vertex.position - joint.position).magnitude();

        if (distance < closest3 && distance > closest2) {
          closest3 = distance;
          closest3Index = i;
        }
      }

      WeightedAnimationJoint w_joint1;
      WeightedAnimationJoint w_joint2;
      WeightedAnimationJoint w_joint3;

      w_joint1.joint = &rig.joints[closestIndex];
      w_joint1.weight = 1.f;

      w_joint2.joint = &rig.joints[closest2Index];
      w_joint2.weight = 0.3f;

      w_joint3.joint = &rig.joints[closest3Index];
      w_joint2.weight = 0.1f;

      a_vertex.joints.push_back(w_joint1);
      a_vertex.joints.push_back(w_joint2);
      a_vertex.joints.push_back(w_joint3);

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
    float speed = state.velocity.xz().magnitude();
    float speedRatio = speed / (speed + MAXIMUM_HORIZONTAL_GROUND_SPEED);
    float alpha = state.totalDistanceTraveled * 0.075f;

    float s_alpha = sinf(alpha);
    float s_alpha_m = sinf(alpha * 0.75f);
    float c_alpha = cosf(alpha);

    // Head/neck
    state.animations.player.joints[0].offset = Vec3f(0, 0.05f, 0) * speedRatio * s_alpha_m;
    state.animations.player.joints[1].offset = Vec3f(0, 0.15f, 0) * speedRatio * s_alpha_m;

    // Torso
    state.animations.player.joints[2].offset = Vec3f(0, 0.2f, 0) * speedRatio * s_alpha_m;

    // Spine
    state.animations.player.joints[3].offset = Vec3f(0, 0.25f, 0) * speedRatio * c_alpha;

    // Tailbone
    state.animations.player.joints[4].offset = Vec3f(0, 0.2f, 0) * speedRatio * s_alpha_m;

    // Front legs
    state.animations.player.joints[6].offset = Vec3f(0, -0.2f, 0) * speedRatio * s_alpha;
    state.animations.player.joints[7].offset = Vec3f(0, 0, 0.8f) * speedRatio * s_alpha;

    state.animations.player.joints[9].offset = Vec3f(0, -0.2f, 0) * speedRatio * c_alpha;
    state.animations.player.joints[10].offset = Vec3f(0, 0, 0.8f) * speedRatio * c_alpha;

    // Back legs
    state.animations.player.joints[11].offset = Vec3f(0, 0.1f, 0) * speedRatio * s_alpha;
    state.animations.player.joints[12].offset = Vec3f(0, 0.1f, 0) * speedRatio * s_alpha;
    state.animations.player.joints[13].offset = Vec3f(0, 0, 0.8f) * speedRatio * s_alpha;

    state.animations.player.joints[14].offset = Vec3f(0, 0.1f, 0) * speedRatio * c_alpha;
    state.animations.player.joints[15].offset = Vec3f(0, 0.1f, 0) * speedRatio * c_alpha;
    state.animations.player.joints[16].offset = Vec3f(0, 0, 0.8f) * speedRatio * c_alpha;

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