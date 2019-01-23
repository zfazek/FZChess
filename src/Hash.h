#pragma once

#include <unordered_map>

class Chess;

class Hash {
  public:
    uint64_t hash;
    uint64_t hash_nodes;

    Hash();
    ~Hash();

    void set_hash(const Chess *chess);
    void reset_counters();
    bool posInHashtable() const;
    int getU() const;
    void setU(const int u);
    void printStatistics(const int nodes) const;
    void clear();

  private:
    std::unordered_map<uint64_t, int> hashes;
    uint64_t hash_side_white;
    uint64_t hash_side_black;
    uint64_t hash_piece[2][7][120];
    uint64_t hash_enpassant[120];
    uint64_t hash_castle[15];
    uint64_t hash_index;

    uint64_t rand64() const;
    uint64_t hash_rand() const;
    void init_hash();
};
