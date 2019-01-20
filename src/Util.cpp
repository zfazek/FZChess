#include "Util.h"
#include <sys/timeb.h>
#include <cstdio>
#include <cstring>

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

int Util::str2move(char move_old[6]) {
    int x_from, y_from, x_to, y_to, move_hi, move_lo;
    x_from = move_old[0] - 'a';
    y_from = move_old[1] - '1';
    x_to   = move_old[2] - 'a';
    y_to   = move_old[3] - '1';
    move_hi = x_from * 32 + y_from * 4;
    move_lo = x_to * 32 + y_to * 4;
    switch (move_old[4]) {
        case 'q': case 'Q': move_hi += 2; break;
        case 'r': case 'R': move_hi += 1; break;
        case 'b': case 'B': move_lo += 2; break;
        case 'n': case 'N': move_lo += 1; break;
        default: break;
    }
    return move_hi * 256 + move_lo;
}

char* Util::move2str(char* move_str, int move) {
    strcpy(move_str,"     ");
    move_str[0] = (move & 0xe000) / 256 / 32 + 'a';
    move_str[1] = (move & 0x1c00) / 256 /  4 + '1';
    move_str[2] = (move & 0x00e0) % 256 / 32 + 'a';
    move_str[3] = (move & 0x001c) % 256 /  4 + '1';
    if ((move & 0x0303) > 0) {
        if ((move & 0x0200) == 0x0200) move_str[4]='q';
        else if ((move & 0x0100) == 0x0100) move_str[4]='r';
        else if ((move & 0x0002) == 0x0002) move_str[4]='b';
        else if ((move & 0x0001) == 0x0001) move_str[4]='n';
    }
    return move_str;
}
