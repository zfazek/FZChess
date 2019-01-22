#pragma once

class Chess;

class Uci {
  public:
    Chess *chess;

    Uci(Chess *chess);
    ~Uci();
    void position_received(char *input);
    void processCommands(char *input);
};
