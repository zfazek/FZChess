#pragma once

class Chess;

class Uci {
  public:
    Uci(Chess *chess);
    ~Uci();

    void position_received(const char *input);
    void processCommands(const char *cmd);

  private:
    Chess *chess;
};
