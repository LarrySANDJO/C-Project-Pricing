#pragma once
#include "Payoff.hpp"

class PayoffCall : public Payoff {
public:
    explicit PayoffCall(double strike);
    double operator()(double spot) const override;

private:
    double strike_;
};
