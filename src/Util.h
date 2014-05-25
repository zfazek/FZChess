#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

int Pawn   = 1;
int Knight = 2;
int Bishop = 3;
int Rook   = 4;
int Queen  = 5;
int King   = 6;

//Values representing the figures in the table
const int WhitePawn   = 1;
const int WhiteKnight = 2;
const int WhiteBishop = 3;
const int WhiteRook   = 4;
const int WhiteQueen  = 5;
const int WhiteKing   = 6;

const int BlackPawn   = 0x81;
const int BlackKnight = 0x82;
const int BlackBishop = 0x83;
const int BlackRook   = 0x84;
const int BlackQueen  = 0x85;
const int BlackKing   = 0x86;

//For printing and for notation
int graphical_figure[14][2] =
    {
	{ 000,  46 }, // "."
	{ 001,  80 }, // "P"
	{ 002,  78 }, // "N"
	{ 003,  66 }, // "B"
	{ 004,  82 }, // "R"
	{ 005,  81 }, // "Q"
	{ 006,  75 }, // "K"
	{ 129, 112 }, // "p"
	{ 130, 110 }, // "n"
	{ 131,  98 }, // "b"
	{ 132, 114 }, // "r"
	{ 133, 113 }, // "q"
	{ 134, 107 }, // "k"
	{ 191,  88 }, // "X"
    };

enum figures { PAWN1, PAWN2, PAWN3, PAWN4, PAWN5, PAWN6, PAWN7, PAWN8,
	       KNIGHT1, KNIGHT2, BISHOP1, BISHOP2, ROOK1, ROOK2, QUEEN, KING};



int DEBUG = 0;
FILE *debugfile;
int stop_received = 0;

const int TRUE  = 1;
const int FALSE = 0;

const int WHITE =  1;
const int BLACK = -1;

const int DRAW =  0;
const int LOST = -22000;
const int WON  =  22000;



//Flushes the output stream
inline void flush() {
    fflush(stdout);
}

#endif
