#include "Chess.h"

#include <cstdio>
#include <cstring>

#include "Eval.h"
#include "Util.h"

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
    // table->print_table();
}

// Inverts who the next player is
void Chess::invert_player_to_move() {
    player_to_move = -player_to_move;
}

void Chess::make_move() {
    const int MAX = 22767;
    int time_elapsed;
    int time_current_depth_start, time_current_depth_stop, time_remaining;
    sort_alfarray = true;
    nodes = 0;
#ifdef HASH
    hash->reset_counters();
#endif
    // max_time = 20 * 1000;
    start_time = Util::get_ms();
    stop_time = start_time + max_time;
    depth = 1;
    // gui_depth = 2; max_time=0;
    stop_search = false;
    for (;;) {
#ifdef HASH
        hash->clear();
#endif
        // Calculate summa of material for end game threshold
        sm = table->eval->sum_material(player_to_move);
        seldepth = depth + 8;
        if (depth > 4) {
            seldepth = depth + 4;
        }
        seldepth = depth + default_seldepth;
        printf("%d %d\n", depth, seldepth);
        Util::flush();

        // Calculates the time of the move
        // Searches the best move with negamax algorithm with alfa-beta cut off
        time_current_depth_start = Util::get_ms();
        printf("info depth %d\n", depth);
        Util::flush();
        alfabeta(1, -MAX, MAX);
        setjmp(env);
        if (stop_search) {
            for (int i = 0; i < curr_seldepth - 1; i++) {
                --move_number;
            }
        }
        time_elapsed = Util::get_ms() - start_time;
        time_current_depth_stop = Util::get_ms();
        time_remaining = stop_time - time_current_depth_stop;

        // If there is no time for another depth search
        if (movetime == 0 && max_time != 0) {
            if ((time_current_depth_stop - time_current_depth_start) * 5 >
                time_remaining) {
                stop_search = true;
            }
        }
        if (depth > 30) {
            stop_search = true;
        }
        printf("info depth %d seldepth %d time %d nodes %ld nps %ld\n", depth,
               seldepth, time_elapsed, nodes,
               (time_elapsed == 0) ? 0 : (uint64_t)((1000.0 * nodes / time_elapsed)));
        Util::flush();
        if (DEBUG) {
            debugfile = fopen("./debug.txt", "a");
            fprintf(debugfile,
                    "<- info depth %d seldepth %d time %d nodes %lu nps %lu\n",
                    depth, seldepth, time_elapsed, nodes,
                    (time_elapsed == 0) ? 0 : (uint64_t)(1000 * nodes / time_elapsed));
            fclose(debugfile);
        }
        calculate_evarray_new();
        for (int i = 0; i < nof_legal_root_moves; i++) {
            printf("(%s:%d) ", Util::move2str(move_str, root_moves[i].move),
                   root_moves[i].value);
            if (i % 8 == 7) {
                puts("");
            }
        }
        puts("");
        // Prints statistics
        // printf("alfabeta: %d\n", a);Util::flush();
        // printf("best %s\n", best_move);Util::flush();
        best_iterative[depth] = best_move;
        if (stop_search) {
            break;
        }
        if (mate_score > 20000 && break_if_mate_found) {
            break;
        }
        if (depth == gui_depth) {
            break;
        }
        // if (legal_pointer == 0) break; //there is only one legal move
        ++depth;
        // if (depth == 1) depth = 5;
    }
    printf("\nbestmove %s\n", Util::move2str(move_str, best_move));
    Util::flush();
#ifdef HASH
    hash->printStatistics(nodes);
#endif
    // Update the table without printing it
    table->update_table(best_move, false);
    invert_player_to_move();
}

