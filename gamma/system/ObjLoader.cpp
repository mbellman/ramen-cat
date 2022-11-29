#include <map>
#include <string>
#include <iostream>

#include "system/ObjLoader.h"

namespace Gamma {
  static std::string VERTEX_LABEL = "v";
  static std::string TEXTURE_COORDINATE_LABEL = "vt";
  static std::string NORMAL_LABEL = "vn";
  static std::string FACE_LABEL = "f";

  ObjLoader::ObjLoader(const char* path) {
    load(path);

    while (isLoading) {
      setChunkDelimiter(" ");

      const std::string& label = readNextChunk();

      if (label == VERTEX_LABEL) {
        handleVertex();
      } else if (label == TEXTURE_COORDINATE_LABEL) {
        handleTextureCoordinate();
      } else if (label == NORMAL_LABEL) {
        handleNormal();
      } else if (label == FACE_LABEL) {
        handleFace();
      }

      nextLine();
    }
  }

  ObjLoader::~ObjLoader() {
    vertices.clear();
    textureCoordinates.clear();
    normals.clear();
    faces.clear();
  }

  void ObjLoader::handleFace() {
    Face face;

    face.v1 = parseVertexData(readNextChunk());
    face.v2 = parseVertexData(readNextChunk());
    face.v3 = parseVertexData(readNextChunk());

    faces.push_back(face);
  }

  void ObjLoader::handleNormal() {
    float x = stof(readNextChunk());
    float y = stof(readNextChunk());
    float z = stof(readNextChunk());

    normals.push_back({ x, y, z });
  }

  void ObjLoader::handleVertex() {
    float x = stof(readNextChunk());
    float y = stof(readNextChunk());
    float z = stof(readNextChunk());

    vertices.push_back({ x, y, z });
  }

  void ObjLoader::handleTextureCoordinate() {
    float u = stof(readNextChunk());
    float v = stof(readNextChunk());

    textureCoordinates.push_back({ u, 1.f - v });
  }

  /**
   * Attempts to parse the primary vertex index, texture coordinate
   * index, and normal index of a polygonal face. A data chunk can
   * be structured in any of the following ways:
   *
   *   v
   *   v/vt
   *   v/vt/vn
   *   v//vn
   *
   * Where v is the primary index, vt the texture coordinate index,
   * and vn the normal index, with respect to previously listed
   * vertex/texture coordinate/normal values.
   */
  VertexData ObjLoader::parseVertexData(const std::string& chunk) {
    VertexData vertexData;
    int offset = 0;
    int indexes[3];

    for (int i = 0; i < 3; i++) {
      int next = chunk.find("/", offset);
      bool hasNext = next > -1;

      if (next - offset == 0 || offset >= chunk.length()) {
        // If the next '/' is immediately after the last,
        // or we've reached the end of the chunk with
        // cycles to spare, this type of vertex index isn't
        // defined.
        indexes[i] = -1;
      } else {
        // As long as characters are found in between the
        // previous '/' and the next, or we still have extra
        // characters in the chunk, attempt to parse the index.
        int len = hasNext ? next : std::string::npos;

        indexes[i] = stoi(chunk.substr(offset, len)) - 1;
      }

      offset = hasNext ? next + 1 : chunk.length();
    }

    vertexData.vertexIndex = indexes[0];
    vertexData.textureCoordinateIndex = indexes[1];
    vertexData.normalIndex = indexes[2];

    return vertexData;
  }
}