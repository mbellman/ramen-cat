#include "animation_system.h"
#include "game.h"
#include "game_constants.h"
#include "macros.h"

using namespace Gamma;

internal void handlePlayerTrottingAnimation(GameState& state, float dt) {
  auto& rig = state.animation.playerRig;

  float speed = state.velocity.xz().magnitude();
  float speedRatio = speed / (speed + MAXIMUM_HORIZONTAL_GROUND_SPEED);
  float alpha = state.totalDistanceTraveled * 0.06f;

  // Head/neck
  rig.joints[0].offset = Vec3f(0, 0.05f, 0) * speedRatio * sinf(alpha * 0.75f);
  rig.joints[1].offset = Vec3f(0, 0.15f, 0) * speedRatio * sinf(alpha * 0.75f);

  // Torso
  rig.joints[2].offset = Vec3f(0, 0.05f, 0) * speedRatio * sinf(alpha);

  // Spine
  rig.joints[3].offset = Vec3f(0, 0.1f, 0) * speedRatio * sinf(alpha + 1.f);

  // Front legs
  rig.joints[5].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * sinf(alpha);
  rig.joints[6].offset = Vec3f(0, 0, 0.2f) * speedRatio * sinf(alpha + 0.5f);
  rig.joints[7].offset = Vec3f(0, 0.2f, 0.8f) * speedRatio * sinf(alpha + 1.f);

  rig.joints[8].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * cosf(alpha);
  rig.joints[9].offset = Vec3f(0, 0, 0.2f) * speedRatio * cosf(alpha + 0.5f);
  rig.joints[10].offset = Vec3f(0, 0.2f, 0.8f) * speedRatio * cosf(alpha + 1.f);

  // Back legs
  rig.joints[11].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * sinf(alpha - 0.5f);
  rig.joints[12].offset = Vec3f(0, 0, 0.2f) * speedRatio * sinf(alpha);
  rig.joints[13].offset = Vec3f(0, 0.2f, 0.8f) * speedRatio * sinf(alpha + 0.5f);

  rig.joints[14].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * cosf(alpha - 0.5f);
  rig.joints[15].offset = Vec3f(0, 0, 0.2f) * speedRatio * cosf(alpha);
  rig.joints[16].offset = Vec3f(0, 0.2f, 0.8f) * speedRatio * cosf(alpha + 0.5f);
}

internal void handlePlayerDashingAnimation(GmContext* context, GameState& state, float dt) {
  auto& rig = state.animation.playerRig;
  float speed = state.velocity.xz().magnitude();
  float speedRatio = speed / (speed + MAXIMUM_HORIZONTAL_GROUND_SPEED);
  float alpha = state.totalDistanceTraveled * 0.025f;

  get_player().scale.z = PLAYER_RADIUS + 5.f * (sinf(alpha) * 0.5f + 0.5f);

  // Head/neck
  rig.joints[0].offset = Vec3f(0, 0.05f, 0) * speedRatio * sinf(alpha * 0.75f);
  rig.joints[1].offset = Vec3f(0, 0.15f, 0) * speedRatio * sinf(alpha * 0.75f);

  // Torso
  rig.joints[2].offset = Vec3f(0, 0.05f, 0) * speedRatio * sinf(alpha);

  // Spine
  rig.joints[3].offset = Vec3f(0, 0.2f, 0) * speedRatio * sinf(alpha - 1.f);

  // Tailbone
  rig.joints[4].offset = Vec3f(0, 0.4f, 0) * speedRatio * sinf(alpha - 0.5f);

  // Front legs
  rig.joints[5].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * sinf(alpha);
  rig.joints[6].offset = Vec3f(0, 0, 0.5f) * speedRatio * sinf(alpha + 0.5f);
  rig.joints[6].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -sinf(alpha + 0.5f) * 0.5f);
  rig.joints[7].offset = Vec3f(0, 0.4f, 1.f) * speedRatio * sinf(alpha + 1.f);
  rig.joints[7].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -sinf(alpha + 0.5f));

  rig.joints[8].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * sinf(alpha);
  rig.joints[9].offset = Vec3f(0, 0, 0.5f) * speedRatio * sinf(alpha + 0.5f);
  rig.joints[9].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -sinf(alpha + 0.5f) * 0.5f);
  rig.joints[10].offset = Vec3f(0, 0.4f, 1.f) * speedRatio * sinf(alpha + 1.f);
  rig.joints[10].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -sinf(alpha + 0.5f));

  // Back legs
  rig.joints[11].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * cosf(alpha - 0.5f);
  rig.joints[12].offset = Vec3f(0, 0, 0.5f) * speedRatio * cosf(alpha);
  rig.joints[12].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -cosf(alpha + 0.5f) * 0.5f);
  rig.joints[13].offset = Vec3f(0, 0.4f, 1.f) * speedRatio * cosf(alpha + 0.5f);
  rig.joints[13].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -cosf(alpha + 0.5f));

  rig.joints[14].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * cosf(alpha - 0.5f);
  rig.joints[15].offset = Vec3f(0, 0, 0.5f) * speedRatio * cosf(alpha);
  rig.joints[15].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -cosf(alpha + 0.5f) * 0.5f);
  rig.joints[16].offset = Vec3f(0, 0.4f, 1.f) * speedRatio * cosf(alpha + 0.5f);
  rig.joints[16].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -cosf(alpha + 0.5f));
}

