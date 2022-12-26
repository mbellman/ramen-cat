#pragma once

#define internal static inline
#define getPlayer() objects("sphere")[0]

#define START_TIMING(label) \
  u64 __start = Gm_GetMicroseconds();\
  std::string __label = label;

#define LOG_TIME() \
    u64 __time = Gm_GetMicroseconds() - __start;\
    u32 __ms = __time / 1000.f;\
    addDebugMessage(__label + ": " + std::to_string(__time) + " us");