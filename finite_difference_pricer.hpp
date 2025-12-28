#pragma once

#include "pricer.hpp"
#include "option.hpp"
#include <vector>

/* =========================================================
   DIFFÉRENCES FINIES – OPTION AMÉRICAINE
   ========================================================= */
class FiniteDifferenceAmericanPricer : public Pricer
{
public:
    enum class Scheme
    {
        Explicit,      // Schéma explicite (simple mais instable)
        Implicit,      // Schéma implicite (stable)
        CrankNicolson  // Crank-Nicolson (plus précis)
    };

    FiniteDifferenceAmericanPricer(const Option& option,
                                   double spot,
                                   double rate,
                                   double carry,
                                   double volatility,
                                   std::size_t M,  // Points dans l'espace
                                   std::size_t N,  // Points dans le temps
                                   Scheme scheme = Scheme::CrankNicolson);

    double price() const override;
    double delta(double spot) const override;

private:
    // Schéma Crank-Nicolson pour meilleure stabilité
    double price_explicit() const;
    double price_implicit() const;
    double price_crank_nicolson() const;
    
    // Résolution système tridiagonal (pour schémas implicites)
    void solve_tridiagonal(const std::vector<double>& a,
                          const std::vector<double>& b,
                          const std::vector<double>& c,
                          const std::vector<double>& d,
                          std::vector<double>& x) const;

    const Option& option_;
    double S0_, r_, b_, sigma_;
    std::size_t M_, N_;
    double Smax_;
    Scheme scheme_;
};