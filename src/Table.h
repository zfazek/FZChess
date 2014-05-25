#ifndef TABLE_H
#define TABLE_H

#include "Move.h"

class Table {
public:
	Chess* chess;
	Table(Chess* chess);
	~Table();
	void reset_movelist();

	//Stores the parameters of the given position
	struct move movelist[1000];
private:
};

#endif
