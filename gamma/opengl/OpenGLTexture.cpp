#include <string>

#include "glew.h"
#include "SDL_image.h"

#include "opengl/OpenGLTexture.h"
#include "system/assert.h"
#include "system/flags.h"

#if GAMMA_DEVELOPER_MODE == 1
  #include "system/console.h"
  #include "system/file.h"
#endif

namespace Gamma {
  // @todo add hot-reloading in dev mode
  OpenGLTexture::OpenGLTexture(const std::string& path, GLenum unit, bool enableMipmaps) {
    this->unit = unit;
    this->path = path;

    initialize(enableMipmaps);

    #if GAMMA_DEVELOPER_MODE == 1
      Gm_WatchFile(path.c_str(), [=]() {
        glDeleteTextures(1, &id);
        initialize(enableMipmaps);

        Console::log("[Gamma] Hot-reloaded texture:", path);
      }); 
    #endif
  }

  OpenGLTexture::~OpenGLTexture() {
    glDeleteTextures(1, &id);
  }

  void OpenGLTexture::bind() {
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_2D, id);
  }

  void OpenGLTexture::initialize(bool enableMipmaps) {
    SDL_Surface* surface = IMG_Load(path.c_str());

    if (surface == 0) {
      Console::warn("[Gamma] Failed to load texture:", path);

      SDL_FreeSurface(surface);

      return;
    }

    GLuint format = surface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;

    glGenTextures(1, &id);

    bind();

    glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);

    if (enableMipmaps) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glGenerateMipmap(GL_TEXTURE_2D);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    SDL_FreeSurface(surface);
  }

  const std::string& OpenGLTexture::getPath() const {
    return path;
  }
}