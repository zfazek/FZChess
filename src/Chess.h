#ifndef CHESS_H
#define CHESS_H

#include <time.h>
#include <setjmp.h>
#include <stdio.h>
#include <thread>
#include <pthread.h>

class Chess {

    public:
        Chess();
        ~Chess();

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

        //Array of sorted legal moves
        int eva_alfabeta_temp[255];
        char move_str[6];
        char input[1001];
        int FZChess; // 1:white, -1:black

        unsigned int nodes;
        unsigned int hash_nodes;
        unsigned int hash_inner_nodes;
        unsigned int hash_collision;
        unsigned int hash_collision_inner;
        time_t t1, t2, t1_last, t2_last;
        jmp_buf env;
        int start_time, stop_time, max_time, movetime;
        int stop_search;
        int depth, seldepth, init_depth, curr_depth, curr_seldepth, gui_depth;
        int sm;
        int legal_pointer;
        int *pt, *ptt, *pt1, *pt2;
        int *ptablelist;
        int *pdir, *pdir1;
        int *pl;
        int last_ply;
        int best_move;
        int mate_score;
        int end_direction;
        unsigned long long knodes = 9999999999999999999LLU;
        std::thread th_make_move;
        pthread_t threads;
        int rc;
        long t;
        int sort_alfarray;


        struct move_t {
            int move;
            int value;
        };

        struct move_t root_moves[255], *proot_moves;

        //Stores the parameters of the given position
        struct move movelist[1000], *pm, *pm1, *pm2;

        struct best_lines {
            int length;
            int value;
            int moves[1000];
        } best_line[99];


        //Startup position
        int new_table[120] =
        {
            0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
            0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
            0xff,0x04,0x02,0x03,0x05,0x06,0x03,0x02,0x04,0xff,  // white - first row
            0xff,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0xff,
            0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
            0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
            0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
            0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
            0xff,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0xff, // black - seventh row
            0xff,0x84,0x82,0x83,0x85,0x86,0x83,0x82,0x84,0xff,
            0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
            0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
        };

        void start_game();
        void make_move();
        int alfabeta(int dpt, int alfa, int beta);
        void list_legal_moves();
        int is_attacked(int field, int color);
        void calculate_evarray();
        void checkup();
        void update_table(int move, int print);
        int third_occurance();
        int not_enough_material();
        int evaluation(int e_legal_pointer, int dpt);
        int evaluation_only_end_game(int dpt);
        void is_really_legal();
        void castling();
        void append_legal_moves(int dir_piece, int i, int j, int kk);
        void append_legal_moves_inner(int dir_piece, int i, int j, int kk);
        void print_table();
        int evaluation_material(int dpt);
        int convA(int k);
        int conv0(int k);
        int evaluation_king(int field, int figure);
        int evaluation_pawn(int field, int figure, int sm);
        void invert_player_to_move();
        int sum_material(int color);
        int str2move(char move_old[6]);
        void move2str(int move);
        void reset_movelist();
        void setboard(char fen_position[255]);
        void flush();
        int get_ms();
        void position_received(char* input);
        void processCommands(char* input);
        void set_hash();
        unsigned long long rand64();
        unsigned long long hash_rand();
        void init_hash();
        void init_hash_inner();




        int Pawn   = 1;
        int Knight = 2;
        int Bishop = 3;
        int Rook   = 4;
        int Queen  = 5;
        int King   = 6;

        //Values representing the figures in the table
        const int WhitePawn   = 1;
        const int WhiteKnight = 2;
        const int WhiteBishop = 3;
        const int WhiteRook   = 4;
        const int WhiteQueen  = 5;
        const int WhiteKing   = 6;

        const int BlackPawn   = 0x81;
        const int BlackKnight = 0x82;
        const int BlackBishop = 0x83;
        const int BlackRook   = 0x84;
        const int BlackQueen  = 0x85;
        const int BlackKing   = 0x86;

        //For printing and for notation
        int graphical_figure[14][2] =
        {
            { 000,  46 }, // "."
            { 001,  80 }, // "P"
            { 002,  78 }, // "N"
            { 003,  66 }, // "B"
            { 004,  82 }, // "R"
            { 005,  81 }, // "Q"
            { 006,  75 }, // "K"
            { 129, 112 }, // "p"
            { 130, 110 }, // "n"
            { 131,  98 }, // "b"
            { 132, 114 }, // "r"
            { 133, 113 }, // "q"
            { 134, 107 }, // "k"
            { 191,  88 }, // "X"
        };

        enum figures { PAWN1, PAWN2, PAWN3, PAWN4, PAWN5, PAWN6, PAWN7, PAWN8,
            KNIGHT1, KNIGHT2, BISHOP1, BISHOP2, ROOK1, ROOK2, QUEEN, KING};

        //Possible direction of figure's move
        int dir_rook[4] = { -10,  -1,   1,  10 };
        int dir_knight[8] = { -21, -19, -12,  -8,  8, 12, 19, 21 };
        int dir_bishop[4] = { -11,  -9,   9,  11 };
        int dir_king[8] = { -11, -10, -9, -1, 1, 9, 10, 11 };

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


        //Values for evaluation
        //empty, pawn, knight, bishop, rook, queen, king
        int figure_value[7] = {0, 100, 330, 330, 500, 900, 0};
        int end_game_threshold = 1200;
        int king_castled       =   40;
        int cant_castle        =  -10;
        int double_bishops     =   50;
        int double_pawn        =   15;
        int friendly_pawn      =    5;
        int pawn_advantage     =   10;


        unsigned long long hash = 9999999999999999999LLU;
        unsigned long long hash_inner = 9999999999999999999LLU;
        unsigned long long hash_side_white = 9999999999999999999LLU;
        unsigned long long hash_side_black = 9999999999999999999LLU;
        unsigned long long hash_piece[2][7][120];
        unsigned long long hash_enpassant[120];
        unsigned long long hash_castle[15];
        unsigned long long hash_index = 9999999999999999999LLU;

        typedef struct {
            unsigned long long lock;
            int u;
        } hash_t;

        typedef struct {
            unsigned long long lock;
            int u;
            int depth;
            int move;
            //byte beta;
        } hash_inner_t;

        hash_t *hashtable;
        hash_inner_t *hashtable_inner;
        unsigned int HASHSIZE = 1024 * 1024 * 128;
        unsigned int HASHSIZE_INNER = 1024 * 1024 * 128;




    private:

};

#endif
