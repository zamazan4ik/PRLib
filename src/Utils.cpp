//
// Created by zamazan4ik on 23.09.17.
//

#include <cmath>

bool eq_d(const double v1, const double v2, const double delta)
{
    return std::abs(v1 - v2) <= delta;
}