#pragma once

#include <stdint.h>

void flush();
uint64_t get_ms();
int str2move(const char move_old[6]);
char *move2str(char *move_str, const int move);
