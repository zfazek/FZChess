#ifndef UCI_H
#define UCI_H

#include "Chess.h"

class UCI {
 public:
	Chess* chess;
    UCI(Chess* chess);
    ~UCI();
    void processCommands();
 private:
    void position_received(char* input);
    int depth, seldepth, init_depth, curr_depth, curr_seldepth, gui_depth;
};

#endif
