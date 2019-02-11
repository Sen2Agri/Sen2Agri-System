#pragma once

#include <cstdint>
#include <valarray>

template <typename T>
using entry = std::valarray<T>;
using entry_real = entry<double>;
using entry_count = entry<uint64_t>;

entry_real operator*(const entry_real &lhs, const entry_count &rhs);
entry_real operator/(const entry_real &lhs, const entry_count &rhs);
