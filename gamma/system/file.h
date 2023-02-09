#pragma once

#include <functional>
#include <string>

namespace Gamma {
  std::string Gm_LoadFileContents(const std::string& path);
  void Gm_WriteFileContents(const std::string& path, const std::string& contents);
  void Gm_WatchFile(const std::string& path, const std::function<void()>& handler);
  void Gm_HandleWatchedFiles();
}