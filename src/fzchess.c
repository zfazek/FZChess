#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/timeb.h>
#include <setjmp.h>
#include <pthread.h>

#define U64 unsigned long long

#define HASH
//#define HASH_INNER
//#define CASTLING_PUNISHMENT
#define ALFABETA
#define SORT_ALFARRAY

FILE *debugfile;

const int TRUE  = 1;
const int FALSE = 0;

const int WHITE =  1;
const int BLACK = -1;

const int DRAW =  0;
const int LOST = -22000;
const int WON  =  22000;

int rc;
long t;
//int thread_running = 0;
pthread_t threads;
int input_valid;
int stop_received = 0;

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

int DEBUG = 0;

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

time_t t1, t2, t1_last, t2_last;
int table[120];
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
int legal_pointer;
int random_number;
int random_window = 10;
int tempmove, tempvalue;
int depth, seldepth, init_depth, curr_depth, curr_seldepth, gui_depth;
int best_dpt, depth_inner;
int global_dpt;
int sort_alfarray;
int mate_score;
unsigned int nodes;
unsigned long long knodes = 9999999999999999999LLU;
unsigned int hash_nodes;
unsigned int hash_inner_nodes;
unsigned int hash_collision;
unsigned int hash_collision_inner;
char move_str[6];
char *pmove_str = move_str;
int best_move;
int last_ply;

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


int i, j, ii;
int k, kk;
int sm;
int figure;
int player_to_move; // 1:white, -1:black
int FZChess; // 1:white, -1:black
char input[1001];
int move_number;
char engine[7];
int end_direction;
int dummy;
jmp_buf env;
int stop_search;
int start_time, stop_time, max_time, movetime;
int *pt, *ptt;
int *pdir, *pdir1;
int *pl;

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

//Stores the parameters of the given position
struct move movelist[1000], *pm, *pm1, *pm2;

struct move_t {
    int move;
    int value;
};

struct move_t root_moves[255], *proot_moves;

struct best_lines {
    int length;
    int value;
    int moves[1000];
} best_line[99];

//Startup position
int new_table[] =
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

#ifdef POS_FIGURE
int new_pos_white_figure[16] = {31, 32, 33, 34, 35, 36, 37, 38, 22, 27, 23, 26, 21, 28, 24, 25};
int new_pos_black_figure[16] = {81, 82, 83, 84, 85, 86, 87, 88, 72, 77, 73, 76, 71, 78, 74, 75};
#endif

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

int ftime_ok = 0;

inline int get_ms()
{
    struct timeb timebuffer;
    ftime(&timebuffer);
    if (timebuffer.millitm != 0)
        ftime_ok = TRUE;
    return (timebuffer.time * 1000) + timebuffer.millitm;
}

inline void checkup() {
    if ((max_time != 0 && get_ms() >= stop_time) || stop_received == TRUE) {
        stop_search = TRUE;
        longjmp(env, 0);
    }
}

int str2move(char move_old[6]) {
    int x_from, y_from, x_to, y_to, move_hi, move_lo;
    x_from = move_old[0] - 'a';
    y_from = move_old[1] - '1';
    x_to   = move_old[2] - 'a';
    y_to   = move_old[3] - '1';
    move_hi = x_from * 32 + y_from * 4;
    move_lo = x_to * 32 + y_to * 4;
    switch (move_old[4]) {
    case 'q': case 'Q': move_hi += 2; break;
    case 'r': case 'R': move_hi += 1; break;
    case 'b': case 'B': move_lo += 2; break;
    case 'n': case 'N': move_lo += 1; break;
    default: break;
    }
    return move_hi * 256 + move_lo;
}

void move2str(int move) {
    strcpy(move_str,"     ");
    move_str[0] = (move & 0xe000) / 256 / 32 + 'a';
    move_str[1] = (move & 0x1c00) / 256 /  4 + '1';
    move_str[2] = (move & 0x00e0) % 256 / 32 + 'a';
    move_str[3] = (move & 0x001c) % 256 /  4 + '1';
    if ((move & 0x0303) > 0) {
        if ((move & 0x0200) == 0x0200) move_str[4]='q';
        else if ((move & 0x0100) == 0x0100) move_str[4]='r';
        else if ((move & 0x0002) == 0x0002) move_str[4]='b';
        else if ((move & 0x0001) == 0x0001) move_str[4]='n';
    }
}

//Resets the parameters and the table
void reset_movelist() {
    int i;
    for (move_number = 0 ; move_number < 255 ; move_number++) {
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
        for ( i = 0; i < 120; i++ ) tablelist[move_number][i] = new_table[i];
    }
    move_number = 0;
}

//Prints the table and the parameters to the console
void print_table()
{
    int i, j, field;
    for ( i = 11; i >= 0; i-- ) {
        for ( j = 0; j < 10; j++) {
            field = ( tablelist[move_number][ i * 10 + j ] );
            if ( field == 255 ) {
                continue;
            }
            printf( "%3x", field );
        }
        printf("\n");
    }
    printf("Move number    : %d\n", move_number);
    printf("Move color     : %d\n", movelist[move_number].color);
    printf("Move from      : %d\n", movelist[move_number].move_from);
    printf("Move to        : %d\n", movelist[move_number].move_to);
    printf("Captured       : %d\n", movelist[move_number].captured_figure);
    printf("Promotion      : %d\n", movelist[move_number].promotion);
    printf("Castle         : %d\n", movelist[move_number].castle);
    printf("Not Pawn Move  : %d\n", movelist[move_number].not_pawn_move);
    printf("En passant     : %d\n", movelist[move_number].en_passant);
    printf("White castled  : %d\n", movelist[move_number].white_king_castled);
    printf("Black castled  : %d\n", movelist[move_number].black_king_castled);
    printf("Pos White king : %d\n", movelist[move_number].pos_white_king);
    printf("Pos Black king : %d\n", movelist[move_number].pos_black_king);
    printf("White2bishops  : %d\n", movelist[move_number].white_double_bishops);
    printf("Black2bishops  : %d\n", movelist[move_number].black_double_bishops);
    printf("Further invest.: %d\n", movelist[move_number].further);
    printf("\n");
    flush();
}

void print_hash(unsigned long long hash, int dpt) {
    hash_index = hash % HASHSIZE;
    //printf("Hash index: %llu, ", hash_index);
    printf("Hash: %llu, ", hash);
    printf("value: %d, ", (hashtable + hash_index) -> u);
    printf("dpt: %d\n", dpt);
    flush();
}
void print_hash_inner(unsigned long long hash, int dpt) {
    hash_index = hash % HASHSIZE_INNER;
    if ((hashtable_inner + hash_index) -> move != 0)
	move2str((hashtable_inner + hash_index) -> move);
    //printf("Hash index: %llu, ", hash_index);
    printf("Hash: %llu, ", hash);
    printf("move: %s, ", move_str);
    printf("value: %d, ", (hashtable_inner + hash_index) -> u);
    printf("depth: %d, ", (hashtable_inner + hash_index) -> depth);
    printf("init_depth: %d, ", init_depth);
    printf("dpt: %d\n", dpt);
    flush();
}

