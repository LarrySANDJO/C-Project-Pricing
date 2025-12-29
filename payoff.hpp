#pragma once

#include "option_type.hpp"
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>

/* =========================================================
   PAYOFF (BASE POLYMORPHE)
   ========================================================= */
class Payoff
{
public:
    Payoff(double strike, OptionType type);
    virtual ~Payoff() = default;

    // Par défaut : payoff européen
    virtual double operator()(const std::vector<double>& path) const;
    
    double payoff_spot(double spot) const;
    
    // Getter pour le type d'option
    OptionType type() const { return type_; }
    
    double strike() const { return K_; }
    
    // Dérivée du payoff pour calcul pathwise
    virtual double payoff_derivative(double spot) const;

protected:
    OptionType type_;

private:
    double K_;
};

/* =========================================================
   PAYOFFS EXOTIQUES
   ========================================================= */

// ========== OPTIONS ASIATIQUES ==========

// Option asiatique (moyenne arithmétique) - Call
class AsianCallPayoff : public Payoff
{
public:
    explicit AsianCallPayoff(double strike);
    double operator()(const std::vector<double>& path) const override;
};

// Option asiatique (moyenne arithmétique) - Put
class AsianPutPayoff : public Payoff
{
public:
    explicit AsianPutPayoff(double strike);
    double operator()(const std::vector<double>& path) const override;
};

// Option asiatique (moyenne géométrique) - Call
class AsianGeometricCallPayoff : public Payoff
{
public:
    explicit AsianGeometricCallPayoff(double strike);
    double operator()(const std::vector<double>& path) const override;
};

// Option asiatique (moyenne géométrique) - Put

class AsianGeometricPutPayoff : public Payoff
{
public:
    explicit AsianGeometricPutPayoff(double strike);
    double operator()(const std::vector<double>& path) const override;
};

// ========== OPTIONS LOOKBACK ==========

// Option lookback (maximum) - Call
class LookbackCallPayoff : public Payoff
{
public:
    explicit LookbackCallPayoff(double strike);
    double operator()(const std::vector<double>& path) const override;
};

// Option lookback (minimum) - Put
class LookbackPutPayoff : public Payoff
{
public:
    explicit LookbackPutPayoff(double strike);
    double operator()(const std::vector<double>& path) const override;
};

// Option lookback flottant (max - S_T) - Call
class LookbackFloatingCallPayoff : public Payoff
{
public:
    LookbackFloatingCallPayoff();
    double operator()(const std::vector<double>& path) const override;
};

// Option lookback flottant (S_T - min) - Put

class LookbackFloatingPutPayoff : public Payoff
{
public:
    LookbackFloatingPutPayoff();
    double operator()(const std::vector<double>& path) const override;
};

// ========== OPTIONS BARRIÈRES ==========

// Option barrière up-and-out - Call
class BarrierUpOutCallPayoff : public Payoff
{
public:
    BarrierUpOutCallPayoff(double strike, double barrier);
    double operator()(const std::vector<double>& path) const override;

private:
    double barrier_;
};

// Option barrière up-and-out - Put
class BarrierUpOutPutPayoff : public Payoff
{
public:
    BarrierUpOutPutPayoff(double strike, double barrier);
    double operator()(const std::vector<double>& path) const override;
private:
    double barrier_;
};

// Option barrière down-and-out - Put
class BarrierDownOutPutPayoff : public Payoff
{
public:
    BarrierDownOutPutPayoff(double strike, double barrier);
    double operator()(const std::vector<double>& path) const override;

private:
    double barrier_;
};

// Option barrière up-and-in - Call
class BarrierUpInCallPayoff : public Payoff
{
public:
    BarrierUpInCallPayoff(double strike, double barrier);
    double operator()(const std::vector<double>& path) const override;

private:
    double barrier_;
};

// Option barrière down-and-in - Put
class BarrierDownInPutPayoff : public Payoff
{
public:
    BarrierDownInPutPayoff(double strike, double barrier);
    double operator()(const std::vector<double>& path) const override;

private:
    double barrier_;
};

// ========== OPTIONS DIGITALES ==========

// Option digitale (cash-or-nothing) - Call
class DigitalCallPayoff : public Payoff
{
public:
    DigitalCallPayoff(double strike, double cash_amount);
    double operator()(const std::vector<double>& path) const override;

private:
    double cash_;
};

// Option digitale (cash-or-nothing) - Put
class DigitalPutPayoff : public Payoff
{
public:
    DigitalPutPayoff(double strike, double cash_amount);
    double operator()(const std::vector<double>& path) const override;

private:
    double cash_;
};

// ========== OPTIONS POWER ==========

// Option power (payoff = (S_T - K)^α)
class PowerCallPayoff : public Payoff
{
public:
    PowerCallPayoff(double strike, double power);
    double operator()(const std::vector<double>& path) const override;

private:
    double power_;
};

// Option power put
class PowerPutPayoff : public Payoff
{
public:
    PowerPutPayoff(double strike, double power);
    double operator()(const std::vector<double>& path) const override;
private:
    double power_;
};

// ========== FACTORY AMÉLIORÉ ==========

// Factory pattern pour créer des payoffs
class PayoffFactory
{
public:
    enum class PayoffStyle
    {
        European,
        Asian,
        AsianGeometric,
        Lookback,
        LookbackFloating,
        BarrierUpOut,
        BarrierDownOut,
        BarrierUpIn,
        BarrierDownIn,
        Digital,
        Power
    };

    static std::shared_ptr<Payoff> create(
        PayoffStyle style,
        OptionType type,
        double strike,
        double param1 = 0.0); // barrier, cash, ou power
        // double param2 = 0.0); // paramètre supplémentaire
};
