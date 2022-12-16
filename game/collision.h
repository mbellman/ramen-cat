#pragma once

#include "Gamma.h"

#include "game.h"
#include "macros.h"

struct Collision {
  Plane plane;
  Gamma::Vec3f point;
  bool hit = false;
};

Collision getLinePlaneCollision(const Gamma::Vec3f& lineStart, const Gamma::Vec3f& lineEnd, const Plane& plane);