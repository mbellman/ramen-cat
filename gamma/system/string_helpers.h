#pragma once

#include <string>
#include <vector>

#include "math/vector.h"
#include "math/Quaternion.h"
#include "system/packed_data.h"

std::vector<std::string> Gm_SplitString(const std::string& str, const std::string& delimiter);
std::string Gm_JoinString(const std::vector<std::string>& segments, const std::string& delimiter);
std::string Gm_TrimString(const std::string& str);
bool Gm_StringStartsWith(const std::string& str, const std::string& start);
bool Gm_StringContains(const std::string& str, const std::string& term);

std::string Gm_Serialize(const Gamma::Vec3f& v);
std::string Gm_Serialize(const Gamma::Quaternion& q);
std::string Gm_Serialize(const Gamma::pVec4& p);

std::string Gm_ToDebugString(float v);
std::string Gm_ToDebugString(const Gamma::Vec3f& v);
std::string Gm_ToDebugString(const Gamma::Quaternion& q);

Gamma::Vec3f Gm_ParseVec3f(const std::string& str);