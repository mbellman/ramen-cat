#include <fstream>
#include <filesystem>
#include <vector>

#include "system/assert.h"
#include "system/console.h"
#include "system/file.h"
#include "system/string_helpers.h"

namespace Gamma {
  struct FileWatcher {
    std::filesystem::path absolutePath;
    std::filesystem::file_time_type lastWriteTime;
    std::function<void()> handler;
  };

  static std::vector<FileWatcher> fileWatchers;

  std::string Gm_LoadFileContents(const char* path) {
    std::string source;
    std::ifstream file(path);

    assert(!file.fail(), "[Gamma] Gm_LoadFileContents failed to load file: " + std::string(path));

    std::string line;

    while (std::getline(file, line)) {
      source.append(line + "\n");
    }

    file.close();

    return source;
  }

  void Gm_WriteFileContents(const char* path, const std::string& contents) {
    // Ensure the directory exists
    auto pathSegments = Gm_SplitString(path, "/");

    pathSegments.pop_back();

    auto directories = Gm_JoinString(pathSegments, "/");

    std::filesystem::create_directories(directories);

    // Write to the file
    std::ofstream file(path);

    file << contents;
    file.flush();
  }

  void Gm_WatchFile(const char* path, const std::function<void()>& handler) {
    FileWatcher watcher;

    watcher.absolutePath = std::filesystem::current_path() / path;
    watcher.handler = handler;
    watcher.lastWriteTime = std::filesystem::last_write_time(watcher.absolutePath);

    fileWatchers.push_back(watcher);
  }

  void Gm_HandleWatchedFiles() {
    for (auto& watcher : fileWatchers) {
      auto lastWriteTime = std::filesystem::last_write_time(watcher.absolutePath);

      if (watcher.lastWriteTime != lastWriteTime) {
        watcher.handler();

        watcher.lastWriteTime = lastWriteTime;
      }
    }
  }
}