#include <algorithm>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "opengl/shader.h"
#include "system/console.h"
#include "system/file.h"
#include "system/flags.h"
#include "system/vector_helpers.h"

#include "glew.h"
#include "SDL.h"

struct FileRecord {
  std::string path;
  std::filesystem::file_time_type lastWriteTime;
};

static std::vector<FileRecord> shaderSourceFileRecords;
static std::vector<std::string> changedSourceFilePaths;
static std::vector<Gamma::OpenGLShader*> glShaderPrograms;

static void Gm_SaveShaderSourceFileRecord(const char* path) {
  for (auto& record : shaderSourceFileRecords) {
    if (record.path.compare(path) == 0) {
      return;
    }
  }

  FileRecord record;

  record.path = path;
  record.lastWriteTime = std::filesystem::last_write_time(std::filesystem::current_path() / path);

  shaderSourceFileRecords.push_back(record);
}

void Gm_CheckAndHotReloadShaders() {
  for (auto& record : shaderSourceFileRecords) {
    auto fullPath = std::filesystem::current_path() / record.path;
    auto lastWriteTime = std::filesystem::last_write_time(fullPath);

    if (lastWriteTime != record.lastWriteTime) {
      changedSourceFilePaths.push_back(record.path);

      record.lastWriteTime = lastWriteTime;
    }
  }

  for (auto& program : glShaderPrograms) {
    program->checkAndHotReloadShaders();
  }

  changedSourceFilePaths.clear();
}

namespace Gamma {
  const static std::string INCLUDE_START = "#include \"";
  const static std::string INCLUDE_END = "\";";
  const static std::string INCLUDE_ROOT_PATH = "./gamma/opengl/shaders/";
  const static std::map<std::string, std::string> emptyMap;

  /**
   * Gm_CompileShader
   * ----------------
   */
  static GLShaderRecord Gm_CompileShader(GLenum shaderType, const char* path, const std::map<std::string, std::string>& defineOverrides = emptyMap) {
    GLuint shader = glCreateShader(shaderType);

    std::string source = Gm_LoadFileContents(path);
    std::vector<std::string> includes;
    u32 currentInclude;

    Gm_SaveShaderSourceFileRecord(path);

    // Handle #include directives
    while ((currentInclude = source.find(INCLUDE_START)) != std::string::npos) {
      u32 pathStart = currentInclude + INCLUDE_START.size();
      u32 pathEnd = source.find(INCLUDE_END, pathStart);
      // @todo store include paths in shader record;
      // check all included files when hot reloading
      std::string includePath = INCLUDE_ROOT_PATH + source.substr(pathStart, pathEnd - pathStart);
      u32 replaceStart = currentInclude;
      u32 replaceLength = (pathEnd + INCLUDE_END.size()) - currentInclude;

      if (Gm_VectorContains(includes, includePath)) {
        // File already included; simply remove the directive
        source.replace(replaceStart, replaceLength, "");
      } else {
        // Replace the directive with the included file contents
        std::string includeSource = Gm_LoadFileContents(includePath.c_str());

        source.replace(replaceStart, replaceLength, includeSource);
        includes.push_back(includePath);

        Gm_SaveShaderSourceFileRecord(includePath.c_str());
      }
    }

    // Handle #define variable overrides
    for (auto& [ name, value ] : defineOverrides) {
      std::string defineDirective = "#define " + name + " ";
      u32 directiveStart = source.find(defineDirective);

      if (directiveStart != std::string::npos) {
        u32 valueStart = directiveStart + defineDirective.size();
        u32 valueEnd = source.find("\n", valueStart);

        source.replace(valueStart, valueEnd - valueStart, value);
      }
    }

    const GLchar* shaderSource = source.c_str();

    glShaderSource(shader, 1, (const GLchar**)&shaderSource, 0);
    glCompileShader(shader);

    GLint status;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status != GL_TRUE) {
      char error[512];

      glGetShaderInfoLog(shader, 512, 0, error);

      Console::log("[Gamma] Failed to compile shader:", path);
      Console::log(error);
    }

    auto fsPath = std::filesystem::current_path() / path;

