#ifndef TASK2_HASHING_H
#define TASK2_HASHING_H

#include <cstdint>

class Hash {
    static const uint64_t MOD1 = 2000050477ull, MOD2 = 1999930027ull;
    uint32_t hash1, hash2;

public:
    Hash();
    Hash(uint32_t first, uint32_t second);
    Hash(Hash const& other) = default;

    friend Hash operator+(Hash a, Hash const& b);
    friend Hash operator-(Hash a, Hash const& b);
    friend Hash operator*(Hash a, Hash const& b);
    Hash& operator+=(Hash const& rhs);
    Hash& operator-=(Hash const& rhs);
    Hash& operator*=(Hash const& rhs);
    std::size_t to_size_t() const;
    bool initialized() const;

    friend bool operator==(Hash const& a, Hash const& b);
    friend bool operator!=(Hash const& a, Hash const& b);
    friend bool operator<(Hash const& a, Hash const& b);
};

Hash operator+(Hash a, Hash const& b);
Hash operator-(Hash a, Hash const& b);
Hash operator*(Hash a, Hash const& b);
bool operator==(Hash const& a, Hash const& b);
bool operator!=(Hash const& a, Hash const& b);
bool operator<(Hash const& a, Hash const& b);

#endif //TASK2_HASHING_H
