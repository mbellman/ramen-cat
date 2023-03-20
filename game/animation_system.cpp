#include "animation_system.h"
#include "game.h"
#include "game_constants.h"
#include "easing.h"
#include "macros.h"

using namespace Gamma;

#define PLAYER_HEAD 0
#define PLAYER_NECK 1
#define PLAYER_TORSO 2
#define PLAYER_SPINE 3
#define PLAYER_TAILBONE 4
#define PLAYER_FRONT_RIGHT_LEG_TOP 5
#define PLAYER_FRONT_RIGHT_LEG_KNEE 6
#define PLAYER_FRONT_RIGHT_LEG_FOOT 7
#define PLAYER_FRONT_LEFT_LEG_TOP 8
#define PLAYER_FRONT_LEFT_LEG_KNEE 9
#define PLAYER_FRONT_LEFT_LEG_FOOT 10
#define PLAYER_BACK_RIGHT_LEG_TOP 11
#define PLAYER_BACK_RIGHT_LEG_KNEE 12
#define PLAYER_BACK_RIGHT_LEG_FOOT 13
#define PLAYER_BACK_LEFT_LEG_TOP 14
#define PLAYER_BACK_LEFT_LEG_KNEE 15
#define PLAYER_BACK_LEFT_LEG_FOOT 16
#define PLAYER_TAIL_JOINT_1 17
#define PLAYER_TAIL_JOINT_2 18

internal float wsinf(float x) {
  float mx = Gm_Modf(-x + Gm_HALF_PI, Gm_PI);

  return (cosf(mx) * 0.5f + 0.5f) * (mx / (mx + 1.f)) * 2.595f * 2.f - 1.f;
}

internal float wcosf(float x) {
  float mx = Gm_Modf(-x, Gm_PI);

  return (cosf(mx) * 0.5f + 0.5f) * (mx / (mx + 1.f)) * 2.595f * 2.f - 1.f;
}

internal float getPeriodicHeadTurn(float sceneTime) {
  return (sinf(sceneTime * 0.5f) + sinf(sceneTime * 1.7f)) * 0.4f;
}

internal void handlePlayerTrottingAnimation(GmContext* context, GameState& state, float dt) {
  auto& rig = state.animation.playerRig;

  float speed = state.velocity.xz().magnitude();
  float speedRatio = speed / (speed + MAXIMUM_HORIZONTAL_GROUND_SPEED);
  float alpha = state.totalDistanceTraveled * 0.03f;
  float timeOnSolidGround = time_since(state.lastTimeInAir);
  float periodicHeadTurnFactor = timeOnSolidGround / (timeOnSolidGround + 1.f);
  float periodicHeadTurn = getPeriodicHeadTurn(get_scene_time()) * periodicHeadTurnFactor;

  rig.joints[PLAYER_HEAD].offset = Vec3f(0, 0.05f, 0) * speedRatio * sinf(alpha * 2.f);
  rig.joints[PLAYER_NECK].offset = Vec3f(0, 0.15f, 0) * speedRatio * sinf(alpha * 2.f);
  rig.joints[PLAYER_HEAD].rotation = Quaternion::fromAxisAngle(Vec3f(0, 1, 0), state.turnFactor * 1.5f + periodicHeadTurn);
  rig.joints[PLAYER_NECK].rotation = Quaternion::fromAxisAngle(Vec3f(0, 1, 0), state.turnFactor + periodicHeadTurn * 0.5f);

  rig.joints[PLAYER_TORSO].offset = Vec3f(0, 0.05f, 0) * speedRatio * sinf(alpha);
  rig.joints[PLAYER_SPINE].offset = Vec3f(0, 0.1f, 0) * speedRatio * sinf(alpha + 1.f);

  rig.joints[PLAYER_FRONT_RIGHT_LEG_TOP].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * wsinf(alpha);
  rig.joints[PLAYER_FRONT_RIGHT_LEG_KNEE].offset = Vec3f(0, 0, 0.3f) * speedRatio * wsinf(alpha + 0.8f);
  rig.joints[PLAYER_FRONT_RIGHT_LEG_FOOT].offset = Vec3f(0, 0.2f, 1.f) * speedRatio * wsinf(alpha + 1.f);

  rig.joints[PLAYER_FRONT_LEFT_LEG_TOP].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * wcosf(alpha);
  rig.joints[PLAYER_FRONT_LEFT_LEG_KNEE].offset = Vec3f(0, 0, 0.3f) * speedRatio * wcosf(alpha + 0.8f);
  rig.joints[PLAYER_FRONT_LEFT_LEG_FOOT].offset = Vec3f(0, 0.2f, 1.f) * speedRatio * wcosf(alpha + 1.f);

  rig.joints[PLAYER_BACK_RIGHT_LEG_TOP].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * wsinf(alpha);
  rig.joints[PLAYER_BACK_RIGHT_LEG_KNEE].offset = Vec3f(0, 0, 0.2f) * speedRatio * wsinf(alpha + 0.5f);
  rig.joints[PLAYER_BACK_RIGHT_LEG_FOOT].offset = Vec3f(0, 0.2f, 0.8f) * speedRatio * wsinf(alpha + 1.5f);

  rig.joints[PLAYER_BACK_LEFT_LEG_TOP].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * wcosf(alpha);
  rig.joints[PLAYER_BACK_LEFT_LEG_KNEE].offset = Vec3f(0, 0, 0.2f) * speedRatio * wcosf(alpha + 0.5f);
  rig.joints[PLAYER_BACK_LEFT_LEG_FOOT].offset = Vec3f(0, 0.2f, 0.8f) * speedRatio * wcosf(alpha + 1.5f);
}

