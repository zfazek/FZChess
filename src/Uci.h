#pragma once

#include <thread>

class Chess;

class Uci {
  public:
    Uci(Chess *chess);

    void position_received(const char *input);

    [[noreturn]] void processCommands(const char *cmd);

  private:
    Chess *chess;
    std::thread th_make_move;
};
