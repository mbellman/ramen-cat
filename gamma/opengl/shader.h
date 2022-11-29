#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "math/matrix.h"
#include "math/vector.h"
#include "system/traits.h"
#include "system/type_aliases.h"

namespace Gamma {
  struct GLShaderRecord {
    GLuint shader;
    GLenum shaderType;
    std::string path;
    std::filesystem::file_time_type lastWriteTime;
  };

  class OpenGLShader : public Initable, public Destroyable {
  public:
    virtual void init() override;
    virtual void destroy() override;
    void attachShader(const GLShaderRecord& record);
    void define(const std::string& name, const std::string& value);
    void define(const std::map<std::string, std::string>& variables);
    void fragment(const char* path);
    void geometry(const char* path);
    void link();
    void setBool(std::string name, bool value) const;
    void setFloat(std::string name, float value) const;
    void setInt(std::string name, int value) const;
    void setMatrix4f(std::string name, const Matrix4f& value) const;
    void setVec2f(std::string name, const Vec2f& value) const;
    void setVec3f(std::string name, const Vec3f& value) const;
    void setVec4f(std::string name, const Vec4f& value) const;
    void use();
    void vertex(const char* path);

  private:
    GLuint program = -1;
    u32 lastShaderFileCheckTime = 0;
    std::vector<GLShaderRecord> glShaderRecords;
    std::map<std::string, std::string> defineVariables;

    void checkAndHotReloadShaders();
    GLint getUniformLocation(const char* name) const;
    GLint getUniformLocation(std::string name) const;
  };
}