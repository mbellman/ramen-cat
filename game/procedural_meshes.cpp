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
      auto random2 = Gm_Modf(random * 2.f, 1.f);

      base.position = source.position;
      base.scale = source.scale;
      base.rotation = source.rotation;
      base.color = Vec3f(0.5f + 0.5f * random, 0.5f + 0.5f * random2, 0.3f + 0.4f * (1.f - random));

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

void ProceduralMeshes::rebuildProceduralMeshes(GmContext* context) {
  for (auto& asset : GameMeshes::proceduralMeshParts) {
    objects(asset.name).reset();
  }

  rebuildPlantStrips(context);
  rebuildConcreteStacks(context);
  rebuildMiniHouses(context);
  rebuildWoodBuildings(context);
}