#include "Chess.h"
#include <cstdio>
#include <cstring>
#include <cassert>

void test_move_h2h4() {
    Chess chess;
    chess.n = 0;
    chess.start_game();
    char input[] = "position startpos moves h2h4";
    chess.position_received(input);
    //chess.max_time = 300000 / 40;
    chess.max_time = 0;
    chess.gui_depth = 4; // "go depth 4"
    chess.make_move();
    printf("n: %d\n", chess.n);
    assert(strcmp(chess.move_str, "e7e6 ") == 0);
    assert(chess.nodes > 100000);
#ifdef HASH
    assert(20000 < chess.hash->hash_nodes);
#endif
}

void test_calculate_evarray(Chess &chess) {
    for (int i = 0; i <= chess.legal_pointer; i++) {
        chess.root_moves[i].move = 100 - i;
        chess.root_moves[i].value = 100 - i;
    }
    chess.calculate_evarray();
    /*
    for (int i = 0; i <= chess.legal_pointer; i++) {
        printf("%2d. %2d %2d\n",
                i + 1,
                chess.root_moves[i].move,
                chess.root_moves[i].value);
    }
    */
    for (int i = 0; i < chess.legal_pointer; i++) {
        assert(chess.root_moves[i].value >= chess.root_moves[i+1].value);
    }
}

void test_calculate_evarray_new(Chess &chess) {
    for (int i = 0; i <= chess.legal_pointer; i++) {
        chess.root_moves[i].move = 100 - i;
        chess.root_moves[i].value = 100 - i;
    }
    chess.calculate_evarray_new();
    for (int i = 0; i <= chess.legal_pointer; i++) {
        /*
        printf("%2d. %2d %2d\n",
                i + 1,
                chess.root_moves[i].move,
                chess.root_moves[i].value);
        */
    }
    for (int i = 0; i < chess.legal_pointer; i++) {
        assert(chess.root_moves[i].value >= chess.root_moves[i+1].value);
    }
}

void test() {
    /*
    {
        Chess chess;
        chess.legal_pointer = 40;
        chess.init_depth = 2;
        test_calculate_evarray(chess);
        test_calculate_evarray_new(chess);
    }
    */
    test_move_h2h4();
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        test();
    } else {

        // Start here
        // Waiting for GUI
        char* ret;
        char input[1001];
        while (1) {
            ret = fgets(input, 1000, stdin);
            if (ret && strstr(input, "uci")) {
                break;
            }
        }
        Chess chess;
        chess.start_game();
        chess.processCommands(input);
    }
}