//Returns if the figure of color is attacked or not
int is_attacked(int field, int color) {
    int k, kk, attacking_figure, coord, color_offset;
    
    int *ptablelist;
    ptablelist = tablelist[move_number];
    if (color == WHITE) color_offset = 128; else color_offset = 0;
    for (k = 0; k < 4; ++k) {
        kk = 1;
        while (*(ptablelist + field + kk * dir_rook[k]) == 0) ++kk;
        coord = field + kk * dir_rook[k];
        if (*(ptablelist + coord) == Queen + color_offset) return TRUE;
        if (*(ptablelist + coord) == Rook  + color_offset) return TRUE;
        if (kk == 1 && *(ptablelist + coord) == King + color_offset) return TRUE;
    }
    for (k = 0; k < 4; ++k) {
        kk = 1;
        while (*(ptablelist + field + kk * dir_bishop[k]) == 0) ++kk;
        coord = field + kk * dir_bishop[k];
        if (*(ptablelist + coord) == Queen  + color_offset) return TRUE;
        if (*(ptablelist + coord) == Bishop + color_offset) return TRUE;
        if (kk == 1 && *(ptablelist + coord) == King + color_offset) return TRUE;
    }
    for (k = 0; k < 8; ++k)
	if (*(ptablelist + field + dir_knight[k]) == Knight + color_offset)
	    return TRUE;
    attacking_figure = Pawn;
    if (color == WHITE) {
        attacking_figure += 128;
        if (*(ptablelist + field + 9) == attacking_figure ||
	    *(ptablelist + field + 11) == attacking_figure)
	    return TRUE;
    } else  //Black
        if (*(ptablelist + field - 9) == attacking_figure ||
	    *(ptablelist + field - 11) == attacking_figure)
	    return TRUE;
    return FALSE;
}

//Set the hash variable of the current position
void set_hash() {
    int i, k;
    int field, figure;
    hash = 0;
    // XOR the side to move
    if (player_to_move == WHITE) {
        hash ^= hash_side_white;
    }
    else {
        hash ^= hash_side_black;
    }
    // XOR the figures;
    for (k = 0; k < 120; k++) {
        field = tablelist[move_number][k];
        if (field > 0 && field < 255) {
            figure = (field & 127);
            if ((field & 128) == 128) i = 1; else i = 0;
            hash ^= hash_piece[i][figure][k];
        }
    }
    // XOR the enpassant position
    hash ^= hash_enpassant[movelist[move_number].en_passant];
    // XOR the castling possibilities
    hash ^= hash_castle[movelist[move_number].castle];
}

//Set the hash_inner variable of the current position
void set_hash_inner() {
    int i, k;
    int field, figure;
    hash_inner = 0;
    // XOR the side to move
    if (player_to_move == WHITE) {
        hash_inner ^= hash_side_white;
    }
    else {
        hash_inner ^= hash_side_black;
    }
    // XOR the figures;
    for (k = 0; k < 120; k++) {
        field = tablelist[move_number][k];
        if (field > 0 && field < 255) {
            figure = (field & 127);
            if ((field & 128) == 128) i = 1; else i = 0;
            hash_inner ^= hash_piece[i][figure][k];
        }
    }
    // XOR the enpassant position
    hash_inner ^= hash_enpassant[movelist[move_number].en_passant];
    // XOR the castling possibilities
    hash_inner ^= hash_castle[movelist[move_number].castle];
}

