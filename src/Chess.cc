#include "Chess.h"
#include "UCI.h"
#include "Table.h"
#include "Util.h"

Chess::Chess() {
    Table *table = new Table(this);
}

Chess::~Chess() {
    delete table;
}

void Chess::start_game() { // new
    table->reset_movelist();
    player_to_move = WHITE;
    //print_table();
}

