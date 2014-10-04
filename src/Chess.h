#pragma once

#include <setjmp.h>
#include <thread>
#include "Hash.h"
#include "Util.h"
#include "Table.h"
#include "Uci.h"

class Chess {

    public:
        Chess();
        ~Chess();

        Hash* hash;
        Util* util;
        Uci* uci;
        Table* table;

        int WHITE; //TODO delete
        int BLACK; //TODO delete

        //Parameters of the given position
        struct move {
            int color;
            int move_from;
            int move_to;
            int captured_figure;
            int promotion;
            int castle;
            int not_pawn_move;
            int en_passant; // eq. e3
            int white_king_castled;
            int black_king_castled;
            int pos_white_king;
            int pos_black_king;
            int white_double_bishops;
            int black_double_bishops;
            int further;
#ifdef POS_FIGURE
            int pos_white_figure[16];
            int pos_black_figure[16];
#endif
        };

        int move_number;
        int player_to_move; // 1:white, -1:black

        //Array of moves. Each time the whole table is stored
        int tablelist[255][120];

        //Array of legal moves
        int legal_moves[255];

        //Array of best line
        int curr_line[255];

        //Array of best move of iterative deepening
        int best_iterative[255];

#ifdef SORT_ALFARRAY

        //Array of sorted legal moves
        int eva_alfabeta_temp[255];
#endif

        char move_str[6];
        char input[1001];
        int FZChess; // 1:white, -1:black

        unsigned long long nodes;
        jmp_buf env;
        int start_time, stop_time, max_time, movetime;
        int stop_search;
        int depth, seldepth, init_depth, curr_depth, curr_seldepth, gui_depth;
        bool last_ply;
        int sm;
        int legal_pointer;
        int best_move;
        int mate_score;
        //unsigned long long knodes = 9999999999999999999LLU;
        std::thread th_make_move;
        int sort_alfarray;

        struct move_t {
            int move;
            int value;
        };

        struct move_t root_moves[255];

        //Stores the parameters of the given position
        struct move movelist[1000];

        void start_game();
        void make_move();
        int alfabeta(int dpt, int alfa, int beta);
        void calculate_evarray();
        void calculate_evarray_new();
        void checkup();
        void invert_player_to_move();
        char* move2str(int move);
        void processCommands(char* input);
        int perft(int dpt);

        int DEBUG;
        FILE *debugfile;
        bool stop_received;

        int n;
};

