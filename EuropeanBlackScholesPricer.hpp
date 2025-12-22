#pragma once

#include "MarketData.hpp"
#include "PayoffCall.hpp"
#include "PayoffPut.hpp"
#include "Option.hpp"

class EuropeanBlackScholesPricer {
public:
    double price(
        const Option& option,
        const MarketData& market
    ) const;

    double price(
        const PayoffCall& call,
        const MarketData& market,
        double maturity
    ) const;

    double price(
        const PayoffPut& put,
        const MarketData& market,
        double maturity
    ) const;
};
