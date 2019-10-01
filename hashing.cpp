#include "hashing.h"

Hash::Hash() : hash1(0), hash2(0) {}

Hash::Hash(uint32_t first, uint32_t second):
        hash1((static_cast<uint64_t>(MOD1) + static_cast<uint64_t>(first)) % static_cast<uint64_t>(MOD1)),
        hash2((static_cast<uint64_t>(MOD2) + static_cast<uint64_t>(second)) % static_cast<uint64_t>(MOD2)) {}

Hash operator+(Hash a, Hash const &b)
{
    return Hash(a.hash1 + b.hash1, a.hash2 + b.hash2);
}

Hash operator-(Hash a, Hash const &b)
{
    return Hash(a.hash1 - b.hash1, a.hash2 - b.hash2);
}

Hash operator*(Hash a, Hash const &b)
{
    return Hash(a.hash1 * b.hash1, a.hash2 * b.hash2);
}

Hash &Hash::operator+=(Hash const &rhs)
{
    return *this = *this + rhs;
}

Hash &Hash::operator-=(Hash const &rhs)
{
    return *this = *this - rhs;
}

Hash &Hash::operator*=(Hash const &rhs)
{
    return *this = *this * rhs;
}

bool operator==(Hash const &a, Hash const &b)
{
    return a.hash1 == b.hash1 && a.hash2 == b.hash2;
}

bool operator!=(Hash const &a, Hash const &b)
{
    return !(a == b);
}

std::size_t Hash::to_size_t() const
{
    return (hash1 << 29ull) ^ hash2;
}

bool operator<(Hash const &a, Hash const &b)
{
    return a.to_size_t() < b.to_size_t();
}

bool Hash::initialized() const
{
    return hash1 != 0 || hash2 != 0;
}
