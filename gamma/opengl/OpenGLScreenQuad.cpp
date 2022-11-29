#include "opengl/OpenGLScreenQuad.h"
#include "glew.h"

namespace Gamma {
  const static float glQuadData[] = {
    -1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 0.0f
  };

  enum GLAttribute {
    QUAD_POSITION,
    QUAD_UV
  };

  /**
   * OpenGLScreenQuad
   * ----------------
   */
  OpenGLScreenQuad::OpenGLScreenQuad() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

    // Buffer quad data
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), glQuadData, GL_STATIC_DRAW);

    // Define vertex attributes
    glEnableVertexAttribArray(GLAttribute::QUAD_POSITION);
    glVertexAttribPointer(GLAttribute::QUAD_POSITION, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(GLAttribute::QUAD_UV);
    glVertexAttribPointer(GLAttribute::QUAD_UV, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  }

  OpenGLScreenQuad::~OpenGLScreenQuad() {
    // @todo
  }

  void OpenGLScreenQuad::draw() {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  void OpenGLScreenQuad::render() {
    if (OpenGLScreenQuad::singleton == nullptr) {
      OpenGLScreenQuad::singleton = new OpenGLScreenQuad();
    }

    OpenGLScreenQuad::singleton->draw();
  }

  OpenGLScreenQuad* OpenGLScreenQuad::singleton = nullptr;
}