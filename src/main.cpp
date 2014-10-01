#include "Chess.h"
#include <cstdio>
#include <cstring>
#include <cassert>
#include <locale.h>
#include <time.h>

void test_move_h2h4() {
    Chess chess;
    chess.n = 0;
    chess.start_game();
    char input[] = "position startpos moves h2h4";
    chess.uci->position_received(input);
    //chess.max_time = 300000 / 40;
    chess.max_time = 0;
    chess.gui_depth = 4; // "go depth 4"
    chess.make_move();
    printf("n: %d\n", chess.n);
    assert(strcmp(chess.move_str, "e7e6 ") == 0);
    assert(chess.nodes == 126027);
#ifdef HASH
    assert(20000 < chess.hash->hash_nodes);
#endif
}

void test_perft_pos1(int depth) {
    char input[] = "position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    puts(input);
    for (int i = 1; i <= depth; i++) {
        Chess chess;
        chess.start_game();
        chess.nodes = 0;
        chess.table->setboard(input);
        chess.max_time = 0;
        int start_time = Util::get_ms();
        unsigned long long nodes = chess.perft(i);
        int stop_time = Util::get_ms();
        int duration = stop_time - start_time;
        printf("depth: %d nodes: %'llu time: %'dms knps: %'llu\n",
                i,
                nodes,
                duration,
                (duration == 0) ? 0 : (nodes / duration));
        switch (i) {
            case 1: assert (nodes == 48); break;
            case 2: assert (nodes == 2039); break;
            case 3: assert (nodes == 97862); break;
            case 4: assert (nodes == 4085603); break;
            case 5: assert (nodes == 193690690); break;
            default: break;
        }
    }
    puts("");
}

void test_perft_startpos(int depth) {
    char input[] = "position startpos";
    puts(input);
    for (int i = 1; i <= depth; i++) {
        Chess chess;
        chess.start_game();
        chess.nodes = 0;
        chess.uci->position_received(input);
        chess.max_time = 0;
        int start_time = Util::get_ms();
        unsigned long long nodes = chess.perft(i);
        int stop_time = Util::get_ms();
        int duration = stop_time - start_time;
        printf("depth: %d nodes: %'llu time: %'dms knps: %'llu\n",
                i,
                nodes,
                duration,
                (duration == 0) ? 0 : (nodes / duration));
        switch (i) {
            case 1: assert (nodes == 20); break;
            case 2: assert (nodes == 400); break;
            case 3: assert (nodes == 8902); break;
            case 4: assert (nodes == 197281); break;
            case 5: assert (nodes == 4865609); break;
            default: break;
        }
    }
    puts("");
}

void test_perft() {
    time_t mytime = time(NULL);
    printf("%s", ctime(&mytime));
    int depth = 4;
    test_perft_startpos(depth);
    test_perft_pos1(depth);
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
    {
        Chess chess;
        chess.legal_pointer = 40;
        chess.init_depth = 2;
        test_calculate_evarray(chess);
        test_calculate_evarray_new(chess);
    }
    test_move_h2h4();
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "");
    if (argc > 1) {
        if (strcmp(argv[1], "1") == 0) {
            test();
        } else if (strcmp(argv[1], "2") == 0) {
            test_perft();
        }
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
