#pragma once

#include <string>

#include "system/type_aliases.h"

namespace Gamma {
  class OpenGLTexture {
  public:
    OpenGLTexture(const std::string& path, GLenum unit, bool enableMipmaps = true);
    ~OpenGLTexture();

    void bind();
    const std::string& getPath() const;

  private:
    GLuint id;
    GLenum unit;
    std::string path;

    void initialize(bool enableMipmaps);
  };
}