internal void handlePlayerMidairAnimation(GmContext* context, GameState& state, float dt) {
  auto& rig = state.animation.playerRig;
  float jumpTime = time_since(state.lastTimeOnSolidGround);
  float dashTime = time_since(state.lastAirDashTime);
  float airTime = jumpTime < dashTime ? jumpTime : dashTime;
  float airTimeFactor = airTime / (airTime + 0.5f);

  float s_alpha = sinf(airTime * 5.f);
  float c_alpha = cosf(airTime * 5.f);

  // Spine
  rig.joints[3].offset = Vec3f(0, -0.1f, 0) * airTimeFactor;

  // Front legs
  rig.joints[5].offset = Vec3f(0, 0.1f, 0.f) * airTimeFactor;
  rig.joints[6].offset = Vec3f(0, 0.2f, -0.3f) * airTimeFactor;
  rig.joints[6].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), 0.5f);
  rig.joints[7].offset = Vec3f(0, 0.6f, -1.3f) * airTimeFactor;
  rig.joints[7].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), Gm_HALF_PI * airTimeFactor);

  rig.joints[8].offset = Vec3f(0, 0.1f, 0.f) * airTimeFactor;
  rig.joints[9].offset = Vec3f(0, 0.2f, -0.3f) * airTimeFactor;
  rig.joints[9].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), 0.5f);
  rig.joints[10].offset = Vec3f(0, 0.6f, -1.3f) * airTimeFactor;
  rig.joints[10].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), Gm_HALF_PI * airTimeFactor);

  // Back legs
  rig.joints[12].offset = Vec3f(0, 0, 0.5f - 0.5f * airTimeFactor);
  rig.joints[12].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -Gm_HALF_PI * 0.25f * airTimeFactor);
  rig.joints[13].offset = Vec3f(0, -0.1f * airTimeFactor, 1.f - 1.5f * airTimeFactor);
  rig.joints[13].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), Gm_HALF_PI * 0.5f* airTimeFactor);

  rig.joints[15].offset = Vec3f(0, 0, 0.5f - 0.5f * airTimeFactor);
  rig.joints[15].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -Gm_HALF_PI * 0.25f * airTimeFactor);
  rig.joints[16].offset = Vec3f(0, -0.1f * airTimeFactor, 1.f - 1.5f * airTimeFactor);
  rig.joints[16].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), Gm_HALF_PI * 0.5f * airTimeFactor);
}

internal void handlePlayerAnimation(GmContext* context, GameState& state, float dt) {
  // Reset player scale/joints every time so they can be calculated cleanly
  {
    get_player().scale = Vec3f(PLAYER_RADIUS);

    for (auto& joint : state.animation.playerRig.joints) {
      joint.offset = Vec3f(0);
      joint.rotation = Quaternion(1.f, 0, 0, 0);
    }
  }

  if (state.isOnSolidGround) {
    if (state.dashLevel == 0) {
      handlePlayerTrottingAnimation(state, dt);
    } else {
      handlePlayerDashingAnimation(context, state, dt);
    }
  } else {
    if (time_since(state.lastWallBumpTime) < WALL_KICK_WINDOW_DURATION) {
      // @todo
    } else {
      handlePlayerMidairAnimation(context, state, dt);
    }
  }

  // Recalculate joint rotation matrices
  {
    for (auto& joint : state.animation.playerRig.joints) {
      joint.r_matrix = joint.rotation.toMatrix4f();
    }
  }
}

internal void handleAnimatedMeshWithRig(Mesh& mesh, AnimationRig& rig) {
  for (u32 i = 0; i < rig.vertices.size(); i++) {
    auto& animatedVertex = rig.vertices[i];

    animatedVertex.vertex = mesh.vertices[i];

    for (auto& [ joint, weight ] : animatedVertex.joints) {
      Vec3f vertexPosition = animatedVertex.vertex.position;
      Vec3f jointToVertex = vertexPosition - joint->position;
      Vec3f rotatedJointToVertex = joint->r_matrix.transformVec3f(jointToVertex);
      Vec3f targetPosition = joint->position + joint->offset + rotatedJointToVertex;

      animatedVertex.vertex.position = Vec3f::lerp(vertexPosition, targetPosition, weight);
    }

    mesh.transformedVertices[i] = animatedVertex.vertex;
  }
}

void AnimationSystem::initializeAnimations(GmContext* context, GameState& state) {
  // Initialize  player rig
  // @todo store joints in a file
  {
    auto& rig = state.animation.playerRig;

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
      .position = Vec3f(-0.237f, -1.f, -0.345f),
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
      .position = Vec3f(0.237f, -1.f, -0.351f),
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
      w_joint1.weight = 0.9f;

      w_joint2.joint = &rig.joints[closest2Index];
      w_joint2.weight = 0.4f;

      w_joint3.joint = &rig.joints[closest3Index];
      w_joint2.weight = 0.2f;

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
    Vec3f movement = player.position - state.previousPlayerPosition;

    // Only turn the player character when not winding up a wall kick.
    // Wall kick wind-ups use a custom rotation animation.
    if (time_since(state.lastWallBumpTime) > WALL_KICK_WINDOW_DURATION) {
      if (state.velocity.magnitude() > 20.f) {
        yaw = atan2f(movement.x, movement.z) + Gm_PI;
        pitch = -1.f * atan2f(movement.y, movement.xz().magnitude());
      }

      if (!state.isOnSolidGround) {
        pitch *= 1.f / (1.f + time_since(state.lastTimeOnSolidGround));
      }
    }

    yaw = Gm_LerpCircularf(state.currentYaw, yaw, 10.f * dt, Gm_PI);
    pitch = Gm_Lerpf(state.currentPitch, pitch, 10.f * dt);

    player.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1, 0), yaw);
    player.rotation *= Quaternion::fromAxisAngle(player.rotation.getLeftDirection(), pitch);

    handlePlayerAnimation(context, state, dt);
    handleAnimatedMeshWithRig(*mesh("player"), state.animation.playerRig);

    state.currentYaw = yaw;
    state.currentPitch = pitch;
  }

  LOG_TIME();
}