#pragma once

class Chess;

class Eval {
    public:
		Chess* chess;

        Eval(Chess* chess);
        ~Eval();

        const int DRAW =  0;
        const int LOST = -22000;
        const int WON  =  22000;

        int end_game_threshold = 1200;
        int king_castled       =   40;
        int cant_castle        =  -10;
        int double_bishops     =   50;
        int double_pawn        =  -15;
        int friendly_pawn      =    5;
        int pawn_advantage     =   10;

        //Values for evaluation
        //empty, pawn, knight, bishop, rook, queen, king
        int figure_value[7] = {0, 100, 330, 330, 500, 900, 0};

        int random_window;

        int evaluation_material(int dpt);
        int evaluation(int e_legal_pointer, int dpt);
        int evaluation_only_end_game(int dpt);
        int evaluation_king(int field, int figure);
        int evaluation_pawn(int field, int figure, int sm);
        int sum_material(int color);

};

