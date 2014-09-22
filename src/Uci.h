#pragma once

class Chess;

class Uci {
    public:
        Uci();
        ~Uci();
        void position_received(Chess* chess, char* input);
        void processCommands(Chess* chess, char* input);
};
