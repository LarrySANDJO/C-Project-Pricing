#pragma once
#include "Payoff.hpp"

class PayoffCall : public Payoff {
public:
    explicit PayoffCall(double strike);
    // Le explicit nous oblige a entrer explicitement le strike lors de la construction du payoff : Tout constructeur à un seul argument doit être explicit, sauf raison très solide

    double operator()(double spot) const override;
    // override pour prendre l'operator de Payoff (polymorphisme)

private:
    double strike_;
};