// Updates the parameters /array movelist/ and the table with the given move.
// If print is true -> prints the table
void update_table(int move, int print) {
    int n, x_from, y_from, x_to, y_to;
    int figure_from, figure_to, square_from, square_to;
    int *pt1, *pt2;
    char promotion = ' ';
    //Copies the table for the next move and updates later according to the move
    pt1 = tablelist[move_number];
    pm1 = movelist + move_number;
    ++move_number;
    pt2 = tablelist[move_number];
    pm2 = movelist + move_number;
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
    pm2 -> color = player_to_move;
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
        if (player_to_move == BLACK) {
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
        if (move_number == 1) {
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
        if (player_to_move == WHITE && y_from > 3) pm2 -> further = 2;
        if (player_to_move == BLACK && y_from < 4) pm2 -> further = 2;
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
    if (is_attacked(player_to_move == WHITE ? pm2 -> pos_black_king :
		    pm2 -> pos_white_king, -player_to_move) == TRUE) {
        pm2 -> further = 2;
    }
    if (print == TRUE) print_table();
}

//Inverts who the next player is
inline void invert_player_to_move() {
    player_to_move = -player_to_move;
}

//Function to calculate x vector from direction (k) for move notation
inline int convA(int k) {
    if (k == -21) return -1;
    if (k == -19) return  1;
    if (k == -12) return -2;
    if (k == -11) return -1;
    if (k == -10) return  0;
    if (k ==  -9) return  1;
    if (k ==  -8) return  2;
    if (k ==  -1) return -1;
    if (k ==   1) return  1;
    if (k ==   8) return -2;
    if (k ==   9) return -1;
    if (k ==  10) return  0;
    if (k ==  11) return  1;
    if (k ==  12) return  2;
    if (k ==  19) return -1;
    if (k ==  21) return  1;
    return 0;
}

//Function to calculate y vector from direction (k) for move notation
inline int conv0(int k) {
    if (k == -21) return -2;
    if (k == -19) return -2;
    if (k == -12) return -1;
    if (k == -11) return -1;
    if (k == -10) return -1;
    if (k ==  -9) return -1;
    if (k ==  -8) return -1;
    if (k ==  -1) return  0;
    if (k ==   1) return  0;
    if (k ==   8) return  1;
    if (k ==   9) return  1;
    if (k ==  10) return  1;
    if (k ==  11) return  1;
    if (k ==  12) return  1;
    if (k ==  19) return  2;
    if (k ==  21) return  2;
    return 0;
}

//Bonus for king's adjacent own pawns
inline int evaluation_king(int field, int figure) {
    int e = 0;
    int k;
    pt = tablelist[move_number];
    pdir = dir_king;
    for (k = 0; k < 8; ++k, ++pdir)
	if (*(pt + field + *pdir) == (figure & 128) + Pawn)
	    e += friendly_pawn;
    return e;
}

inline int evaluation_pawn(int field, int figure, int sm) {
    int e = 0;
    //punishing double pawns
    int dir = 0;
    pt = tablelist[move_number];
    do {
        dir += 10;
        if (*(pt + field + dir) == figure) e -= double_pawn;
    } while (*(pt + field + dir) != 255);
    dir = 0;
    do {
        dir -= 10;
        if (*(pt + field + dir) == figure) e -= double_pawn;
    } while (*(pt + field + dir) != 255);
    //Bonus for pawn advantage
    if (sm < 2000) {
        if ((figure & 128) == 0) {
            e += (field / 10) * pawn_advantage;
        }
        else {
            e += (11 -(field / 10)) * pawn_advantage;
        }
    }
    return e;
}

//Evaluates material and king position, pawn structure, etc
inline int evaluation_material(int dpt) {
    int i, c, e, figure;
    int evaking;
    e = 0;
    pt = tablelist[move_number];
    //Calculate summa of material for end game threshold
    //sm = sum_material(player_to_move);
    random_window = 10;
    //Goes through the table
    for (i = 0; i < 120; ++i) {
        figure = *(pt + i);
        if (figure > 0 && figure < 255) {
            //c=1 : own figure is found, c=-1 opposite figure is found
            if ((player_to_move == WHITE && (figure & 128) == 0) ||
		(player_to_move == BLACK && (figure & 128) == 128)) c = 1; else c = -1;
            //Evaulates King position (friendly pawn structure)
            if ((figure & 127) == King && move_number > 6) e += c * evaluation_king(i, figure);
            //Evaulates Pawn structures
            if ((figure & 127) == Pawn) e += c * evaluation_pawn(i, figure, sm);
            //Calculates figure material values
            e += c * figure_value[(figure & 127)];
        }
    }
    if (player_to_move == WHITE) c = 1; else c = -1;
    //Bonus for castling
    e += c * ((movelist + move_number) -> white_king_castled - (movelist + move_number) -> black_king_castled);
    //Bonus for double bishops
    e += c * ((movelist + move_number) -> white_double_bishops - (movelist + move_number) -> black_double_bishops);
    //In the end game bonus if the enemy king is close
    if (sm < end_game_threshold) {
        random_window = 2;
        evaking  = 3 * abs(((movelist + move_number) -> pos_white_king / 10) - ((movelist + move_number) -> pos_black_king / 10));
        evaking += 3 * abs(((movelist + move_number) -> pos_white_king % 10) - ((movelist + move_number) -> pos_black_king % 10));
        if ((dpt % 2) == 0) {
            e +=  evaking;
        }
        else {
            e -= evaking;
        }
        if ((player_to_move == WHITE) && ((dpt % 2) == 1))
            evaking =  5 * (abs(5 - ((movelist + move_number) -> pos_black_king / 10)) + abs(5 - ((movelist + move_number) -> pos_black_king % 10)));
        if ((player_to_move == WHITE) && ((dpt % 2) == 0))
            evaking = -5 * (abs(5 - ((movelist + move_number) -> pos_white_king / 10)) + abs(5 - ((movelist + move_number) -> pos_white_king % 10)));
        if ((player_to_move == BLACK) && ((dpt % 2) == 0))
            evaking = -5 * (abs(5 - ((movelist + move_number) -> pos_black_king / 10)) + abs(5 - ((movelist + move_number) -> pos_black_king % 10)));
        if ((player_to_move == BLACK) && ((dpt % 2) == 1))
            evaking =  5 * (abs(5 - ((movelist + move_number) -> pos_white_king / 10)) + abs(5 - ((movelist + move_number) -> pos_white_king % 10)));
        e += evaking;
        //printf("player_to_move: %d, dpt: %d, pos_white_king: %d, pos_black_king: %d, evaking: %d\n", player_to_move, dpt, pm -> pos_white_king, pm -> pos_black_king, evaking);flush();
    } else {
#ifdef CASTLING_PUNISHMENT
        //Punishment if can not castle
        if ((movelist + move_number) -> white_king_castled == 0 &&
	    ((movelist + move_number) -> castle & 3) == 0) {
            e += c * cant_castle;
        }
        if ((movelist + move_number) -> black_king_castled == 0 &&
	    ((movelist + move_number) -> castle & 12) == 0) {
            e += c * cant_castle;
        }
#endif
    }
    return e;
}

//Checks weather the move is legal. If the king is attacked then not legal.
//Decreases the legal pointer -> does not store the move
void is_really_legal() {
    update_table(legal_moves[legal_pointer], FALSE);
    if (is_attacked(player_to_move == WHITE ? (movelist + move_number) -> pos_white_king :
		    (movelist + move_number) -> pos_black_king, player_to_move) == TRUE) {
        --legal_pointer;
    }
#ifdef SORT_ALFARRAY
    else
        if (global_dpt != 1 && sort_alfarray == TRUE) {
            //printf("EVA\n");flush();
            eva_alfabeta_temp[legal_pointer] = evaluation_material(global_dpt);
        }
#endif
    --move_number;
}

//Co-function of append_legal_moves()
inline void append_legal_moves_inner(int dir_piece) {
    ++legal_pointer;
    int move = 0;
    move |= (j - 1) * 0x2000;
    move |= (i - 2) * 0x0400;
    move |= (j - 1 + kk * convA(dir_piece)) * 0x0020;
    move |= (i - 2 + kk * conv0(dir_piece)) * 0x0004;
    //move=(j-1)*0x2000 + (i-2)*0x0400 + (j - 1 + kk * convA(dir_piece)) * 0x0020 +(i - 2 + kk * conv0(dir_piece)) * 0x0004;
    legal_moves[legal_pointer] = move;
    is_really_legal();
}

//Tries if a particular move is legal
inline void append_legal_moves(int dir_piece) {
    //dir_piece : direction of the move
    //kk : distance
    //No more moves in that direction if field is not empty
    int tmp;
    tmp = i * 10 + j + kk * dir_piece;
    if (*(pt + tmp) != 0) end_direction = TRUE;
    if (player_to_move == WHITE) {
        //Tries move if field occupied by black or empty
        if (*(pt + tmp) > 128 || *(pt + tmp) == 0) {
            append_legal_moves_inner(dir_piece);
        }
    }
    if (player_to_move == BLACK) {
        //Tries move if field occupied by white or empty
        if (*(pt + tmp) < 128) {
            append_legal_moves_inner(dir_piece);
        }
    }
}

//Checks castlings and adds to legal moves if possible
void castling() {
    if (player_to_move == WHITE) {
        //Checks the conditions of castling
        //e1g1
        if (tablelist[move_number][28] == WhiteRook)
            if (tablelist[move_number][25] == WhiteKing)
                if ((movelist[move_number].castle & 1) == 1)
                    if (is_attacked(25, WHITE) == FALSE)
                        if (is_attacked(26, WHITE) == FALSE)
                            if (is_attacked(27, WHITE) == FALSE)
                                if (tablelist[move_number][26] == 0)
                                    if (tablelist[move_number][27] == 0)  {
                                        ++legal_pointer;
                                        legal_moves[legal_pointer] = 0x80c0;
                                        is_really_legal();
                                    }
        //e1c1
        if (tablelist[move_number][25] == WhiteKing)
            if (tablelist[move_number][21] == WhiteRook)
                if ((movelist[move_number].castle & 2) == 2)
                    if (is_attacked(25, WHITE) == FALSE)
                        if (is_attacked(24, WHITE) == FALSE)
                            if (is_attacked(23, WHITE) == FALSE)
                                if (tablelist[move_number][24] == 0)
                                    if (tablelist[move_number][23] == 0)
                                        if (tablelist[move_number][22] == 0) {
                                            ++legal_pointer;
                                            legal_moves[legal_pointer] = 0x8040;
                                            is_really_legal();
                                        }
    } else
        //if (player_to_move == BLACK)
	{
	    if ((movelist[move_number].castle & 4) == 4)
		if (is_attacked(95, BLACK) == FALSE)
		    if (is_attacked(96, BLACK) == FALSE)
			if (is_attacked(97, BLACK) == FALSE)
			    if (tablelist[move_number][96] == 0)
				if (tablelist[move_number][97] == 0)
				    if (tablelist[move_number][98] == BlackRook)
					if (tablelist[move_number][95] == BlackKing) {
					    ++legal_pointer;
					    //e8g8
					    legal_moves[legal_pointer] = 0x9cdc;
					    is_really_legal();
					}
	    //e8c8
	    if (tablelist[move_number][95] == BlackKing)
		if (tablelist[move_number][91] == BlackRook)
		    if ((movelist[move_number].castle & 8) == 8)
			if (is_attacked(95, BLACK) == FALSE)
			    if (is_attacked(94, BLACK) == FALSE)
				if (is_attacked(93, BLACK) == FALSE)
				    if (tablelist[move_number][94] == 0)
					if (tablelist[move_number][93] == 0)
					    if (tablelist[move_number][92] == 0) {
						++legal_pointer;
						legal_moves[legal_pointer] = 0x9c5c;
						is_really_legal();
					    }
	}
}

//Searches and stores all the legal moves
void list_legal_moves() {
    int field;
    int move;
    //int coord;
    int en_pass;
    //legal_pointer = -1 means no legal moves
    legal_pointer= -1;
    //Maps the table
    pt = tablelist[move_number];
    ptt = pt;
    --ptt;
    for ( i = 0; i < 12; i++ ) {
        for (j = 0; j < 10; j++) {
            ++ptt;
            //coord = i * 10 + j;
            field = *ptt;
            //255 = border of the table
            if ( field == 255 ) {
                continue;
            }
            //figure without color
            figure = (field & 127);

	    // Right color found
            if (( player_to_move == WHITE && ((field & 128) == 0) ) ||
		( player_to_move == BLACK && ((field & 128) == 128 ))) {
                if (field == WhitePawn) {
		    
                    //If upper field is empty
                    if (*(ptt + 10) == 0) {
			
                        //If white pawn is in the 7th rank -> promotion
                        if (i - 1 == 7) {
                            ++legal_pointer;
			    
                            //Calculates only with Queen promotion
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j - 1) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        } else {
			    
                            //Normal pawn move
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j - 1) * 0x0020;
                            move |= (i - 1) * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
			
                        //If white pawn is in the 2nd rank
                        if (i - 1 == 2) if (*(ptt + 20) == 0) {
				++legal_pointer;
				move = 0;
				move |= (j - 1) * 0x2000;
				move |= 0x0400;
				move |= (j - 1) * 0x0020;
				move |= 3 * 0x0004;
				legal_moves[legal_pointer] = move;
				is_really_legal();
			    }
                    }
		    
                    //Pawn capture
                    if ((*(ptt + 9) & 128) == 128 && *(ptt + 9) != 255) {
			
                        //With Queen promotion
                        if (i - 1 == 7) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        } else {
			    
                            //Normal capture
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= (i - 1) * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    }
		    
                    //Pawn capture of the other direction
                    if ((*(ptt + 11) & 128) == 128 && *(ptt + 11) != 255) {
			
                        //With Queen promotion
                        if (i - 1 == 7) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        } else {
			    
                            //Normal capture
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= (i - 1) * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    }
		    
                    //If en passant is possible
                    en_pass = (movelist + move_number) -> en_passant;
                    if (en_pass > 1)
                        //If it is the right field
                        if (en_pass == i * 10 + j + 9) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 4 * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= 5 * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    if (en_pass > 1)
			
                        //If it is the right field
                        if (en_pass == i * 10 + j + 11) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 4 * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= 5 * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    continue;
                }
		
                //The same for black pawn
                if (field == BlackPawn) {
                    if (*(ptt - 10) == 0) {
                        if (i - 1 == 2) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 1 * 0x0400;
                            move |= (j - 1) * 0x0020;
                            //move |= (i - 3) * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        } else {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j - 1) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                        if (i - 1 == 7) if (*(ptt - 20) == 0) {
				++legal_pointer;
				move = 0;
				move |= (j - 1) * 0x2000;
				move |= 6 * 0x0400;
				move |= (j - 1) * 0x0020;
				move |= 4 * 0x0004;
				legal_moves[legal_pointer] = move;
				is_really_legal();
			    }
                    }
                    if (*(ptt - 9) > 0 && *(ptt - 9) < 128) {
                        if (i - 1 == 2) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        } else {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    }
                    if (*(ptt - 11) > 0 && *(ptt - 11) < 128) {
                        if (i - 1 == 2) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        } else {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    }
                    en_pass = (movelist + move_number) -> en_passant;
                    if (en_pass > 1)
                        if (en_pass == i * 10 + j - 9) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 3 * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= 2 * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    if (en_pass > 1)
                        if (en_pass == i * 10 + j - 11) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 3 * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= 2 * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    continue;
                }
                if (figure == Knight) {
		    
                    //kk : distance
                    //k : number of directions of possible knight moves
                    kk = 1;
                    for (k = 0; k < 8; ++k) {
                        if (*(ptt + *(dir_knight + k)) != 255) {
                            append_legal_moves(*(dir_knight + k));
                        }
                    }
                    continue;
                }
                if (figure == King) {
		    
                    //Appends castling moves if possible
                    castling();
                    kk = 1;
                    for (k = 0; k < 8; ++k) {
                        if (*(ptt + *(dir_king + k)) != 255) {
                            append_legal_moves(*(dir_king + k));
                        }
                    }
                    continue;
                }
                if (figure == Queen) {
                    for (k = 0; k < 8; ++k) {
                        kk = 1;
                        end_direction = FALSE;
			
                        //Increases kk while queen can move in that direction
                        while (end_direction == FALSE && *(ptt + kk * (*(dir_king + k))) < 255) {
                            append_legal_moves(*(dir_king + k));
                            ++kk;
                        }
                    }
                    continue;
                }
                if (figure == Bishop) {
                    for (k = 0; k < 4; ++k) {
                        kk = 1;
                        end_direction = FALSE;
                        while (end_direction == FALSE && *(ptt + kk * (*(dir_bishop + k))) < 255) {
                            append_legal_moves(*(dir_bishop + k));
                            ++kk;
                        }
                    }
                    continue;
                }
                if (figure == Rook) {
                    for (k = 0; k < 4; ++k) {
                        kk = 1;
                        end_direction = FALSE;
                        while (end_direction == FALSE && *(ptt + kk * (*(dir_rook + k))) < 255) {
                            append_legal_moves(*(dir_rook + k));
                            ++kk;
                        }
                    }
                    continue;
                }
            }
        }
    }
}

