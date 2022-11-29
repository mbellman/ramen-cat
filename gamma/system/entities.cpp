#include <cstdio>
#include <cstdlib>
#include <map>
#include <utility>

#include "math/vector.h"
#include "system/assert.h"
#include "system/entities.h"
#include "system/ObjLoader.h"

namespace Gamma {
  /**
   * Defines positions for each corner of the unit cube
   */
  const static Vec3f cubeCornerPositions[8] = {
    { -1.0f, -1.0f, -1.0f },  // rear side, bottom left
    { 1.0f, -1.0f, -1.0f },   // rear side, bottom right
    { 1.0f, 1.0f, -1.0f },    // rear side, top right
    { -1.0f, 1.0f, -1.0f },   // rear side, top left
    { -1.0f, -1.0f, 1.0f },   // far side, bottom left
    { 1.0f, -1.0f, 1.0f },    // far side, bottom right
    { 1.0f, 1.0f, 1.0f },     // far side, top right
    { -1.0f, 1.0f, 1.0f }     // far side, top left
  };

  /**
   * Defines UV coordinates for each cube side
   */
  const static Vec2f cubeUvs[4] = {
    { 1.0f, 1.0f },           // bottom right
    { 0.0f, 1.0f },           // bottom left
    { 0.0f, 0.0f },           // top left
    { 1.0f, 0.0f }            // top right
  };

  /**
   * Maps the corners of each cube side to corner position indexes
   */
  const static int cubeFaces[6][4] = {
    { 1, 0, 3, 2 },           // back
    { 7, 6, 2, 3 },           // top
    { 4, 5, 6, 7 },           // front
    { 0, 1, 5, 4 },           // bottom
    { 0, 4, 7, 3 },           // left
    { 5, 1, 2, 6 }            // right
  };

  /**
   * Gm_ComputeNormals
   * -----------------
   */
  static void Gm_ComputeNormals(Mesh* mesh) {
    auto& vertices = mesh->vertices;
    auto& faceElements = mesh->faceElements;

    for (auto& vertex : vertices) {
      vertex.normal = Vec3f(0.f);
    }

    for (u32 i = 0; i < faceElements.size(); i += 3) {
      Vertex& v1 = vertices[faceElements[i]];
      Vertex& v2 = vertices[faceElements[i + 1]];
      Vertex& v3 = vertices[faceElements[i + 2]];

      Vec3f normal = Vec3f::cross(v2.position - v1.position, v3.position - v1.position).unit();

      v1.normal += normal;
      v2.normal += normal;
      v3.normal += normal;
    }
  }

  /**
   * Gm_ComputeTangents
   * ------------------
   */
  static void Gm_ComputeTangents(Mesh* mesh) {
    auto& vertices = mesh->vertices;
    auto& faceElements = mesh->faceElements;

    for (u32 i = 0; i < faceElements.size(); i += 3) {
      Vertex& v1 = vertices[faceElements[i]];
      Vertex& v2 = vertices[faceElements[i + 1]];
      Vertex& v3 = vertices[faceElements[i + 2]];

      Vec3f e1 = v2.position - v1.position;
      Vec3f e2 = v3.position - v1.position;

      float deltaU1 = v2.uv.x - v1.uv.x;
      float deltaV1 = v2.uv.y - v1.uv.y;
      float deltaU2 = v3.uv.x - v1.uv.x;
      float deltaV2 = v3.uv.y - v1.uv.y;

      float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

      Vec3f tangent = {
        f * (deltaV2 * e1.x - deltaV1 * e2.x),
        f * (deltaV2 * e1.y - deltaV1 * e2.y),
        f * (deltaV2 * e1.z - deltaV1 * e2.z)
      };

      v1.tangent += tangent;
      v2.tangent += tangent;
      v3.tangent += tangent;
    }
  }

