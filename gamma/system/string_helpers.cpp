#include "system/string_helpers.h"
#include "system/type_aliases.h"

using namespace Gamma;

/**
 * Gm_SplitString
 * --------------
 *
 * Splits a string into an arbitrary number of parts,
 * stored in a vector, based on a provided delimiter.
 */
std::vector<std::string> Gm_SplitString(const std::string& str, const std::string& delimiter) {
  std::vector<std::string> values;
  u32 offset = 0;
  u32 found = 0;

  // Add each delimited string segment to the list
  while ((found = str.find(delimiter, offset)) != std::string::npos) {
    values.push_back(str.substr(offset, found - offset));

    offset = found + delimiter.size();
  }

  // Include the remaining string segment after the final delimiter
  values.push_back(str.substr(offset, str.size() - offset));

  return values;
}

/**
 * Gm_JoinString
 * -------------
 *
 * Joins a vector of strings into a single string, using
 * the provided delimiter as a joining character.
 */  
std::string Gm_JoinString(const std::vector<std::string>& segments, const std::string& delimiter) {
  std::string joined;

  for (u32 i = 0; i < segments.size(); i++) {
    joined += segments[i];

    if (i < segments.size() - 1) {
      joined += delimiter;
    }
  }

  return joined;
}

/**
 * Gm_TrimString
 * -------------
 *
 * Returns a modified version of a string with the beginning
 * and ending whitespace removed.
 *
 * @todo handle tab characters
 */
std::string Gm_TrimString(const std::string& str) {
  std::string trimmed;
  u32 start = 0;
  u32 end = str.size();

  for (;;) {
    if (str[start] == ' ') start++;
    else if (str[--end] != ' ')  break;
  }

  return str.substr(start, end - start + 1);
}

/**
 * Gm_StringStartsWith
 * -------------------
 *
 * Determines whether a string begins with a specified starting string.
 */
bool Gm_StringStartsWith(const std::string& str, const std::string& start) {
  return str.substr(0, start.size()) == start;
}

bool Gm_StringContains(const std::string& str, const std::string& term) {
  return str.find(term) != std::string::npos;
}

std::string Gm_ToString(const Vec3f& v) {
  return std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z);
}

std::string Gm_ToString(const Quaternion& q) {
  return std::to_string(q.w) + "," + std::to_string(q.x) + "," + std::to_string(q.y) + "," + std::to_string(q.z);
}

std::string Gm_ToString(const pVec4& p) {
  return std::to_string(p.r) + "," + std::to_string(p.g) + "," + std::to_string(p.b) + "," + std::to_string(p.a);
}

Gamma::Vec3f Gm_ParseVec3f(const std::string& str) {
  auto parts = Gm_SplitString(str, ",");

  return Vec3f(stof(parts[0]), stof(parts[1]), stof(parts[2]));
}