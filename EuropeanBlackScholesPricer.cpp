#include "EuropeanBlackScholesPricer.hpp"

#include <cmath>

namespace {
    double norm_cdf(double x) {
        return 0.5 * std::erfc(-x / std::sqrt(2));
    }
}

double EuropeanBlackScholesPricer::price(
    const Option& option,
    const MarketData& market
) const {
    return option.payoff().accept(
        *this,
        market,
        option.maturity()
    );
}

double EuropeanBlackScholesPricer::price(
    const PayoffCall& call,
    const MarketData& market,
    double maturity
) const {
    double S = market.spot();
    double K = call.strike();
    double r = market.rate();
    double sigma = market.volatility();

    double d1 =
        (std::log(S / K)
        + (r + 0.5 * sigma * sigma) * maturity)
        / (sigma * std::sqrt(maturity));

    double d2 = d1 - sigma * std::sqrt(maturity);

    return S * norm_cdf(d1)
         - K * std::exp(-r * maturity) * norm_cdf(d2);
}

double EuropeanBlackScholesPricer::price(
    const PayoffPut& put,
    const MarketData& market,
    double maturity
) const {
    double S = market.spot();
    double K = put.strike();
    double r = market.rate();
    double sigma = market.volatility();

    double d1 =
        (std::log(S / K)
        + (r + 0.5 * sigma * sigma) * maturity)
        / (sigma * std::sqrt(maturity));

    double d2 = d1 - sigma * std::sqrt(maturity);

    return K * std::exp(-r * maturity) * norm_cdf(-d2)
         - S * norm_cdf(-d1);
}
