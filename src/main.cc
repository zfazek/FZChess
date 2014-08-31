#include "Chess.h"
#include <cstdio>
#include <cstring>
#include <cassert>

void test_move_h2h4() {
    Chess chess;
    chess.start_game();
    char input[] = "position startpos moves h2h4";
    chess.position_received(input);
    chess.max_time = 300000 / 40;
    chess.make_move();
    assert(strcmp(chess.move_str, "e7e6 ") == 0);
    assert(chess.nodes > 120000);
    assert(27425 == chess.hash->hash_nodes);
}

void test() {
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
