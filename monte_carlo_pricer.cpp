#include "monte_carlo_pricer.hpp"
#include <cmath>
#include <numeric>
#include <stdexcept>

/* =========================================================
   MONTE CARLO - IMPLÉMENTATION
   ========================================================= */

MonteCarloPricer::MonteCarloPricer(const Option& option,
                                   double spot,
                                   double rate,
                                   double carry,
                                   double volatility,
                                   std::size_t paths,
                                   std::size_t steps,
                                   unsigned seed, //Seed paramétrable
                                   bool use_antithetic)
    : option_(option),
      S0_(spot),
      r_(rate),
      b_(carry),
      sigma_(volatility),
      paths_(paths),
      steps_(steps),
      seed_(seed),
      use_antithetic_(use_antithetic)
{
    // Controles pour validation
    if (spot <= 0.0)
        throw std::invalid_argument("Spot must be positive");
    if (volatility <= 0.0)
        throw std::invalid_argument("Volatility must be positive");
    if (paths == 0)
        throw std::invalid_argument("Number of paths must be positive");
    if (steps == 0)
        throw std::invalid_argument("Number of steps must be positive");
}

double MonteCarloPricer::price() const
{
    double T = option_.maturity();
    double dt = T / static_cast<double>(steps_);

    std::mt19937 gen(seed_);
    std::normal_distribution<> N(0.0, 1.0);

    double sum = 0.0;

    // Variables antithétiques pour réduction de variance
    if (use_antithetic_)
    {
        std::size_t half_paths = paths_ / 2;
        
        for (std::size_t i = 0; i < half_paths; ++i)
        {
            // Path 1 avec Z
            std::vector<double> path1(steps_ + 1);
            path1[0] = S0_;

            for (std::size_t j = 1; j <= steps_; ++j)
            {
                double Z = N(gen);
                path1[j] = path1[j - 1] * std::exp(
                    (b_ - 0.5 * sigma_ * sigma_) * dt
                    + sigma_ * std::sqrt(dt) * Z
                );
            }

            // Path 2 avec -Z (antithétique)
            std::vector<double> path2(steps_ + 1);
            path2[0] = S0_;

            for (std::size_t j = 1; j <= steps_; ++j)
            {
                double Z = -N(gen);  // Utilise la même variable aléatoire négative
                path2[j] = path2[j - 1] * std::exp(
                    (b_ - 0.5 * sigma_ * sigma_) * dt
                    + sigma_ * std::sqrt(dt) * Z
                );
            }

            sum += option_.payoff()(path1);
            sum += option_.payoff()(path2);
        }
        
        // Si nombre impair de paths, ajouter un dernier
        if (paths_ % 2 == 1)
        {
            auto path = simulate_path(gen);
            sum += option_.payoff()(path);
        }
    }
    else
    {
        // Sans variables antithétiques
        for (std::size_t i = 0; i < paths_; ++i)
        {
            auto path = simulate_path(gen);
            sum += option_.payoff()(path);
        }
    }

    return std::exp(-r_ * T) * (sum / static_cast<double>(paths_));
}

// Prix avec intervalle de confiance
MCResult MonteCarloPricer::price_with_confidence() const
{
    double T = option_.maturity();
    std::mt19937 gen(seed_);

    std::vector<double> payoffs(paths_);

    // Calculer tous les payoffs
    for (std::size_t i = 0; i < paths_; ++i)
    {
        auto path = simulate_path(gen);
        payoffs[i] = option_.payoff()(path);
    }

    // Calculer la moyenne
    double mean = std::accumulate(payoffs.begin(), payoffs.end(), 0.0) / static_cast<double>(paths_);
    
    // Calculer la variance
    double variance = 0.0;
    for (double p : payoffs)
        variance += (p - mean) * (p - mean);
    variance /= static_cast<double>(paths_ - 1);
    
    // Prix actualisé
    double price = std::exp(-r_ * T) * mean;
    
    // Erreur standard
    double std_error = std::sqrt(variance / static_cast<double>(paths_)) * std::exp(-r_ * T);
    
    // Intervalle de confiance à 95% (1.96 * σ)
    MCResult result;
    result.price = price;
    result.std_error = std_error;
    result.ci_lower_95 = price - 1.96 * std_error;
    result.ci_upper_95 = price + 1.96 * std_error;
    
    return result;
}

double MonteCarloPricer::delta(double spot) const
{
    // Méthode par différences finies
    double h = 1e-4 * spot;

    MonteCarloPricer up(option_, spot + h, r_, b_, sigma_, paths_, steps_, seed_, use_antithetic_);
    MonteCarloPricer down(option_, spot - h, r_, b_, sigma_, paths_, steps_, seed_, use_antithetic_);

    return (up.price() - down.price()) / (2.0 * h);
}

// Delta pathwise
double MonteCarloPricer::delta_pathwise() const
{
    // Cette méthode fonctionne pour les options européennes
    // Pour les exotiques, il faut une implémentation spécifique
    
    double T = option_.maturity();
    // double dt = T / static_cast<double>(steps_);

    std::mt19937 gen(seed_);
    std::normal_distribution<> N(0.0, 1.0);

    double sum_delta = 0.0;

    for (std::size_t i = 0; i < paths_; ++i)
    {
        std::vector<double> randoms(steps_);
        for (auto& z : randoms)
            z = N(gen);

        auto path = simulate_path_with_randoms(randoms);
        
        double ST = path.back();
        
        // Dérivée du terminal par rapport au spot initial
        double dST_dS0 = ST / S0_;
        
        // Dérivée du payoff par rapport au terminal
        double dpayoff_dST = option_.payoff().payoff_derivative(ST);
        
        sum_delta += dpayoff_dST * dST_dS0;
    }

    return std::exp(-r_ * T) * (sum_delta / static_cast<double>(paths_));
}

// Vega par Monte Carlo
double MonteCarloPricer::vega() const
{
    double h = 1e-4;  // Perturbation de volatilité

    MonteCarloPricer up(option_, S0_, r_, b_, sigma_ + h, paths_, steps_, seed_, use_antithetic_);
    MonteCarloPricer down(option_, S0_, r_, b_, sigma_ - h, paths_, steps_, seed_, use_antithetic_);

    return (up.price() - down.price()) / (2.0 * h);
}

/* =========================================================
   FONCTIONS PRIVÉES
   ========================================================= */

std::vector<double> MonteCarloPricer::simulate_path(std::mt19937& gen) const
{
    double T = option_.maturity();
    double dt = T / static_cast<double>(steps_);
    
    std::normal_distribution<> N(0.0, 1.0);
    
    std::vector<double> path(steps_ + 1);
    path[0] = S0_;

    for (std::size_t j = 1; j <= steps_; ++j)
    {
        double Z = N(gen);
        path[j] = path[j - 1] * std::exp(
            (b_ - 0.5 * sigma_ * sigma_) * dt
            + sigma_ * std::sqrt(dt) * Z
        );
    }

    return path;
}

std::vector<double> MonteCarloPricer::simulate_path_with_randoms(
    const std::vector<double>& randoms) const
{
    double T = option_.maturity();
    double dt = T / static_cast<double>(steps_);
    
    std::vector<double> path(steps_ + 1);
    path[0] = S0_;

    for (std::size_t j = 1; j <= steps_; ++j)
    {
        path[j] = path[j - 1] * std::exp(
            (b_ - 0.5 * sigma_ * sigma_) * dt
            + sigma_ * std::sqrt(dt) * randoms[j - 1]
        );
    }

    return path;
}