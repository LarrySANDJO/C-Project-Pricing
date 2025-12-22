#pragma once
#include "Payoff.hpp"

class PayoffCall final : public Payoff {
public:
    explicit PayoffCall(double strike);

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
