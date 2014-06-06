#include "Table.h"
#include "Move.h"
#include "Evaulate.h"
#include <cstdlib>
#include <cstring>

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

// Updates the parameters /array movelist/ and the table with the given move.
// If print is true -> prints the table
void Table::update_table(int move, int print) {
	struct move *pm, *pm1, *pm2;
	int n, x_from, y_from, x_to, y_to;
    int figure_from, figure_to, square_from, square_to;
    int *pt1, *pt2;
    char promotion = ' ';
    //Copies the table for the next move and updates later according to the move
    pt1 = chess->tablelist[chess->move_number];
    pm1 = movelist + chess->move_number;
    ++chess->move_number;
    pt2 = chess->tablelist[chess->move_number];
    pm2 = movelist + chess->move_number;
    memcpy(pt2, pt1, 120*sizeof(int));
    x_from = (move & 0xe000) / 256 / 32;
    y_from = (move & 0x1c00) / 256 /  4;
    x_to   = (move & 0x00e0) % 256 / 32;
    y_to   = (move & 0x001c) % 256 /  4;
    figure_from = *(pt2 + 1 + x_from + (y_from + 2) * 10);
    figure_to   = *(pt2 + 1 + x_to + (y_to + 2) * 10);
    //Copies the parameters according to the previous status and updates later
    pm2 -> white_double_bishops = pm1 -> white_double_bishops;
    pm2 -> black_double_bishops = pm1 -> black_double_bishops;
    pm2 -> pos_white_king = pm1 -> pos_white_king;
    pm2 -> pos_black_king = pm1 -> pos_black_king;
    pm2 -> white_king_castled = pm1 -> white_king_castled;
    pm2 -> black_king_castled = pm1 -> black_king_castled;
    if (figure_to == WhiteBishop) pm2 -> white_double_bishops = 0;
    if (figure_to == BlackBishop) pm2 -> black_double_bishops = 0;
    if (pm1 -> further == 2)
	pm2 -> further = 1;
    else
	pm2 -> further = 0;
    square_from = 1 + x_from + (y_from + 2) * 10;
    square_to   = 1 + x_to + (y_to + 2) * 10;
    pm2 -> color = chess->player_to_move;
    pm2 -> move_from = square_from;
    pm2 -> move_to = square_to;
    pm2 -> castle = pm1 -> castle;
    pm2 -> captured_figure = figure_to;

    // If captured is occured -> not_pawn_move parameters is set to 1,
    // and further is set to 1
    if (figure_to != 0) {
        pm2 -> not_pawn_move = 1;
        pm2 -> further = 1;
    }
    //Promotion
    if ((move & 0x0303) > 0) {
        if ((move & 0x0200) == 0x0200) promotion = 'q'; else
            if ((move & 0x0100) == 0x0100) promotion = 'r'; else
                if ((move & 0x0002) == 0x0002) promotion = 'b'; else
                    if ((move & 0x0001) == 0x0001) promotion = 'n';
        for (n = 0; n < 14; n++) {
            if (graphical_figure[n][1] == promotion) {
                pm2 -> promotion = (graphical_figure[n][0] & 127);
                *(pt2 + square_to) = (graphical_figure[n][0] & 127);
                break;
            }
        }
        if (chess->player_to_move == BLACK) {
            *(pt2 + square_to) += 128;
        }
    }

    // Not promotion
    else {
        pm2 -> promotion = 0;
        *(pt2 + square_to) = figure_from;
    }

    // If not pawn move
    if ( ! (figure_from == WhitePawn || figure_from == BlackPawn) ) {
        pm2 -> en_passant = 0;
        if (chess->move_number == 1) {
            pm2 -> not_pawn_move = 1;

	    // not_pawn_move++
        } else {
            pm2 -> not_pawn_move = pm1 -> not_pawn_move + 1;
        }
        if (figure_from == WhiteKing) {
            pm2 -> pos_white_king = square_to;

	    // White Castling is not possible any more
            pm2 -> castle = pm2 -> castle & 12;

	    // Castling
            if ((square_from == 25 && square_to == 27) ||
		(square_from == 25 && square_to == 23)) {
                pm2 -> white_king_castled = king_castled;
                if (square_to ==  27) {
                    *(pt2 + 26) = WhiteRook;
                    *(pt2 + 28) = 0;
                }
                if (square_to ==  23) {
                    *(pt2 + 24) = WhiteRook;
                    *(pt2 + 21) = 0;
                }
            }
        }
        if (figure_from == BlackKing) {
            pm2 -> pos_black_king = square_to;

	    // Black Castling is not possible any more
            pm2 -> castle = pm2 -> castle & 3;
            if ((square_from == 95 && square_to == 97) ||
		(square_from == 95 && square_to == 93)) {
                pm2 -> black_king_castled = king_castled;
                if (square_to ==  97) {
                    *(pt2 + 96) = BlackRook;
                    *(pt2 + 98) = 0;
                }
                if (square_to ==  93) {
                    *(pt2 + 94) = BlackRook;
                    *(pt2 + 91) = 0;
                }
            }
        }
        if (figure_from == WhiteRook) {

	    // White Long Castling is not possible any more
            if (square_from == 21) pm2 -> castle = pm2 -> castle & 13;

	    // White Short Castling is not possible any more
            if (square_from == 28) pm2 -> castle = pm2 -> castle & 14;
        }
        if (figure_from == BlackRook) {

	    // Black Long Castling is not possible any more
            if (square_from == 91) pm2 -> castle = pm2 -> castle & 7;

	    // Black Short Castling is not possible any more
            if (square_from == 98) pm2 -> castle = pm2 -> castle & 11;
        }
    } else { // If pawn move
        if (chess->player_to_move == WHITE && y_from > 3) pm2 -> further = 2;
        if (chess->player_to_move == BLACK && y_from < 4) pm2 -> further = 2;
        pm2 -> not_pawn_move = 0;
        if (square_to - square_from == 20) {
            pm2 -> en_passant = square_from + 10; // en passant possible
        }
        if (square_from - square_to == 20) {
            pm2 -> en_passant = square_to + 10; // en passant possible
        }
        if (abs(square_to - square_from) != 20) {
            pm2 -> en_passant = 0; // en passant not possible
        }
        if (pm1 -> en_passant > 1) { // en passant possible
            if (figure_from == WhitePawn) {
                if (pm1 -> en_passant == square_from + 11 &&
		    square_to - square_from == 11) {
                    pm2 -> captured_figure = BlackPawn;
                    *(pt2 + square_to - 10) = 0;
                }
                if (pm1 -> en_passant == square_from + 9 &&
		    square_to - square_from == 9) {
                    pm2 -> captured_figure = BlackPawn;
                    *(pt2 + square_to - 10) = 0;
                }
            }
            if (figure_from == BlackPawn) {
                if (pm1 -> en_passant == square_from - 11 &&
		    square_to - square_from == -11) {
                    pm2 -> captured_figure = WhitePawn;
                    *(pt2 + square_to + 10) = 0;
                }
                if (pm1 -> en_passant == square_from - 9 &&
		    square_to - square_from == -9) {
                    pm2 -> captured_figure = WhitePawn;
                    *(pt2 + square_to + 10) = 0;
                }
            }
        }
    }
    *(pt2 + square_from) = 0;
    if (is_attacked(chess->player_to_move == WHITE ? pm2 -> pos_black_king :
		    pm2 -> pos_white_king, -chess->player_to_move) == TRUE) {
        pm2 -> further = 2;
    }
    if (print == TRUE) print_table();
}

