#pragma once

#include "math/matrix.h"
#include "system/packed_data.h"
#include "system/type_aliases.h"

namespace Gamma {
  struct Object;
  struct ObjectRecord;
  struct Camera;

  /**
   * ObjectPool
   * ----------
   *
   * A collection of Objects tied to a given Mesh, designed
   * to facilitate instanced/batched rendering.
   *
   * @todo u16 -> u32 to support >65K objects per pool
   */
  class ObjectPool {
  public:
    Object& operator[](u32 index);

    Object* begin() const;
    Object& createObject();
    Object* end() const;
    void free();
    Object* getById(u16 objectId) const;
    Object* getByRecord(const ObjectRecord& record) const;
    pVec4* getColors() const;
    Matrix4f* getMatrices() const;
    u16 max() const;
    u16 partitionByDistance(u16 start, float distance, const Vec3f& cameraPosition);
    void partitionByVisibility(const Camera& camera);
    void removeById(u16 objectId);
    void reset();
    void reserve(u16 size);
    void setColorById(u16 objectId, const pVec4& color);
    void showAll();
    u16 totalActive() const;
    u16 totalVisible() const;
    void transformById(u16 objectId, const Matrix4f& matrix);

  private:
    Object* objects = nullptr;
    Matrix4f* matrices = nullptr;
    pVec4* colors = nullptr;
    // @todo if we cycle through indices to find unoccupied slots
    // when creating new objects, we needn't preallocate the entire
    // indices array up to USHRT_MAX/0xffff
    u16 indices[0xffff];
    u16 maxObjects = 0;
    u16 totalActiveObjects = 0;
    u16 totalVisibleObjects = 0;
    u16 runningId = 0;

    void swapObjects(u16 indexA, u16 indexB);
  };
}