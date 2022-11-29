#include <map>
#include <string>

#include "glew.h"
#include "opengl/errors.h"
#include "system/console.h"

namespace Gamma {
  static std::map<GLenum, std::string> glErrorMap = {
    { GL_INVALID_ENUM, "GL_INVALID_ENUM "},
    { GL_INVALID_VALUE, "GL_INVALID_VALUE" },
    { GL_INVALID_OPERATION, "GL_INVALID_OPERATION" },
    { GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW" },
    { GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW" },
    { GL_OUT_OF_MEMORY, "GL_OUT_OF_MEMORY" },
    { GL_INVALID_FRAMEBUFFER_OPERATION, "GL_INVALID_FRAMEBUFFER_OPERATION" },
    { GL_CONTEXT_LOST, "GL_CONTEXT_LOST" },
    { GL_TABLE_TOO_LARGE, "GL_TABLE_TOO_LARGE" }
  };

  /**
   * Gm_CheckOpenGLErrors
   * --------------------
   */
  void Gm_CheckOpenGLErrors(const char* message) {
    GLenum error;

    while ((error = glGetError()) != GL_NO_ERROR) {
      Console::log("[Gamma] OpenGL Error:", message, glErrorMap[error]);
    }
  }
}