  /**
   * Gm_BufferObjData
   * ----------------
   *
   * Streams vertex and face element data from an ObjLoader,
   * defined in a preliminary state, into vertex/face element
   * buffers defined on Meshes or other global buffers.
   *
   * @todo we may not want to add the base vertex offset here;
   * once this is used to pack multiple (distinct, not merely LOD)
   * meshes into a common vertex/element buffer, it may be preferable
   * to associate a baseVertex/firstIndex with each mesh. firstIndex
   * alone is technically feasible though. reconsider when revisiting
   * this for glMultiDrawElementsIndirect().
   */
  static void Gm_BufferObjData(const ObjLoader& obj, std::vector<Vertex>& vertices, std::vector<u32>& faceElements) {
    u32 baseVertex = vertices.size();

    if (obj.textureCoordinates.size() == 0 && obj.normals.size() == 0) {
      // Only vertex positions defined, so simply load in vertices,
      // and then load in face element indexes
      for (u32 i = 0; i < obj.vertices.size(); i++) {
        Vertex vertex;
        vertex.position = obj.vertices[i];

        vertices.push_back(vertex);
      }

      for (u32 i = 0; i < obj.faces.size(); i++) {
        faceElements.push_back(baseVertex + obj.faces[i].v1.vertexIndex);
        faceElements.push_back(baseVertex + obj.faces[i].v2.vertexIndex);
        faceElements.push_back(baseVertex + obj.faces[i].v3.vertexIndex);
      }
    } else {
      // Texture coordinates and/or normals defined, so we need
      // to create a unique vertex for each position/uv/normal
      // tuple, and add face elements based on created vertices
      typedef std::tuple<u32, u32, u32> VertexTuple;

      std::map<VertexTuple, u32> vertexTupleToIndexMap;

      for (const auto& face : obj.faces) {
        VertexTuple vertexTuples[3] = {
          { face.v1.vertexIndex, face.v1.textureCoordinateIndex, face.v1.normalIndex },
          { face.v2.vertexIndex, face.v2.textureCoordinateIndex, face.v2.normalIndex },
          { face.v3.vertexIndex, face.v3.textureCoordinateIndex, face.v3.normalIndex }
        };

        // Add face elements, creating vertices if necessary
        for (u32 p = 0; p < 3; p++) {
          auto& vertexTuple = vertexTuples[p];
          auto indexRecord = vertexTupleToIndexMap.find(vertexTuple);

          if (indexRecord != vertexTupleToIndexMap.end()) {
            // Vertex tuple already exists, so we can just
            // add the stored face element index
            faceElements.push_back(indexRecord->second);
          } else {
            // Vertex doesn't exist, so we need to create it
            Vertex vertex;
            u32 index = vertices.size();

            // @todo Have the option to run through existing vertices,
            // compare position, and re-use any which are within a minuscule
            // distance threshold of this one to avoid duplicates. Certain
            // obj files may do this on purpose, so it should be opt-in.
            vertex.position = obj.vertices[std::get<0>(vertexTuple)];

            if (obj.textureCoordinates.size() > 0) {
              vertex.uv = obj.textureCoordinates[std::get<1>(vertexTuple)];
            }

            if (obj.normals.size() > 0) {
              vertex.normal = obj.normals[std::get<2>(vertexTuple)];
            }

            vertices.push_back(vertex);
            faceElements.push_back(index);

            vertexTupleToIndexMap.emplace(vertexTuple, index);
          }
        }
      }
    }
  }

  /**
   * Mesh::Cube()
   * ------------
   * 
   * Constructs a cube Mesh using predefined vertex data.
   * None of a Cube's vertices are shared between its sides,
   * ensuring that normals remain constant along them.
   */
  Mesh* Mesh::Cube() {
    auto* mesh = new Mesh();
    auto& vertices = mesh->vertices;
    auto& faceElements = mesh->faceElements;

    vertices.resize(24);
    faceElements.resize(36);

    // For each cube side
    for (u8 i = 0; i < 6; i++) {
      auto& face = cubeFaces[i];
      u32 f_offset = i * 6;
      u32 v_offset = i * 4;

      // Define vertex indexes for the two triangle faces on each cube side
      faceElements[f_offset] = v_offset;
      faceElements[f_offset + 1] = v_offset + 1;
      faceElements[f_offset + 2] = v_offset + 2;

      faceElements[f_offset + 3] = v_offset;
      faceElements[f_offset + 4] = v_offset + 2;
      faceElements[f_offset + 5] = v_offset + 3;

      // For each corner on this side
      for (u8 j = 0; j < 4; j++) {
        auto& vertex = vertices[v_offset++];

        // Define the corner vertex position/uvs
        vertex.position = cubeCornerPositions[face[j]];
        vertex.uv = cubeUvs[j];
      }
    }

    Gm_ComputeNormals(mesh);
    Gm_ComputeTangents(mesh);

    return mesh;
  }

  /**
   * Mesh::Model()
   * -------------
   *
   * Loads an .obj model file into a Mesh.
   */
  Mesh* Mesh::Model(const char* path) {
    ObjLoader obj(path);

    auto* mesh = new Mesh();

    Gm_BufferObjData(obj, mesh->vertices, mesh->faceElements);

    if (obj.normals.size() == 0) {
      Gm_ComputeNormals(mesh);
    }

    Gm_ComputeTangents(mesh);

    return mesh;
  }

