#pragma once

#include <stdint.h>
#include <stdio.h>

void flush();
void open_debug_file();
void LOG(const char *format, ...);
uint64_t get_ms();
int str2move(const char *move_old);
char *move2str(const int move);
