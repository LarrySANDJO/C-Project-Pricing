#include "option.hpp"
#include <stdexcept>

/* =========================================================
   OPTION - IMPLÉMENTATION
   ========================================================= */

Option::Option(double maturity, std::shared_ptr<Payoff> payoff)
    : T_(maturity), payoff_(std::move(payoff))
{
    if (T_ <= 0.0)
        throw std::invalid_argument("Maturity must be positive");
    
    if (!payoff_)
        throw std::invalid_argument("Payoff cannot be null");
}

bool Option::is_valid() const
{
    return T_ > 0.0 && payoff_ != nullptr;
}