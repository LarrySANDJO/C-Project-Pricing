#include "black_scholes_pricer.hpp"
#include <cmath>
#include <stdexcept>

/* =========================================================
   BLACK-SCHOLES - IMPLÉMENTATION
   ========================================================= */

BlackScholesPricer::BlackScholesPricer(const Option& option,
                                       double spot,
                                       double rate,
                                       double carry,
                                       double volatility)
    : option_(option),
      S_(spot),
      r_(rate),
      b_(carry),
      sigma_(volatility),
      cached_price_(-1.0),
      price_cached_(false)
{
    // AMÉLIORATION #7 : Validation des paramètres
    if (spot <= 0.0)
        throw std::invalid_argument("Spot must be positive");
    if (volatility <= 0.0)
        throw std::invalid_argument("Volatility must be positive");
    if (rate < 0.0)
        throw std::invalid_argument("Rate cannot be negative");
}

double BlackScholesPricer::price() const
{
    // AMÉLIORATION #5 : Cache pour éviter recalculs
    if (price_cached_)
        return cached_price_;

    double T = option_.maturity();
    double K = option_.payoff().strike();
    double d1 = calc_d1(S_);
    double d2 = calc_d2(S_);

    double df = std::exp(-r_ * T);       // Facteur d'actualisation
    double ff = std::exp((b_ - r_) * T); // Facteur forward

    double result;
    
    // Utilisation le type()
    if (option_.payoff().type() == OptionType::Call)
        result = S_ * ff * N(d1) - K * df * N(d2);
    else
        result = K * df * N(-d2) - S_ * ff * N(-d1);

    cached_price_ = result;
    price_cached_ = true;
    
    return result;
}

double BlackScholesPricer::delta(double spot) const
{
    double T = option_.maturity();
    double d1 = calc_d1(spot);
    double ff = std::exp((b_ - r_) * T);
    
    // AMÉLIORATION #2 : Correction selon le type
    if (option_.payoff().type() == OptionType::Call)
        return ff * N(d1);
    else
        return ff * (N(d1) - 1.0);
}

// AMÉLIORATION #11 : Greeks analytiques
double BlackScholesPricer::gamma(double spot) const
{
    double T = option_.maturity();
    double d1 = calc_d1(spot);
    double ff = std::exp((b_ - r_) * T);
    
    return (ff * n(d1)) / (spot * sigma_ * std::sqrt(T));
}

double BlackScholesPricer::vega() const
{
    double T = option_.maturity();
    double d1 = calc_d1(S_);
    double ff = std::exp((b_ - r_) * T);
    
    // Vega est identique pour Call et Put
    return S_ * ff * n(d1) * std::sqrt(T);
}

double BlackScholesPricer::theta() const
{
    double T = option_.maturity();
    double K = option_.payoff().strike();
    double d1 = calc_d1(S_);
    double d2 = calc_d2(S_);
    
    double df = std::exp(-r_ * T);
    double ff = std::exp((b_ - r_) * T);
    
    double term1 = -(S_ * ff * n(d1) * sigma_) / (2.0 * std::sqrt(T));
    
    if (option_.payoff().type() == OptionType::Call)
    {
        double term2 = -(b_ - r_) * S_ * ff * N(d1);
        double term3 = -r_ * K * df * N(d2);
        return term1 + term2 + term3;
    }
    else
    {
        double term2 = (b_ - r_) * S_ * ff * N(-d1);
        double term3 = r_ * K * df * N(-d2);
        return term1 + term2 + term3;
    }
}

double BlackScholesPricer::rho() const
{
    double T = option_.maturity();
    double K = option_.payoff().strike();
    double d2 = calc_d2(S_);
    double df = std::exp(-r_ * T);
    
    if (option_.payoff().type() == OptionType::Call)
        return K * T * df * N(d2);
    else
        return -K * T * df * N(-d2);
}

/* =========================================================
   FONCTIONS PRIVÉES
   ========================================================= */

double BlackScholesPricer::calc_d1(double spot) const
{
    double T = option_.maturity();
    double K = option_.payoff().strike();
    
    return (std::log(spot / K) + (b_ + 0.5 * sigma_ * sigma_) * T)
           / (sigma_ * std::sqrt(T));
}

double BlackScholesPricer::calc_d2(double spot) const
{
    double T = option_.maturity();
    return calc_d1(spot) - sigma_ * std::sqrt(T);
}

double BlackScholesPricer::N(double x)
{
    // Fonction de répartition normale standard
    return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

double BlackScholesPricer::n(double x)
{
    // Densité normale standard : φ(x) = (1/√(2π)) * e^(-x²/2)
    static const double inv_sqrt_2pi = 0.3989422804014327;
    return inv_sqrt_2pi * std::exp(-0.5 * x * x);
}