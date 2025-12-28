#include "binomial_tree_pricer.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <numbers>

/* =========================================================
   ARBRES BINOMIAUX - IMPLÉMENTATION
   ========================================================= */

BinomialTreePricer::BinomialTreePricer(const Option& option,
                                       double spot,
                                       double rate,
                                       double carry,
                                       double volatility,
                                       std::size_t steps,
                                       bool is_american,
                                       TreeType type)
    : option_(option),
      S0_(spot),
      r_(rate),
      b_(carry),
      sigma_(volatility),
      N_(steps),
      is_american_(is_american),
      type_(type),
      dt_(option.maturity() / static_cast<double>(steps)) // On convertit en double
{
    // Validation
    if (spot <= 0.0)
        throw std::invalid_argument("Spot must be positive");
    if (volatility <= 0.0)
        throw std::invalid_argument("Volatility must be positive");
    if (steps == 0)
        throw std::invalid_argument("Number of steps must be positive");
    
    // Calculer les paramètres selon le type d'arbre
    compute_tree_parameters();
}

void BinomialTreePricer::compute_tree_parameters()
{
    switch (type_)
    {
    case TreeType::CoxRossRubinstein:
        compute_crr_parameters();
        break;
    case TreeType::JarrowRudd:
        compute_jr_parameters();
        break;
    default:
        throw std::runtime_error("Unknown tree type");
    }
}

/* =========================================================
   COX-ROSS-RUBINSTEIN (CRR)
   Standard : u = e^(σ√Δt), d = 1/u
   ========================================================= */
void BinomialTreePricer::compute_crr_parameters()
{
    u_ = std::exp(sigma_ * std::sqrt(dt_));
    d_ = 1.0 / u_;
    df_ = std::exp(-r_ * dt_);
    
    // Probabilité risque-neutre
    double growth = std::exp(b_ * dt_);
    p_ = (growth - d_) / (u_ - d_);
    
    // Validation de la probabilité
    if (p_ < 0.0 || p_ > 1.0)
        throw std::runtime_error("Invalid risk-neutral probability (check parameters)");
}

/* =========================================================
   JARROW-RUDD (JR)
   Matching du drift : u = e^((b-σ²/2)Δt + σ√Δt)
   ========================================================= */
void BinomialTreePricer::compute_jr_parameters()
{
    double drift = (b_ - 0.5 * sigma_ * sigma_) * dt_;
    double diffusion = sigma_ * std::sqrt(dt_);
    
    u_ = std::exp(drift + diffusion);
    d_ = std::exp(drift - diffusion);
    df_ = std::exp(-r_ * dt_);
    
    // Probabilité risque-neutre (toujours 0.5 pour JR)
    p_ = 0.5;
}

/* =========================================================
   ÉVALUATION DE L'ARBRE
   ========================================================= */
double BinomialTreePricer::price() const
{
    return evaluate_tree();
}

double BinomialTreePricer::evaluate_tree() const
{
    // Vecteur pour stocker les valeurs aux nœuds
    std::vector<double> values(N_ + 1);
    
    // Condition terminale (payoff à maturité)
    for (std::size_t i = 0; i <= N_; ++i)
    {
        // Prix du sous-jacent au nœud (i, N)
        // S(i,N) = S₀ · u^i · d^(N-i)
        double S = S0_ * std::pow(u_, static_cast<double>(i)) 
                       * std::pow(d_, static_cast<double>(N_ - i));
        
        values[i] = option_.payoff().payoff_spot(S);
    }
    
    // Remontée dans l'arbre (backward induction)
    for (std::size_t n = N_; n-- > 0;)
    {
        for (std::size_t i = 0; i <= n; ++i)
        {
            // Prix du sous-jacent au nœud (i, n)
            double S = S0_ * std::pow(u_, static_cast<double>(i))
                           * std::pow(d_, static_cast<double>(n - i));
            
            // Valeur de continuation (espérance actualisée)
            double continuation = df_ * (p_ * values[i + 1] + (1.0 - p_) * values[i]);
            
            if (is_american_)
            {
                // Option américaine : max(continuation, exercice immédiat)
                double exercise = option_.payoff().payoff_spot(S);
                values[i] = std::max(continuation, exercise);
            }
            else
            {
                // Option européenne : continuation uniquement
                values[i] = continuation;
            }
        }
    }
    
    // La valeur à la racine est le prix de l'option
    return values[0];
}

/* =========================================================
   GREEKS PAR ARBRES BINOMIAUX
   ========================================================= */

double BinomialTreePricer::delta(double spot) const
{
    // Delta par différences finies : (V(S+h) - V(S-h)) / 2h
    double h = 1e-4 * spot;
    
    BinomialTreePricer up(option_, spot + h, r_, b_, sigma_, N_, is_american_, type_);
    BinomialTreePricer down(option_, spot - h, r_, b_, sigma_, N_, is_american_, type_);
    
    return (up.price() - down.price()) / (2.0 * h);
}

double BinomialTreePricer::gamma(double spot) const
{
    // Gamma à partir de l'arbre directement
    // Après un pas : on a 3 valeurs (haut, milieu, bas)
    
    if (N_ < 2)
    {
        // Fallback : différences finies
        return Pricer::gamma(spot);
    }
    
    // Valeurs après un pas
    double S_u = spot * u_;
    double S_d = spot * d_;
    
    // Construire des sous-arbres
    BinomialTreePricer tree_u(option_, S_u, r_, b_, sigma_, N_-1, is_american_, type_);
    BinomialTreePricer tree_d(option_, S_d, r_, b_, sigma_, N_-1, is_american_, type_);
    
    double V_u = tree_u.price();
    double V_d = tree_d.price();
    
    // Delta vers le haut et vers le bas
    double delta_u = (V_u - price()) / (S_u - spot);
    double delta_d = (price() - V_d) / (spot - S_d);
    
    // Gamma
    return (delta_u - delta_d) / (0.5 * (S_u - S_d));
}

double BinomialTreePricer::theta() const
{
    // Theta : dérivée par rapport au temps
    // On compare avec un arbre à N-2 pas (2Δt de différence)
    
    if (N_ < 3)
        throw std::runtime_error("Not enough steps to compute theta");
    
    BinomialTreePricer shorter(option_, S0_, r_, b_, sigma_, N_-2, is_american_, type_);
    
    double dt_diff = 2.0 * dt_;
    return (shorter.price() - price()) / dt_diff;
}