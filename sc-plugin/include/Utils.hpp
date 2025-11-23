#pragma once

#include <cmath>
#include <algorithm>

namespace junox {

// Fast tanh approximation
inline float fastTanh(float x) {
    const float ax = std::abs(x);
    const float x2 = x * x;
    return (x * (2.45550750702956f + 2.45550750702956f * ax + 
                  (0.893229853513558f + 0.821226666969744f * ax) * x2) /
            (2.44506634652299f + (2.44506634652299f + x2) * 
             (1.0f + 0.814642734961073f * x2)));
}

// Interpolated lookup in a table
inline float interpolatedLookup(float index, const float* table, int tableSize) {
    if (index <= 0.0f) return table[0];
    if (index >= tableSize - 1) return table[tableSize - 1];
    
    const int i = static_cast<int>(index);
    const float frac = index - i;
    return table[i] + frac * (table[i + 1] - table[i]);
}

} // namespace junox
