#pragma once
#include <memory>

#include "Payoff.hpp"

class Option {
public:
    Option(double maturity,
           std::unique_ptr<Payoff> payoff);

    double maturity() const;
    const Payoff& payoff() const;

private:
    double maturity_;
    std::unique_ptr<Payoff> payoff_;
};
