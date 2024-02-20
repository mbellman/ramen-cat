#include "game_meshes.h"
#include "procedural_meshes.h"
#include "macros.h"

using namespace Gamma;

internal float randomFromVec3f(const Vec3f& v) {
  float a = Gm_Modf(v.x + v.y + v.z, 1.f);
  float b = a * 1024.345267f;

  return Gm_Modf(sqrtf(b), 1.f);
}

internal float randomVariance(float random, float variance) {
  return -variance + random * variance * 2.f;
}

internal void rebuildCollectableStrips(GmContext* context, const std::string& collectableMeshName) {
  const float DEFAULT_SCALE = 40.f;

  for (auto& strip : objects(collectableMeshName + "-strip")) {
    Vec3f start;
    Vec3f end;

    if (strip.scale.x > strip.scale.z) {
      start = strip.position + strip.rotation.getLeftDirection() * strip.scale.x;
      end = strip.position + strip.rotation.getLeftDirection().invert() * strip.scale.x;
    } else {
      start = strip.position + strip.rotation.getDirection() * strip.scale.z;
      end = strip.position + strip.rotation.getDirection().invert() * strip.scale.z;
    }

    float distance = (end - start).magnitude();
    Vec3f direction = (end - start).unit();
    u8 current = 0;
    u8 total = u8(distance / 100.f);

    while (current <= total) {
      auto& collectable = create_object_from(collectableMeshName);

      collectable.position = start + direction * 100.f * float(current);
      collectable.scale = Vec3f(DEFAULT_SCALE);

      commit(collectable);

      current++;
    }
  }
}

internal void rebuildCollectableSpawns(GmContext* context, const std::string& collectableMeshName) {
  const float DEFAULT_SCALE = 40.f;
  const float DEFAULT_RADIUS = 80.f;

  for (auto& spawn : objects(collectableMeshName + "-spawn")) {
    for (u8 i = 0; i < 5; i++) {
      auto& collectable = create_object_from(collectableMeshName);
      auto alpha = Gm_TAU * float(i) / 5.f;

      collectable.position = spawn.position + Vec3f(sinf(alpha), 0, cosf(alpha)) * DEFAULT_RADIUS;
      collectable.scale = Vec3f(DEFAULT_SCALE);

      commit(collectable);
    }
  }
}

internal void rebuildCollectables(GmContext* context) {
  rebuildCollectableStrips(context, "onigiri");
  rebuildCollectableStrips(context, "nitamago");
  rebuildCollectableStrips(context, "chashu");

  rebuildCollectableSpawns(context, "onigiri");
  // rebuildCollectableSpawns(context, "nitamago");
  // rebuildCollectableSpawns(context, "chashu");
}

internal void rebuildPlantStrips(GmContext* context) {
  for (auto& strip : objects("plant-strip")) {
    auto start = Vec3f(0);
    auto end = Vec3f(0);
    auto matRotation = strip.rotation.toMatrix4f();

    if (strip.scale.x > strip.scale.z) {
      start = strip.position + matRotation.transformVec3f(Vec3f(-strip.scale.x, 0, 0));
      end = strip.position + matRotation.transformVec3f(Vec3f(strip.scale.x, 0, 0));
    } else {
      start = strip.position + matRotation.transformVec3f(Vec3f(0, 0, -strip.scale.z));
      end = strip.position + matRotation.transformVec3f(Vec3f(0, 0, strip.scale.z));
    }

    u8 total = u8((end - start).magnitude() / 150.f);

    // Weeds
    for (u8 i = 0; i < total; i++) {
      auto& weeds = create_object_from("p_weeds");
      auto alpha = float(i) / float(total);
      auto basePosition = Vec3f::lerp(start, end, alpha);
      auto random = randomFromVec3f(basePosition + Vec3f(1.23f, 0, 0));
      auto random2 = randomFromVec3f(basePosition + Vec3f(4.46f, 0, 0));

      weeds.position = basePosition + Vec3f(
        randomVariance(random, 20.f),
        -strip.scale.y * 0.5f,
        randomVariance(random2, 30.f)
      );

      weeds.scale = Vec3f(50.f + randomVariance(random, 20.f));
      weeds.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), random * Gm_PI * 2.f);

      commit(weeds);
    }

    // Bushes
    const float DEFAULT_SIZE = 80.f;
    const float SIZE_VARIANCE = 40.f;

    for (u8 i = 0; i < total; i++) {
      auto alpha = float(i) / float(total);
      auto basePosition = Vec3f::lerp(start, end, alpha);
      auto random = randomFromVec3f(basePosition);
      auto random2 = randomFromVec3f(basePosition + Vec3f(1.23f, 0, 0));

      auto& bush =
        random > 0.7f ? create_object_from("p_shrub") :
        random > 0.3f ? create_object_from("p_shrub-2") :
        create_object_from("p_banana-plant");

      bush.position = basePosition + Vec3f(
        randomVariance(random, 50.f),
        randomVariance(random, 10.f),
        randomVariance(random2, 70.f)
      );

      bush.scale = Vec3f(DEFAULT_SIZE + randomVariance(random2, SIZE_VARIANCE));
      bush.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), random * Gm_PI * 2.f);

      bush.color = Vec3f(
        1.f,
        0.9f + randomVariance(random, 0.1f),
        0.9f + randomVariance(random2, 0.1f)
      );

      commit(bush);
    }
  }
}

