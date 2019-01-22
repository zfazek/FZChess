#include "Eval.h"
#include "Chess.h"
#include <cmath>

using namespace std;

Eval::Eval(Chess *ch) { chess = ch; }

Eval::~Eval() {}

// Evaluates material and king position, pawn structure, etc
int Eval::evaluation_material(int dpt) {
    int c, e;
    int evaking;
    e = 0;
    int *pt = chess->tablelist[chess->move_number];
    struct Chess::move *pm = chess->movelist + chess->move_number;

    // Calculate summa of material for end game threshold
    // sm = sum_material(player_to_move);
    random_window = 10;

    // Goes through the table
    for (int i = 0; i < 120; ++i) {
        int figure = *(pt + i);
        if (figure > 0 && figure < OFFBOARD) {
            // c=1 : own figure is found, c=-1 opposite figure is found
            if ((chess->player_to_move == chess->WHITE &&
                 (figure & 128) == 0) ||
                (chess->player_to_move == chess->BLACK &&
                 (figure & 128) == 128)) {
                c = 1;
            } else {
                c = -1;
            }

            // Evaulates King position (friendly pawn structure)
            if ((figure & 127) == chess->table->King &&
                chess->move_number > 6) {
                e += c * evaluation_king(i, figure);
            }

            // Evaulates Pawn structures
            if ((figure & 127) == chess->table->Pawn) {
                e += c * evaluation_pawn(i, figure, chess->sm);
            }

            // Calculates figure material values
            e += c * figure_value[(figure & 127)];
        }
    }

    if (chess->player_to_move == chess->WHITE) {
        c = 1;
    } else {
        c = -1;
    }

    // Bonus for castling
    e += c * (pm->white_king_castled - pm->black_king_castled);

    // Bonus for double bishops
    e += c * (pm->white_double_bishops - pm->black_double_bishops);

    // In the end game bonus if the enemy king is close
    if (chess->sm < end_game_threshold) {
        random_window = 2;
        evaking =
            3 * abs((pm->pos_white_king / 10) - (pm->pos_black_king / 10));
        evaking +=
            3 * abs((pm->pos_white_king % 10) - (pm->pos_black_king % 10));
        if ((dpt % 2) == 0) {
            e += evaking;
        } else {
            e -= evaking;
        }

        // force to the corner
        if ((chess->player_to_move == chess->WHITE) && ((dpt % 2) == 1)) {
            evaking = 5 * (abs(5 - (pm->pos_black_king / 10)) +
                           abs(5 - (pm->pos_black_king % 10)));
        } else if ((chess->player_to_move == chess->WHITE) &&
                   ((dpt % 2) == 0)) {
            evaking = -5 * (abs(5 - (pm->pos_white_king / 10)) +
                            abs(5 - (pm->pos_white_king % 10)));
        } else if ((chess->player_to_move == chess->BLACK) &&
                   ((dpt % 2) == 0)) {
            evaking = -5 * (abs(5 - (pm->pos_black_king / 10)) +
                            abs(5 - (pm->pos_black_king % 10)));
        } else if ((chess->player_to_move == chess->BLACK) &&
                   ((dpt % 2) == 1)) {
            evaking = 5 * (abs(5 - (pm->pos_white_king / 10)) +
                           abs(5 - (pm->pos_white_king % 10)));
        }
        e += evaking;
        // printf("player_to_move: %d, dpt: %d, pos_white_king: %d,
        // pos_black_king: %d, evaking: %d\n", player_to_move, dpt,
        // pm->pos_white_king, pm->pos_black_king, evaking);Util::flush();
    } else {
#ifdef CASTLING_PUNISHMENT

        // Punishment if can not castle
        if (pm->white_king_castled == 0 && (pm->castle & 3) == 0) {
            e += c * cant_castle;
        }
        if (pm->black_king_castled == 0 && (pm->castle & 12) == 0) {
            e += c * cant_castle;
        }
#endif
    }

    return e;
}

// Evaluates material plus number of legal moves of both sides plus a random
// number
int Eval::evaluation(int e_legal_pointer, int dpt) {
    int lp = e_legal_pointer;
    chess->invert_player_to_move();
    chess->table->list_legal_moves();
    // printf("lp own: %d, lp opposite: %d\n", lp, chess->legal_pointer);
    lp -= chess->legal_pointer;
    // if (dpt % 2 == 0) lp = -lp;
    chess->invert_player_to_move();
    if (chess->legal_pointer == -1) { // No legal move
        if (chess->table->is_attacked(
                chess->player_to_move == chess->WHITE
                    ? (chess->movelist + chess->move_number)->pos_black_king
                    : (chess->movelist + chess->move_number)->pos_white_king,
                -chess->player_to_move) == false) {
            // printf("DRAW: ");Util::flush();
            return DRAW;
        } else {
            // printf("WON: ");Util::flush();
            // dpt : #1 (checkmate in one move) is better than #2
            return WON - dpt;
        }
    }

    // Adds random to evaluation
    // random_number = (rand() % random_window);
    int random_number = 0;
    // return evaluation_material(dpt);
    return evaluation_material(dpt) + 2 * lp + random_number;
}

int Eval::evaluation_only_end_game(int dpt) {
    chess->invert_player_to_move();
    chess->table->list_legal_moves();
    chess->invert_player_to_move();
    if (chess->legal_pointer == -1) { // No legal move
        if (chess->table->is_attacked(
                chess->player_to_move == chess->WHITE
                    ? (chess->movelist + chess->move_number)->pos_black_king
                    : (chess->movelist + chess->move_number)->pos_white_king,
                -chess->player_to_move) == false) {
            // Stalemate
            // printf("DRAW: ");Util::flush();
            return DRAW;
        } else {
            // Mate
            // printf("WON: ");Util::flush();
            return WON - dpt;
        }
    }
    return 32767; // not end
}

// Bonus for king's adjacent own pawns
int Eval::evaluation_king(int field, int figure) {
    int e = 0;
    int *pt = chess->tablelist[chess->move_number];
    int *pdir = chess->table->dir_king;
    for (int k = 0; k < 8; ++k, ++pdir) {
        if (*(pt + field + *pdir) == (figure & 128) + chess->table->Pawn) {
            e += chess->table->eval->friendly_pawn;
        }
    }
    return e;
}

int Eval::evaluation_pawn(int field, int figure, int sm) {
    int e = 0;

    // punishing double pawns
    int dir = 0;
    int *pt = chess->tablelist[chess->move_number];
    do {
        dir += 10;
        if (*(pt + field + dir) == figure) {
            e += double_pawn;
        }
    } while (*(pt + field + dir) != OFFBOARD);
    dir = 0;
    do {
        dir -= 10;
        if (*(pt + field + dir) == figure) {
            e += double_pawn;
        }
    } while (*(pt + field + dir) != OFFBOARD);

    // Bonus for pawn advantage
    if (sm < 2000) {
        if ((figure & 128) == 0) {
            e += (field / 10) * pawn_advantage;
        } else {
            e += (11 - (field / 10)) * pawn_advantage;
        }
    }
    return e;
}

// Calculates material for evaluating end game threshold
int Eval::sum_material(int color) {
    int i, figure, e;
    e = 0;
    //    int* pt = tablelist + move_number;
    for (i = 0; i < 120; ++i) {
        figure = chess->tablelist[chess->move_number][i];
        if (figure > 0 && figure < OFFBOARD) {
            if ((color == chess->WHITE && (figure & 128) == 0) ||
                (color == chess->BLACK && (figure & 128) == 128)) {
                e += figure_value[(figure & 127)];
            }
        }
    }
    return e;
}
