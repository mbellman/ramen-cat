#include "entity_system.h"
#include "camera_system.h"
#include "ui_system.h"
#include "inventory_system.h"
#include "world.h"
#include "game_meshes.h"
#include "macros.h"
#include "easing.h"
#include "game_constants.h"

#define for_moving_objects(meshName, code)\
  u16 __activeMeshIndex = context->scene.meshMap.at(meshName)->index;\
  for (auto& initial : state.initialMovingObjects) {\
    if (initial._record.meshIndex == __activeMeshIndex) {\
      auto* __object = get_object_by_record(initial._record);\
      if (__object != nullptr) {\
        auto& object = *__object;\
        code\
      }\
    }\
  }

using namespace Gamma;

/**
 * Adapted from http://paulbourke.net/miscellaneous/interpolation/
 *
 * Performs cubic spline interpolation.
 */
internal float interpolateCubicSpline(float a, float b, float c, float d, float alpha) {
  float m = alpha * alpha;

  float a0 = d - c - a + b;
  float a1 = a - b - a0;
  float a2 = c - a;
  float a3 = b;
  
  return (a0 * alpha * m) + (a1 * m) + (a2 * alpha) + a3;
}

internal Vec3f interpolateCubicSpline(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d, float alpha) {
  return Vec3f(
    interpolateCubicSpline(a.x, b.x, c.x, d.x, alpha),
    interpolateCubicSpline(a.y, b.y, c.y, d.y, alpha),
    interpolateCubicSpline(a.z, b.z, c.z, d.z, alpha)
  );
}

internal u32 getWrappedIndex(s32 index, u32 total) {
  if (index < 0) {
    return u32(total + index);
  } else if (u32(index) >= total) {
    return u32(index % total);
  }

  return u32(index);
}

internal void addJetstream(GmContext* context, GameState& state, const std::vector<Vec3f>& points) {
  // Particles
  {
    add_mesh("jetstream", 5000, Mesh::Particles(true));

    auto& particles = mesh("jetstream")->particles;

    for (auto& point : points) {
      particles.path.push_back(point);
    }

    // @todo make configurable
    particles.medianSize = 3.f;
    particles.sizeVariation = 2.5f;
    particles.medianSpeed = 0.2f;
    particles.spread = 500.f;
    particles.deviation = 100.f;
  }

  // Entity
  {
    // @todo pass as a parameter
    const float TOTAL_POINTS = 50;

    Jetstream stream;

    for (u8 i = 0; i < TOTAL_POINTS; i++) {
      float progress = float(i) / float(TOTAL_POINTS);
      float completion = progress * points.size();
      float alpha = completion - u32(completion);
      u32 currentIndex = u32(std::floorf(completion));

      Vec3f streamPoint = interpolateCubicSpline(
        points[getWrappedIndex(currentIndex - 1, points.size())],
        points[getWrappedIndex(currentIndex, points.size())],
        points[getWrappedIndex(currentIndex + 1, points.size())],
        points[getWrappedIndex(currentIndex + 2, points.size())],
        alpha
      );

      stream.points.push_back(streamPoint);
    }

    state.jetstreams.push_back(stream);
  }
}

internal void interactWithNpc(GmContext* context, GameState& state, NonPlayerCharacter& npc) {
  auto& player = get_player();
  Vec3f npcFacePosition = npc.position + Vec3f(0, 30.f, 0);
  Vec3f npcToPlayer = (player.position - npc.position);

  state.activeNpc = &npc;
  state.velocity = Vec3f(0.f);

  CameraSystem::setTargetCameraState(context, state, {
    .camera3p = {
      .azimuth = atan2f(npcToPlayer.z, npcToPlayer.x) - Gm_TAU / 8.f,
      .altitude = 0.f,
      .radius = NPC_INTERACTION_CAMERA_RADIUS
    },
    .lookAtTarget = {
      (npcFacePosition.x + player.position.x) / 2.f,
      npcFacePosition.y,
      (npcFacePosition.z + player.position.z) / 2.f
    }
  });

  UISystem::queueDialogue(context, state, npc.dialogue);
}

