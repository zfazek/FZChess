#pragma once

#include <stdint.h>
#include <stdio.h>

void flush();
void open_debug_file();
void LOG(const char *format, ...);
uint64_t get_ms();
int str2move(const char move_old[6]);
char *move2str(char *move_str, const int move);
