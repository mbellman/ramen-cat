#pragma once

#include <string>

namespace Gamma {
  class AbstractLoader {
  public:
    virtual ~AbstractLoader() {};

  protected:
    bool isLoading = false;

    void load(const char* filePath);
    const std::string& readNextChunk();
    void nextLine();
    void setChunkDelimiter(const std::string& delimiter);

  private:
    std::string buffer = "";
    std::string delimiter = " ";
    FILE* file = 0;

    bool bufferEndsWith(const std::string& str);
    void fillBufferUntil(const std::string& end);
    int getDelimiterOffset();
    bool isAtDelimiter();
    bool isAtEOL();
    char nextChar();
  };
}