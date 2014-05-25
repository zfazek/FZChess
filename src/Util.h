#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

int DEBUG = 0;
FILE *debugfile;
int stop_received = 0;

const int TRUE  = 1;
const int FALSE = 0;

const int WHITE =  1;
const int BLACK = -1;

const int DRAW =  0;
const int LOST = -22000;
const int WON  =  22000;



//Flushes the output stream
inline void flush() {
    fflush(stdout);
}

#endif