internal void interactWithSlingshot(GmContext* context, GameState& state, Object& object) {
  auto& player = get_player();

  // @temporary
  float xzVelocity = 350.f;
  float yVelocity = 1500.f;

  // @temporary
  for (auto& slingshot : state.slingshots) {
    if (slingshot.position == object.position) {
      xzVelocity = slingshot.xzVelocity;
      yVelocity = slingshot.yVelocity;

      break;
    }
  }

  Vec3f slingshotToPlayer = player.position - object.position;
  float slingshotToPlayerAngle = atan2f(slingshotToPlayer.z, slingshotToPlayer.x);
  Vec3f playerDirection = slingshotToPlayer.xz().unit();

  // @todo make configurable
  float xVelocity = xzVelocity * playerDirection.x * -1.f;
  float zVelocity = xzVelocity * playerDirection.z * -1.f;
  Vec3f slingshotVelocity = Vec3f(xVelocity, yVelocity, zVelocity);
  Vec3f slingshotDirection = object.rotation.getDirection();

  state.lastSlingshotInteractionTime = get_scene_time();
  state.startingSlingshotAngle = Gm_Modf(-atan2f(slingshotDirection.z, slingshotDirection.x) + Gm_HALF_PI, Gm_TAU);
  state.targetSlingshotAngle = Gm_Modf(-slingshotToPlayerAngle + Gm_HALF_PI, Gm_TAU);
  state.activeSlingshotRecord = object._record;
  state.slingshotVelocity = slingshotVelocity;
  state.velocity = Vec3f(0.f);

  // Control camera override behavior
  {
    CameraSystem::setTargetCameraState(context, state, {
      .camera3p = {
        .azimuth = slingshotToPlayerAngle - Gm_PI / 16.f,
        .altitude = 0.f,
        .radius = SLINGSHOT_WIND_UP_CAMERA_RADIUS
      },
      .lookAtTarget = object.position
    });

    // Have the restored camera state center behind/above
    // the player, once launched from the slingshot
    state.originalCameraState.camera3p.azimuth = Gm_Modf(atan2f(slingshotVelocity.z, slingshotVelocity.x) - Gm_PI, Gm_TAU);
    state.originalCameraState.camera3p.altitude = Gm_HALF_PI * 0.8f;
    state.originalCameraState.camera3p.radius =
      (state.cameraMode == CameraMode::NORMAL ? CAMERA_NORMAL_BASE_RADIUS : CAMERA_ZOOM_OUT_BASE_RADIUS)
      + CAMERA_RADIUS_ALTITUDE_MULTIPLIER * (state.originalCameraState.camera3p.altitude / Gm_HALF_PI);
  }
}

internal void handleNpcs(GmContext* context, GameState& state) {
  auto& input = get_input();
  auto& player = get_player();

  // Handle talking to NPCs
  {
    if (input.didPressKey(Key::SPACE)) {
      if (state.activeNpc == nullptr) {
        for (auto& npc : state.npcs) {
          float npcXzDistance = (npc.position - player.position).xz().magnitude();

          if (
            npcXzDistance < NPC_INTERACTION_TRIGGER_DISTANCE &&
            player.position.y < npc.position.y + NPC_HEIGHT &&
            player.position.y > npc.position.y - NPC_HEIGHT
          ) {
            interactWithNpc(context, state, npc);

            break;
          }
        }
      }
    }
  }

  // Handle ending conversions with NPCs
  {
    if (state.activeNpc != nullptr && UISystem::isDialogueQueueEmpty()) {
      state.activeNpc = nullptr;

      CameraSystem::restoreOriginalCameraState(context, state);
    }
  }
}

internal void handlePeople(GmContext* context, GameState& state) {
  auto& player = get_player();

  for_moving_objects("person", {
    Vec3f personToPlayer = player.position - object.position;
    Vec3f baseDirection = initial.rotation.getDirection();
    float baseAngle = atan2f(baseDirection.x, baseDirection.z);
    float facingAngle = atan2f(personToPlayer.x, personToPlayer.z);
    float distance = personToPlayer.magnitude();
    float alpha = powf(1.f - distance / (distance + 1000.f), 2.f);
    float angle = Gm_LerpCircularf(baseAngle, facingAngle, alpha, Gm_PI);

    object.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), angle);

    commit(object);
  });
}

