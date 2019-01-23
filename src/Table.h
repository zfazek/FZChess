#pragma once

#define EMPTY 0x00
#define OFFBOARD 0xff

class Chess;
class Eval;

class Table {
  public:
    Eval *eval;

    const int WHITE = 1;
    const int BLACK = -1;

    const int Pawn = 1;
    const int Knight = 2;
    const int Bishop = 3;
    const int Rook = 4;
    const int Queen = 5;
    const int King = 6;

    // Possible direction of figure's move
    const int dir_rook[4] = {-10, -1, 1, 10};
    const int dir_knight[8] = {-21, -19, -12, -8, 8, 12, 19, 21};
    const int dir_bishop[4] = {-11, -9, 9, 11};
    const int dir_king[8] = {-11, -10, -9, -1, 1, 9, 10, 11};

    Table(Chess *chess);
    ~Table();

    void list_legal_moves();
    void print_table();
    void reset_movelist();
    void setboard(const char *fen_position);
    bool is_attacked(const int field, const int color);
    bool is_not_enough_material();
    void update_table(const int move, bool print, bool fake = false);
    bool third_occurance();

  private:
    Chess *chess;
    int *pt, *ptt;
    int end_direction;

    // Values representing the figures in the table
    const int WhitePawn = 1;
    const int WhiteKnight = 2;
    const int WhiteBishop = 3;
    const int WhiteRook = 4;
    const int WhiteQueen = 5;
    const int WhiteKing = 6;

    const int BlackPawn = 0x81;
    const int BlackKnight = 0x82;
    const int BlackBishop = 0x83;
    const int BlackRook = 0x84;
    const int BlackQueen = 0x85;
    const int BlackKing = 0x86;

    // For printing and for notation
    const int graphical_figure[14][2] = {
        {000, 46},  // "."
        {001, 80},  // "P"
        {002, 78},  // "N"
        {003, 66},  // "B"
        {004, 82},  // "R"
        {005, 81},  // "Q"
        {006, 75},  // "K"
        {129, 112}, // "p"
        {130, 110}, // "n"
        {131, 98},  // "b"
        {132, 114}, // "r"
        {133, 113}, // "q"
        {134, 107}, // "k"
        {191, 88},  // "X"
    };

    void is_really_legal();
    void append_legal_moves(const int dir_piece, const int i, const int j, const int kk);
    void append_legal_moves_inner(const int dir_piece, const int i, const int j, const int kk);
    int convA(const int k);
    int conv0(const int k);
    void castling();
};
