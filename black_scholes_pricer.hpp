#pragma once


#include "pricer.hpp"
#include "option.hpp"

/* =========================================================
   BLACK–SCHOLES ANALYTIQUE (EUROPÉEN)
   ========================================================= */
class BlackScholesPricer : public Pricer
{
public:
    BlackScholesPricer(const Option& option,
                       double spot,
                       double rate,
                       double carry,
                       double volatility);

    double price() const override;
    double delta(double spot) const override;
    
    // Greeks 
    double gamma(double spot) const override;
    double vega() const override;
    double theta() const override;
    double rho() const override;

private:
    double calc_d1(double spot) const;
    double calc_d2(double spot) const;
    
    // Fonction de répartition normale standard
    static double N(double x); // on met en static car elle ne dépend pas de l'état de l'objet
    
    // Densité normale standard (pour gamma et vega)
    static double n(double x); // on met aussi en static

    const Option& option_;
    double S_, r_, b_, sigma_;
    
    // Mise en cache pour éviter les recalculs (surtout pour l'appel depuis Python)
    mutable double cached_price_;
    mutable bool price_cached_;
};

