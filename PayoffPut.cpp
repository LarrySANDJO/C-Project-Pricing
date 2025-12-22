#include "PayoffPut.hpp"
#include "EuropeanBlackScholesPricer.hpp"

#include <algorithm>

PayoffPut::PayoffPut(double strike)
    : strike_(strike)
{}

double PayoffPut::operator()(double spot) const {
    return std::max(strike_ - spot, 0.0);
}

double PayoffPut::accept(
    const EuropeanBlackScholesPricer& pricer,
    const MarketData& market,
    double maturity
) const {
    return pricer.price(*this, market, maturity);
}

double PayoffPut::strike() const {
    return strike_;
}
