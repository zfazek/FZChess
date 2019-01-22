#include "Uci.h"
#include "Chess.h"
#include <cstdio>
#include <cstring>

Uci::Uci(Chess *ch) : chess(ch) {}

Uci::~Uci() {}

void Uci::position_received(const char *input) {
    static char move_old[6];
    strncpy(move_old, "     ", 6);
    chess->start_game();
    chess->player_to_move = chess->WHITE;
    if (!strstr(input, "move")) {
        return;
    }
    for (size_t i = 24; i < strlen(input) - 1; i++) {
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
        chess->table->update_table(Util::str2move(move_old), false);
        chess->invert_player_to_move();
    }
    // print_table();
}

void Uci::processCommands(const char *cmd) {
    static char input[1000] = {0};
    int movestogo = 40;
    int wtime = 0;
    int btime = 0;
    int winc = 0;
    int binc = 0;
    chess->gui_depth = 0;
    if (strstr(cmd, "uci")) {
        printf("id name FZChess++\n");
        printf("id author Zoltan FAZEKAS\n");
        printf("option name OwnBook type check defult false\n");
        printf("option name Ponder type check default false\n");
        printf("option name MultiPV type spin default 1 min 1 max 1\n");
        printf("uciok\n");
        chess->util->flush();
    }
    if (chess->DEBUG) {
        chess->debugfile = fopen("./debug.txt", "w");
        fclose(chess->debugfile);
    }
    while (true) {
    char *ret = fgets(input, 1000, stdin);
        if (ret && strstr(input, "stop")) {
            chess->stop_received = true;
        }
        if (chess->th_make_move.joinable()) {
            chess->th_make_move.join();
        }
        // printf("Process input: %s\n", input);util->flush();
        if (chess->DEBUG) {
            chess->debugfile = fopen("./debug.txt", "w");
            fclose(chess->debugfile);
            if (!strstr(input, "quit")) {
                chess->debugfile = fopen("./debug.txt", "a");
                fprintf(chess->debugfile, "-> %s", input);
                fclose(chess->debugfile);
            }
        }
        if (strstr(input, "isready")) {
            printf("readyok\n");
            chess->util->flush();
        }
        if (strstr(input, "position startpos")) {
            position_received(input);
        }
        if (strstr(input, "position fen")) {
            chess->table->setboard(input);
        }
        if (strstr(input, "go")) {
            chess->FZChess = chess->player_to_move;
            // if (strstr(input, "ponder")) continue;
            chess->movetime = 0;
            if (strstr(input, "movetime")) {
                sscanf(strstr(input, "movetime"), "movetime %d",
                       &chess->max_time);
                chess->movetime = chess->max_time;
            } else {
                if (strstr(input, "movestogo")) {
                    sscanf(strstr(input, "movestogo"), "movestogo %d",
                           &movestogo);
                }
                if (strstr(input, "wtime")) {
                    sscanf(strstr(input, "wtime"), "wtime %d", &wtime);
                }
                if (strstr(input, "btime")) {
                    sscanf(strstr(input, "btime"), "btime %d", &btime);
                }
                if (strstr(input, "winc")) {
                    sscanf(strstr(input, "winc"), "winc %d", &winc);
                }
                if (strstr(input, "binc")) {
                    sscanf(strstr(input, "binc"), "binc %d", &binc);
                }
                if (chess->FZChess == chess->WHITE) {
                    chess->max_time = (wtime + movestogo * winc) / movestogo;
                } else {
                    chess->max_time = (btime + movestogo * binc) / movestogo;
                }
                if (strstr(input, "depth")) {
                    sscanf(strstr(input, "depth"), "depth %d",
                           &chess->gui_depth);
                    chess->max_time = 0;
                }
                if (strstr(input, "infinite")) {
                    chess->gui_depth = 99;
                    chess->max_time = 0;
                }
            }
            if (chess->DEBUG) {
                chess->debugfile = fopen("./debug.txt", "a");
                fprintf(chess->debugfile,
                        "max time: %d wtime: %d btime: %d winc: %d "
                        "binc: %d movestogo: %d\n",
                        chess->max_time, wtime, btime, winc, binc, movestogo);
                chess->util->flush();
                fclose(chess->debugfile);
            }
            chess->stop_received = false;
            chess->th_make_move = std::thread(&Chess::make_move, chess);
        }
    }
}