  /**
   * Mesh::Model()
   * -------------
   *
   * Loads a sequence of .obj model files into a Mesh,
   * treating each consecutive model as a lower level
   * of detail.
   */
  Mesh* Mesh::Model(const std::vector<std::string>& paths) {
    if (paths.size() == 1) {
      return Mesh::Model(paths[0].c_str());
    }

    auto* mesh = new Mesh();

    mesh->lods.resize(paths.size());

    for (u32 i = 0; i < paths.size(); i++) {
      const char* path = paths[i].c_str();

      ObjLoader obj(path);

      mesh->lods[i].elementOffset = mesh->faceElements.size();
      mesh->lods[i].vertexOffset = mesh->vertices.size();

      Gm_BufferObjData(obj, mesh->vertices, mesh->faceElements);

      mesh->lods[i].elementCount = mesh->faceElements.size() - mesh->lods[i].elementOffset;
      mesh->lods[i].vertexCount = mesh->vertices.size() - mesh->lods[i].vertexOffset;
    }

    Gm_ComputeNormals(mesh);
    Gm_ComputeTangents(mesh);

    return mesh;
  }

  /**
   * Mesh::Particles()
   * -----------------
   *
   * @todo description
   */
  Mesh* Mesh::Particles() {
    auto* mesh = new Mesh();

    mesh->type = MeshType::PARTICLE_SYSTEM;

    mesh->vertices.push_back(Vertex());

    return mesh;
  }

  /**
   * Mesh::Plane()
   * -------------
   *
   * @todo description
   * @todo make back face optional
   */
  Mesh* Mesh::Plane(u32 size, bool useLoopingTexture) {
    auto* mesh = new Mesh();
    auto& vertices = mesh->vertices;
    auto& faceElements = mesh->faceElements;

    // Front + back face vertices
    for (u8 i = 0; i < 2; i++) {
      // @bug size should represent the total number of
      // tiles across the plane, not the total vertices
      for (u32 x = 0; x < size; x++) {
        for (u32 z = 0; z < size; z++) {
          Vertex vertex;

          float xr = (float)x / (float)(size - 1);
          float zr = (float)z / (float)(size - 1);

          vertex.position = Vec3f(xr - 0.5f, 0.f, -zr + 0.5f) * 2.f;

          if (useLoopingTexture) {
            vertex.uv = Vec2f((float)x, (float)z);
          } else {
            vertex.uv = Vec2f(xr, 1.f - zr);
          }

          vertices.push_back(vertex);
        }
      }
    }

    // Front face
    for (u32 z = 0; z < size - 1; z++) {
      for (u32 x = 0; x < size - 1; x++) {
        u32 offset = z * size + x;

        faceElements.push_back(offset);
        faceElements.push_back(offset + 1 + size);
        faceElements.push_back(offset + 1);

        faceElements.push_back(offset);
        faceElements.push_back(offset + size);
        faceElements.push_back(offset + 1 + size);
      }
    }

    // Back face
    auto startingOffset = size * size;

    for (u32 z = 0; z < size - 1; z++) {
      for (u32 x = 0; x < size - 1; x++) {
        u32 offset = startingOffset + z * size + x;

        faceElements.push_back(offset);
        faceElements.push_back(offset + 1);
        faceElements.push_back(offset + 1 + size);

        faceElements.push_back(offset);
        faceElements.push_back(offset + 1 + size);
        faceElements.push_back(offset + size);
      }
    }

    Gm_ComputeNormals(mesh);
    Gm_ComputeTangents(mesh);

    return mesh;
  }

  /**
   * Mesh::transformGeometry()
   * -------------------------
   *
   * @todo description
   */
  void Mesh::transformGeometry(std::function<void(const Vertex&, Vertex&)> handler) {
    // Copy vertices the first time
    if (transformedVertices.size() == 0) {
      for (auto& vertex : vertices) {
        transformedVertices.push_back(vertex);
      }
    }

    for (u32 i = 0; i < vertices.size(); i++) {
      handler(vertices[i], transformedVertices[i]);
    }
  }

  /**
   * Gm_FreeMesh
   * -----------
   */
  void Gm_FreeMesh(Mesh* mesh) {
    mesh->vertices.clear();
    mesh->faceElements.clear();
    mesh->objects.free();
  }
}