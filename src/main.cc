#include "Chess.h"
#include "UCI.h"
#include <stdio.h>
#include <string.h>

int main( int argc, char* argv[] ) {
    // Start here
    // Waiting for GUI
    char* ret;
    char input[1001];
    /*
#ifdef HASH
    init_hash();
    printf("hashsize: %d\n", HASHSIZE);flush();
#endif
#ifdef HASH_INNER
    init_hash_inner();
    printf("hashsize_inner: %d\n", HASHSIZE_INNER);flush();
#endif
    */
    Chess *chess = new Chess();
    chess->start_game();
    while (1) {
        ret = fgets(input, 1000, stdin);
        if (ret && strstr(input, "uci")) {
	    UCI *uci = new UCI();
	    uci->processCommands();
        delete uci;
	}
    }
    delete chess;
}