internal void handleBirds(GmContext* context, GameState& state, float dt) {
  auto& player = get_player();
  float t = get_scene_time() * 10.f;

  for_moving_objects("bird-at-rest", {
    if (object.scale.x < 1.f) {
      continue;
    }

    float alpha = t + float(object._record.id) * 2.f;
    float yOffset = powf(sinf(alpha) * 0.5f + 0.5f, 6.f) * 5.f;
    float distance = (player.position - object.position).magnitude();

    // @todo base distance on player speed
    if (distance < 200.f) {
      auto& flyingBird = create_object_from("bird-flying");

      flyingBird.position = object.position;
      flyingBird.scale = object.scale;
      flyingBird.rotation = object.rotation;

      commit(flyingBird);

      object.scale = Vec3f(0.f);
    } else {
      object.position = initial.position + Vec3f(0, yOffset, 0);
    }

    commit(object);
  });

  for (auto& bird : objects("bird-flying")) {
    bird.position += bird.rotation.getDirection() * 600.f * dt;
    bird.position.y += 100.f * dt;

    commit(bird);
  }
}

internal void handleSeagulls(GmContext* context, GameState& state, float dt) {
  float t = get_scene_time() * 0.4f;

  for_moving_objects("seagull", {
    float alpha = t + float(object._record.id);
    float xOffset = sinf(2.f * alpha) * 1000.f;
    float yOffset = sin(2.f * alpha) * 50.f;
    float zOffset = 2.f * cosf(alpha) * 1000.f;

    Vec3f targetPosition = initial.position + Vec3f(xOffset, yOffset, zOffset);
    Vec3f delta = targetPosition - object.position;
    float turnAngle = atan2f(delta.x, delta.z) + Gm_PI;
    float tiltAngle = 0.75f * cosf(alpha);

    Quaternion turn = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), turnAngle);
    Quaternion tilt = Quaternion::fromAxisAngle(Vec3f(0, 0, 1.f), tiltAngle);

    object.position = targetPosition;
    object.rotation = turn * tilt;

    commit(object);
  });
}

internal void handleSlingshots(GmContext* context, GameState& state, float dt) {
  auto& player = get_player();
  auto& input = get_input();

  // Handle proximity to slingshots
  {
    for (auto& slingshot : objects("slingshot")) {
      Vec3f targetColor = DEFAULT_SLINGSHOT_COLOR;

      if ((slingshot.position - player.position).magnitude() < SLINGSHOT_INTERACTION_TRIGGER_DISTANCE) {
        targetColor = HIGHLIGHT_SLINGSHOT_COLOR;

        if (input.didPressKey(Key::SPACE)) {
          state.velocity = Vec3f(0.f);

          interactWithSlingshot(context, state, slingshot);
        }
      }

      // @optimize we don't need to constantly lerp the color for
      // every slingshot; only ones affected by player proximity
      slingshot.color = Vec3f::lerp(slingshot.color.toVec3f(), targetColor, 10.f * dt);

      commit(slingshot);
    }
  }

  // Handle launching from slingshots
  {
    if (state.lastSlingshotInteractionTime != 0.f) {
      float timeSinceLastSlingshotInteraction = time_since(state.lastSlingshotInteractionTime);
      auto* slingshot = get_object_by_record(state.activeSlingshotRecord);

      if (timeSinceLastSlingshotInteraction < SLINGSHOT_WIND_UP_DURATION_SECONDS) {
        // Wind-up
        if (slingshot != nullptr) {
          // @todo change the easing function to not require artificially reducing alpha
          float alpha = 0.3f * (1.f / SLINGSHOT_WIND_UP_DURATION_SECONDS) * timeSinceLastSlingshotInteraction;
          float angle = Gm_LerpCircularf(state.startingSlingshotAngle, state.targetSlingshotAngle, easeOutElastic(alpha), Gm_HALF_PI);

          slingshot->rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), angle);

          commit(*slingshot);
        }
      } else {
        // Launch
        CameraSystem::restoreOriginalCameraState(context, state);

        state.velocity = state.slingshotVelocity;
        state.lastSlingshotInteractionTime = 0.f;

        state.isOnSolidGround = false;
        state.canPerformAirDash = true;
        state.canPerformWallKick = true;

        if (slingshot != nullptr) {
          slingshot->rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), state.targetSlingshotAngle);

          commit(*slingshot);
        }
      }
    }
  }
}