//Searches and stores all the legal moves
void list_legal_moves_old() {
    int field;
    int move;
    int coord;
    int en_pass;
    //legal_pointer = -1 means no legal moves
    legal_pointer= -1;
    //Maps the table
    pt = tablelist[move_number];
    for ( i = 0; i < 12; i++ ) {
        for (j = 0; j < 10; j++) {
            coord = i * 10 + j;
            field = *(pt + coord);
            //255 = border of the table
            if ( field == 255 ) {
                continue;
            }
            //figure without color
            figure = (field & 127);
            if (( player_to_move == WHITE && ((field & 128) == 0) ) ||
		( player_to_move == BLACK && ((field & 128) == 128 ))) { // Right color found
                if (field == WhitePawn) {
                    //If upper field is empty
                    if (*(pt + coord + 10) == 0) {
                        //If white pawn is in the 7th rank -> promotion
                        if (i - 1 == 7) {
                            ++legal_pointer;
                            //Calculates only with Queen promotion
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j - 1) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        } else {
                            //Normal pawn move
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j - 1) * 0x0020;
                            move |= (i - 1) * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                        //If white pawn is in the 2nd rank
                        if (i - 1 == 2) if (*(pt + coord + 20) == 0) {
				++legal_pointer;
				move = 0;
				move |= (j - 1) * 0x2000;
				move |= 0x0400;
				move |= (j - 1) * 0x0020;
				move |= 3 * 0x0004;
				legal_moves[legal_pointer] = move;
				is_really_legal();
			    }
                    }
                    //Pawn capture
                    if ((*(pt + coord + 9) & 128) == 128 && *(pt + coord + 9) != 255) {
                        //With Queen promotion
                        if (i - 1 == 7) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        } else {
                            //Normal capture
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= (i - 1) * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    }
                    //Pawn capture of the other direction
                    if ((*(pt + coord + 11) & 128) == 128 && *(pt + coord + 11) != 255) {
                        //With Queen promotion
                        if (i - 1 == 7) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        } else {
                            //Normal capture
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= (i - 1) * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    }
                    //If en passant is possible
                    en_pass = movelist[move_number].en_passant;
                    if (en_pass > 1)
                        //If it is the right field
                        if (en_pass == coord + 9) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 4 * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= 5 * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    if (en_pass > 1)
                        //If it is the right field
                        if (en_pass == coord + 11) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 4 * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= 5 * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    continue;
                }
                //The same for black pawn
                if (field == BlackPawn) {
                    if (*(pt + coord - 10) == 0) {
                        if (i - 1 == 2) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 1 * 0x0400;
                            move |= (j - 1) * 0x0020;
                            //move |= (i - 3) * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        } else {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j - 1) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                        if (i - 1 == 7) if (*(pt + coord - 20) == 0) {
				++legal_pointer;
				move = 0;
				move |= (j - 1) * 0x2000;
				move |= 6 * 0x0400;
				move |= (j - 1) * 0x0020;
				move |= 4 * 0x0004;
				legal_moves[legal_pointer] = move;
				is_really_legal();
			    }
                    }
                    if (*(pt + coord - 9) > 0 && *(pt + coord - 9) < 128) {
                        if (i - 1 == 2) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        } else {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    }
                    if (*(pt + coord - 11) > 0 && *(pt + coord - 11) < 128) {
                        if (i - 1 == 2) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        } else {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    }
                    en_pass = movelist[move_number].en_passant;
                    if (en_pass > 1)
                        if (en_pass == coord - 9) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 3 * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= 2 * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    if (en_pass > 1)
                        if (en_pass == coord - 11) {
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 3 * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= 2 * 0x0004;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();
                        }
                    continue;
                }
                if (figure == Knight) {
                    //kk : distance
                    //k : number of directions of possible knight moves
                    kk = 1;
                    for (k = 0; k < 8; ++k) {
                        if (*(pt + coord + *(dir_knight + k)) != 255) {
                            append_legal_moves(*(dir_knight + k));
                        }
                    }
                    continue;
                }
                if (figure == King) {
                    //Appends castling moves if possible
                    castling();
                    kk = 1;
                    for (k = 0; k < 8; ++k) {
                        if (*(pt + coord + *(dir_king + k)) != 255) {
                            append_legal_moves(*(dir_king + k));
                        }
                    }
                    continue;
                }
                if (figure == Queen) {
                    for (k = 0; k < 8; ++k) {
                        kk = 1;
                        end_direction = FALSE;
                        //Increases kk while queen can move in that direction
                        while (end_direction == FALSE && *(pt + coord + kk * (*(dir_king + k))) < 255) {
                            append_legal_moves(*(dir_king + k));
                            ++kk;
                        }
                    }
                    continue;
                }
                if (figure == Bishop) {
                    for (k = 0; k < 4; ++k) {
                        kk = 1;
                        end_direction = FALSE;
                        while (end_direction == FALSE && *(pt + coord + kk * (*(dir_bishop + k))) < 255) {
                            append_legal_moves(*(dir_bishop + k));
                            ++kk;
                        }
                    }
                    continue;
                }
                if (figure == Rook) {
                    for (k = 0; k < 4; ++k) {
                        kk = 1;
                        end_direction = FALSE;
                        while (end_direction == FALSE && *(pt + coord + kk * (*(dir_rook + k))) < 255) {
                            append_legal_moves(*(dir_rook + k));
                            ++kk;
                        }
                    }
                    continue;
                }
            }
        }
    }
}

