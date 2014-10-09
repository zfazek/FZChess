#include "Chess.h"
#include <cstring>
#include <cstdio>
#include <cmath>

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
    default_seldepth = 8;
    break_if_mate_found = true;
    //table->print_table();
}

//Inverts who the next player is
void Chess::invert_player_to_move() {
    player_to_move = -player_to_move;
}

void Chess::make_move() {
    unsigned long long knodes = 9999999999999999999LLU;
    int MAX = 22767;
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
        //Calculate summa of material for end game threshold
        sm = table->eval->sum_material(player_to_move);
        depth = init_depth;
        seldepth = depth + 8;
        if (depth > 4) seldepth = depth + 4;
        seldepth = depth + default_seldepth;
        printf("%d %d\n", depth, seldepth);Util::flush();

        //Calculates the time of the move
        //Searches the best move with negamax algorithm with alfa-beta cut off
        time_current_depth_start = Util::get_ms();
        printf("info depth %d\n", init_depth);Util::flush();
        alfabeta(1, -MAX, MAX);
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
        calculate_evarray_new();
        for (int i = 0; i < nof_legal; i++) {
            printf("(%s:%d) ", Util::move2str(move_str, root_moves[i].move), root_moves[i].value);
            if (i % 8 == 7) puts("");
        }
        puts("");
        //Prints statistics
        //printf("alfabeta: %d\n", a);Util::flush();
        //printf("best %s\n", best_move);Util::flush();
        best_iterative[init_depth] = best_move;
        if (stop_search == true) break;
        if (mate_score > 20000 && break_if_mate_found) break;
        if (init_depth == gui_depth) break;
        //if (legal_pointer == 0) break; //there is only one legal move
        //++init_depth;
        if (init_depth == 1) init_depth = 5;
    }
    printf( "\nbestmove %s\n", Util::move2str(move_str, best_move));Util::flush();
#ifdef HASH
    hash->printStatistics(nodes);
#endif
    table->update_table(best_move, false); //Update the table without printing it
    invert_player_to_move();
}

int Chess::alfabeta(int dpt, int alfa, int beta) {
    int i, nbr_legal, value, u;
    int b;
    int uu; // Evaluation if checkmate is found
    int alfarray[MAX_LEGAL_MOVES];
    value = -22767;

    //if (dpt >=depth) printf("info depth %d seldepth %d\n", dpt, seldepth);Util::flush();
    table->list_legal_moves();
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
        nof_legal = legal_pointer + 1;
        calculate_evarray();
        for (i = 0; i < nof_legal; i++) {
            alfarray[i] = root_moves[i].move;
        }
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
            //if (strcmp(Util::move2str(move_str, alfarray[i]), "d6d1 ") == 0) continue;
            printf("info currmove %s currmovenumber %d\n", Util::move2str(move_str, alfarray[i]), i + 1);Util::flush();
        }
        table->update_table(alfarray[i], false);
        curr_line[dpt] = alfarray[i];

        // ZOLI
        if (depth == 5 && dpt == 1) {
            //table->print_table();   
        }

        // If last ply->evaluating
        if ((dpt >= depth && movelist[move_number].further == 0) ||
                dpt >= seldepth) {
            last_ply = true;
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
                if (table->third_occurance() == true ||
                        table->not_enough_material() == true ||
                        movelist[move_number].not_pawn_move >= 100) {
                    u = table->eval->DRAW;
                    --move_number;
                } else {
                    table->list_legal_moves();
                    u = table->eval->evaluation(legal_pointer, dpt);
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
            if (table->third_occurance() == true ||
                    table->not_enough_material() == true ||
                    movelist[move_number].not_pawn_move >= 100) {
                u = table->eval->DRAW;
                --move_number;
            } else {
                u = table->eval->evaluation_only_end_game(dpt);
                if (u == 32767) { //not end
                    invert_player_to_move();
                    u = -alfabeta(dpt + 1, -beta, -alfa);
                    last_ply = false;
                    invert_player_to_move();
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
        if (value > alfa) alfa = value;
#endif
    } // for
    return value;
}

int Chess::perft(int dpt) {
    int i, nbr_legal;
    int alfarray[MAX_LEGAL_MOVES];
    unsigned long long nodes = 0;

    if (dpt == 0) return 1;

    table->list_legal_moves();
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

// Sorts legal moves
// Bubble sort
void Chess::calculate_evarray() {
    int i, j, v;
    int tempmove;
    if (init_depth == 1) {
        for (i = 0; i < nof_legal; i++) {
            (root_moves + i)->move = legal_moves[i];
            (root_moves + i)->value = 0;
        }
    } else {
        for (i = 0; i < nof_legal; i++) {
            for (j = i + 1; j < nof_legal; j++) {
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

void Chess::calculate_evarray_new() {
    int i, j, v;
    int tempmove;
    for (i = 0; i < nof_legal; i++) {
        for (j = i + 1; j < nof_legal; j++) {
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

/*
// Sorts legal moves
// Insertion sort
void Chess::calculate_evarray_new() {
    int v;
    int tempmove;
    for (int i = 1; i < nof_legal; i++) {
        for (int j = i;
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
*/

void Chess::checkup() {
    if ((max_time != 0 && Util::get_ms() >= stop_time) || stop_received) {
        stop_search = true;
        longjmp(env, 0);
    }
}

void Chess::processCommands(char* input) {
    if (strstr(input, "uci")) {
        uci->processCommands(input);
    }
}
