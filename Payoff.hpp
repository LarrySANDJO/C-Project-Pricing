#pragma once

class MarketData;
class EuropeanBlackScholesPricer;

class Payoff {
public:
    virtual ~Payoff() = default;

    // Payoff pur : f(S_T)
    virtual double operator()(double spot) const = 0;

    // Double dispatch
    virtual double accept(
        const EuropeanBlackScholesPricer& pricer,
        const MarketData& market,
        double maturity
    ) const = 0;
};
