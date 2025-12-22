#include "Option.hpp"

Option::Option(double maturity,
               std::unique_ptr<Payoff> payoff)
    : maturity_(maturity),
      payoff_(std::move(payoff))
      // le move permet de transferer payoff et ainsi de mieux gérer l'allocation de memoire (on n'a pas besoin de conserver le payoff apres calculs)
{}

double Option::maturity() const {
    return maturity_;
}

double Option::payoff(double spot) const {
    return (*payoff_)(spot);
}