internal void handleLanterns(GmContext* context, GameState& state, float dt) {
  {
    for_moving_objects("lantern", {
      float alpha = get_scene_time() * 1.5f + initial.position.z / 200.f;
      float sineAlpha = sinf(alpha);
      float swing = 0.2f * sineAlpha;

      float x = LANTERN_HORIZONTAL_DRIFT * sineAlpha;
      float y = LANTERN_VERTICAL_DRIFT * powf(Gm_Absf(x) / LANTERN_HORIZONTAL_DRIFT, 2);

      object.rotation = Quaternion::fromAxisAngle(Vec3f(0, 0, 1.f), swing); 
      object.position = initial.position + Vec3f(x, y, 0);

      commit(object);
    });
  }

  {
    for_moving_objects("paper-lantern", {
      float alpha = get_scene_time() * 1.5f + initial.position.z / 200.f;
      float sineAlpha = sinf(alpha);
      float swing = 0.2f * sineAlpha;

      float x = LANTERN_HORIZONTAL_DRIFT * sineAlpha;
      float y = LANTERN_VERTICAL_DRIFT * powf(Gm_Absf(x) / LANTERN_HORIZONTAL_DRIFT, 2);

      object.rotation = Quaternion::fromAxisAngle(Vec3f(0, 0, 1.f), swing); 
      object.position = initial.position + Vec3f(x, y, 0);

      commit(object);
    });
  }

  {
    for_moving_objects("ramen-lamp", {
      float alpha = get_scene_time() * 1.5f + initial.position.z / 200.f;
      float sineAlpha = sinf(alpha);
      float swing = 0.05f * sineAlpha;

      float x = 0.25f * LANTERN_HORIZONTAL_DRIFT * sineAlpha;
      float y = 0.25f * LANTERN_VERTICAL_DRIFT * powf(Gm_Absf(x) / LANTERN_HORIZONTAL_DRIFT, 2);

      object.rotation = Quaternion::fromAxisAngle(Vec3f(0, 0, 1.f), swing); 
      object.position = initial.position + Vec3f(x, y, 0);

      commit(object);
    });
  }
}

internal void handleWindmillWheels(GmContext* context, GameState& state, float dt) {
  float t = get_scene_time();

  for_moving_objects("windmill-wheel", {
    auto rotationAxis = initial.rotation.getDirection();
    // Rotate larger windmill wheels more slowly
    float scaleRatio = Gm_Clampf(initial.scale.magnitude() / 500.f, 0.f, 1.f);
    float rotationSpeedFactor = Gm_Lerpf(2.f, 0.2f, scaleRatio);
    float angle = rotationSpeedFactor * get_scene_time();

    object.rotation = Quaternion::fromAxisAngle(rotationAxis, angle) * initial.rotation;

    commit(object);
  });
}

internal void handleWindTurbines(GmContext* context, GameState& state, float dt) {
  float t = get_scene_time();

  for_moving_objects("wind-turbine", {
    auto rotationAxis = initial.rotation.getDirection();
    float angle = 0.25f * t + float(object._record.id);

    object.rotation = Quaternion::fromAxisAngle(rotationAxis, angle) * initial.rotation;

    commit(object);
  });
}

