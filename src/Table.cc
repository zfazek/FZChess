#include "Table.h"
#include "Move.h"
#include "Evaulate.h"

Table::Table(Chess* ch) {
	chess = ch;
}

Table::~Table() {
}

//Resets the parameters and the table
void Table::reset_movelist() {
    int i;
    for (int move_number = 0 ; move_number < 255 ; move_number++) {
        movelist[move_number].color = 0;
        movelist[move_number].move_from = 0;
        movelist[move_number].move_to = 0;
        movelist[move_number].captured_figure = 0;
        movelist[move_number].promotion = 0;

        // Both color, KQside castle possible
        // 1:WhiteShort, 2:WhiteLong, 4:BlackShort, 8:BlackLong
        movelist[move_number].castle = 15;
        movelist[move_number].not_pawn_move = 0;
        movelist[move_number].en_passant = 0;
        movelist[move_number].white_king_castled = 0;
        movelist[move_number].black_king_castled = 0;
        movelist[move_number].pos_white_king = 25;
        movelist[move_number].pos_black_king = 95;
        movelist[move_number].white_double_bishops = double_bishops;
        movelist[move_number].black_double_bishops = double_bishops;
        movelist[move_number].further = 0;
#ifdef POS_FIGURE
        for ( i = 0; i < 16; i++)
	    movelist[move_number].pos_white_figure[i] = new_pos_white_figure[i];
        for ( i = 0; i < 16; i++)
	    movelist[move_number].pos_black_figure[i] = new_pos_black_figure[i];
#endif
        for ( i = 0; i < 120; i++ ) {
        	chess->tablelist[move_number][i] = chess->new_table[i];
        }
    }
    chess->move_number = 0;
}

