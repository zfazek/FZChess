#include "Table.h"
#include "Chess.h"

Table::Table(Chess* chess) {
    eval = new Eval(chess);
}

Table::~Table() {
    delete eval;
}
