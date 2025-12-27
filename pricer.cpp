#include "pricer.hpp"
#include <stdexcept>

/* =========================================================
   PRICER - IMPLÉMENTATION PAR DÉFAUT DES GREEKS
   ========================================================= */

// Implémentation par défaut via différences finies
// Les classes dérivées peuvent surcharger pour des méthodes plus précises

double Pricer::gamma(double spot) const
{
    // Approximation par différences finies centrées : f''(x) ≈ [f(x+h) - 2f(x) + f(x-h)] / h²
    double h = 1e-4 * spot;
    double delta_up = delta(spot + h);
    double delta_down = delta(spot - h);
    
    return (delta_up - delta_down) / (2.0 * h);
}

double Pricer::vega() const
{
    // Implémentation par défaut - doit être surchargée
    throw std::runtime_error("Vega not implemented for this pricer");
}

double Pricer::theta() const
{
    // Implémentation par défaut - doit être surchargée
    throw std::runtime_error("Theta not implemented for this pricer");
}

double Pricer::rho() const
{
    // Implémentation par défaut - doit être surchargée
    throw std::runtime_error("Rho not implemented for this pricer");
}