internal void handlePlayerDashingAnimation(GmContext* context, GameState& state, float dt) {
  auto& rig = state.animation.playerRig;
  float speed = state.velocity.xz().magnitude();
  float speedRatio = speed / (speed + MAXIMUM_HORIZONTAL_GROUND_SPEED);
  float alpha = state.totalDistanceTraveled * 0.025f;

  get_player().scale.z = PLAYER_RADIUS + 5.f * (sinf(alpha) * 0.5f + 0.5f);

  rig.joints[PLAYER_HEAD].offset = Vec3f(0, 0.05f, 0) * speedRatio * sinf(alpha * 0.75f);
  rig.joints[PLAYER_NECK].offset = Vec3f(0, 0.15f, 0) * speedRatio * sinf(alpha * 0.75f);
  rig.joints[PLAYER_HEAD].rotation = Quaternion::fromAxisAngle(Vec3f(0, 1, 0), state.turnFactor);
  rig.joints[PLAYER_NECK].rotation = Quaternion::fromAxisAngle(Vec3f(0, 1, 0), state.turnFactor * 0.5f);

  rig.joints[PLAYER_TORSO].offset = Vec3f(0, 0.05f, 0) * speedRatio * sinf(alpha);
  rig.joints[PLAYER_SPINE].offset = Vec3f(0, 0.2f, 0) * speedRatio * sinf(alpha - 1.f);
  rig.joints[PLAYER_TAILBONE].offset = Vec3f(0, 0.2f, 0) * speedRatio * sinf(alpha - 0.5f);

  rig.joints[PLAYER_FRONT_RIGHT_LEG_TOP].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * sinf(alpha);
  rig.joints[PLAYER_FRONT_RIGHT_LEG_KNEE].offset = Vec3f(0, 0, 0.5f) * speedRatio * sinf(alpha + 0.5f);
  rig.joints[PLAYER_FRONT_RIGHT_LEG_KNEE].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -sinf(alpha + 0.5f) * 0.5f);
  rig.joints[PLAYER_FRONT_RIGHT_LEG_FOOT].offset = Vec3f(0, 0.4f, 1.f) * speedRatio * sinf(alpha + 1.f);
  rig.joints[PLAYER_FRONT_RIGHT_LEG_FOOT].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -sinf(alpha + 0.5f));

  rig.joints[PLAYER_FRONT_LEFT_LEG_TOP].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * sinf(alpha);
  rig.joints[PLAYER_FRONT_LEFT_LEG_KNEE].offset = Vec3f(0, 0, 0.5f) * speedRatio * sinf(alpha + 0.5f);
  rig.joints[PLAYER_FRONT_LEFT_LEG_KNEE].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -sinf(alpha + 0.5f) * 0.5f);
  rig.joints[PLAYER_FRONT_LEFT_LEG_FOOT].offset = Vec3f(0, 0.4f, 1.f) * speedRatio * sinf(alpha + 1.f);
  rig.joints[PLAYER_FRONT_LEFT_LEG_FOOT].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -sinf(alpha + 0.5f));

  rig.joints[PLAYER_BACK_RIGHT_LEG_TOP].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * cosf(alpha - 0.5f);
  rig.joints[PLAYER_BACK_RIGHT_LEG_KNEE].offset = Vec3f(0, 0, 0.5f) * speedRatio * cosf(alpha);
  rig.joints[PLAYER_BACK_RIGHT_LEG_KNEE].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -cosf(alpha + 0.5f) * 0.5f);
  rig.joints[PLAYER_BACK_RIGHT_LEG_FOOT].offset = Vec3f(0, 0.4f, 1.f) * speedRatio * cosf(alpha + 0.5f);
  rig.joints[PLAYER_BACK_RIGHT_LEG_FOOT].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -cosf(alpha + 0.5f));

  rig.joints[PLAYER_BACK_LEFT_LEG_TOP].offset = Vec3f(0, 0.1f, 0.05f) * speedRatio * cosf(alpha - 0.5f);
  rig.joints[PLAYER_BACK_LEFT_LEG_KNEE].offset = Vec3f(0, 0, 0.5f) * speedRatio * cosf(alpha);
  rig.joints[PLAYER_BACK_LEFT_LEG_KNEE].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -cosf(alpha + 0.5f) * 0.5f);
  rig.joints[PLAYER_BACK_LEFT_LEG_FOOT].offset = Vec3f(0, 0.4f, 1.f) * speedRatio * cosf(alpha + 0.5f);
  rig.joints[PLAYER_BACK_LEFT_LEG_FOOT].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -cosf(alpha + 0.5f));
}

