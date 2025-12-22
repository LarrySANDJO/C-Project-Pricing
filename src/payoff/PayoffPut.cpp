#include "/home/onyxia/C-Project-Pricing/include/payoff/PayoffPut.hpp"
#include <algorithm>

PayoffPut::PayoffPut(double strike)
    : strike_(strike)
{}

double PayoffPut::operator()(double spot) const {
    return std::max(strike_ - spot, 0.0);
}
