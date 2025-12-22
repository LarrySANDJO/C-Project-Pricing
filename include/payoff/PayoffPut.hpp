#pragma once
#include "Payoff.hpp"

class PayoffPut : public Payoff {
public:
    explicit PayoffPut(double strike);
    double operator()(double spot) const override;

private:
    double strike_;
};