    return {
      shader,
      shaderType,
      path,
      includes
    };
  }

  /**
   * Gm_CompileFragmentShader
   * ------------------------
   */
  static GLShaderRecord Gm_CompileFragmentShader(const char* path) {
    return Gm_CompileShader(GL_FRAGMENT_SHADER, path);
  }

  /**
   * Gm_CompileGeometryShader
   * ------------------------
   */
  static GLShaderRecord Gm_CompileGeometryShader(const char* path) {
    return Gm_CompileShader(GL_GEOMETRY_SHADER, path);
  }

  /**
   * Gm_CompileVertexShader
   * ----------------------
   */
  static GLShaderRecord Gm_CompileVertexShader(const char* path) {
    return Gm_CompileShader(GL_VERTEX_SHADER, path);
  }

  /**
   * OpenGLShader
   * ------------
   */
  void OpenGLShader::init() {
    program = glCreateProgram();

    glShaderPrograms.push_back(this);
  }

  void OpenGLShader::destroy() {
    glDeleteProgram(program);
    
    // @todo remove from glShaderPrograms
  }

  void OpenGLShader::attachShader(const GLShaderRecord& record) {
    glAttachShader(program, record.shader);

    glShaderRecords.push_back(record);
  }

  void OpenGLShader::checkAndHotReloadShaders() {
    for (auto& record : glShaderRecords) {
      bool shouldHotReload = false;

      // Check to see if any dependencies have changed
      for (auto& dependencyPath : record.dependencyPaths) {
        if (Gm_VectorContains(changedSourceFilePaths, dependencyPath)) {
          shouldHotReload = true;

          break;
        }
      }

      // Check to see if the shader entry point was changed
      if (Gm_VectorContains(changedSourceFilePaths, record.path)) {
        shouldHotReload = true;
      }

      if (shouldHotReload) {
        glDetachShader(program, record.shader);
        glDeleteShader(record.shader);

        GLShaderRecord updatedRecord = Gm_CompileShader(record.shaderType, record.path.c_str(), defineVariables);

        glAttachShader(program, updatedRecord.shader);
        glLinkProgram(program);

        record = updatedRecord;

        Console::log("[Gamma] Hot-reloaded shader:", record.path);

        break;
      }
    }
  }

  void OpenGLShader::define(const std::string& name, const std::string& value) {
    define({ { name, value } });
  }

  void OpenGLShader::define(const std::map<std::string, std::string>& defineOverrides) {
    for (auto& [ name, value ] : defineOverrides) {
      defineVariables[name] = value;
    }

    for (auto& record : glShaderRecords) {
      glDetachShader(program, record.shader);
      glDeleteShader(record.shader);

      GLShaderRecord updatedRecord = Gm_CompileShader(record.shaderType, record.path.c_str(), defineVariables);

      glAttachShader(program, updatedRecord.shader);

      record = updatedRecord;
    }

    glLinkProgram(program);
  }

  void OpenGLShader::fragment(const char* path) {
    attachShader(Gm_CompileFragmentShader(path));
  }

  void OpenGLShader::geometry(const char* path) {
    attachShader(Gm_CompileGeometryShader(path));
  }

  GLint OpenGLShader::getUniformLocation(const char* name) const {
    return glGetUniformLocation(program, name);
  }

  GLint OpenGLShader::getUniformLocation(std::string name) const {
    return glGetUniformLocation(program, name.c_str());
  }

  void OpenGLShader::link() {
    glLinkProgram(program);

    #if GAMMA_DEVELOPER_MODE
      for (auto& record : glShaderRecords) {
        Console::log("[Gamma] Loaded shader:", record.path);
      }
    #endif
  }

  void OpenGLShader::setBool(std::string name, bool value) const {
    setInt(name, value);
  }

  void OpenGLShader::setFloat(std::string name, float value) const {
    glUniform1f(getUniformLocation(name), value);
  }

  void OpenGLShader::setInt(std::string name, int value) const {
    glUniform1i(getUniformLocation(name), value);
  }

  void OpenGLShader::setMatrix4f(std::string name, const Matrix4f& value) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, value.m);
  }

  void OpenGLShader::setVec2f(std::string name, const Vec2f& value) const {
    glUniform2fv(getUniformLocation(name), 1, &value.x);
  }

  void OpenGLShader::setVec3f(std::string name, const Vec3f& value) const {
    glUniform3fv(getUniformLocation(name), 1, &value.x);
  }

  void OpenGLShader::setVec4f(std::string name, const Vec4f& value) const {
    glUniform4fv(getUniformLocation(name), 1, &value.x);
  }

  void OpenGLShader::use() {
    glUseProgram(program);
  }

  void OpenGLShader::vertex(const char* path) {
    attachShader(Gm_CompileVertexShader(path));
  }
}