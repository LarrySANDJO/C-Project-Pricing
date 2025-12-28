#pragma once


/* =========================================================
   INTERFACE PRICER
   ========================================================= */
class Pricer
{
public:
    virtual ~Pricer() = default;

    virtual double price() const = 0;
    virtual double delta(double spot) const = 0;

    // Ajout des sensibilités importantes pour la gestion du risque
    virtual double gamma(double spot) const;  // ∂²V/∂S²
    virtual double vega() const;              // ∂V/∂σ
    virtual double theta() const;             // ∂V/∂t
    virtual double rho() const;               // ∂V/∂r

    // Position en obligations pour réplication
    double bond_position(double spot) const
    {
        return price() - delta(spot) * spot;
    }
};