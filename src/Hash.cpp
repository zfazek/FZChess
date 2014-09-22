#include "Hash.h"
#include "Chess.h"
#include "Util.h"
#include <cstdlib>
#include <cstdio>

const unsigned int HASHSIZE = 1024 * 1024 * 64;
const unsigned int HASHSIZE_INNER = 1024 * 1024 * 1;

Hash::Hash() {
    init_hash();
}

Hash::~Hash() {
    free(hashtable);
}

void Hash::reset_counters() {
    hash_nodes = 0;
    hash_inner_nodes = 0;
    hash_collision = 0;
    hash_collision_inner = 0;
}

void Hash::init_hash() {
    int i, j, k;
    srand(0);

    //WHITE: i = 0, BLACK: i = 1
    for (i = 0; i < 2; i++)

        //PAWN = 1, KNIGHT = 2, ..., KING = 7
        for (j = 1; j < 7; j++)
            for (k = 0; k < 120; k++)
                hash_piece[i][j][k] = hash_rand();
    hash_side_white = hash_rand();
    hash_side_black = hash_rand();
    for (i = 0; i < 120; i++) hash_enpassant[i] = hash_rand();
    for (i = 0; i < 15; i++) hash_castle[i] = hash_rand();
    if ((hashtable = (hash_t*)malloc(sizeof(hash_t[HASHSIZE]))) == NULL) {
        printf("HASH memory fault!\n");
        exit(2);
    }
    printf("hashsize: %d, sizeof: %d\n",
            HASHSIZE,
            sizeof(hash_t[HASHSIZE]));
    Util::flush();
}

void Hash::init_hash_inner() {
    int i, j, k;
    srand(0);

    //WHITE: i = 0, BLACK: i = 1
    for (i = 0; i < 2; i++)

        //PAWN = 1, KNIGHT = 2, ..., KING = 7
        for (j = 1; j < 7; j++)
            for (k = 0; k < 120; k++)
                hash_piece[i][j][k] = hash_rand();
    hash_side_white = hash_rand();
    hash_side_black = hash_rand();
    for (i = 0; i < 120; i++) hash_enpassant[i] = hash_rand();
    for (i = 0; i < 15; i++) hash_castle[i] = hash_rand();
    if ((hashtable_inner = (hash_inner_t*)malloc(
                    sizeof(hash_inner_t[HASHSIZE_INNER]))) == NULL) {
        printf("HASH_INNER memory fault!\n");
        exit(2);
    }
    printf("hashsize_inner: %d\n", HASHSIZE_INNER);fflush(stdout);
}

//Set the hash variable of the current position
void Hash::set_hash(Chess* chess) {
    int i, k;
    int field, figure;
    hash = 0;
    // XOR the side to move
    if (chess->player_to_move == chess->WHITE) {
        hash ^= hash_side_white;
    }
    else {
        hash ^= hash_side_black;
    }
    // XOR the figures;
    for (k = 0; k < 120; k++) {
        field = chess->tablelist[chess->move_number][k];
        if (field > 0 && field < 255) {
            figure = (field & 127);
            if ((field & 128) == 128) i = 1; else i = 0;
            hash ^= hash_piece[i][figure][k];
        }
    }
    // XOR the enpassant position
    hash ^= hash_enpassant[chess->movelist[chess->move_number].en_passant];
    // XOR the castling possibilities
    hash ^= hash_castle[chess->movelist[chess->move_number].castle];
    hash_index = hash % HASHSIZE;
}

bool Hash::posInHashtable() {
    return hashes.find(hash) != hashes.end();

    /*
    // if this position is in the hashtable
    if ((hashtable + hash_index)->lock != hash &&
            (hashtable + hash_index)->lock != 0) {
        hash_collision++;
        return false;
    }
    if ((hashtable + hash_index)->lock == hash) {
        //printf("##Last Ply HASH FOUND##"); print_hash(hash, dpt);
        return true;
    }
    return false;
    */
}

int Hash::getU() {
    return hashes[hash];
    //return (hashtable + hash_index)->u;
}

void Hash::setU(int u) {
    hashes[hash] = u;
    /*
    (hashtable + hash_index)->lock = hash;
    (hashtable + hash_index)->u = u;
    */
}

unsigned long long Hash::rand64() {
    unsigned long long output;
    output = rand();
    output <<= 15;
    output ^= rand();
    output <<= 15;
    output ^= rand();
    output <<= 15;
    output ^= rand();
    output <<= 15;
    output ^= rand();
    return output;
}

unsigned long long Hash::hash_rand() {
    int i;
    unsigned long long r = 0LLU;
    for (i = 0; i < 32; i++) r ^= rand64() << i;
    return r;
}

void Hash :: printStatistics(int nodes) {
    printf("Hash found %d, hash inner nodes: %d, hash/nodes: %d%%\n",
            hash_nodes,
            hash_inner_nodes,
            100 * hash_nodes/nodes);
    fflush(stdout);
}
