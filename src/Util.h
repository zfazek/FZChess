#pragma once

#include <cstdint>
#include <cstdio>

class Util {
  public:
    static void flush();
    static void open_debug_file();
    static void LOG(const char *format, ...);
    static uint64_t get_ms();
    static int str2move(const char *move_old);
    static char *move2str(const int move);
};