int Chess::alfabeta(int dpt, int alfa, int beta) {
    int u;
    int uu; // Evaluation if checkmate is found
    int alfarray[MAX_LEGAL_MOVES];
    int value = -22767;

    // if (dpt >=depth) printf("info depth %d seldepth %d\n", dpt, seldepth);Util::flush();
    table->list_legal_moves();
    if (legal_pointer == -1) {
        if (!table->is_attacked(player_to_move == WHITE
                                   ? (movelist + move_number)->pos_white_king
                                   : (movelist + move_number)->pos_black_king,
                               player_to_move)) {
            // printf("DRAW: ");Util::flush();
            return table->eval->DRAW;
        } else {
            // printf("LOST: ");Util::flush();
            return table->eval->LOST;
        }
    }
    const int nbr_legal = legal_pointer + 1;

    // Sorts legal moves
    if (dpt == 1) {
        nof_legal_root_moves = legal_pointer + 1;
        calculate_evarray();
        for (int i = 0; i < nof_legal_root_moves; ++i) {
            alfarray[i] = root_moves[i].move;
        }
    } else {
        for (int i = 0; i < nbr_legal; ++i) {
            alfarray[i] = legal_moves[i];
        }
    }
    for (int i = 0; i < nbr_legal; ++i) {
        ++nodes;
        if ((nodes & 1023) == 0) {
            checkup();
        }
        if (dpt > depth) {
            curr_depth = depth;
        } else {
            curr_depth = dpt;
        }
        curr_seldepth = dpt;
        if (dpt == 1) {
            // if (strcmp(Util::move2str(move_str, alfarray[i]), "d6d1 ") == 0) continue;
            printf("info currmove %s currmovenumber %d\n",
                   Util::move2str(move_str, alfarray[i]), i + 1);
            Util::flush();
        }
        table->update_table(alfarray[i], false);
        curr_line[dpt] = alfarray[i];

        // If last ply->evaluating
        if ((dpt >= depth && movelist[move_number].further == 0) ||
            dpt >= seldepth) {
            last_ply = true;
#ifdef HASH
            hash->set_hash(this);
            if (hash->posInHashtable()) {
                u = hash->getU();
                --move_number;
                ++hash->hash_nodes;
            }

            // not in the hashtable. normal evaluating
            else {
#endif
                if (table->third_occurance() ||
                    table->is_not_enough_material() ||
                    movelist[move_number].not_pawn_move >= 100) {
                    u = table->eval->DRAW;
                    --move_number;
                } else {
                    table->list_legal_moves();
                    u = table->eval->evaluation(legal_pointer, dpt);
                    --move_number;
                }
#ifdef HASH
                // if this position is not in the hashtable->insert it to hashtable
                hash->setU(u);
            }
#endif
        }
        // Not last ply
        else {
            if (table->third_occurance() ||
                table->is_not_enough_material() ||
                movelist[move_number].not_pawn_move >= 100) {
                u = table->eval->DRAW;
                --move_number;
            } else {
                u = table->eval->evaluation_only_end_game(dpt);
                if (u == 32767) { // not end
                    invert_player_to_move();
                    u = -alfabeta(dpt + 1, -beta, -alfa);
                    last_ply = false;
                    invert_player_to_move();
                }
                --move_number;
            }
        }
        if (dpt == 1) {
            root_moves[i].move = alfarray[i];
            root_moves[i].value = u;
        }

        // Better move is found
        if (u > value) {
            // printf("dpt: %d, %d > %d\n", dpt, u, value);Util::flush();
            value = u;
            for (int b = 1; b <= curr_seldepth; b++) {
                best_line[dpt].moves[b] = curr_line[b];
            }
            best_line[dpt].value = u;
            if (last_ply) {
                best_line[dpt].length = dpt;
            }
            if (!last_ply) {
                best_line[dpt].length = best_line[dpt + 1].length;
                best_line[dpt].value = best_line[dpt + 1].value;
                for (int b = 1; b <= best_line[dpt + 1].length; b++) {
                    best_line[dpt].moves[b] = best_line[dpt + 1].moves[b];
                }
                // printf("best_line[%d].length: %d\n", dpt, best_line[dpt].length);Util::flush();
            }
            if (dpt == 1) {
                best_move = alfarray[i];
#ifdef HASH
                hash->printStatistics(nodes);
#endif
                int time_elapsed = Util::get_ms() - start_time;
                // If checkmate is found
                if (abs(u) > 20000) {
                    if (u > 0) {
                        uu = (22001 - u) / 2;
                    } else {
                        uu = -(u + 22001) / 2;
                    }
                    if (uu == 0 && u > 0) {
                        uu = 1;
                    }
                    if (uu == 0 && u < 0) {
                        uu = -1;
                    }
                    printf("info multipv 1 depth %d seldepth %d time %d score "
                           "mate %d "
                           "nodes %lu pv ",
                           curr_depth, curr_seldepth, time_elapsed, uu, nodes);
                    for (int b = 1; b <= best_line[dpt].length; ++b) {
                        printf("%s ", Util::move2str(move_str,
                                                     best_line[dpt].moves[b]));
                    }
                    printf("\n");
                    Util::flush();
                    mate_score = abs(u);
                } else {
                    printf("info multipv 1 depth %d seldepth %d time %d score "
                           "cp %d "
                           "nodes %lu pv ",
                           curr_depth, curr_seldepth, time_elapsed, u, nodes);
                    for (int b = 1; b <= best_line[dpt].length; ++b) {
                        printf("%s ", Util::move2str(move_str,
                                                     best_line[dpt].moves[b]));
                    }
                    printf("\n");
                    Util::flush();
                    mate_score = 0;
                }
            }
        }
#ifdef ALFABETA

        // Alfa Beta cut-off
        if (value >= beta) {
            return value;
        }
        if (value > alfa) {
            alfa = value;
        }
#endif
    } // for
    return value;
}

