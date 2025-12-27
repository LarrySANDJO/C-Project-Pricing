#include "finite_difference_pricer.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>

/* =========================================================
   FINITE DIFFERENCE - IMPLÉMENTATION
   ========================================================= */

FiniteDifferenceAmericanPricer::FiniteDifferenceAmericanPricer(
    const Option& option,
    double spot,
    double rate,
    double carry,
    double volatility,
    std::size_t M,
    std::size_t N,
    Scheme scheme)
    : option_(option),
      S0_(spot),
      r_(rate),
      b_(carry),
      sigma_(volatility),
      M_(M),
      N_(N),
      Smax_(3.0 * spot),
      scheme_(scheme)
{
    // AMÉLIORATION #7 : Validation
    if (spot <= 0.0)
        throw std::invalid_argument("Spot must be positive");
    if (volatility <= 0.0)
        throw std::invalid_argument("Volatility must be positive");
    if (M < 10)
        throw std::invalid_argument("M too small (need at least 10 space points)");
    if (N < 10)
        throw std::invalid_argument("N too small (need at least 10 time points)");
}

double FiniteDifferenceAmericanPricer::price() const
{
    switch (scheme_)
    {
    case Scheme::Explicit:
        return price_explicit();
    case Scheme::Implicit:
        return price_implicit();
    case Scheme::CrankNicolson:
        return price_crank_nicolson();
    default:
        throw std::runtime_error("Unknown scheme");
    }
}

double FiniteDifferenceAmericanPricer::price_explicit() const
{
    // Schéma explicite 
    double T = option_.maturity();
    double dS = Smax_ / static_cast<double>(M_);
    double dt = T / static_cast<double>(N_);

    std::vector<double> grid(M_ + 1), newGrid(M_ + 1);

    // Condition terminale
    for (std::size_t i = 0; i <= M_; ++i)
        grid[i] = option_.payoff().payoff_spot(static_cast<double>(i) * dS);

    // Remonter dans le temps
    for (std::size_t n = N_; n-- > 0;)
    {
        for (std::size_t i = 1; i < M_; ++i)
        {
            double S = static_cast<double>(i) * dS;

            double delta = (grid[i + 1] - grid[i - 1]) / (2.0 * dS);
            double gamma = (grid[i + 1] - 2.0 * grid[i] + grid[i - 1]) / (dS * dS);

            double cont = grid[i] + dt * (
                0.5 * sigma_ * sigma_ * S * S * gamma
                + b_ * S * delta
                - r_ * grid[i]);

            newGrid[i] = std::max(cont, option_.payoff().payoff_spot(S));
        }

        newGrid[0] = option_.payoff().payoff_spot(0.0);
        newGrid[M_] = option_.payoff().payoff_spot(Smax_);

        grid.swap(newGrid);
    }

    // Interpolation linéaire
    std::size_t i = static_cast<std::size_t>(S0_ / dS);
    double w = (S0_ - static_cast<double>(i) * dS) / dS;
    return (1.0 - w) * grid[i] + w * grid[i + 1];
}

// Schéma implicite (plus stable)
double FiniteDifferenceAmericanPricer::price_implicit() const
{
    double T = option_.maturity();
    double dS = Smax_ / static_cast<double>(M_);
    double dt = T / static_cast<double>(N_);

    std::vector<double> grid(M_ + 1), newGrid(M_ + 1);

    // Condition terminale
    for (std::size_t i = 0; i <= M_; ++i)
        grid[i] = option_.payoff().payoff_spot(static_cast<double>(i) * dS);

    // Coefficients pour système tridiagonal
    std::vector<double> a(M_ + 1), b(M_ + 1), c(M_ + 1), d(M_ + 1);

    // Remonter dans le temps
    for (std::size_t n = N_; n-- > 0;)
    {
        // Construire système tridiagonal
        for (std::size_t i = 1; i < M_; ++i)
        {
            double i_dbl = static_cast<double>(i);
            // double S = i_dbl * dS;
            
            double alpha = 0.5 * dt * (sigma_ * sigma_ * i_dbl * i_dbl - b_ * i_dbl / dS);
            double beta = -dt * (sigma_ * sigma_ * i_dbl * i_dbl + r_);
            double gamma = 0.5 * dt * (sigma_ * sigma_ * i_dbl * i_dbl + b_ * i_dbl / dS);

            a[i] = alpha;
            b[i] = 1.0 - beta;
            c[i] = gamma;
            d[i] = grid[i];
        }

        // Conditions aux bords
        b[0] = 1.0;
        c[0] = 0.0;
        d[0] = option_.payoff().payoff_spot(0.0);

        a[M_] = 0.0;
        b[M_] = 1.0;
        d[M_] = option_.payoff().payoff_spot(Smax_);

        // Résoudre système
        solve_tridiagonal(a, b, c, d, newGrid);

        // Contrainte américaine
        for (std::size_t i = 0; i <= M_; ++i)
        {
            double S = static_cast<double>(i) * dS;
            newGrid[i] = std::max(newGrid[i], option_.payoff().payoff_spot(S));
        }

        grid.swap(newGrid);
    }

    std::size_t i = static_cast<std::size_t>(S0_ / dS);
    double w = (S0_ - static_cast<double>(i) * dS) / dS;
    return (1.0 - w) * grid[i] + w * grid[i + 1];
}

