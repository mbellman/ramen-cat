#pragma once

#include <functional>
#include <string>

namespace Gamma {
  std::string Gm_LoadFileContents(const char* path);
  void Gm_WriteFileContents(const char* path, const std::string& contents);
  void Gm_WatchFile(const char* path, const std::function<void()>& handler);
  void Gm_HandleWatchedFiles();
}