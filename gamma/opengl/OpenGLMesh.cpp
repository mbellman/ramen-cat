#include "opengl/errors.h"
#include "opengl/indirect_buffer.h"
#include "opengl/OpenGLMesh.h"
#include "system/console.h"
#include "system/flags.h"

#include "glew.h"

namespace Gamma {
  const enum GLBuffer {
    VERTEX,
    COLOR,
    MATRIX
  };

  const enum GLAttribute {
    VERTEX_POSITION,
    VERTEX_NORMAL,
    VERTEX_TANGENT,
    VERTEX_UV,
    MODEL_COLOR,
    MODEL_MATRIX
  };

  OpenGLMesh::OpenGLMesh(const Mesh* mesh) {
    sourceMesh = mesh;

    glGenVertexArrays(1, &vao);
    glGenBuffers(3, &buffers[0]);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);

    auto& vertices = mesh->vertices;
    auto& faceElements = mesh->faceElements;

    // Buffer vertex data
    glBindBuffer(GL_ARRAY_BUFFER, buffers[GLBuffer::VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    
    // Buffer vertex element data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceElements.size() * sizeof(u32), faceElements.data(), GL_STATIC_DRAW);

    // Define vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, buffers[GLBuffer::VERTEX]);

    glEnableVertexAttribArray(GLAttribute::VERTEX_POSITION);
    glVertexAttribPointer(GLAttribute::VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(GLAttribute::VERTEX_NORMAL);
    glVertexAttribPointer(GLAttribute::VERTEX_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(GLAttribute::VERTEX_TANGENT);
    glVertexAttribPointer(GLAttribute::VERTEX_TANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

    glEnableVertexAttribArray(GLAttribute::VERTEX_UV);
    glVertexAttribPointer(GLAttribute::VERTEX_UV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    // Define color attributes
    glBindBuffer(GL_ARRAY_BUFFER, buffers[GLBuffer::COLOR]);
    glEnableVertexAttribArray(GLAttribute::MODEL_COLOR);
    glVertexAttribIPointer(GLAttribute::MODEL_COLOR, 1, GL_UNSIGNED_INT, sizeof(pVec4), (void*)0);
    glVertexAttribDivisor(GLAttribute::MODEL_COLOR, 1);

    // Define matrix attributes
    glBindBuffer(GL_ARRAY_BUFFER, buffers[GLBuffer::MATRIX]);

    for (u32 i = 0; i < 4; i++) {
      glEnableVertexAttribArray(GLAttribute::MODEL_MATRIX + i);
      glVertexAttribPointer(GLAttribute::MODEL_MATRIX + i, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4f), (void*)(i * 4 * sizeof(float)));
      glVertexAttribDivisor(GLAttribute::MODEL_MATRIX + i, 1);
    }
  }

  OpenGLMesh::~OpenGLMesh() {
    // @todo
  }

  void OpenGLMesh::checkAndLoadTexture(const std::string& path, OpenGLTexture*& texture, GLenum unit) {
    #if GAMMA_DEVELOPER_MODE
      if (texture != nullptr && texture->getPath() != path) {
        Console::log("[Gamma] Destroying OpenGLTexture:", texture->getPath());

        delete texture;

        texture = nullptr;
      }
    #endif

    if (path.size() > 0 && texture == nullptr) {
      // @todo use a texture factory/cache
      texture = new OpenGLTexture(path.c_str(), unit);

      #if GAMMA_DEVELOPER_MODE
        Console::log("[Gamma] OpenGLTexture created:", path);
      #endif
    }

    if (texture != nullptr) {
      texture->bind();
    }
  }

  u16 OpenGLMesh::getId() const {
    return sourceMesh->id;
  }

  u16 OpenGLMesh::getObjectCount() const {
    return sourceMesh->objects.totalActive();
  }

  const Mesh* OpenGLMesh::getSourceMesh() const {
    return sourceMesh;
  }

  bool OpenGLMesh::hasNormalMap() const {
    return glNormalMap != nullptr;
  }

  bool OpenGLMesh::hasTexture() const {
    return glTexture != nullptr;
  }

  bool OpenGLMesh::isMeshType(MeshType type) const {
    return sourceMesh->type == type;
  }

  // @todo provide a parameter to render total visible vs. total active
  void OpenGLMesh::render(GLenum primitiveMode, bool useLowestLevelOfDetail) {
    auto& mesh = *sourceMesh;

    if (mesh.objects.totalVisible() == 0 || mesh.disabled) {
      return;
    }

    if (mesh.type != MeshType::REFRACTIVE) {
      // Don't bind textures for refractive objects, since in
      // the refractive geometry frag shader we need to read
      // from the G-Buffer color texture.
      //
      // @todo if we use texture units which won't conflict with
      // the G-Buffer, we can have textured refractive objects.
      checkAndLoadTexture(mesh.texture, glTexture, GL_TEXTURE0);
      checkAndLoadTexture(mesh.normalMap, glNormalMap, GL_TEXTURE1);
      checkAndLoadTexture(mesh.specularityMap, glSpecularityMap, GL_TEXTURE2);
    }

    if (sourceMesh->transformedVertices.size() > 0) {
      // Re-buffer geometry
      auto& transformedVertices = sourceMesh->transformedVertices;

      glBindBuffer(GL_ARRAY_BUFFER, buffers[GLBuffer::VERTEX]);
      // @todo glMapBuffer (?)
      glBufferData(GL_ARRAY_BUFFER, transformedVertices.size() * sizeof(Vertex), transformedVertices.data(), GL_DYNAMIC_DRAW);
    }

    if (!hasCreatedInstanceBuffers || mesh.type != MeshType::PARTICLE_SYSTEM) {
      // Buffer instance colors/matrices
      glBindBuffer(GL_ARRAY_BUFFER, buffers[GLBuffer::COLOR]);
      glBufferData(GL_ARRAY_BUFFER, mesh.objects.totalVisible() * sizeof(pVec4), mesh.objects.getColors(), GL_DYNAMIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, buffers[GLBuffer::MATRIX]);
      glBufferData(GL_ARRAY_BUFFER, mesh.objects.totalVisible() * sizeof(Matrix4f), mesh.objects.getMatrices(), GL_DYNAMIC_DRAW);

      hasCreatedInstanceBuffers = true;
    }

    // Bind VAO/EBO and draw instances
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    if (mesh.lods.size() > 0) {
      if (useLowestLevelOfDetail) {
        // Render all instances using the last LOD
        auto& lod = mesh.lods.back();

        glDrawElementsInstanced(primitiveMode, lod.elementCount, GL_UNSIGNED_INT, (void*)(lod.elementOffset * sizeof(u32)), mesh.objects.totalVisible());
      } else {
        // Generate draw commands for mesh instances at each
        // level of detail, and dispatch them all together
        //
        // @todo preallocate draw commands array
        auto* commands = new GlDrawElementsIndirectCommand[mesh.lods.size()];

        for (u32 i = 0; i < mesh.lods.size(); i++) {
          auto& command = commands[i];
          auto& lod = mesh.lods[i];

          command.count = lod.elementCount;
          command.firstIndex = lod.elementOffset;
          command.instanceCount = lod.instanceCount;
          command.baseInstance = lod.instanceOffset;
          // @todo base vertexes are already added to elements;
          // this may need to change if we revise the way mesh
          // data is stored/use glMultiDraw more broadly
          command.baseVertex = 0;
        }

        Gm_BufferDrawElementsIndirectCommands(commands, 2);

        glMultiDrawElementsIndirect(primitiveMode, GL_UNSIGNED_INT, 0, 2, 0);

        delete[] commands;
      }
    } else if (mesh.type == MeshType::PARTICLE_SYSTEM) {
      // @todo description
      glBindBuffer(GL_ARRAY_BUFFER, buffers[GLBuffer::VERTEX]);

      glDrawArraysInstanced(GL_POINTS, 0, 1, mesh.objects.totalVisible());
    } else {
      // No distinct level of detail meshes defined;
      // draw all mesh instances together
      glDrawElementsInstanced(primitiveMode, mesh.faceElements.size(), GL_UNSIGNED_INT, (void*)0, mesh.objects.totalVisible());
    }
  }
}