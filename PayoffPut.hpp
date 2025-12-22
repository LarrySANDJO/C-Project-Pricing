#pragma once
#include "Payoff.hpp"

class PayoffPut final : public Payoff {
public:
    explicit PayoffPut(double strike);

    double operator()(double spot) const override;

    double accept(
        const EuropeanBlackScholesPricer& pricer,
        const MarketData& market,
        double maturity
    ) const override;

    double strike() const;

private:
    double strike_;
};
