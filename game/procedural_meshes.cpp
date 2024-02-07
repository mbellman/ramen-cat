#include "procedural_meshes.h"
#include "macros.h"

using namespace Gamma;

internal float randomFromVec3f(const Vec3f& v) {
  float a = Gm_Modf(v.x + v.y + v.z, 1.f);
  float b = a * 1024.345267f;

  return Gm_Modf(sqrtf(b), 1.f);
}

internal void rebuildConcreteStacks(GmContext* context) {
  objects("p_concrete").reset();

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
          piece.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), yaw) * stack.rotation;

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

  Console::log("Created", objects("p_concrete").totalActive(), "p_concrete meshes");
}

internal void rebuildMiniHouses(GmContext* context) {
  objects("p_mini-house").reset();
  objects("p_mini-house-roof").reset();

  for (auto& source : objects("mini-house")) {
    auto origin = source.position;
    auto random = randomFromVec3f(origin);
    auto random2 = Gm_Modf(random * 2.f, 1.f);

    // Base
    {
      auto& base = create_object_from("p_mini-house");

      base.position = source.position;
      base.scale = source.scale;
      base.rotation = source.rotation;
      base.color = Vec3f(0.5f + 0.5f * random, 0.5f + 0.5f * random2, 0.3f + 0.4f * (1.f - random));

      commit(base);
    }

    // Roof
    {
      auto& roof = create_object_from("p_mini-house-roof");
      auto offset = Vec3f(0, source.scale.y * 1.035f, source.scale.z * 0.5f);

      roof.position = origin + source.rotation.toMatrix4f().transformVec3f(offset);
      roof.scale = Vec3f(source.scale.x, source.scale.x, source.scale.z) * 1.1f;

      roof.rotation =
        source.rotation * (
          Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), Gm_PI) *
          Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), -15.f * (Gm_PI / 180.f))
        );

      roof.color = Vec3f(0.4f, 0.8f, 0.5f);

      commit(roof);
    }
  }
}

void ProceduralMeshes::rebuildProceduralMeshes(GmContext* context) {
  rebuildConcreteStacks(context);
  rebuildMiniHouses(context);
}