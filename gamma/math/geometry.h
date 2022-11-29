#pragma once

#include "math/vector.h"

namespace Gamma {
  struct Vertex {
    Vec3f position;
    Vec3f normal;
    Vec3f tangent;
    Vec2f uv;
  };
}