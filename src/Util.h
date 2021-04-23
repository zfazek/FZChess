#pragma once

#include <cstdint>
#include <cstdio>

class Util {
  public:
    static void flush();
    static uint64_t get_ms();
    static int str2move(const char *move_old);
    static char *move2str(const int move);
};
