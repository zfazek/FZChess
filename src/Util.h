#pragma once

class Util {
  public:
    static void flush();
    static int get_ms();
    static int str2move(const char move_old[6]);
    static char *move2str(char *move_str, const int move);
};
