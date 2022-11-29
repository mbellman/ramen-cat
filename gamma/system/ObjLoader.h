#pragma once

#include <map>
#include <vector>
#include <string>

#include "math/vector.h"
#include "system/AbstractLoader.h"
#include "system/type_aliases.h"

namespace Gamma {
  /**
   * VertexData
   * ----------
   *
   * A small data container for a particular vertex, representing its
   * primary vertex index, texture coordinate index, and vertex normal
   * index among the lists defined in an .obj file.
   */
  struct VertexData {
    u32 vertexIndex;
    u32 textureCoordinateIndex;
    u32 normalIndex;
  };

  /**
   * Face
   * ----
   *
   * Contains vertex data for polygonal faces.
   */
  struct Face {
    VertexData v1;
    VertexData v2;
    VertexData v3;
  };

  /**
   * ObjLoader
   * ---------
   *
   * Opens and parses .obj files into an intermediate representation
   * for conversion into Model instances.
   *
   * Usage:
   *
   *  ObjLoader modelObj("path/to/file.obj");
   */
  class ObjLoader : public AbstractLoader {
  public:
    std::vector<Vec3f> vertices;
    std::vector<Vec2f> textureCoordinates;
    std::vector<Vec3f> normals;
    std::vector<Face> faces;

    ObjLoader(const char* path);
    ~ObjLoader();

  private:
    void handleFace();
    void handleNormal();
    void handleVertex();
    void handleTextureCoordinate();
    VertexData parseVertexData(const std::string& data);
  };
}