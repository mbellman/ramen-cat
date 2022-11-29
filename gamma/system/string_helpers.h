#pragma once

#include <string>
#include <vector>

namespace Gamma {
  std::vector<std::string> Gm_SplitString(const std::string& str, const std::string& delimiter);
  std::string Gm_JoinString(const std::vector<std::string>& segments, const std::string& delimiter);
  std::string Gm_TrimString(const std::string& str);
  bool Gm_StringStartsWith(const std::string& str, const std::string& start);
}