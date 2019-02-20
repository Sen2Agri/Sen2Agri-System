#include <cstdint>

#include "entry.h"

entry_real operator*(const entry_real &lhs, const entry_count &rhs)
{
    entry_real result(lhs.size());
    for (size_t i = 0; i < result.size(); i++) {
        result[i] = lhs[i] * rhs[i];
    }
    return result;
}

entry_real operator/(const entry_real &lhs, const entry_count &rhs)
{
    entry_real result(lhs.size());
    for (size_t i = 0; i < result.size(); i++) {
        result[i] = lhs[i] / rhs[i];
    }
    return result;
}
