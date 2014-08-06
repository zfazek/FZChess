#ifndef UCI_H
#define UCI_H

#include "Chess.h"
#include <pthread.h>
#include <thread>

class UCI {
 public:
	Chess* chess;
    UCI(Chess* chess);
    ~UCI();
    void processCommands();
 private:
    int FZChess; // 1:white, -1:black
    int start_time, stop_time, max_time, movetime;
    int rc;
    long t;
    pthread_t threads;
    std::thread th_make_move;

    void position_received(char* input);
    int depth, seldepth, init_depth, curr_depth, curr_seldepth, gui_depth;
	static void* make_move(void *threadid);

};

#endif