//Calculates material for evaluating end game threshold
inline int sum_material(int color) {
    int i, figure, e;
    e = 0;
    pt = tablelist[move_number];
    for (i = 0; i < 120; ++i) {
        figure = *(pt + i);
        if (figure > 0 && figure < 255) {
            if ((color == WHITE && (figure & 128) == 0) ||
		(color == BLACK && (figure & 128) == 128))
                e += figure_value[(figure & 127)];
        }
    }
    return e;
}

//Sorts legal moves
inline void calculate_evarray() {
    int i, j, v;
    int tempmove;
    //pl = legal_moves;
    if (init_depth == 1)
        for (i = 0; i <= legal_pointer; i++/*, ++pl*/) {
            (root_moves + i) -> move = legal_moves[i];
            (root_moves + i) -> value = 0;
        }
    else {
        for (i = 0; i <= legal_pointer; i++)
            for (j = i + 1; j <= legal_pointer; j++)
                if (root_moves[j].value > root_moves[i].value) {
                    tempmove = root_moves[j].move;
                    root_moves[j].move = root_moves[i].move;
                    root_moves[i].move = tempmove;
                    v = root_moves[j].value;
                    root_moves[j].value = root_moves[i].value;
                    root_moves[i].value = v;
                }
    }
}

inline int evaluation_only_end_game(int dpt) {
    invert_player_to_move();
    list_legal_moves();
    invert_player_to_move();
    if (legal_pointer == -1) { //No legal move
        if (is_attacked(player_to_move == WHITE ? (movelist + move_number) -> pos_black_king :
			(movelist + move_number) -> pos_white_king, -player_to_move) == FALSE) {
	    
            //Stalemate
            //printf("DRAW: ");flush();
            return DRAW;
        }
        else {
	    
            //Mate
            //printf("WON: ");flush();
            return WON - dpt;
        }
    }
    return 32767; //not end
}

