#include "collisions.h"
#include "macros.h"

#define min(a, b) (a > b ? b : a)
#define max(a, b) (a > b ? a : b)

using namespace Gamma;

internal std::vector<std::vector<Vec3f>> facePlanePoints = {
  // Top
  { Vec3f(-1.f, 1.f, -1.f ), Vec3f(-1.f, 1.f, 1.f), Vec3f(1.f, 1.f, 1.f), Vec3f(1.f, 1.f, -1.f) },
  // Bottom
  { Vec3f(-1.f, -1.f, 1.f), Vec3f(-1.f, -1.f, -1.f), Vec3f(1.f, -1.f, -1.f), Vec3f(1.f, -1.f, 1.f) },
  // Left
  { Vec3f(-1.f, -1.f, -1.f), Vec3f(-1.f, -1.f, 1.f), Vec3f(-1.f, 1.f, 1.f), Vec3f(-1.f, 1.f, -1.f) },
  // Right
  { Vec3f(1.f, -1.f, -1.f), Vec3f(1.f, 1.f, -1.f), Vec3f(1.f, 1.f, 1.f), Vec3f(1.f, -1.f, 1.f) },
  // Front
  { Vec3f(-1.f, -1.f, -1.f), Vec3f(-1.f, 1.f, -1.f), Vec3f(1.f, 1.f, -1.f), Vec3f(1.f, -1.f, -1.f) },
  // Back
  { Vec3f(1.f, -1.f, 1.f), Vec3f(1.f, 1.f, 1.f), Vec3f(-1.f, 1.f, 1.f), Vec3f(-1.f, -1.f, 1.f) }
};

internal bool isInBetween(float n, float a, float b) {
  return n >= min(a, b) && n <= max(a, b);
}

// @todo rename addObjectBoundingBoxCollisionPlanes (or similar)
void Collisions::addObjectCollisionPlanes(const Object& object, std::vector<Plane>& planes, const Vec3f& hitboxScale, const Vec3f& hitboxOffset) {
  Matrix4f rotation = object.rotation.toMatrix4f();
  Vec3f adjustedScale = object.scale * hitboxScale;

  // @todo support side masking
  for (auto& points : facePlanePoints) {
    Plane plane;

    // @todo see comment on this within the Plane struct definition (game.h)
    plane.sourceObjectRecord = object._record;

    // Determine the four points of the plane
    {
      auto p1 = points[0] + hitboxOffset;
      auto p2 = points[1] + hitboxOffset;
      auto p3 = points[2] + hitboxOffset;
      auto p4 = points[3] + hitboxOffset;

      plane.p1 = object.position + (rotation * (adjustedScale * p1)).toVec3f();
      plane.p2 = object.position + (rotation * (adjustedScale * p2)).toVec3f();
      plane.p3 = object.position + (rotation * (adjustedScale * p3)).toVec3f();
      plane.p4 = object.position + (rotation * (adjustedScale * p4)).toVec3f();

      plane.minY = plane.p1.y;
      if (plane.p2.y < plane.minY) plane.minY = plane.p2.y;
      if (plane.p3.y < plane.minY) plane.minY = plane.p3.y;
      if (plane.p4.y < plane.minY) plane.minY = plane.p4.y;

      plane.maxY = plane.p1.y;
      if (plane.p2.y > plane.maxY) plane.maxY = plane.p2.y;
      if (plane.p3.y > plane.maxY) plane.maxY = plane.p3.y;
      if (plane.p4.y > plane.maxY) plane.maxY = plane.p4.y;
    }

    // Precalculate the plane normal/tangents/up factor
    {
      plane.normal = Vec3f::cross(plane.p2 - plane.p1, plane.p3 - plane.p2).unit();

      plane.t1 = Vec3f::cross(plane.normal, plane.p2 - plane.p1);
      plane.t2 = Vec3f::cross(plane.normal, plane.p3 - plane.p2);
      plane.t3 = Vec3f::cross(plane.normal, plane.p4 - plane.p3);
      plane.t4 = Vec3f::cross(plane.normal, plane.p1 - plane.p4);

      plane.nDotU = Vec3f::dot(plane.normal, Vec3f(0, 1.f, 0));
    }

    planes.push_back(plane);
  }
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