uint64_t Chess::perft(const int dpt) {
    int alfarray[MAX_LEGAL_MOVES];
    uint64_t nodes = 0;

    if (dpt == 0) {
        return 1;
    }

    table->list_legal_moves();
    const int nbr_legal = legal_pointer + 1;
    for (int i = 0; i < nbr_legal; ++i) {
        alfarray[i] = legal_moves[i];
    }
    for (int i = 0; i < nbr_legal; ++i) {
        if ((nodes & 1023) == 0) {
            checkup();
        }
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
    if (depth == 1) {
        for (int i = 0; i < nof_legal_root_moves; i++) {
            root_moves[i].move = legal_moves[i];
            root_moves[i].value = 0;
        }
    } else {
        for (int i = 0; i < nof_legal_root_moves; i++) {
            for (int j = i + 1; j < nof_legal_root_moves; j++) {
                if (root_moves[j].value > root_moves[i].value) {
                    const int tempmove = root_moves[j].move;
                    root_moves[j].move = root_moves[i].move;
                    root_moves[i].move = tempmove;
                    const int v = root_moves[j].value;
                    root_moves[j].value = root_moves[i].value;
                    root_moves[i].value = v;
                }
            }
        }
    }
}

void Chess::calculate_evarray_new() {
    for (int i = 0; i < nof_legal_root_moves; i++) {
        for (int j = i + 1; j < nof_legal_root_moves; j++) {
            if (root_moves[j].value > root_moves[i].value) {
                const int tempmove = root_moves[j].move;
                root_moves[j].move = root_moves[i].move;
                root_moves[i].move = tempmove;
                const int v = root_moves[j].value;
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
    for (int i = 1; i < nof_legal_root_moves; i++) {
        for (int j = i;
                j > 0 && root_moves[j].value > root_moves[j-1].value;
                j--) {
            const int tempmove = root_moves[j].move;
            root_moves[j].move = root_moves[i].move;
            root_moves[i].move = tempmove;
            const int v = root_moves[j].value;
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

void Chess::processCommands(const char *input) {
    if (strstr(input, "uci")) {
        uci->processCommands(input);
    }
}