// Crank-Nicolson (θ = 0.5, plus précis)
double FiniteDifferenceAmericanPricer::price_crank_nicolson() const
{
    double T = option_.maturity();
    double dS = Smax_ / static_cast<double>(M_);
    double dt = T / static_cast<double>(N_);

    std::vector<double> grid(M_ + 1), newGrid(M_ + 1);

    // Condition terminale
    for (std::size_t i = 0; i <= M_; ++i)
        grid[i] = option_.payoff().payoff_spot(static_cast<double>(i) * dS);

    std::vector<double> a(M_ + 1), b(M_ + 1), c(M_ + 1), d(M_ + 1);

    // Remonter dans le temps
    for (std::size_t n = N_; n-- > 0;)
    {
        // Crank-Nicolson : moyenne entre explicite et implicite
        for (std::size_t i = 1; i < M_; ++i)
        {
            double i_dbl = static_cast<double>(i);
            // double S = i_dbl * dS;
            
            double alpha = 0.25 * dt * (sigma_ * sigma_ * i_dbl * i_dbl - b_ * i_dbl / dS);
            double beta = -0.5 * dt * (sigma_ * sigma_ * i_dbl * i_dbl + r_);
            double gamma = 0.25 * dt * (sigma_ * sigma_ * i_dbl * i_dbl + b_ * i_dbl / dS);

            // Partie implicite (LHS)
            a[i] = -alpha;
            b[i] = 1.0 - beta;
            c[i] = -gamma;

            // Partie explicite (RHS)
            d[i] = alpha * grid[i - 1] + (1.0 + beta) * grid[i] + gamma * grid[i + 1];
        }

        // Conditions aux bords
        b[0] = 1.0;
        c[0] = 0.0;
        d[0] = option_.payoff().payoff_spot(0.0);

        a[M_] = 0.0;
        b[M_] = 1.0;
        d[M_] = option_.payoff().payoff_spot(Smax_);

        // Résoudre système
        solve_tridiagonal(a, b, c, d, newGrid);

        // Contrainte américaine
        for (std::size_t i = 0; i <= M_; ++i)
        {
            double S = static_cast<double>(i) * dS;
            newGrid[i] = std::max(newGrid[i], option_.payoff().payoff_spot(S));
        }

        grid.swap(newGrid);
    }

    std::size_t i = static_cast<std::size_t>(S0_ / dS);
    double w = (S0_ - static_cast<double>(i) * dS) / dS;
    return (1.0 - w) * grid[i] + w * grid[i + 1];
}

double FiniteDifferenceAmericanPricer::delta(double spot) const
{
    double h = 1e-4 * spot;

    FiniteDifferenceAmericanPricer up(option_, spot + h, r_, b_, sigma_, M_, N_, scheme_);
    FiniteDifferenceAmericanPricer down(option_, spot - h, r_, b_, sigma_, M_, N_, scheme_);

    return (up.price() - down.price()) / (2.0 * h);
}

/* =========================================================
   ALGORITHME THOMAS POUR SYSTÈMES TRIDIAGONAUX
   ========================================================= */
void FiniteDifferenceAmericanPricer::solve_tridiagonal(
    const std::vector<double>& a,
    const std::vector<double>& b,
    const std::vector<double>& c,
    const std::vector<double>& d,
    std::vector<double>& x) const
{
    std::size_t n = d.size();
    std::vector<double> c_prime(n);
    std::vector<double> d_prime(n);

    // Forward sweep
    c_prime[0] = c[0] / b[0];
    d_prime[0] = d[0] / b[0];

    for (std::size_t i = 1; i < n; ++i)
    {
        double m = 1.0 / (b[i] - a[i] * c_prime[i - 1]);
        c_prime[i] = c[i] * m;
        d_prime[i] = (d[i] - a[i] * d_prime[i - 1]) * m;
    }

    // Back substitution
    x[n - 1] = d_prime[n - 1];
    for (std::size_t i = n - 1; i-- > 0;)
        x[i] = d_prime[i] - c_prime[i] * x[i + 1];
}