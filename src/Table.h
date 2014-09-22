#pragma once
#include "Eval.h"

class Chess;

class Table {
    public:
        Table(Chess* chess);
        ~Table();
        
        Eval* eval;
};
