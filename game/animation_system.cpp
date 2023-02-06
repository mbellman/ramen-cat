#include "animation_system.h"
#include "game.h"
#include "game_constants.h"
#include "macros.h"

using namespace Gamma;

// @todo cleanup
internal void updatePlayerAnimationRig(GmContext* context, GameState& state, float dt) {
  auto& rig = state.animations.player;

  if (state.isOnSolidGround) {
    float speed = state.velocity.xz().magnitude();
    float speedRatio = speed / (speed + MAXIMUM_HORIZONTAL_GROUND_SPEED);
    float alpha = state.totalDistanceTraveled * 0.075f;

    float s_alpha = sinf(alpha);
    float s_alpha_m = sinf(alpha * 0.75f);
    float c_alpha = cosf(alpha);
    float c_alpha_m = cosf(alpha * 0.75f);

    // Head/neck
    rig.joints[0].offset = Vec3f(0, 0.05f, 0) * speedRatio * s_alpha_m;
    rig.joints[1].offset = Vec3f(0, 0.15f, 0) * speedRatio * s_alpha_m;

    // Torso
    rig.joints[2].offset = Vec3f(0, 0.05f, 0) * speedRatio * sinf(alpha);

    // Spine
    rig.joints[3].offset = Vec3f(0, 0.1f, 0) * speedRatio * sinf(alpha + 1.f);

    // Front legs
    rig.joints[5].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * s_alpha;
    rig.joints[6].offset = Vec3f(0, 0, 0.2f) * speedRatio * sinf(alpha + 0.5f);
    rig.joints[7].offset = Vec3f(0, 0.2f, 0.8f) * speedRatio * sinf(alpha + 1.f);

    rig.joints[8].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * c_alpha;
    rig.joints[9].offset = Vec3f(0, 0, 0.2f) * speedRatio * cosf(alpha + 0.5f);
    rig.joints[10].offset = Vec3f(0, 0.2f, 0.8f) * speedRatio * cosf(alpha + 1.f);

    // Back legs
    rig.joints[11].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * sinf(alpha - 0.5f);
    rig.joints[12].offset = Vec3f(0, 0, 0.2f) * speedRatio * sinf(alpha);
    rig.joints[12].rotation = Quaternion(1.f, 0, 0, 0);
    rig.joints[13].offset = Vec3f(0, 0.2f, 0.8f) * speedRatio * sinf(alpha + 0.5f);
    rig.joints[13].rotation = Quaternion(1.f, 0, 0, 0);

    rig.joints[14].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * cosf(alpha - 0.5f);
    rig.joints[15].offset = Vec3f(0, 0, 0.2f) * speedRatio * c_alpha;
    rig.joints[15].rotation = Quaternion(1.f, 0, 0, 0);
    rig.joints[16].offset = Vec3f(0, 0.2f, 0.8f) * speedRatio * cosf(alpha + 0.5f);
    rig.joints[16].rotation = Quaternion(1.f, 0, 0, 0);
  } else {
    // Animate in mid-air

    // Head/neck
    rig.joints[0].offset = Vec3f(0);
    rig.joints[1].offset = Vec3f(0);

    // Torso
    rig.joints[2].offset = Vec3f(0);

    // Spine
    rig.joints[3].offset = Vec3f(0);

    // Front legs
    rig.joints[5].offset = Vec3f(0);
    rig.joints[6].offset = Vec3f(0, 0, -0.3f);
    rig.joints[7].offset = Vec3f(0, 0, -0.5f);

    rig.joints[8].offset = Vec3f(0);
    rig.joints[9].offset = Vec3f(0, 0, -0.3f);
    rig.joints[10].offset = Vec3f(0, 0, -0.5f);

    // Back legs
    rig.joints[11].offset = Vec3f(0);
    rig.joints[12].offset = Vec3f(0, 0, 0.2f);
    rig.joints[12].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -Gm_HALF_PI * 0.8f);
    rig.joints[13].offset = Vec3f(0, 0.3f, 1.f);
    rig.joints[13].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -Gm_HALF_PI);

    rig.joints[14].offset = Vec3f(0);
    rig.joints[15].offset = Vec3f(0, 0, 0.2f);
    rig.joints[15].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -Gm_HALF_PI * 0.8f);
    rig.joints[16].offset = Vec3f(0, 0.3f, 1.f);
    rig.joints[16].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -Gm_HALF_PI);
  }
}

internal void playRiggedMeshAnimation(Mesh& mesh, AnimationRig& rig) {
  for (u32 i = 0; i < rig.vertices.size(); i++) {
    auto& animatedVertex = rig.vertices[i];

    animatedVertex.vertex = mesh.vertices[i];

    for (auto& weightedJoint : animatedVertex.joints) {
      auto& joint = *weightedJoint.joint;
      Vec3f vertexPosition = animatedVertex.vertex.position;
      Vec3f jointToVertex = vertexPosition - joint.position;
      // @optimize don't recalculate the rotation matrix for each joint, for each vertex!
      // Just recalculate it once per joint (where necessary) beforehand. Optionally, store
      // matrices instead of quaternions.
      // @optimize only calculate the x/y/z transform - we throw away the w component when calling toVec3f()!
      Vec3f rotatedJointToVertex = (joint.rotation.toMatrix4f() * jointToVertex).toVec3f();
      Vec3f targetPosition = joint.position + joint.offset + rotatedJointToVertex;

      animatedVertex.vertex.position = Vec3f::lerp(vertexPosition, targetPosition, weightedJoint.weight);
    }

    mesh.transformedVertices[i] = animatedVertex.vertex;
  }
}

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
      .position = Vec3f(0, 0.1f, -0.05f),
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
      w_joint2.weight = 0.5f;

      w_joint3.joint = &rig.joints[closest3Index];
      w_joint2.weight = 0.25f;

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
    auto& player = get_player();
    float yaw = state.currentYaw;
    float pitch = state.currentPitch;
    Vec3f motion = player.position - state.previousPlayerPosition;

    if (
      // Only calculate yaw/pitch when we've actually moved
      player.position != state.previousPlayerPosition &&
      // @hack Skip the first frame, since we fall for a single
      // frame at startup, which will confuse the calculation
      context->scene.frame != 0
    ) {
      yaw = atan2f(motion.x, motion.z) + Gm_PI;
      pitch = -1.f * atan2f(motion.y, motion.xz().magnitude());
    }

    if (!state.isOnSolidGround) {
      pitch *= 0.2f;
    }

    yaw = Gm_LerpCircularf(state.currentYaw, yaw, 10.f * dt, Gm_PI);
    pitch = Gm_Lerpf(state.currentPitch, pitch, 10.f * dt);

    player.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1, 0), yaw);
    player.rotation *= Quaternion::fromAxisAngle(player.rotation.getLeftDirection(), pitch);

    updatePlayerAnimationRig(context, state, dt);
    playRiggedMeshAnimation(*mesh("player"), state.animations.player);

    state.currentYaw = yaw;
    state.currentPitch = pitch;
  }

  LOG_TIME();
}