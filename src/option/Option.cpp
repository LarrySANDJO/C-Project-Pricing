#include "Option.hpp"

Option::Option(double maturity,
               std::unique_ptr<Payoff> payoff)
    : maturity_(maturity),
      payoff_(std::move(payoff))
{}

double Option::maturity() const {
    return maturity_;
}

double Option::payoff(double spot) const {
    return (*payoff_)(spot);
}