//Evaluates material plus number of legal moves of both sides plus a random number
inline int evaluation(int e_legal_pointer, int dpt) {
    int lp;
    //int e=0;
    lp = e_legal_pointer;
    invert_player_to_move();
    list_legal_moves();
    //legal_pointer=1;
    e_legal_pointer -= legal_pointer;
    legal_pointer = lp;
    invert_player_to_move();
    if (legal_pointer == -1) { //No legal move
        if (is_attacked(player_to_move == WHITE ? (movelist + move_number) -> pos_black_king :
			(movelist + move_number) -> pos_white_king, -player_to_move) == FALSE) {
            //printf("DRAW: ");flush();
            return DRAW;
        }
        else {
            //printf("WON: ");flush();
            //dpt : #1 (checkmate in one move) is better than #2
            return WON - dpt;
        }
    }
    //Adds random to evaluation
    random_number = (rand() % random_window);
    //random_number = 0;
    //return evaluation_material(dpt);
    return evaluation_material(dpt) + 1 * e_legal_pointer + random_number;
}

//Checks not enough material
inline int not_enough_material() {
    int i, c;
    int white_knight = 0;
    int white_bishop = 0;
    int black_knight = 0;
    int black_bishop = 0;
    for (i = 0; i < 120; ++i) {
        c = tablelist[move_number][i];
        if (c == 0 || c == 255) continue;
        if (c == WhitePawn)  return FALSE;
        if (c == BlackPawn)  return FALSE;
        if (c == WhiteQueen) return FALSE;
        if (c == BlackQueen) return FALSE;
        if (c == WhiteRook)  return FALSE;
        if (c == BlackRook)  return FALSE;
        if (c == WhiteKnight) {
            ++white_knight;
            continue;
        }
        if (c == BlackKnight) {
            ++black_knight;
            continue;
        }
        if (c == WhiteBishop) {
            ++white_bishop;
            continue;
        }
        if (c == BlackBishop) {
            ++black_bishop;
            continue;
        }
    }
    if (white_bishop == 2) return FALSE;
    if (black_bishop == 2) return FALSE;
    if (white_knight + white_bishop > 1) return FALSE;
    if (black_knight + black_bishop > 1) return FALSE;
    //printf("not enough material\n");flush();
    return TRUE;
}

//Checks third occurance
inline int third_occurance() {
    int i;
    int j;
    int equal;
    int occurance = 0;
    if (move_number < 6) return FALSE;
    i = move_number - 2;
    while (i >= 0 && occurance < 2) {
        //printf("Third occurance: i: %d\n", i);flush();
        equal = TRUE;
        for (j = 0; j < 120; ++j) {
            if (tablelist[move_number][j] != tablelist[i][j]) {
                equal = FALSE;
                break;
            }
        }
        if (equal == TRUE) {
            ++occurance;
        }
        i -= 2;
    }
    if (occurance >= 2) return TRUE; else return FALSE;
}

int alfabeta(int dpt, int alfa, int beta) {
    int i, nbr_legal, value, u;
    int b;
    int uu; // Evaluation if checkmate is found
    int alfarray[255];
    value = -22767;
    //if (dpt >=depth) printf("info depth %d seldepth %d\n", dpt, seldepth);flush();
#ifdef HASH_INNER
    hash_index = hash_inner % HASHSIZE_INNER;
    
    // if this position is in the hashtable
    if ( (hashtable_inner + hash_index) -> lock != hash_inner &&
	 (hashtable_inner + hash_index) -> lock != 0)
	hash_collision_inner++;
    if ( (hashtable_inner + hash_index) -> lock == hash_inner) {
        if (dpt > init_depth) depth_inner = 0;
	else depth_inner = init_depth - dpt;
        if ((hashtable_inner + hash_index) -> depth > depth_inner) {
            hash_inner_nodes++;
            printf("##HASH_INNER FOUND##");
	    print_hash_inner(hash_inner, dpt);
            return (hashtable_inner + hash_index) -> u;
        }
    }
#endif
    list_legal_moves();
    //printf("nbr_legal: %d\n", nbr_legal);
    if (legal_pointer == -1) {
        if (is_attacked(player_to_move == WHITE ? (movelist + move_number) -> pos_white_king :
			(movelist + move_number) -> pos_black_king, player_to_move) == FALSE) {
            //printf("DRAW: ");flush();
            return DRAW;
        }
        else {
            //printf("LOST: ");flush();
            return LOST;
        }
    }
    nbr_legal = legal_pointer;

    // Sorts legal moves
    if (dpt == 1) {
        calculate_evarray();
        for (i = 0; i <= legal_pointer; i++) alfarray[i] = root_moves[i].move;
    }
    else {
        for (i = 0; i <= nbr_legal; ++i) alfarray[i] = legal_moves[i];
    }
    for (i = 0; i <= nbr_legal; ++i) {
        ++nodes;
        if ((nodes & 1023) == 0) checkup();
        if (dpt > depth) curr_depth = depth; else curr_depth = dpt;
        curr_seldepth = dpt;
        if (dpt == 1) {
            move2str(alfarray[i]);
            printf("info currmove %s currmovenumber %d\n", move_str, i + 1);flush();
        }
        update_table(alfarray[i], FALSE);
        curr_line[dpt] = alfarray[i];
        //printf("\ncurrent line:");for (b=1; b<=dpt; ++b) { move2str(curr_line[b]); printf("%s ", move_str); } printf("");flush();

	// If last ply -> evaluating
        if ((dpt >= depth && movelist[move_number].further == 0) ||
	    dpt >= seldepth) {
            last_ply = TRUE;
#ifdef HASH
            set_hash();
            hash_index = hash % HASHSIZE;

	    // if this position is in the hashtable
            if ( (hashtable + hash_index) -> lock != hash &&
		 (hashtable + hash_index) -> lock != 0)
		hash_collision++;
            if ( (hashtable + hash_index) -> lock == hash) {
                u = (hashtable + hash_index) -> u;
                --move_number;
                hash_nodes++;
                //printf("##Last Ply HASH FOUND##"); print_hash(hash, dpt);
            }
	    
            //not in the hashtable. normal evaluating
            else {
#endif
                if (third_occurance() == TRUE) {
                    u=DRAW;
                    --move_number;
                }
                else
                    if (not_enough_material() == TRUE) {
                        u=DRAW;
                        --move_number;
                    }
                    else
                        if (movelist[move_number].not_pawn_move >= 100) {
                            //printf("Fifty pawn move (last ply): move_number: %d, not_pawn_move: %d\n ", move_number, movelist[move_number].not_pawn_move);
                            u=DRAW;
                            --move_number;
                        }
                        else {
                            list_legal_moves();
                            //legal_pointer=1;
                            u=evaluation(legal_pointer, dpt);
                            //for (b=1; b<=curr_seldepth; ++b) { move2str(curr_line[b]); printf("%s ", move_str); } printf(" u: %d\n", u);flush();
                            --move_number;
                        }
#ifdef HASH
                //if this position is not in the hashtable -> insert it to hashtable
                (hashtable + hash_index) -> lock = hash;
                (hashtable + hash_index) -> u = u;
                //printf("last ply, insert into hash: "); print_hash(hash, dpt);
            }
#endif
        }
        //Not last ply
        else {
            if (third_occurance() == TRUE) {
                u = DRAW;
                --move_number;
            } else
                if (not_enough_material() == TRUE) {
                    u = DRAW;
                    --move_number;
                } else
                    if (movelist[move_number].not_pawn_move >= 100) {
                        //printf("Fifty pawn move: move_number: %d, not_pawn_move: %d\n", move_number, movelist[move_number].not_pawn_move);
                        u = DRAW;
                        --move_number;
                    }
                    else {
                        u = evaluation_only_end_game(dpt);
                        if (u == 32767) { //not end
#ifdef HASH_INNER
                            set_hash_inner();
#endif
                            invert_player_to_move();
                            u = -alfabeta(dpt + 1, -beta, -alfa);
                            last_ply = FALSE;
                            invert_player_to_move();
                        }
                        --move_number;
#ifdef HASH_INNER
                        set_hash_inner();
#endif
                    }
        }
        if (dpt == 1) {
            root_moves[i].move  = alfarray[i];
            root_moves[i].value = u;
        }
	
        // Better move is found
        if (u > value) {
            //printf("dpt: %d, %d > %d\n", dpt, u, value);flush();
            value = u;
            for(b = 1; b <= curr_seldepth; b++)
		best_line[dpt].moves[b] = curr_line[b];
            best_line[dpt].value = u;
            if (last_ply == TRUE)
		best_line[dpt].length = dpt;
            if (last_ply == FALSE) {
                best_line[dpt].length = best_line[dpt + 1].length;
                best_line[dpt].value = best_line[dpt + 1].value;
                for (b = 1; b <= best_line[dpt + 1].length; b++)
		    best_line[dpt].moves[b] = best_line[dpt + 1].moves[b];
            }
#ifdef HASH_INNER
	    
            // if this position is not in the hashtable -> insert it to hashtable
            hash_index = hash_inner % HASHSIZE_INNER;
            (hashtable_inner + hash_index) -> lock = hash_inner;
            (hashtable_inner + hash_index) -> u = u;
            if (dpt < init_depth)
		(hashtable_inner + hash_index) -> depth = init_depth - dpt;
            else (hashtable_inner + hash_index) -> depth = 0;
            (hashtable_inner + hash_index) -> move = curr_line[dpt];
            //print_hash_inner(hash_inner, dpt);
#endif
            //printf("Best line[%d], value: %d, moves: ", dpt, best_line[dpt].value);for (b=1; b<=best_line[dpt].length; ++b) { move2str(best_line[dpt].moves[b]); printf("%s ", move_str); } printf("\n");flush();
            if (dpt == 1) {
                best_move = alfarray[i];
#ifdef HASH
                printf("Hash found %d, inner found %d times, hash collision: %d, has collision_inner: %d, hash/nodes: %d%%\n",
		       hash_nodes, hash_inner_nodes, hash_collision, hash_collision_inner, 100 * hash_nodes/nodes);flush();
#endif
                // If checkmate is found
                if (abs(u) > 20000) {
                    if (u > 0) uu = (22001 - u) / 2;
		    else uu = - (u + 22001) / 2;
                    if (uu == 0 && u > 0)
			uu =  1;
                    if (uu == 0 && u < 0)
			uu = -1;
                    printf("info multipv 1 depth %d seldepth %d score mate %d nodes %d pv ",
			   curr_depth, curr_seldepth, uu, nodes);flush();
                    for (b = 1; b <= best_line[dpt].length; ++b) {
			move2str(best_line[dpt].moves[b]);
			printf("%s ", move_str);
		    }
		    printf("\n");flush();
                    mate_score = abs(u);
                } else {
                    printf("info multipv 1 depth %d seldepth %d score cp %d nodes %d pv ",
			   curr_depth, curr_seldepth, u, nodes);flush();
                    for (b = 1; b <= best_line[dpt].length; ++b) {
			move2str(best_line[dpt].moves[b]);
			printf("%s ", move_str);
		    }
		    printf("\n");flush();
                    mate_score = 0;
                }
            }
        }
#ifdef ALFABETA
	
        // Alfa Beta cut-off
        if (value >= beta) return value;
        if (value > alfa) alfa=value;
#endif
    }
    return value;
}