//Prints the table and the parameters to the console
void Table::print_table() {
    int i, j, field;
    for ( i = 11; i >= 0; i-- ) {
        for ( j = 0; j < 10; j++) {
            field = ( chess->tablelist[chess->move_number][ i * 10 + j ] );
            if ( field == 255 ) {
                continue;
            }
            printf( "%3x", field );
        }
        printf("\n");
    }
    printf("Move number    : %d\n", chess->move_number);
    printf("Move color     : %d\n", movelist[chess->move_number].color);
    printf("Move from      : %d\n", movelist[chess->move_number].move_from);
    printf("Move to        : %d\n", movelist[chess->move_number].move_to);
    printf("Captured       : %d\n", movelist[chess->move_number].captured_figure);
    printf("Promotion      : %d\n", movelist[chess->move_number].promotion);
    printf("Castle         : %d\n", movelist[chess->move_number].castle);
    printf("Not Pawn Move  : %d\n", movelist[chess->move_number].not_pawn_move);
    printf("En passant     : %d\n", movelist[chess->move_number].en_passant);
    printf("White castled  : %d\n", movelist[chess->move_number].white_king_castled);
    printf("Black castled  : %d\n", movelist[chess->move_number].black_king_castled);
    printf("Pos White king : %d\n", movelist[chess->move_number].pos_white_king);
    printf("Pos Black king : %d\n", movelist[chess->move_number].pos_black_king);
    printf("White2bishops  : %d\n", movelist[chess->move_number].white_double_bishops);
    printf("Black2bishops  : %d\n", movelist[chess->move_number].black_double_bishops);
    printf("Further invest.: %d\n", movelist[chess->move_number].further);
    printf("\n");
    flush();
}

