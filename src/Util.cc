#include "Util.h"
#include <sys/timeb.h>
#include <cstdio>

//Flushes the output stream
void Util::flush() {
    fflush(stdout);
}

int Util::get_ms() {
    struct timeb timebuffer;
    ftime(&timebuffer);
    if (timebuffer.millitm != 0)
        return (timebuffer.time * 1000) + timebuffer.millitm;
    return 0;
}

