#include "PayoffCall.hpp"
#include <algorithm>

PayoffCall::PayoffCall(double strike)
    : strike_(strike)
{}

double PayoffCall::operator()(double spot) const {
    return std::max(spot - strike_, 0.0);
}
