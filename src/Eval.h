#ifndef EVAL_H
#define EVAL_H

class Chess;

class Eval {
    public:
        Eval();
        ~Eval();

        //Values for evaluation
        //empty, pawn, knight, bishop, rook, queen, king
        int figure_value[7] = {0, 100, 330, 330, 500, 900, 0};

};

#endif
