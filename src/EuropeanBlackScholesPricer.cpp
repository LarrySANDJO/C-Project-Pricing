#include "/home/onyxia/C-Project-Pricing/include/EuropeanBlackScholesPricer.hpp"

#include "/home/onyxia/C-Project-Pricing/include/payoff/PayoffCall.hpp"
#include "/home/onyxia/C-Project-Pricing/include/payoff/PayoffPut.hpp"

#include <cmath>
#include <stdexcept>

namespace {
    double norm_cdf(double x) {
        return 0.5 * std::erfc(-x / std::sqrt(2));
    }
}

double EuropeanBlackScholesPricer::price(
    const Option& option,
    const MarketData& market) const
{
    double S = market.spot();
    double r = market.rate();
    double sigma = market.volatility();
    double T = option.maturity();

    // Tentative Call
    if (auto call = dynamic_cast<const PayoffCall*>(
            &(*option.payoff_))) {
        double K = call->strike();

        double d1 = (std::log(S / K)
            + (r + 0.5 * sigma * sigma) * T)
            / (sigma * std::sqrt(T));

        double d2 = d1 - sigma * std::sqrt(T);

        return S * norm_cdf(d1)
             - K * std::exp(-r * T) * norm_cdf(d2);
    }

    // Tentative Put
    if (auto put = dynamic_cast<const PayoffPut*>(
            &(*option.payoff_))) {
        double K = put->strike();

        double d1 = (std::log(S / K)
            + (r + 0.5 * sigma * sigma) * T)
            / (sigma * std::sqrt(T));

        double d2 = d1 - sigma * std::sqrt(T);

        return K * std::exp(-r * T) * norm_cdf(-d2)
             - S * norm_cdf(-d1);
    }

    throw std::logic_error("Unsupported payoff type");
}