void make_move_uci() {
    int a = 0;
    int alfa = -22767;
    int beta =  22767;
    int time_elapsed;
    int time_current_depth_start, time_current_depth_stop, time_remaining;
    sort_alfarray = FALSE;
    nodes = 0;
    hash_nodes = 0;
    hash_inner_nodes = 0;
    hash_collision = 0;
    hash_collision_inner = 0;
    (void) time(&t1);
    //max_time = 20 * 1000;
    start_time = get_ms();
    stop_time = start_time + max_time;
    init_depth = 1;
    //gui_depth = 2; max_time=0;
    stop_search = FALSE;
    for (;;) {
#ifdef HASH_INNER
        set_hash_inner();
        print_hash_inner(hash_inner, 0);
#endif
        //Calculate summa of material for end game threshold
        sm = sum_material(player_to_move);
        depth = init_depth;
	/*
        seldepth = depth + 5;
        if (depth > 1) seldepth = depth + 4;
        if (depth > 2) seldepth = depth + 4;
        if (depth > 3) seldepth = depth + 8;
	*/
        seldepth = depth + 8;
        printf("%d %d\n", depth, seldepth);flush();
        //Calculates the time of the move
        //Searches the best move with negamax algorithm with alfa-beta cut off
        //mate_score = 0;
        (void) time(&t1_last);
        time_current_depth_start = get_ms();
        printf("info depth %d\n", init_depth);flush();
        a = alfabeta(1, alfa, beta);
        setjmp(env);
        if (stop_search == TRUE) {
            for (i = 0; i < curr_seldepth - 1; i++) --move_number;
        }
        (void) time(&t2_last);
        time_elapsed = get_ms() - start_time;
        time_current_depth_stop = get_ms();
        time_remaining = stop_time - time_current_depth_stop;
        //If there is no time for another depth search
        if (movetime == 0 && max_time != 0)
	    if ((time_current_depth_stop - time_current_depth_start) * 5 > time_remaining)
		stop_search = TRUE;
        if (init_depth > 30)
	    stop_search = TRUE;
        knodes = 1000LLU * nodes;
        printf("info depth %d seldepth %d time %d nodes %d nps %lld\n",
	       init_depth, seldepth, time_elapsed, nodes,
	       (time_elapsed==0)?0:(knodes/time_elapsed));flush();
        printf("nodes: %d, knodes: %lld\n", nodes, knodes);flush();
        if (DEBUG) {
            debugfile=fopen("./debug.txt", "a");
            fprintf(debugfile, "<- info depth %d seldepth %d time %d nodes %d nps %d\n",
		    init_depth, seldepth, time_elapsed, nodes,
		    (time_elapsed==0)?0:(int)(1000*nodes/time_elapsed));
            fclose(debugfile);
        }
        //Prints statistics
        //printf("alfabeta: %d\n", a);flush();
        //printf("best %s\n", best_move);flush();
        best_iterative[init_depth] = best_move;
        if (stop_search == TRUE) break;
        if (mate_score > 20000) break;
        if (init_depth == gui_depth) break;
        //if (legal_pointer == 0) break; //there is only one legal move
        ++init_depth;
    }
    (void) time(&t2);
    move2str(best_move);
    printf( "\nbestmove %s\n", move_str);flush();
#ifdef HASH
    printf("Hash found %d, inner found %d times\n", hash_nodes, hash_inner_nodes);flush();
#endif
    update_table(best_move, FALSE); //Update the table without printing it
    invert_player_to_move();
}

