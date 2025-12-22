#pragma once
#include <memory>
#include "/home/onyxia/C-Project-Pricing/include/payoff/Payoff.hpp"

class Option {
public:
    Option(double maturity,
           std::unique_ptr<Payoff> payoff);

    double maturity() const;
    double payoff(double spot) const;

private:
    double maturity_;
    std::unique_ptr<Payoff> payoff_;
};
