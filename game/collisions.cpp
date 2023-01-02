#include "collisions.h"
#include "macros.h"

#define min(a, b) (a > b ? b : a)
#define max(a, b) (a > b ? a : b)

using namespace Gamma;

internal bool isInBetween(float n, float a, float b) {
  return n >= min(a, b) && n <= max(a, b);
}

Collision Collisions::getLinePlaneCollision(const Vec3f& lineStart, const Vec3f& lineEnd, const Plane& plane) {
  Collision collision;
  Vec3f line = lineEnd - lineStart;

  if (Vec3f::dot(plane.normal, line) != 0.f) {
    float nDotP = Vec3f::dot(plane.normal, plane.p1);
    float length = (nDotP - Vec3f::dot(plane.normal, lineStart)) / Vec3f::dot(plane.normal, line);
    Vec3f point = lineStart + line * length;

    if (
      // If the point is on the line segment
      isInBetween(point.x, lineStart.x, lineEnd.x) &&
      isInBetween(point.y, lineStart.y, lineEnd.y) &&
      isInBetween(point.z, lineStart.z, lineEnd.z) &&
      // And the point is inside the plane area
      Vec3f::dot(point - plane.p1, plane.t1) >= 0.f &&
      Vec3f::dot(point - plane.p2, plane.t2) >= 0.f &&
      Vec3f::dot(point - plane.p3, plane.t3) >= 0.f &&
      Vec3f::dot(point - plane.p4, plane.t4) >= 0.f
    ) {
      collision.plane = plane;
      collision.point = point;
      collision.hit = true;
    }
  }

  return collision;
}