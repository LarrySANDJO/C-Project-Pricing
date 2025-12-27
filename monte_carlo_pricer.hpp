#pragma once

#include "pricer.hpp"
#include "option.hpp"
#include <vector>
#include <random>

/* =========================================================
   STRUCTURE POUR RÉSULTATS MONTE CARLO
   ========================================================= */
struct MCResult
{
    double price;
    double std_error;
    double ci_lower_95;  // Intervalle de confiance à 95%
    double ci_upper_95;
};

/* =========================================================
   MONTE CARLO (EUROPÉEN + EXOTIQUE)
   ========================================================= */
class MonteCarloPricer : public Pricer
{
public:

    MonteCarloPricer(const Option& option,
                     double spot,
                     double rate,
                     double carry,
                     double volatility,
                     std::size_t paths,
                     std::size_t steps,
                     unsigned seed = std::random_device{}(), // Seed paramétrable
                     bool use_antithetic = true);

    double price() const override;
    double delta(double spot) const override;
    
    // Prix avec intervalle de confiance
    MCResult price_with_confidence() const;
    
    // Delta par méthode pathwise
    double delta_pathwise() const;
    
    // Vega par Monte Carlo
    double vega() const override;

private:
    // Simuler un path complet
    std::vector<double> simulate_path(std::mt19937& gen) const;
    
    // Simuler un path avec des nombres aléatoires spécifiques
    std::vector<double> simulate_path_with_randoms(const std::vector<double>& randoms) const;

    const Option& option_;
    double S0_, r_, b_, sigma_;
    std::size_t paths_, steps_;
    unsigned seed_;
    bool use_antithetic_;  // Variables antithétiques
};