#include "system/assert.h"
#include "system/camera.h"
#include "system/lights_objects_meshes.h"
#include "system/ObjectPool.h"

#define UNUSED_OBJECT_INDEX 0xffff

namespace Gamma {
  /**
   * ObjectPool
   * ----------
   */
  Object& ObjectPool::operator[](u32 index) {
    return objects[index];
  }

  Object* ObjectPool::begin() const {
    return objects;
  }

  Object& ObjectPool::createObject() {
    u16 id = runningId++;

    assert(max() > totalActive(), "Object Pool out of space: " + std::to_string(max()) + " objects allowed in this pool");
    // @todo cycle through indices until an unoccupied slot is found
    assert(indices[id] == UNUSED_OBJECT_INDEX, "Attempted to create an Object in an occupied slot");

    if (runningId > highestId) {
      highestId = runningId;
    }

    // Retrieve and initialize object
    u16 index = totalActiveObjects;
    Object& object = objects[index];

    object._record.id = id;
    object._record.generation++;

    // Reset object matrix/color
    matrices[index] = Matrix4f::identity();
    colors[index] = pVec4(255, 255, 255);

    // Enable object lookup by ID -> index
    indices[id] = index;

    totalActiveObjects++;
    totalVisibleObjects++;

    changed = true;

    return object;
  }

  Object* ObjectPool::end() const {
    return &objects[totalActiveObjects];
  }

  void ObjectPool::free() {
    for (u16 i = 0; i < USHRT_MAX; i++) {
      indices[i] = UNUSED_OBJECT_INDEX;
    }

    if (objects != nullptr) {
      delete[] objects;
    }

    if (matrices != nullptr) {
      delete[] matrices;
    }

    if (colors != nullptr) {
      delete[] colors;
    }

    objects = nullptr;
    matrices = nullptr;
    colors = nullptr;
    changed = true;
  }

  Object* ObjectPool::getById(u16 objectId) const {
    u16 index = indices[objectId];

    return index == UNUSED_OBJECT_INDEX ? nullptr : &objects[index];
  }

  Object* ObjectPool::getByRecord(const ObjectRecord& record) const {
    auto* object = getById(record.id);

    if (object == nullptr || object->_record.generation != record.generation) {
      return nullptr;
    }

    return object;
  }

  pVec4* ObjectPool::getColors() const {
    return colors;
  }

  u16 ObjectPool::getHighestId() const {
    return highestId;
  }

  Matrix4f* ObjectPool::getMatrices() const {
    return matrices;
  }

  u16 ObjectPool::max() const {
    return maxObjects;
  }

  u16 ObjectPool::partitionByDistance(u16 start, float distance, const Vec3f& cameraPosition, bool checkAllObjects) {
    u16 current = start;
    u16 end = checkAllObjects ? totalActive() : totalVisible();

    while (end > current) {
      float currentObjectDistance = (objects[current].position - cameraPosition).magnitude();

      if (currentObjectDistance <= distance) {
        current++;
      } else {
        float endObjectDistance;

        do {
          endObjectDistance = (objects[--end].position - cameraPosition).magnitude();
        } while (endObjectDistance > distance && end > current);

        if (current != end) {
          swapObjects(current, end);
        }

        // @optimize we can increment 'current' here if endObjectDistance < distance
        // to avoid recalculating the same distance on the next while loop cycle
        // @todo test this out and make sure it works
      }
    }

    changed = true;

    return current;
  }

  void ObjectPool::partitionByVisibility(const Camera& camera, float distanceThreshold, float fovDivisor) {
    const float visibilityDotThreshold = 1.f - camera.fov / fovDivisor;

    u16 current = 0;
    u16 end = totalActive();
    Vec3f cameraDirection = camera.orientation.getDirection();

    while (end > current) {
      Vec3f cameraToObject = objects[current].position - camera.position;
      float distance = cameraToObject.magnitude();
      Vec3f unitCameraToObject = cameraToObject / distance;

      if (Vec3f::dot(cameraDirection, unitCameraToObject) >= visibilityDotThreshold || distance < distanceThreshold) {
        current++;
      } else {
        float endDistance = 0.f;
        Vec3f endUnitCameraToObject;

        do {
          Vec3f endCameraToObject = (objects[--end].position - camera.position);
          endDistance = endCameraToObject.magnitude();
          endUnitCameraToObject = endCameraToObject / endDistance;
        } while (
          Vec3f::dot(cameraDirection, endUnitCameraToObject) < visibilityDotThreshold &&
          endDistance > distanceThreshold &&
          end > current
        );

        if (current != end) {
          swapObjects(current, end);
        }
      }
    }

    changed = true;

    totalVisibleObjects = current;
  }

  void ObjectPool::removeById(u16 objectId) {
    u16 index = indices[objectId];

    if (index == UNUSED_OBJECT_INDEX) {
      return;
    }

    totalActiveObjects--;
    totalVisibleObjects--;

    u16 lastIndex = totalActiveObjects;

    // Move last object/matrix/color into removed index
    objects[index] = objects[lastIndex];
    matrices[index] = matrices[lastIndex];
    colors[index] = colors[lastIndex];

    // Update ID -> index lookup table
    indices[objects[index]._record.id] = index;
    indices[objectId] = UNUSED_OBJECT_INDEX;

    changed = true;
  }

  void ObjectPool::reset() {
    for (u16 i = 0; i < totalActiveObjects; i++) {
      indices[objects[i]._record.id] = UNUSED_OBJECT_INDEX;
    }

    totalActiveObjects = 0;
    totalVisibleObjects = 0;
    runningId = 0;
    changed = true;
  }

  void ObjectPool::reserve(u16 size) {
    free();

    maxObjects = size;
    totalActiveObjects = 0;
    totalVisibleObjects = 0;
    objects = new Object[size];
    matrices = new Matrix4f[size];
    colors = new pVec4[size];
    changed = true;
  }

  void ObjectPool::showAll() {
    totalVisibleObjects = totalActiveObjects;
    changed = true;
  }

  void ObjectPool::swapObjects(u16 indexA, u16 indexB) {
    Object objectA = objects[indexA];
    Matrix4f matrixA = matrices[indexA];
    pVec4 colorA = colors[indexA];

    objects[indexA] = objects[indexB];
    matrices[indexA] = matrices[indexB];
    colors[indexA] = colors[indexB];

    objects[indexB] = objectA;
    matrices[indexB] = matrixA;
    colors[indexB] = colorA;

    indices[objects[indexA]._record.id] = indexA;
    indices[objects[indexB]._record.id] = indexB;
  }

  void ObjectPool::setColorById(u16 objectId, const pVec4& color) {
    colors[indices[objectId]] = color;
    changed = true;
  }

  void ObjectPool::setTotalVisible(u16 total) {
    totalVisibleObjects = total;
  }

  u16 ObjectPool::totalActive() const {
    return totalActiveObjects;
  }

  u16 ObjectPool::totalVisible() const {
    return totalVisibleObjects;
  }

  void ObjectPool::transformById(u16 objectId, const Matrix4f& matrix) {
    matrices[indices[objectId]] = matrix;
    changed = true;
  }
}