void* make_move(void *threadid) {
    if (strstr(engine, "uci")) make_move_uci();
}

void Chess::start_game() { // new
    reset_movelist();
    player_to_move = WHITE;
    //print_table();
}

// FEN interpreter
void setboard(char fen_position[255]) {
    unsigned int n = 13;
    int x = 1;
    int y = 9;
    int m, j;
    int factor;
    char move_old[6]="     ";
    start_game();
    m = 5;
    for (j = 0; j < m; j++) {
        while (fen_position[n] != ' ') {
            n++;
        }
        n++;
    }
    factor = 1;
    move_number = 0;
    while (fen_position[n] != ' ' && n < strlen(input) - 1) {
	/*
        printf("move_number: %d, factor: %d, n: %d, fen_position[n]: %d, strlen(input): %d\n",
	       move_number, factor, n, (int)(fen_position[n] - '0'), strlen(input));
	*/
        fflush(stdout);
        move_number = move_number * factor + (int)(fen_position[n] - '0');
        factor =factor * 10;
        n++;
    }
    n = 13;
    while (fen_position[n] != ' ') {
        if (fen_position[n] > '0' && fen_position[n] < '9') {
            for(m = 0; m < (int)(fen_position[n] - '0'); m++) {
                tablelist[move_number][y * 10 + x] = 0;
                x++;
            }
            --x;
        }
        if (fen_position[n] != '/') {
            for (m = 0; m < 14; m++) {
                if (fen_position[n] == graphical_figure[m][1]) {
                    tablelist[move_number][y * 10 + x]=graphical_figure[m][0];
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
        player_to_move = WHITE;
        FZChess = WHITE;
        movelist[move_number].color = WHITE;
    } else {
        player_to_move = BLACK;
        FZChess = BLACK;
        movelist[move_number].color = BLACK;
    }
    n++;
    n++;
    movelist[move_number].castle = 0;
    while  (fen_position[n] != ' ') {
        if (fen_position[n] == '-') movelist[move_number].castle = 0;
        if (fen_position[n] == 'K') movelist[move_number].castle=movelist[move_number].castle | 1;
        if (fen_position[n] == 'Q') movelist[move_number].castle=movelist[move_number].castle | 2;
        if (fen_position[n] == 'k') movelist[move_number].castle=movelist[move_number].castle | 4;
        if (fen_position[n] == 'q') movelist[move_number].castle=movelist[move_number].castle | 8;
        n++;
    }
    movelist[move_number - 1].castle = movelist[move_number].castle;
    n++;
    if (fen_position[n] == '-') movelist[move_number].en_passant = 0;
    if (fen_position[n] >= 'a' && fen_position[n] <= 'h') { // en passant possible, target square
        movelist[move_number].en_passant = 1 + fen_position[n] - 'a' + (fen_position[n+1] - '1' + 2) * 10;
        n++;
    }
    n++;
    n++;
    movelist[move_number].not_pawn_move = 0;
    factor = 1;
    while (fen_position[n] != ' ') {
        movelist[move_number].not_pawn_move=movelist[move_number].not_pawn_move * factor + (int)(fen_position[n] - '0');
        factor = factor * 10;
        n++;
    }
    for (i = 0; i < 120; ++i) if ((tablelist[move_number][i] & 127) == King) {
	    if (tablelist[move_number][i] == WhiteKing) movelist[move_number].pos_white_king = i;
	    if (tablelist[move_number][i] == BlackKing) movelist[move_number].pos_black_king = i;
	}
    if (strstr(input, "moves")) {
        m = strlen(input) - 1;
        for (i = (int) (strstr(input, "moves") - input + 6); i < m; i++) {
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

void position_received() {
    int i;
    int m;
    char move_old[6];
    strcpy(move_old,"     ");
    start_game();
    player_to_move = WHITE;
    if (! strstr(input, "move")) return;
    m = strlen(input) - 1;
    for (i = 24; i < m; i++) {
        move_old[0] = input[i];
        i++;
        move_old[1] = input[i];
        i++;
        move_old[2] = input[i];
        i++;
        move_old[3] = input[i];
        i++;
        if (input[i] != ' ' && input[i] != '\n') {
            move_old[4] = input[i];
            i++;
        } else {
            move_old[4] = '\0';
        }
        update_table(str2move(move_old), FALSE);
        invert_player_to_move();
    }
    //print_table();
}

void process_input() {
    //printf("Process input: %s\n", input);flush();
    if (DEBUG) {
        debugfile = fopen("./debug.txt", "w");
        fclose(debugfile);
        if ( ! strstr(input,"quit") ) {
            debugfile = fopen("./debug.txt", "a");
            fprintf(debugfile, "-> %s", input);
            fclose(debugfile);
        }
    }
    if (strstr(input, "isready")) {
        printf("readyok\n");flush();
    }
    if (strstr(input, "position startpos")) {
        position_received();
    }
    if (strstr(input, "position fen")) setboard(input);
}

unsigned long long rand64() {
    unsigned long long output = 9999999999999999999LLU;
    output = rand();
    output <<= 15;
    output ^= rand();
    output <<= 15;
    output ^= rand();
    output <<= 15;
    output ^= rand();
    output <<= 15;
    output ^= rand();
    return output;
}

unsigned long long hash_rand() {
    int i;
    unsigned long long r = 0LLU;
    for (i = 0; i < 32; i++) r ^= rand64() << i;
    return r;
}

void init_hash() {
    int i, j, k;
    srand(0);
    //WHITE: i = 0, BLACK: i = 1
    for (i = 0; i < 2; i++)
        //PAWN = 1, KNIGHT = 2, ..., KING = 7
        for (j = 1; j < 7; j++)
            for (k = 0; k < 120; k++)
                hash_piece[i][j][k] = hash_rand();
    hash_side_white = hash_rand();
    hash_side_black = hash_rand();
    for (i = 0; i < 120; i++) hash_enpassant[i] = hash_rand();
    for (i = 0; i < 15; i++) hash_castle[i] = hash_rand();
    if ((hashtable = (hash_t*)malloc(sizeof(hash_t[HASHSIZE]))) == NULL) {
        printf("HASH memory fault!\n");
        exit(-2);
    }
}

void init_hash_inner() {
    int i, j, k;
    srand(0);
    //WHITE: i = 0, BLACK: i = 1
    for (i = 0; i < 2; i++)
        //PAWN = 1, KNIGHT = 2, ..., KING = 7
        for (j = 1; j < 7; j++)
            for (k = 0; k < 120; k++)
                hash_piece[i][j][k] = hash_rand();
    hash_side_white = hash_rand();
    hash_side_black = hash_rand();
    for (i = 0; i < 120; i++) hash_enpassant[i] = hash_rand();
    for (i = 0; i < 15; i++) hash_castle[i] = hash_rand();
    if ((hashtable_inner = (hash_inner_t*)
	 malloc(sizeof(hash_inner_t[HASHSIZE_INNER]))) == NULL) {
        printf("HASH_INNER memory fault!\n");
        exit(-2);
    }
}