internal void handleAcFans(GmContext* context, GameState& state, float dt) {
  for_moving_objects("ac-fan", {
    auto rotationAxis = initial.rotation.getDirection();
    float angle = 3.f * get_scene_time();

    object.rotation = Quaternion::fromAxisAngle(rotationAxis, angle) * initial.rotation;

    commit(object);
  });
}

internal void handleHotAirBalloons(GmContext* context, GameState& state, float dt) {
  for_moving_objects("hot-air-balloon", {
    float offset = object.position.x + object.position.z;
    float heightRate = 0.5f * get_scene_time() + offset;
    float heightOscillation = object.scale.x / 10.f;
    float rotationRate = 0.7f * get_scene_time() + offset;

    object.position = initial.position + Vec3f(0, heightOscillation, 0) * sinf(heightRate);
    object.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), 0.05f * sinf(rotationRate));

    commit(object);
  });
}

internal void handleCollectables(GmContext* context, GameState& state, float dt) {
  auto& player = get_player();
  float t = get_scene_time();
  float t2 = t * 2.f;

  // @todo generalize collectable behavior, store special entities for collectables to determine appropriate handling
  {
    for_moving_objects("onigiri", {
      if (object.scale.x < 0.1f) {
        // Already collected
        object.scale = Vec3f(0.f);
      } else if (object.scale.x != initial.scale.x) {
        // Collection animation
        if (object.position.y - initial.position.y > 50.f) {
          object.scale -= 200.f * dt;
        } else {
          object.scale -= 15.f * dt;
        }

        float alpha = easeOutBack(1.f - object.scale.x / initial.scale.x, 1.7f);

        object.position = Vec3f::lerp(object.position, initial.position + Vec3f(0, 60.f, 0), alpha);
      } else {
        // Idle (uncollected) animation
        float yOffset = sinf(t2 + float(object._record.id) * 0.5f);

        object.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), t);
        object.position = initial.position + Vec3f(0, yOffset, 0) * 10.f;

        if ((object.position - player.position).magnitude() < 100.f) {
          // Item collected!
          object.scale *= 0.99f;
          object.position = Vec3f::lerp(object.position, player.position, 10.f * dt);

          if (state.isInToriiGateZone) {
            InventorySystem::collectItem(context, state.inventory.demonOnigiri);
          } else {
            InventorySystem::collectItem(context, state.inventory.onigiri);
          }
        }
      }

      commit(object);
    });
  }

  {
    for_moving_objects("nitamago", {
      if (object.scale.x < 0.1f) {
        // Already collected
        object.scale = Vec3f(0.f);
      } else if (object.scale.x != initial.scale.x) {
        // Collection animation
        if (object.position.y - initial.position.y > 50.f) {
          object.scale -= 200.f * dt;
        } else {
          object.scale -= 15.f * dt;
        }

        float alpha = easeOutBack(1.f - object.scale.x / initial.scale.x, 1.7f);

        object.position = Vec3f::lerp(object.position, initial.position + Vec3f(0, 60.f, 0), alpha);
      } else {
        // Idle (uncollected) animation
        float yOffset = sinf(t2 + float(object._record.id) * 0.5f);

        object.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), t);
        object.position = initial.position + Vec3f(0, yOffset, 0) * 10.f;

        if ((object.position - player.position).magnitude() < 100.f) {
          // Item collected!
          object.scale *= 0.99f;
          object.position = Vec3f::lerp(object.position, player.position, 10.f * dt);

          if (state.isInToriiGateZone) {
            InventorySystem::collectItem(context, state.inventory.demonNitamago);
          } else {
            InventorySystem::collectItem(context, state.inventory.nitamago);
          }
        }
      }

      commit(object);
    });
  }
}