internal void rebuildConcreteStacks(GmContext* context) {
  const auto PIECE_SIZE = 600.f;

  for (auto& stack : objects("concrete-stack")) {
    auto width = stack.scale.x;
    auto height = stack.scale.y;
    auto depth = stack.scale.z;

    auto totalX = (u8)std::ceilf(width / PIECE_SIZE);
    auto totalY = (u8)std::ceilf(height / PIECE_SIZE);
    auto totalZ = (u8)std::ceilf(depth / PIECE_SIZE);

    auto base = Vec3f(
      stack.position.x + width,
      stack.position.y + height,
      stack.position.z + depth
    );

    auto scale = Vec3f(
      width / float(totalX),
      height / float(totalY),
      depth / float (totalZ)
    );

    for (u8 x = 0; x < totalX; x++) {
      for (u8 y = 0; y < totalY; y++) {
        for (u8 z = 0; z < totalZ; z++) {
          auto worldPosition = Vec3f(
            base.x - x * scale.x * 2.f - scale.x,
            base.y - y * scale.y * 2.f - scale.y,
            base.z - z * scale.z * 2.f - scale.z
          );

          auto offset = worldPosition - stack.position;
          auto& piece = create_object_from("p_concrete");

          piece.position = stack.position + stack.rotation.toMatrix4f().transformVec3f(offset);

          auto random = randomFromVec3f(piece.position);
          auto yaw = (random * 10.f - 5.f) * (Gm_PI / 180.f);

          piece.scale = scale;
          piece.rotation = stack.rotation * Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), yaw);

          if (y == 0 || y == totalY - 1) {
            piece.scale.y += random * 5.f;
          } else {
            piece.scale.y += random * 200.f;
            piece.scale.x += random * 200.f;
            piece.scale.z += random * 200.f;
          }

          piece.color = stack.color.toVec3f() * Vec3f(
            0.5f + random * 0.5f
          );

          commit(piece);
        }
      }
    }
  }
}

