#include "Chess.h"
#include <cstring>
#include <cstdio>
#include <cmath>

#define HASH
#define ALFABETA
//#define SORT_ALFARRAY

using namespace std;

enum figures { PAWN1, PAWN2, PAWN3, PAWN4, PAWN5, PAWN6, PAWN7, PAWN8,
    KNIGHT1, KNIGHT2, BISHOP1, BISHOP2, ROOK1, ROOK2, QUEEN, KING};

struct best_lines {
    int length;
    int value;
    int moves[1000];
} best_line[99];

Chess::Chess() {
#ifdef HASH
    hash = new Hash();
#endif
#ifdef HASH_INNER
    hash->init_hash_inner();
#endif
    uci = new Uci(this);
    table = new Table(this);
    stop_received = false;
    DEBUG = 0;
    WHITE = 1;
    BLACK = -1;
}

Chess::~Chess() {
#ifdef HASH
    delete hash;
#endif
    delete uci;
    delete table;
}

void Chess::start_game() { // new
    table->reset_movelist();
    player_to_move = WHITE;
    //table->print_table();
}

//Inverts who the next player is
void Chess::invert_player_to_move() {
    player_to_move = -player_to_move;
}

int Chess::alfabeta(int dpt, int alfa, int beta) {
    int i, nbr_legal, value, u;
    int b;
    int uu; // Evaluation if checkmate is found
    int alfarray[255];
    value = -22767;

    //if (dpt >=depth) printf("info depth %d seldepth %d\n", dpt, seldepth);Util::flush();
#ifdef HASH_INNER
    hash_index = hash_inner % HASHSIZE_INNER;

    // if this position is in the hashtable
    if ( (hashtable_inner + hash_index)->lock != hash_inner &&
            (hashtable_inner + hash_index)->lock != 0)
        hash_collision_inner++;
    if ( (hashtable_inner + hash_index)->lock == hash_inner) {
        if (dpt > init_depth) depth_inner = 0;
        else depth_inner = init_depth - dpt;
        if ((hashtable_inner + hash_index)->depth > depth_inner) {
            hash_inner_nodes++;
            printf("##HASH_INNER FOUND##");
            print_hash_inner(hash_inner, dpt);
            return (hashtable_inner + hash_index)->u;
        }
    }
#endif
    list_legal_moves();
    //printf("nbr_legal: %d\n", nbr_legal);
    if (legal_pointer == -1) {
        if (table->is_attacked(player_to_move == WHITE ? (movelist + move_number)->pos_white_king :
                    (movelist + move_number)->pos_black_king, player_to_move) == false) {
            //printf("DRAW: ");Util::flush();
            return table->eval->DRAW;
        }
        else {
            //printf("LOST: ");Util::flush();
            return table->eval->LOST;
        }
    }
    nbr_legal = legal_pointer;

    // Sorts legal moves
    if (dpt == 1) {
        calculate_evarray();
        for (i = 0; i <= legal_pointer; i++) {
            alfarray[i] = root_moves[i].move;
            printf("(%s:%d) ", Util::move2str(move_str, root_moves[i].move), root_moves[i].value);
        }
        puts("");
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
            printf("info currmove %s currmovenumber %d\n", Util::move2str(move_str, alfarray[i]), i + 1);Util::flush();
        }
        table->update_table(alfarray[i], false);
        curr_line[dpt] = alfarray[i];

        // If last ply->evaluating
        if ((dpt >= depth && movelist[move_number].further == 0) ||
                dpt >= seldepth) {
            last_ply = true;
            n++;
#ifdef HASH
            hash->set_hash(this);
            if (hash->posInHashtable()) {
                u = hash->getU();
                --move_number;
                hash->hash_nodes++;
            }

            //not in the hashtable. normal evaluating
            else {
#endif
                if (third_occurance() == true ||
                        table->not_enough_material() == true ||
                        movelist[move_number].not_pawn_move >= 100) {
                    u = table->eval->DRAW;
                    --move_number;
                } else {
                    list_legal_moves();
                    //legal_pointer=1;
                    u = table->eval->evaluation(legal_pointer, dpt);
                    //u=evaluation_material(dpt);
                    --move_number;
                }
#ifdef HASH
                //if this position is not in the hashtable->insert it to hashtable
                hash->setU(u);
            }
#endif
        }
        //Not last ply
        else {
            if (third_occurance() == true ||
                    table->not_enough_material() == true ||
                    movelist[move_number].not_pawn_move >= 100) {
                u = table->eval->DRAW;
                --move_number;
            } else {
                u = table->eval->evaluation_only_end_game(dpt);
                if (u == 32767) { //not end
                    /*
                       hash->set_hash(this);
                       if (hash->posInHashtable()) {
                       u = hash->getU();
                       hash->hash_inner_nodes++;
                       } else {
                       */
                    invert_player_to_move();
                    u = -alfabeta(dpt + 1, -beta, -alfa);
                    last_ply = false;
                    invert_player_to_move();
                    //}
                }
                --move_number;
            }
        }
        if (dpt == 1) {
            root_moves[i].move  = alfarray[i];
            root_moves[i].value = u;
        }

        // Better move is found
        if (u > value) {
            //printf("dpt: %d, %d > %d\n", dpt, u, value);Util::flush();
            value = u;
            for(b = 1; b <= curr_seldepth; b++)
                best_line[dpt].moves[b] = curr_line[b];
            best_line[dpt].value = u;
            if (last_ply == true) {
                best_line[dpt].length = dpt;
            }
            if (last_ply == false) {
                best_line[dpt].length = best_line[dpt + 1].length;
                best_line[dpt].value = best_line[dpt + 1].value;
                for (b = 1; b <= best_line[dpt + 1].length; b++)
                    best_line[dpt].moves[b] = best_line[dpt + 1].moves[b];
                //printf("best_line[%d].length: %d\n", dpt, best_line[dpt].length);Util::flush();
            }
#ifdef HASH_INNER

            // if this position is not in the hashtable->insert it to hashtable
            hash_index = hash_inner % HASHSIZE_INNER;
            (hashtable_inner + hash_index)->lock = hash_inner;
            (hashtable_inner + hash_index)->u = u;
            if (dpt < init_depth)
                (hashtable_inner + hash_index)->depth = init_depth - dpt;
            else (hashtable_inner + hash_index)->depth = 0;
            (hashtable_inner + hash_index)->move = curr_line[dpt];
            //print_hash_inner(hash_inner, dpt);
#endif
            if (dpt == 1) {
                best_move = alfarray[i];
#ifdef HASH
                hash->printStatistics(nodes);
#endif
                // If checkmate is found
                if (abs(u) > 20000) {
                    if (u > 0) uu = (22001 - u) / 2;
                    else uu = - (u + 22001) / 2;
                    if (uu == 0 && u > 0)
                        uu =  1;
                    if (uu == 0 && u < 0)
                        uu = -1;
                    printf("info multipv 1 depth %d seldepth %d score mate %d nodes %llu pv ",
                            curr_depth, curr_seldepth, uu, nodes);
                    for (b = 1; b <= best_line[dpt].length; ++b) {
                        printf("%s ", Util::move2str(move_str, best_line[dpt].moves[b]));
                    }
                    printf("\n");Util::flush();
                    mate_score = abs(u);
                } else {
                    printf("info multipv 1 depth %d seldepth %d score cp %d nodes %llu pv ",
                            curr_depth, curr_seldepth, u, nodes);
                    for (b = 1; b <= best_line[dpt].length; ++b) {
                        printf("%s ", Util::move2str(move_str, best_line[dpt].moves[b]));
                    }
                    printf("\n");Util::flush();
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

int Chess::perft(int dpt) {
    int i, nbr_legal;
    int alfarray[255];
    unsigned long long nodes = 0;

    if (dpt == 0) return 1;

    list_legal_moves();
    nbr_legal = legal_pointer;
    for (i = 0; i <= nbr_legal; ++i) alfarray[i] = legal_moves[i];
    for (i = 0; i <= nbr_legal; ++i) {
        if ((nodes & 1023) == 0) checkup();
            table->update_table(alfarray[i], false);
            invert_player_to_move();
            nodes += perft(dpt - 1);
            invert_player_to_move();
            --move_number;
    }
    return nodes;
}

//Searches and stores all the legal moves
void Chess::list_legal_moves() {
    int field;
    int figure;
    int move;
    //int coord;
    int en_pass;
    //legal_pointer = -1 means no legal moves
    legal_pointer= -1;
    //Maps the table
    pt = tablelist[move_number];
    ptt = pt;
    --ptt;
    for (int i = 0; i < 12; i++ ) {
        for (int j = 0; j < 10; j++) {
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
                if (field == table->WhitePawn) {

                    //If upper field is empty
                    if (*(ptt + 10) == 0) {

                        //If white pawn is in the 7th rank->promotion
                        if (i - 1 == 7) {

                            //Calculates Queen promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j - 1) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                            //Calculates Rook promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j - 1) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0100;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                            //Calculates Bishop promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j - 1) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0002;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                            //Calculates Knight promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j - 1) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0001;
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

                        //With Rook promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0100;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                        //With Bishop promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0002;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                        //With Knight promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0001;
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

                        if (i - 1 == 7) {

                            //With Queen promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                            //With Rook promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0100;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                            //With Bishop promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0002;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                            //With Knight promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 6 * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= 7 * 0x0004;
                            move |= 0x0001;
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
                    en_pass = (movelist + move_number)->en_passant;
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
                if (field == table->BlackPawn) {
                    if (*(ptt - 10) == 0) {
                        if (i - 1 == 2) {

                            // With Queen promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 1 * 0x0400;
                            move |= (j - 1) * 0x0020;
                            //move |= (i - 3) * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                            // With Rook promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 1 * 0x0400;
                            move |= (j - 1) * 0x0020;
                            //move |= (i - 3) * 0x0004;
                            move |= 0x0100;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                            // With Bishop promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 1 * 0x0400;
                            move |= (j - 1) * 0x0020;
                            //move |= (i - 3) * 0x0004;
                            move |= 0x0002;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                            // With Knight promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= 1 * 0x0400;
                            move |= (j - 1) * 0x0020;
                            //move |= (i - 3) * 0x0004;
                            move |= 0x00001;
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

                            // With Queen promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                            // With Rook promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            move |= 0x0100;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                            // With Bishop promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            move |= 0x0002;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                            // With Knight promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j    ) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            move |= 0x0001;
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

                            // With Queen promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            move |= 0x0200;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                            // With Rook promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            move |= 0x0100;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                            // With Bishop promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            move |= 0x0002;
                            legal_moves[legal_pointer] = move;
                            is_really_legal();

                            // With Knight promotion
                            ++legal_pointer;
                            move = 0;
                            move |= (j - 1) * 0x2000;
                            move |= (i - 2) * 0x0400;
                            move |= (j - 2) * 0x0020;
                            move |= (i - 3) * 0x0004;
                            move |= 0x0001;
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
                    en_pass = (movelist + move_number)->en_passant;
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
                int kk;
                if (figure == table->Knight) {

                    //kk : distance
                    //k : number of directions of possible knight moves
                    kk = 1;
                    for (int k = 0; k < 8; ++k) {
                        if (*(ptt + *(table->dir_knight + k)) != 255) {
                            append_legal_moves(*(table->dir_knight + k), i, j, kk);
                        }
                    }
                    continue;
                }
                if (figure == table->King) {

                    //Appends castling moves if possible
                    table->castling();
                    kk = 1;
                    for (int k = 0; k < 8; ++k) {
                        if (*(ptt + *(table->dir_king + k)) != 255) {
                            append_legal_moves(*(table->dir_king + k), i, j, kk);
                        }
                    }
                    continue;
                }
                if (figure == table->Queen) {
                    for (int k = 0; k < 8; ++k) {
                        kk = 1;
                        end_direction = false;

                        //Increases kk while queen can move in that direction
                        while (end_direction == false && *(ptt + kk * (*(table->dir_king + k))) < 255) {
                            append_legal_moves(*(table->dir_king + k), i, j, kk);
                            ++kk;
                        }
                    }
                    continue;
                }
                if (figure == table->Bishop) {
                    for (int k = 0; k < 4; ++k) {
                        kk = 1;
                        end_direction = false;
                        while (end_direction == false && *(ptt + kk * (*(table->dir_bishop + k))) < 255) {
                            append_legal_moves(*(table->dir_bishop + k), i, j, kk);
                            ++kk;
                        }
                    }
                    continue;
                }
                if (figure == table->Rook) {
                    for (int k = 0; k < 4; ++k) {
                        kk = 1;
                        end_direction = false;
                        while (end_direction == false && *(ptt + kk * (*(table->dir_rook + k))) < 255) {
                            append_legal_moves(*(table->dir_rook + k), i, j, kk);
                            ++kk;
                        }
                    }
                    continue;
                }
            }
        }
    }
}

void Chess::make_move() {
    unsigned long long knodes = 9999999999999999999LLU;
    int alfa = -22767;
    int beta =  22767;
    int time_elapsed;
    int time_current_depth_start, time_current_depth_stop, time_remaining;
    sort_alfarray = false;
    nodes = 0;
#ifdef HASH
    hash->reset_counters();
#endif
    //max_time = 20 * 1000;
    start_time = Util::get_ms();
    stop_time = start_time + max_time;
    init_depth = 1;
    //gui_depth = 2; max_time=0;
    stop_search = false;
    for (;;) {
#ifdef HASH
        hash->hashes.clear();
#endif
#ifdef HASH_INNER
        set_hash_inner();
        print_hash_inner(hash_inner, 0);
#endif
        //Calculate summa of material for end game threshold
        sm = sum_material(player_to_move);
        depth = init_depth;
        seldepth = depth + 8;
        if (depth > 4) seldepth = depth + 4;
        //seldepth = depth + 0;
        printf("%d %d\n", depth, seldepth);Util::flush();

        //Calculates the time of the move
        //Searches the best move with negamax algorithm with alfa-beta cut off
        //mate_score = 0;
        time_current_depth_start = Util::get_ms();
        printf("info depth %d\n", init_depth);Util::flush();
        alfabeta(1, alfa, beta);
        setjmp(env);
        if (stop_search == true) {
            for (int i = 0; i < curr_seldepth - 1; i++) --move_number;
        }
        time_elapsed = Util::get_ms() - start_time;
        time_current_depth_stop = Util::get_ms();
        time_remaining = stop_time - time_current_depth_stop;

        //If there is no time for another depth search
        if (movetime == 0 && max_time != 0)
            if ((time_current_depth_stop - time_current_depth_start) * 5 > time_remaining)
                stop_search = true;
        if (init_depth > 30)
            stop_search = true;
        knodes = 1000LLU * nodes;
        printf("info depth %d seldepth %d time %d nodes %lld nps %lld\n",
                init_depth, seldepth, time_elapsed, nodes,
                (time_elapsed==0)?0:(knodes/time_elapsed));Util::flush();
        printf("nodes: %lld, knodes: %lld\n", nodes, knodes);Util::flush();
        if (DEBUG) {
            debugfile=fopen("./debug.txt", "a");
            fprintf(debugfile, "<- info depth %d seldepth %d time %d nodes %llu nps %d\n",
                    init_depth, seldepth, time_elapsed, nodes,
                    (time_elapsed==0)?0:(int)(1000*nodes/time_elapsed));
            fclose(debugfile);
        }
        //Prints statistics
        //printf("alfabeta: %d\n", a);Util::flush();
        //printf("best %s\n", best_move);Util::flush();
        best_iterative[init_depth] = best_move;
        if (stop_search == true) break;
        if (mate_score > 20000) break;
        if (init_depth == gui_depth) break;
        //if (legal_pointer == 0) break; //there is only one legal move
        ++init_depth;
    }
    printf( "\nbestmove %s\n", Util::move2str(move_str, best_move));Util::flush();
#ifdef HASH
    hash->printStatistics(nodes);
#endif
    table->update_table(best_move, false); //Update the table without printing it
    invert_player_to_move();
}

// Sorts legal moves
// Bubble sort
void Chess::calculate_evarray() {
    int i, j, v;
    int tempmove;
    if (init_depth == 1) {
        for (i = 0; i <= legal_pointer; i++) {
            (root_moves + i)->move = legal_moves[i];
            (root_moves + i)->value = 0;
        }
    } else {
        for (i = 0; i <= legal_pointer; i++) {
            for (j = i + 1; j <= legal_pointer; j++) {
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
    }
}

// Sorts legal moves
// Insertion sort
void Chess::calculate_evarray_new() {
    int i, j, v;
    int tempmove;
    if (init_depth == 1) {
        for (i = 0; i <= legal_pointer; i++) {
            (root_moves + i)->move = legal_moves[i];
            (root_moves + i)->value = 0;
        }
    } else {
        for (i = 1; i <= legal_pointer; i++) {
            for (j = i;
                    j > 0 && root_moves[j].value > root_moves[j-1].value;
                    j--) {
                tempmove = root_moves[j].move;
                root_moves[j].move = root_moves[i].move;
                root_moves[i].move = tempmove;
                v = root_moves[j].value;
                root_moves[j].value = root_moves[i].value;
                root_moves[i].value = v;
            }
        }
    }
}

void Chess::checkup() {
    if ((max_time != 0 && Util::get_ms() >= stop_time) || stop_received) {
        stop_search = true;
        longjmp(env, 0);
    }
}

//Checks third occurance
bool Chess::third_occurance() {
    int i;
    int j;
    int equal;
    int occurance = 0;
    if (move_number < 6) return false;
    i = move_number - 2;
    while (i >= 0 && occurance < 2) {
        //printf("Third occurance: i: %d\n", i);Util::flush();
        equal = true;
        for (j = 0; j < 120; ++j) {
            if (tablelist[move_number][j] != tablelist[i][j]) {
                equal = false;
                break;
            }
        }
        if (equal == true) {
            ++occurance;
        }
        i -= 2;
    }
    if (occurance >= 2) return true; else return false;
}



//Checks weather the move is legal. If the king is attacked then not legal.
//Decreases the legal pointer->does not store the move
void Chess::is_really_legal() {
    table->update_table(legal_moves[legal_pointer], false);
    if (table->is_attacked(player_to_move == WHITE ? (movelist + move_number)->pos_white_king :
                (movelist + move_number)->pos_black_king, player_to_move) == true) {
        --legal_pointer;
    }
#ifdef SORT_ALFARRAY
    else
        if (global_dpt != 1 && sort_alfarray == true) {
            //printf("EVA\n");Util::flush();
            eva_alfabeta_temp[legal_pointer] = evaluation_material(global_dpt);
        }
#endif
    --move_number;
}


//Co-function of append_legal_moves()
void Chess::append_legal_moves_inner(int dir_piece, int i, int j, int kk) {
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
void Chess::append_legal_moves(int dir_piece, int i, int j, int kk) {

    //dir_piece : direction of the move
    //kk : distance
    //No more moves in that direction if field is not empty
    int tmp;
    tmp = i * 10 + j + kk * dir_piece;
    if (*(pt + tmp) != 0) end_direction = true;
    if (player_to_move == WHITE) {

        //Tries move if field occupied by black or empty
        if (*(pt + tmp) > 128 || *(pt + tmp) == 0) {
            append_legal_moves_inner(dir_piece, i, j, kk);
        }
    }
    if (player_to_move == BLACK) {

        //Tries move if field occupied by white or empty
        if (*(pt + tmp) < 128) {
            append_legal_moves_inner(dir_piece, i, j, kk);
        }
    }
}


//Function to calculate x vector from direction (k) for move notation
int Chess::convA(int k) {
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
int Chess::conv0(int k) {
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

//Calculates material for evaluating end game threshold
int Chess::sum_material(int color) {
    int i, figure, e;
    e = 0;
    //    int* pt = tablelist + move_number;
    for (i = 0; i < 120; ++i) {
        figure = tablelist[move_number][i];
        if (figure > 0 && figure < 255) {
            if ((color == WHITE && (figure & 128) == 0) ||
                    (color == BLACK && (figure & 128) == 128))
                e += table->eval->figure_value[(figure & 127)];
        }
    }
    return e;
}


void Chess::processCommands(char* input) {
    if (strstr(input, "uci")) {
        uci->processCommands(input);
    }
}