internal void handlePlayerWallKickAnimation(GmContext* context, GameState& state, float dt) {
  auto& player = get_player();
  auto& rig = state.animation.playerRig;
  float windUpTime = time_since(state.lastWallBumpTime);
  float timeFactor = windUpTime / WALL_KICK_WINDOW_DURATION;
  float compression = 0.5f * timeFactor;

  float kickDot = Vec3f::dot(state.lastWallBumpNormal, player.rotation.getLeftDirection());
  float headTurn = timeFactor * Gm_Signf(kickDot);

  rig.joints[PLAYER_HEAD].rotation = Quaternion::fromAxisAngle(Vec3f(0, 1, 0), headTurn);
  rig.joints[PLAYER_NECK].rotation = Quaternion::fromAxisAngle(Vec3f(0, 1, 0), headTurn * 0.5f);

  rig.joints[PLAYER_TORSO].rotation = Quaternion::fromAxisAngle(Vec3f(0, 1, 0), headTurn * 0.25f);
  rig.joints[PLAYER_SPINE].offset = Vec3f(0, -0.1f, -0.3f * timeFactor);
  rig.joints[PLAYER_TAILBONE].offset = Vec3f(0, 0, 0.5f - compression);

  rig.joints[PLAYER_FRONT_RIGHT_LEG_TOP].offset = Vec3f(0, 0.1f, 0.f);
  rig.joints[PLAYER_FRONT_RIGHT_LEG_KNEE].offset = Vec3f(0, 0.2f, -0.3f + compression * 0.5f);
  rig.joints[PLAYER_FRONT_RIGHT_LEG_KNEE].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), 0.5f);
  rig.joints[PLAYER_FRONT_RIGHT_LEG_FOOT].offset = Vec3f(0, 0.5f, -1.3f + compression);
  rig.joints[PLAYER_FRONT_RIGHT_LEG_FOOT].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), Gm_HALF_PI);

  rig.joints[PLAYER_FRONT_LEFT_LEG_TOP].offset = Vec3f(0, 0.1f, 0.f);
  rig.joints[PLAYER_FRONT_LEFT_LEG_KNEE].offset = Vec3f(0, 0.2f, -0.3f + compression * 0.5f);
  rig.joints[PLAYER_FRONT_LEFT_LEG_KNEE].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), 0.5f);
  rig.joints[PLAYER_FRONT_LEFT_LEG_FOOT].offset = Vec3f(0, 0.5f, -1.3f + compression);
  rig.joints[PLAYER_FRONT_LEFT_LEG_FOOT].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), Gm_HALF_PI);

  rig.joints[PLAYER_BACK_RIGHT_LEG_TOP].offset = Vec3f(0, 0, 0.5f - compression);
  rig.joints[PLAYER_BACK_RIGHT_LEG_KNEE].offset = Vec3f(0, 0, 1.f - compression * 1.2f);
  rig.joints[PLAYER_BACK_RIGHT_LEG_KNEE].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -Gm_HALF_PI * 0.25f);
  rig.joints[PLAYER_BACK_RIGHT_LEG_FOOT].offset = Vec3f(0, -0.1f + 0.1f, 1.5f - compression * 1.4f);
  rig.joints[PLAYER_BACK_RIGHT_LEG_FOOT].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -Gm_HALF_PI * 0.5);

  rig.joints[PLAYER_BACK_LEFT_LEG_TOP].offset = Vec3f(0, 0, 0.5f - compression);
  rig.joints[PLAYER_BACK_LEFT_LEG_KNEE].offset = Vec3f(0, 0, 1.f - compression * 1.2f);
  rig.joints[PLAYER_BACK_LEFT_LEG_KNEE].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -Gm_HALF_PI * 0.25f);
  rig.joints[PLAYER_BACK_LEFT_LEG_FOOT].offset = Vec3f(0, 0, 1.5f - compression * 1.4f);
  rig.joints[PLAYER_BACK_LEFT_LEG_FOOT].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -Gm_HALF_PI * 0.5f);
}

