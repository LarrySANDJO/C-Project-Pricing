#include "/home/onyxia/C-Project-Pricing/include/payoff/PayoffCall.hpp"
#include <algorithm> // pour la fonction max

PayoffCall::PayoffCall(double strike)
    : strike_(strike)
{}

double PayoffCall::operator()(double spot) const {
    return std::max(spot - strike_, 0.0);
    // On met 0.0 au lieu de 0 pour eviter les convertions inutiles
}
