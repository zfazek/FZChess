#ifndef MOVE_H
#define MOVE_H


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



#endif