// FEN interpreter
void Table::setboard(char fen_position[255]) {
    unsigned int n = 13;
    int x = 1;
    int y = 9;
    int m, j;
    int factor;
    char move_old[6]="     ";
    chess->start_game();
    m = 5;
    for (j = 0; j < m; j++) {
        while (fen_position[n] != ' ') {
            n++;
        }
        n++;
    }
    factor = 1;
    chess->move_number = 0;
    while (fen_position[n] != ' ' && n < strlen(input) - 1) {
	/*
        printf("move_number: %d, factor: %d, n: %d, fen_position[n]: %d, strlen(input): %d\n",
	       move_number, factor, n, (int)(fen_position[n] - '0'), strlen(input));
	*/
        fflush(stdout);
        chess->move_number = chess->move_number * factor +
        		(int)(fen_position[n] - '0');
        factor =factor * 10;
        n++;
    }
    n = 13;
    while (fen_position[n] != ' ') {
        if (fen_position[n] > '0' && fen_position[n] < '9') {
            for(m = 0; m < (int)(fen_position[n] - '0'); m++) {
            	chess->tablelist[chess->move_number][y * 10 + x] = 0;
                x++;
            }
            --x;
        }
        if (fen_position[n] != '/') {
            for (m = 0; m < 14; m++) {
                if (fen_position[n] == graphical_figure[m][1]) {
                	chess->tablelist[chess->move_number][y * 10 + x] =
                			graphical_figure[m][0];
                    break;
                }
            }
            x++;
        }
        if (fen_position[n] == '/') {
            y--;
            x = 1;
        }
        n++;
    }
    n++;
    if (fen_position[n] == 'w') {
    	chess->player_to_move = WHITE;
        FZChess = WHITE;
        movelist[chess->move_number].color = WHITE;
    } else {
    	chess->player_to_move = BLACK;
        FZChess = BLACK;
        movelist[chess->move_number].color = BLACK;
    }
    n++;
    n++;
    movelist[chess->move_number].castle = 0;
    while  (fen_position[n] != ' ') {
        if (fen_position[n] == '-') movelist[chess->move_number].castle = 0;
        if (fen_position[n] == 'K') movelist[chess->move_number].castle =
        		movelist[chess->move_number].castle | 1;
        if (fen_position[n] == 'Q') movelist[chess->move_number].castle =
        		movelist[chess->move_number].castle | 2;
        if (fen_position[n] == 'k') movelist[chess->move_number].castle =
        		movelist[chess->move_number].castle | 4;
        if (fen_position[n] == 'q') movelist[chess->move_number].castle =
        		movelist[chess->move_number].castle | 8;
        n++;
    }
    movelist[chess->move_number - 1].castle =
    		movelist[chess->move_number].castle;
    n++;
    if (fen_position[n] == '-') movelist[chess->move_number].en_passant = 0;
    if (fen_position[n] >= 'a' && fen_position[n] <= 'h') { // en passant possible, target square
        movelist[chess->move_number].en_passant = 1 +
        		fen_position[n] - 'a' + (fen_position[n+1] - '1' + 2) * 10;
        n++;
    }
    n++;
    n++;
    movelist[chess->move_number].not_pawn_move = 0;
    factor = 1;
    while (fen_position[n] != ' ') {
        movelist[chess->move_number].not_pawn_move =
        		movelist[chess->move_number].not_pawn_move * factor +
        		(int)(fen_position[n] - '0');
        factor = factor * 10;
        n++;
    }
    for (int i = 0; i < 120; ++i)
    	if ((chess->tablelist[chess->move_number][i] & 127) == King) {
    		if (chess->tablelist[chess->move_number][i] == WhiteKing)
    			movelist[chess->move_number].pos_white_king = i;
    		if (chess->tablelist[chess->move_number][i] == BlackKing)
    			movelist[chess->move_number].pos_black_king = i;
    	}
    if (strstr(input, "moves")) {
        m = strlen(input) - 1;
        for (int i = (int) (strstr(input, "moves") - input + 6); i < m; i++) {
            //printf("i: %d, input[i]: %c\n", i, input[i]);flush();
            move_old[0] = input[i++];
            //i++;
            move_old[1] = input[i++];
            //i++;
            move_old[2] = input[i++];
            //i++;
            move_old[3] = input[i++];
            //i++;
            if (input[i] != ' ' && input[i] != '\n') {
                move_old[4] = input[i++];
                //i++;
            } else {
                move_old[4] = '\0';
            }
            //printf("move: %s\n", move);flush();
            update_table(str2move(move_old), FALSE);
            invert_player_to_move();
        }
    }
#ifdef HASH
    //set_hash();
#endif
    //print_table();
}



