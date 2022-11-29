#include <iostream>
#include <string>
#include <algorithm>

#include "system/AbstractLoader.h"
#include "system/console.h"

namespace Gamma {
  void AbstractLoader::fillBufferUntil(const std::string& end) {
    if (!isLoading) {
      return;
    }

    setChunkDelimiter(end);

    char c;

    while (!isAtDelimiter() && !isAtEOL() && (c = nextChar()) != EOF) {
      buffer += c;
    }

    if (c == EOF) {
      fclose(file);

      isLoading = false;
    } else if (isAtDelimiter()) {
      int pos = (int)(buffer.length() - delimiter.length());
      int len = delimiter.length();

      // Ignore the delimiter string at the end of the buffer
      // so chunks can be cleanly parsed with only their contents.
      buffer.erase(pos, len);
    }
  }

  bool AbstractLoader::bufferEndsWith(const std::string& str) {
    int pos = std::max((int)(buffer.length() - str.length()), 0);
    int len = str.length();

    return buffer.length() > 0 && buffer.compare(pos, len, str) == 0;
  }

  bool AbstractLoader::isAtDelimiter() {
    return bufferEndsWith(delimiter);
  }

  bool AbstractLoader::isAtEOL() {
    return bufferEndsWith("\n");
  }

  void AbstractLoader::load(const char* filePath) {
    FILE* f;
    errno_t err = fopen_s(&f, filePath, "r");

    if (err == 0) {
      file = f;
      isLoading = true;
    } else {
      Console::log("[Gamma] AbstractLoader failed to load file:", filePath);
    }
  }

  char AbstractLoader::nextChar() {
    return fgetc(file);
  }

  const std::string& AbstractLoader::readNextChunk() {
    buffer.clear();

    fillBufferUntil(delimiter);

    return buffer.size() == 0 && isLoading ? readNextChunk() : buffer;
  }

  void AbstractLoader::nextLine() {
    fillBufferUntil("\n");

    buffer.clear();
  }

  void AbstractLoader::setChunkDelimiter(const std::string& delimiter) {
    this->delimiter = delimiter;
  }
}