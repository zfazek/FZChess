#include "Chess.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <locale.h>
#include <time.h>

void test_move_h2h4() {
    Chess chess;
    chess.n = 0;
    chess.start_game();
    char input[] = "position startpos moves h2h4";
    chess.uci->position_received(input);
    // chess.max_time = 300000 / 40;
    chess.max_time = 0;
    chess.gui_depth = 4; // "go depth 4"
    chess.make_move();
    printf("n: %d\n", chess.n);
    assert(strcmp(chess.move_str, "e7e6 ") == 0);
    // assert(chess.nodes == 136617 || chess.nodes == 163754 || chess.nodes ==
    // 208434);
    assert(chess.nodes == 201747);
#ifdef HASH
    assert(20000 < chess.hash->hash_nodes);
#endif
}

void test_perft_pos1(int depth) {
    char input[] = "position fen "
                   "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w "
                   "KQkq - 0 1";
    puts(input);
    for (int i = 1; i <= depth; i++) {
        Chess chess;
        chess.start_game();
        chess.table->setboard(input);
        int start_time = Util::get_ms();
        uint64_t nodes = chess.perft(i);
        int stop_time = Util::get_ms();
        int duration = stop_time - start_time;
        printf("depth: %d nodes: %'lu time: %'d ms knps: %'lu\n", i, nodes,
               duration, (duration == 0) ? 0 : (nodes / duration));
        switch (i) {
        case 1:
            assert(nodes == 48);
            break;
        case 2:
            assert(nodes == 2039);
            break;
        case 3:
            assert(nodes == 97862);
            break;
        case 4:
            assert(nodes == 4085603);
            break;
        case 5:
            assert(nodes == 193690690);
            break;
        default:
            break;
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
        uint64_t nodes = chess.perft(i);
        int stop_time = Util::get_ms();
        int duration = stop_time - start_time;
        printf("depth: %d nodes: %'lu time: %'d ms knps: %'lu\n", i, nodes,
               duration, (duration == 0) ? 0 : (nodes / duration));
        switch (i) {
        case 1:
            assert(nodes == 20);
            break;
        case 2:
            assert(nodes == 400);
            break;
        case 3:
            assert(nodes == 8902);
            break;
        case 4:
            assert(nodes == 197281);
            break;
        case 5:
            assert(nodes == 4865609);
            break;
        default:
            break;
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

void test_eval_depth_1() {
    char input[] = "position fen 4k3/8/6p1/6P1/6p1/6P1/4K3/8 w - - 0 1";
    puts(input);
    Chess chess;
    chess.start_game();
    chess.table->setboard(input);
    chess.max_time = 0;
    chess.gui_depth = 1;
    chess.default_seldepth = 0;
    chess.make_move();
}

void test_eval_depth_2() {
    char input[] = "position fen 4k2n/8/6p1/6P1/6p1/6P1/8/4K2N w - - 0 1 ";
    puts(input);
    Chess chess;
    chess.start_game();
    chess.table->setboard(input);
    chess.max_time = 0;
    chess.gui_depth = 2;
    chess.default_seldepth = 0;
    chess.make_move();
}

void test_bratko_kopec_1() {
    char input[] =
        "position fen 1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - "
        "- 0 1 bm Qd1+; id BK.01;";
    puts(input);
    Chess chess;
    chess.start_game();
    chess.table->setboard(input);
    chess.max_time = 0;
    chess.gui_depth = 5;
    chess.default_seldepth = 0;
    chess.break_if_mate_found = false; // remove
    chess.make_move();
    assert(strcmp(chess.move_str, "d6d1 ") == 0);
    assert(chess.mate_score == 21995);
    assert(chess.root_moves[0].value == 21995);
}

void test_bratko_kopec_1a() {
    char input[] =
        "position fen 1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - "
        "- 0 1 bm Qd1+; id BK.01;";
    puts(input);
    Chess chess;
    chess.start_game();
    chess.table->setboard(input);
    chess.max_time = 0;
    chess.gui_depth = 6;
    chess.default_seldepth = 0;
    chess.make_move();
    assert(strcmp(chess.move_str, "d6d1 ") == 0);
    assert(chess.mate_score == 21995);
    assert(chess.root_moves[0].value == 21995);
    assert(chess.depth == 5);
}

void test_bratko_kopec_1b() {
    char input[] =
        "position fen 1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - "
        "- 0 1 bm Qd1+; id BK.01;";
    puts(input);
    Chess chess;
    chess.start_game();
    chess.table->setboard(input);
    chess.max_time = 0;
    chess.gui_depth = 1;
    // chess.default_seldepth = 0;
    chess.make_move();
    assert(strcmp(chess.move_str, "d6d1 ") == 0);
    assert(chess.mate_score == 21995);
    assert(chess.root_moves[0].value == 21995);
    assert(chess.depth == 1);
}

void test_bratko_kopec_2() {
    char input[] =
        "position fen 3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w - "
        "- 0 1 bm d5; id BK.02;";
    puts(input);
    Chess chess;
    chess.start_game();
    chess.table->setboard(input);
    chess.max_time = 0;
    chess.default_seldepth = 8;
    chess.gui_depth = 5;
    chess.make_move();
    assert(strcmp(chess.move_str, "d4d5 ") == 0);
}

void test_bratko_kopec_10() {
    char input[] = "position fen "
                   "3rr1k1/pp3pp1/1qn2np1/8/3p4/PP1R1P2/2P1NQPP/R1B3K1 b - - 0 "
                   "1 bm Ne5; id BK.10;";
    puts(input);
    Chess chess;
    chess.start_game();
    chess.table->setboard(input);
    chess.max_time = 0;
    chess.gui_depth = 5;
    chess.make_move();
    assert(strcmp(chess.move_str, "c6e5 ") == 0);
}

void test_bratko_kopec_12() {
    char input[] =
        "position fen r3r1k1/ppqb1ppp/8/4p1NQ/8/2P5/PP3PPP/R3R1K1 b - "
        "- 0 1 bm Bf5; id BK.12;";
    puts(input);
    Chess chess;
    chess.start_game();
    chess.table->setboard(input);
    chess.max_time = 0;
    chess.gui_depth = 2;
    chess.make_move();
    assert(strcmp(chess.move_str, "d7f5 ") == 0);
}

void test_bratko_kopec() {
    /*
  test_bratko_kopec_1();
  test_bratko_kopec_1a();
  test_bratko_kopec_1b();
  test_bratko_kopec_2();
  test_bratko_kopec_10();
  */
    test_bratko_kopec_12();
}

void test_calculate_evarray(Chess &chess) {
    for (int i = 0; i < chess.nof_legal; i++) {
        chess.root_moves[i].move = 100 - i;
        chess.root_moves[i].value = i;
    }
    chess.calculate_evarray();
    for (int i = 0; i < chess.nof_legal - 1; i++) {
        assert(chess.root_moves[i].value >= chess.root_moves[i + 1].value);
    }
}

void test_calculate_evarray_new(Chess &chess) {
    for (int i = 0; i < chess.nof_legal; i++) {
        chess.root_moves[i].move = 100 - i;
        chess.root_moves[i].value = i;
    }
    /*
  for (int i = 0; i < chess.nof_legal; i++) {
      printf("%2d. %2d %2d\n",
              i + 1,
              chess.root_moves[i].move,
              chess.root_moves[i].value);
  }
  */
    chess.calculate_evarray_new();
    /*
  for (int i = 0; i < chess.nof_legal; i++) {
      printf("%2d. %2d %2d\n",
              i + 1,
              chess.root_moves[i].move,
              chess.root_moves[i].value);
  }
  */
    for (int i = 0; i < chess.nof_legal - 1; i++) {
        assert(chess.root_moves[i].value >= chess.root_moves[i + 1].value);
    }
}

void test_mate_in_2() {
    char input[] =
        "position fen 2bqkbn1/2pppp2/np2N3/r3P1p1/p2N2B1/5Q2/PPPPKPP1/RNB2r2 w "
        "KQkq - 0 1";
    puts(input);
    Chess chess;
    chess.start_game();
    chess.table->setboard(input);
    chess.max_time = 0;
    chess.gui_depth = 5;
    chess.default_seldepth = 0;
    chess.make_move();
}

void test() {
    {
        Chess chess;
        chess.nof_legal = 40;
        chess.depth = 2;
        test_calculate_evarray(chess);
        test_calculate_evarray_new(chess);
    }
    test_move_h2h4();
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");
    if (argc > 1) {
        if (strcmp(argv[1], "1") == 0) {
            test();
        } else if (strcmp(argv[1], "2") == 0) {
            test_perft();
        } else if (strcmp(argv[1], "3") == 0) {
            test_bratko_kopec();
        } else if (strcmp(argv[1], "4") == 0) {
            test_mate_in_2();
            test_eval_depth_1();
            test_eval_depth_2();
        }
    } else {
        // Start here
        // Waiting for GUI
        char *ret;
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
