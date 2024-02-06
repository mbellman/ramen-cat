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

  for (auto& stack : objects("concrete-stack")) {
    float width = stack.scale.x;
    float height = stack.scale.y;
    float depth = stack.scale.z;

    auto totalX = (u8)std::ceilf(width / 750.f);
    auto totalY = (u8)std::ceilf(height / 750.f);
    auto totalZ = (u8)std::ceilf(depth / 750.f);

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

          float random = randomFromVec3f(piece.position);
          float yaw = (random * 10.f - 5.f) * (Gm_PI / 180.f);

          piece.scale = scale;
          piece.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), yaw) * stack.rotation;

          if (y == 0 || y == totalY - 1) {
            piece.scale.y += random * 5.f;
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

void ProceduralMeshes::rebuildProceduralMeshes(GmContext* context) {
  rebuildConcreteStacks(context);
}