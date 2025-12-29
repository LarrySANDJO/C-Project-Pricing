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
    virtual double gamma(double spot) const;  
    virtual double vega() const;              
    virtual double theta() const;             
    virtual double rho() const;               

    // Position en obligations pour réplication (definie directement)
    double bond_position(double spot) const
    {
        return price() - delta(spot) * spot;
    }
};