internal void rebuildMiniHouses(GmContext* context) {
  for (auto& source : objects("mini-house")) {
    auto origin = source.position;
    auto random = randomFromVec3f(origin);
    auto sourceRotationMatrix = source.rotation.toMatrix4f();

    // Base
    {
      auto& base = create_object_from("p_mini-house");
      auto r1 = randomFromVec3f(origin + Vec3f(0, 1.23f, 0));
      auto r2 = randomFromVec3f(origin + Vec3f(0, 2.34f, 0));
      auto r3 = randomFromVec3f(origin + Vec3f(0, 3.45f, 0));

      base.position = source.position;
      base.scale = source.scale;
      base.rotation = source.rotation;
      base.color = Vec3f(0.5f + 0.5f * r1, 0.5f + 0.5f * r2, 0.3f + 0.4f * (1.f - r3));

      commit(base);
    }

    // Roof
    {
      auto& roof = create_object_from("p_mini-house-roof");
      auto offset = Vec3f(0, source.scale.y * 1.04f, source.scale.z * 0.5f);

      roof.position = origin + sourceRotationMatrix.transformVec3f(offset);
      roof.scale = Vec3f(source.scale.x, source.scale.x, source.scale.z) * 1.1f;

      roof.rotation =
        source.rotation * (
          Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), Gm_PI) *
          Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -15.f * (Gm_PI / 180.f))
        );

      if (random < 0.3f) {
        roof.color = Vec3f(0.4f, 0.8f, 0.5f);
      } else if (random < 0.6f) {
        roof.color = Vec3f(0.8f, 0.5f, 0.4f);
      } else {
        roof.color = Vec3f(0.4f, 0.6f, 0.8f);
      }

      commit(roof);
    }

    // Beams
    {
      {
        auto& left = create_object_from("p_mini-house-wood-beam");
        auto offset = Vec3f(source.scale.x, 0, source.scale.z);

        left.position = origin + sourceRotationMatrix.transformVec3f(offset);
        left.scale = Vec3f(10.f, source.scale.y * 0.99f, 12.f);
        left.rotation = source.rotation;
        left.color = Vec3f(0.7f);

        commit(left);
      }

      {
        auto& right = create_object_from("p_mini-house-wood-beam");
        auto offset = Vec3f(-source.scale.x, 0, source.scale.z);

        right.position = origin + sourceRotationMatrix.transformVec3f(offset);
        right.scale = Vec3f(10.f, source.scale.y * 0.99f, 12.f);
        right.rotation = source.rotation;
        right.color = Vec3f(0.7f);

        commit(right);
      }
    }

    // Window + associated parts
    {
      // Window
      auto& window = create_object_from("p_mini-house-window");
      auto windowOffset = Vec3f(source.scale.x * (random * 0.5f - 0.25f), source.scale.y * random * 0.5f, source.scale.z * 1.05f);

      window.position = origin + sourceRotationMatrix.transformVec3f(windowOffset);
      window.scale = Vec3f(source.scale.x * 0.5f, source.scale.y * 0.5f, source.scale.z);
      window.rotation = source.rotation;

      commit(window);

      // Allow window beams to be aligned with or below the window
      float beamOffset = random > 0.5f ? -window.scale.y : 0.f;

      // Upper beam
      auto& topBeam = create_object_from("p_mini-house-wood-beam");
      auto topBeamOffset = Vec3f(0, windowOffset.y + window.scale.y * 0.5f + beamOffset, source.scale.z);

      topBeam.position = origin + sourceRotationMatrix.transformVec3f(topBeamOffset);
      topBeam.scale = Vec3f(10.f, source.scale.x, 10.f);
      topBeam.rotation = source.rotation * Quaternion::fromAxisAngle(Vec3f(0, 0, 1.f), Gm_HALF_PI);
      topBeam.color = Vec3f(0.7f);

      commit(topBeam);

      // Lower beam
      auto& bottomBeam = create_object_from("p_mini-house-wood-beam");
      auto bottomBeamOffset = Vec3f(0, windowOffset.y - window.scale.y * 0.5f + beamOffset, source.scale.z);

      bottomBeam.position = origin + sourceRotationMatrix.transformVec3f(bottomBeamOffset);
      bottomBeam.scale = Vec3f(10.f, source.scale.x, 10.f);
      bottomBeam.rotation = source.rotation * Quaternion::fromAxisAngle(Vec3f(0, 0, 1.f), Gm_HALF_PI);
      bottomBeam.color = Vec3f(0.7f);

      commit(bottomBeam);

      // Wooden boards beneath the window panel
      auto& leftBoard = create_object_from("p_mini-house-board");
      auto& rightBoard = create_object_from("p_mini-house-board");
      auto leftBoardOffset = Vec3f(source.scale.x * 0.5f, windowOffset.y + beamOffset, source.scale.z * 1.05f);
      auto rightBoardOffset = Vec3f(-source.scale.x * 0.5f, windowOffset.y + beamOffset, source.scale.z * 1.05f);

      leftBoard.position = origin + sourceRotationMatrix.transformVec3f(leftBoardOffset);
      rightBoard.position = origin + sourceRotationMatrix.transformVec3f(rightBoardOffset);

      leftBoard.scale =
      rightBoard.scale = Vec3f(source.scale.x * 0.5f, 1.f, window.scale.y * 0.48f);

      leftBoard.rotation =
      rightBoard.rotation = source.rotation * Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), Gm_HALF_PI);

      commit(leftBoard);
      commit(rightBoard);
    }
  }
}

internal void rebuildWoodBuildings(GmContext* context) {
  // @todo
}

internal void rebuildTownSigns(GmContext* context) {
  for (auto& sign : objects("town-sign")) {
    auto& s1 = create_object_from("p_town-sign-spinner");
    auto& s2 = create_object_from("p_town-sign-spinner");
    auto& s3 = create_object_from("p_town-sign-spinner");

    s1.position = sign.position + Vec3f(0, sign.scale.y * 0.9f, 0);
    s2.position = sign.position + Vec3f(0, sign.scale.y * 0.7f, 0);
    s3.position = sign.position + Vec3f(0, sign.scale.y * 0.5f, 0);

    s1.scale = s2.scale = s3.scale = sign.scale;

    commit(s1);
    commit(s2);
    commit(s3);
  }
}

void ProceduralMeshes::rebuildProceduralMeshes(GmContext* context) {
  for (auto& asset : GameMeshes::proceduralMeshParts) {
    objects(asset.name).reset();
  }

  rebuildCollectables(context);
  rebuildPlantStrips(context);
  rebuildConcreteStacks(context);
  rebuildMiniHouses(context);
  rebuildWoodBuildings(context);
  rebuildTownSigns(context);
}

void ProceduralMeshes::handleProceduralMeshes(GmContext* context, GameState& state, float dt) {
  START_TIMING("handleProceduralMeshes");

  // @todo handleBalloonWindmills()
  objects("p_balloon-windmill-blades").reset();

  auto t = get_scene_time();

  for (auto& windmill : objects("balloon-windmill")) {
    auto& blades = create_object_from("p_balloon-windmill-blades");

    blades.position = windmill.position;
    blades.scale = windmill.scale;
    blades.rotation = windmill.rotation * Quaternion::fromAxisAngle(Vec3f(0, 0, 1.f), t * 0.5f);
    blades.color = Vec3f(1.f, 0.8f, 0.6f);

    commit(blades);
  }

  LOG_TIME();
}