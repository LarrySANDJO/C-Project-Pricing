#include "PayoffCall.hpp"
#include "EuropeanBlackScholesPricer.hpp"

#include <algorithm>

PayoffCall::PayoffCall(double strike)
    : strike_(strike)
{}

double PayoffCall::operator()(double spot) const {
    return std::max(spot - strike_, 0.0);
}

double PayoffCall::accept(
    const EuropeanBlackScholesPricer& pricer,
    const MarketData& market,
    double maturity
) const {
    return pricer.price(*this, market, maturity);
}

double PayoffCall::strike() const {
    return strike_;
}
