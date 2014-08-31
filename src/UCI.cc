#include "UCI.h"
#include <string.h>
#include <stdio.h>
#include <thread>

UCI::UCI(Chess* ch) {
    chess = ch;
    depth = 0;
    seldepth = 0;
    init_depth = 0;
    curr_depth = 0;
    curr_seldepth = 0;
    gui_depth = 0;
    rc = 0;
    t = 0;
    //threads = 0;
    start_time = stop_time = max_time = movetime = 0;
    FZChess = 1;
}

UCI::~UCI() {
}

void UCI::position_received(char* input) {
    int i;
    int m;
    char move_old[6];
    strcpy(move_old,"     ");
    chess->start_game();
    chess->player_to_move = chess->WHITE;
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
        chess->update_table(chess->str2move(move_old), chess->FALSE);
        chess->invert_player_to_move();
    }
    //print_table();
}

void UCI::processCommands(char* input) {
    int movestogo = 40;
    int wtime, btime;
    int  winc = 0;
    int  binc = 0;
    char* ret;
    gui_depth = 0;
    if (strstr(input, "uci")) {
        printf("id name FZChess++\n");
        printf("id author Zoltan FAZEKAS\n");
        printf("option name OwnBook type check defult false\n");
        printf("option name Ponder type check default false\n");
        printf("option name MultiPV type spin default 1 min 1 max 1\n");
        printf("uciok\n");
        chess->flush();
    }
    if (chess->DEBUG) {
        chess->debugfile=fopen("./debug.txt", "w");
        fclose(chess->debugfile);
    }
    while (1) {
        ret = fgets(input, 1000, stdin);
        if (ret && strstr(input, "stop")) {
            chess->stop_received = chess->TRUE;
        }
        //printf("Process input: %s\n", input);flush();
        if (chess->DEBUG) {
            chess->debugfile = fopen("./debug.txt", "w");
            fclose(chess->debugfile);
            if ( ! strstr(input,"quit") ) {
                chess->debugfile = fopen("./debug.txt", "a");
                fprintf(chess->debugfile, "-> %s", input);
                fclose(chess->debugfile);
            }
        }
        if (strstr(input, "isready")) {
            printf("readyok\n");
            chess->flush();
        }
        if (strstr(input, "position startpos")) {
            position_received(input);
        }
        if (strstr(input, "position fen")) chess->setboard(input);
        if (strstr(input, "go")) {
            FZChess=chess->player_to_move;
            //if (strstr(input, "ponder")) continue;
            movetime = 0;
            if (strstr(input, "movetime"))  {
                sscanf(strstr(input, "movetime"),  "movetime %d",  &max_time);
                movetime = max_time;
            }
            else {
                if (strstr(input, "movestogo")) 
                    sscanf(strstr(input, "movestogo"), "movestogo %d", &movestogo);
                if (strstr(input, "wtime")) 
                    sscanf(strstr(input, "wtime"), "wtime %d", &wtime);
                if (strstr(input, "btime")) 
                    sscanf(strstr(input, "btime"), "btime %d", &btime);
                if (strstr(input, "winc")) 
                    sscanf(strstr(input, "winc"), "winc %d", &winc);
                if (strstr(input, "binc")) 
                    sscanf(strstr(input, "binc"), "binc %d", &binc);
                if (FZChess == chess->WHITE) 
                    max_time = (wtime + movestogo * winc) / movestogo;
                else max_time = (btime + movestogo * binc) / movestogo;
                if (strstr(input, "depth")) {
                    sscanf(strstr(input, "depth"), "depth %d", &gui_depth);
                    max_time = 0;
                }
                if (strstr(input, "infinite")) {
                    gui_depth = 99;
                    max_time = 0;
                }
            }
            if (chess->DEBUG) {
                chess->debugfile = fopen("./debug.txt", "a");
                fprintf(chess->debugfile, 
                        "max time: %d wtime: %d btime: %d winc: %d binc: %d movestogo: %d\n",
                        max_time, wtime, btime, winc, binc, movestogo);
                chess->flush();
                fclose(chess->debugfile);
            }
            chess->stop_received = chess->FALSE;
//            rc = pthread_create(&threads, NULL, make_move_uci, (void *)t);
            th_make_move = std::thread(&Chess::make_move, this->chess);
            //TODO c++11 thread
        }
    }
}

//void* UCI::make_move_uci(void *threadid) {
//    if (strstr(Chess::engine, "uci")) make_move_uci();
//}


