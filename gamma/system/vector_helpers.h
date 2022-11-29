#pragma once

#include <vector>

/**
 * Gm_VectorContains
 * -----------------
 */
template<typename T>
static bool Gm_VectorContains(const std::vector<T>& vector, T element) {
  return std::find(vector.begin(), vector.end(), element) != vector.end();
}

template<typename T>
static void Gm_VectorRemove(std::vector<T>& vector, T element) {
  vector.erase(std::remove(vector.begin(), vector.end(), element), vector.end());
}