internal void handlePlayerMidairAnimation(GmContext* context, GameState& state, float dt) {
  auto& rig = state.animation.playerRig;
  float jumpTime = time_since(state.lastTimeOnSolidGround);
  float dashTime = time_since(state.lastAirDashTime);
  float ringLaunchTime = time_since(state.lastBoostRingLaunchTime);
  float somersaultAlpha = ringLaunchTime < SOMERSAULT_DURATION ? easeOutQuint(time_since(state.lastBoostRingLaunchTime) * (1.f / SOMERSAULT_DURATION)) : 0.f;
  float airTime = jumpTime < dashTime ? jumpTime : dashTime;
  float airTimeFactor = airTime / (airTime + 0.5f);
  float legSwingAlpha = airTime * 10.f;
  float periodicHeadTurnFactor = 1.f - airTimeFactor;
  float periodicHeadTurn = getPeriodicHeadTurn(get_scene_time()) * periodicHeadTurnFactor;
  Vec3f turnOffset = state.isGliding ? Vec3f(0.f) : Vec3f(-0.5f * state.turnFactor, 0, 0);

  float somersault = state.isGliding ? 0.f : sinf(somersaultAlpha * Gm_PI);
  Vec3f somersaultOffset = Vec3f(0, -somersault, 0);
  Quaternion somersaultRotation = Quaternion::fromAxisAngle(Vec3f(1, 0, 0), -somersault);

  float s_alpha = sinf(airTime * 5.f);
  float c_alpha = cosf(airTime * 5.f);

  rig.joints[PLAYER_HEAD].rotation = Quaternion::fromAxisAngle(Vec3f(0, 1, 0), state.turnFactor + periodicHeadTurn);
  rig.joints[PLAYER_HEAD].rotation *= somersaultRotation;
  rig.joints[PLAYER_HEAD].offset = turnOffset + somersaultOffset;

  rig.joints[PLAYER_NECK].rotation = Quaternion::fromAxisAngle(Vec3f(0, 1, 0), state.turnFactor * 0.5f + periodicHeadTurn * 0.5f);
  rig.joints[PLAYER_NECK].rotation *= somersaultRotation;
  rig.joints[PLAYER_NECK].offset = turnOffset + somersaultOffset;

  rig.joints[PLAYER_TORSO].offset = turnOffset + somersaultOffset;
  rig.joints[PLAYER_TORSO].rotation = somersaultRotation;
  rig.joints[PLAYER_SPINE].offset = Vec3f(0, -0.1f * airTimeFactor, 0) + turnOffset * 0.5f + somersaultOffset * 0.2f;
  rig.joints[PLAYER_TAILBONE].offset = Vec3f(0, 0, 0.5f * airTimeFactor);

  rig.joints[PLAYER_FRONT_RIGHT_LEG_TOP].offset = Vec3f(0, 0.1f, 0.f) * airTimeFactor + turnOffset + somersaultOffset;
  rig.joints[PLAYER_FRONT_RIGHT_LEG_TOP].rotation = somersaultRotation;
  rig.joints[PLAYER_FRONT_RIGHT_LEG_KNEE].offset = Vec3f(0, 0.2f, -0.3f) * airTimeFactor + turnOffset + somersaultOffset;
  rig.joints[PLAYER_FRONT_RIGHT_LEG_KNEE].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), Gm_HALF_PI * airTimeFactor) * somersaultRotation;
  rig.joints[PLAYER_FRONT_RIGHT_LEG_FOOT].offset = Vec3f(0, 0.6f + 0.2f * sinf(legSwingAlpha), -1.3f) * airTimeFactor + turnOffset + Vec3f(0, -somersault * 2.f, somersault);
  rig.joints[PLAYER_FRONT_RIGHT_LEG_FOOT].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), Gm_HALF_PI * airTimeFactor) * somersaultRotation;

  rig.joints[PLAYER_FRONT_LEFT_LEG_TOP].offset = Vec3f(0, 0.1f, 0.f) * airTimeFactor + turnOffset + somersaultOffset;
  rig.joints[PLAYER_FRONT_LEFT_LEG_TOP].rotation = somersaultRotation;
  rig.joints[PLAYER_FRONT_LEFT_LEG_KNEE].offset = Vec3f(0, 0.2f, -0.3f) * airTimeFactor + turnOffset + somersaultOffset;
  rig.joints[PLAYER_FRONT_LEFT_LEG_KNEE].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), Gm_HALF_PI * airTimeFactor) * somersaultRotation;
  rig.joints[PLAYER_FRONT_LEFT_LEG_FOOT].offset = Vec3f(0, 0.6f + 0.2f * cosf(legSwingAlpha), -1.3f) * airTimeFactor + turnOffset + Vec3f(0, -somersault * 2.f, somersault);
  rig.joints[PLAYER_FRONT_LEFT_LEG_FOOT].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), Gm_HALF_PI * airTimeFactor) * somersaultRotation;

  rig.joints[PLAYER_BACK_RIGHT_LEG_TOP].offset = Vec3f(0, 0, 0.5f * airTimeFactor);
  rig.joints[PLAYER_BACK_RIGHT_LEG_KNEE].offset = Vec3f(0, 0, 1.f * airTimeFactor);
  rig.joints[PLAYER_BACK_RIGHT_LEG_KNEE].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -Gm_HALF_PI * 0.25f * airTimeFactor);
  rig.joints[PLAYER_BACK_RIGHT_LEG_FOOT].offset = Vec3f(0, -0.1f + 0.1f * sinf(legSwingAlpha), 1.5f) * airTimeFactor;
  rig.joints[PLAYER_BACK_RIGHT_LEG_FOOT].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -Gm_HALF_PI * 0.5f* airTimeFactor);

  rig.joints[PLAYER_BACK_LEFT_LEG_TOP].offset = Vec3f(0, 0, 0.5f * airTimeFactor);
  rig.joints[PLAYER_BACK_LEFT_LEG_KNEE].offset = Vec3f(0, 0, 1.f * airTimeFactor);
  rig.joints[PLAYER_BACK_LEFT_LEG_KNEE].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -Gm_HALF_PI * 0.25f * airTimeFactor);
  rig.joints[PLAYER_BACK_LEFT_LEG_FOOT].offset = Vec3f(0, -0.1f + 0.1f * cosf(legSwingAlpha), 1.5f) * airTimeFactor;
  rig.joints[PLAYER_BACK_LEFT_LEG_FOOT].rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -Gm_HALF_PI * 0.5f * airTimeFactor);
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
      handlePlayerTrottingAnimation(context, state, dt);
    } else {
      handlePlayerDashingAnimation(context, state, dt);
    }
  } else {
    if (time_since(state.lastWallBumpTime) < WALL_KICK_WINDOW_DURATION) {
      // @todo rewrite handlePlayerWallKickAnimation()
      handlePlayerMidairAnimation(context, state, dt);
    } else {
      handlePlayerMidairAnimation(context, state, dt);
    }
  }

  // @temporary
  // @todo use dynamic tail animations based on action
  {
    auto& rig = state.animation.playerRig;
    float t = get_scene_time();

    rig.joints[PLAYER_TAIL_JOINT_1].offset = Vec3f(
      0 - state.turnFactor * 0.5f,
      sinf(t * 2.f) * cosf(t * 1.1f) * 0.05f,
      0
    );

    rig.joints[PLAYER_TAIL_JOINT_2].offset = Vec3f(
      cosf(t * 2.f) * sinf(t * 0.7f) * 0.2f - state.turnFactor,
      sinf(t * 2.f + 1.f) * cosf(t * 1.7f) * 0.2f - 0.1f,
      0
    );
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

    // @todo see if this is necessary
    // animatedVertex.vertex.position = Vec3f::lerp(mesh.transformedVertices[i].position, animatedVertex.vertex.position, 0.5f);

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

    // Rear right leg bottom [12]
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

    // Tail joint 1 [17]
    rig.joints.push_back({
      .position = Vec3f(0, 0.1f, 0.7f),
      .rotation = Quaternion(1.f, 0, 0, 0)
    });

    // Tail joint 2 [18]
    rig.joints.push_back({
      .position = Vec3f(0, 0.2f, 1.5f),
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
    bool usedBoostRing = time_since(state.lastBoostRingLaunchTime) < SOMERSAULT_DURATION;

    if (state.velocity.magnitude() > 20.f && (!state.isGliding || usedBoostRing)) {
      // Determine the updated character rotation, based on movement direction,
      // as long as we're moving + not gliding, or if we just used a boost ring.
      yaw = atan2f(movement.x, movement.z) + Gm_PI;
      pitch = -1.f * atan2f(movement.y, movement.xz().magnitude());
    }

    // Perform somersaults when launching through boost rings without the glider
    if (usedBoostRing && !state.isGliding) {
      float alpha = easeOutQuint(time_since(state.lastBoostRingLaunchTime) * (1.f / SOMERSAULT_DURATION));

      pitch += Gm_TAU * alpha;
    }

    // Spin when air dashing
    float timeSinceLastAirDash = time_since(state.lastAirDashTime);
    float totalRotation = state.airDashSpinEndYaw - state.airDashSpinStartYaw;

    if (
      state.lastAirDashTime != 0.f &&
      timeSinceLastAirDash < AIR_DASH_SPIN_DURATION &&
      // Only keep spinning if we're in midair, OR if we're still a bit
      // early in the spin animation. We use an arbitrary threshold which
      // "feels" right to ensure that manual movement takes over the yaw,
      // canceling the spin, but not so quickly as to look discontinuous.
      (!state.isOnSolidGround || timeSinceLastAirDash < AIR_DASH_SPIN_DURATION * 0.3f)
    ) {
      float alpha = easeOutBack(timeSinceLastAirDash / AIR_DASH_SPIN_DURATION, 1.2f);
      float targetTurnFactor = 2.f * (1.f - alpha);

      yaw = state.airDashSpinStartYaw + alpha * totalRotation;

      state.turnFactor = Gm_Lerpf(state.turnFactor, targetTurnFactor, 10.f * dt);
    }

    yaw = Gm_LerpCircularf(state.currentYaw, yaw, 10.f * dt, Gm_PI);

    if (time_since(state.lastBoostRingLaunchTime) < SOMERSAULT_DURATION) {
      pitch = Gm_Lerpf(state.currentPitch, pitch, 10.f * dt);
    } else {
      pitch = Gm_LerpCircularf(state.currentPitch, pitch, 10.f * dt, Gm_PI);
    }

    player.rotation = Quaternion::fromAxisAngle(Vec3f(0, 0, 1.f), state.turnFactor * 0.5f);
    player.rotation *= Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), yaw);
    player.rotation *= Quaternion::fromAxisAngle(player.rotation.getLeftDirection(), pitch);

    handlePlayerAnimation(context, state, dt);
    handleAnimatedMeshWithRig(*mesh("player"), state.animation.playerRig);

    if (state.lastBoostRingLaunchTime != 0.f && time_since(state.lastBoostRingLaunchTime) < 1.f) {
      float somersaultAlpha = time_since(state.lastBoostRingLaunchTime) * (1.f / SOMERSAULT_DURATION);
      float somersaultScale = Gm_Maxf(0.f, sinf(somersaultAlpha * Gm_PI) * 0.5f);

      player.scale.z = PLAYER_RADIUS * (1.f + somersaultScale);
    }

    state.currentYaw = yaw;
    state.currentPitch = pitch;

    // Normalize yaw to within the range [-Gm_PI, Gm_PI]
    if (state.currentYaw < -Gm_PI) state.currentYaw += Gm_TAU;
    if (state.currentYaw > Gm_PI) state.currentYaw -= Gm_TAU;
  }

  LOG_TIME();
}