internal void handleJetstreams(GmContext* context, GameState& state, float dt) {
  auto& player = get_player();
  auto& input = get_input();

  for (auto& stream : state.jetstreams) {
    float closestDistance = Gm_FLOAT_MAX;
    Vec3f closestPoint;
    Vec3f nextPoint;

    for (u32 p = 0; p < stream.points.size(); p++) {
      auto& point = stream.points[p];
      float distance = (player.position - point).magnitude();

      if (distance < closestDistance) {
        closestDistance = distance;
        closestPoint = point;
        nextPoint = p == stream.points.size() - 1 ? stream.points[0] : stream.points[p + 1];
      }
    }

    if (closestDistance < stream.radius) {
      if (time_since(state.lastAirDashTime) > 1.f) {
        // Reset active air dashes when inside the stream for sufficiently long
        state.dashLevel = 0;
        state.canPerformAirDash = true;
      }

      float alpha = sqrtf(1.f - closestDistance / stream.radius);
      Vec3f streamDirection = (nextPoint - player.position).unit();
      float speed;

      if (!input.isKeyHeld(Key::W) && !input.isKeyHeld(Key::A) && !input.isKeyHeld(Key::S) && !input.isKeyHeld(Key::D)) {
        state.velocity *= 1.f - 2.f * dt;

        speed = 5000.f;
      } else {
        speed = 100.f;
      }

      float targetCameraAzimuth = atan2f(streamDirection.z, streamDirection.x) + Gm_PI;

      state.velocity += streamDirection * speed * alpha * dt;
      state.camera3p.azimuth = Gm_LerpCircularf(state.camera3p.azimuth, targetCameraAzimuth, 0.75f * dt, Gm_PI);
    }
  }
}

internal void handleOcean(GmContext* context) {
  auto& camera = get_camera();
  auto& ocean = objects("ocean")[0];
  auto& floor = objects("ocean-floor")[0];

  ocean.position.x = camera.position.x;
  floor.position.x = camera.position.x;

  ocean.position.z = camera.position.z;
  floor.position.z = camera.position.z;

  commit(ocean);
  commit(floor);
}

internal void handleToriiGates(GmContext* context, GameState& state) {
  auto& player = get_player();

  for (auto& gate : objects("torii-gate")) {
    Vec3f forward = gate.rotation.getDirection();
    Vec3f lastPlayerDirection = (state.previousPlayerPosition - gate.position);
    Vec3f currentPlayerDirection = (player.position - gate.position);
    float lastDot = Vec3f::dot(forward, lastPlayerDirection);
    float currentDot = Vec3f::dot(forward, currentPlayerDirection);

    if (
      Gm_Signf(lastDot) != Gm_Signf(currentDot) &&
      currentPlayerDirection.magnitude() < gate.scale.x * 0.8f
    ) {
      state.isInToriiGateZone = !state.isInToriiGateZone;
      state.toriiGateTransitionTime = get_scene_time();

      context->scene.fx.redshiftSpawn = gate.position;

      break;
    }
  }
}

void EntitySystem::initializeGameEntities(GmContext* context, GameState& state) {
  // @temporary
  {
    // addJetstream(context, state, {
    //   Vec3f(475.f, 2785.f, 3550.f),
    //   Vec3f(2100.f, 2050.f, 1700.f),
    //   Vec3f(1985.f, 2360.f, -875.f),
    //   Vec3f(-1000.f, 2850.f, -2250.f),
    //   Vec3f(-2300.f, 3500.f, -350.f),
    //   Vec3f(-1500.f, 2600.f, 2600.f)
    // });
  }
}

void EntitySystem::handleGameEntities(GmContext* context, GameState& state, float dt) {
  START_TIMING("handleGameEntities");

  // Inert entities
  handleBirds(context, state, dt);
  handleSeagulls(context, state, dt);
  handleLanterns(context, state, dt);
  handleWindmillWheels(context, state, dt);
  handleWindTurbines(context, state, dt);
  handleAcFans(context, state, dt);
  handleOcean(context);

  // Interactible/player-dependent entities
  handleSlingshots(context, state, dt);
  handleNpcs(context, state);
  handlePeople(context, state);
  handleHotAirBalloons(context, state, dt);
  handleCollectables(context, state, dt);
  handleJetstreams(context, state, dt);
  handleToriiGates(context, state);

  LOG_TIME();
}

bool EntitySystem::isInteractingWithEntity(GmContext* context, GameState& state) {
  return state.lastSlingshotInteractionTime != 0.f;
}