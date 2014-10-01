#pragma once
#include "Eval.h"

class Chess;

class Table {
    public:
        Chess* chess;
        Eval* eval;

        int WHITE = 1;
        int BLACK = -1;

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

        //Possible direction of figure's move
        int dir_rook[4] = { -10,  -1,   1,  10 };
        int dir_knight[8] = { -21, -19, -12,  -8,  8, 12, 19, 21 };
        int dir_bishop[4] = { -11,  -9,   9,  11 };
        int dir_king[8] = { -11, -10, -9, -1, 1, 9, 10, 11 };

        //For printing and for notation
        int graphical_figure[14][2] = {
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

        Table(Chess* chess);
        ~Table();
        void print_table();
        void reset_movelist();
        void setboard(char* fen_position);
        int is_attacked(int field, int color);
        bool not_enough_material();
        void castling();
        void update_table(int move, bool print);
        bool third_occurance();
};
