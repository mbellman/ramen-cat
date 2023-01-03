#pragma once

#include "Gamma.h"

#include "game.h"
#include "macros.h"

struct Collision {
  Plane plane;
  Gamma::Vec3f point;
  bool hit = false;
};

namespace Collisions {
  void addObjectCollisionPlanes(const Gamma::Object& object, std::vector<Plane>& planes);
  Collision getLinePlaneCollision(const Gamma::Vec3f& lineStart, const Gamma::Vec3f& lineEnd, const Plane& plane);
}