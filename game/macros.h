#pragma once

#define internal static inline
#define get_player() objects("player")[0]

#define START_TIMING(label) \
  u64 __start = Gm_GetMicroseconds();\
  std::string __label = label;

#define LOG_TIME() \
    u64 __time = Gm_GetMicroseconds() - __start;\
    u32 __ms = u32(__time / 1000.f);\
    add_debug_message(__label + ": " + std::to_string(__time) + " us");