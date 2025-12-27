#pragma once

#include "payoff.hpp"
#include <memory>

/* =========================================================
   OPTION
   ========================================================= */
class Option
{
public:
    // On utilise shared_ptr au lieu de référence
    // Évite les références pendantes (dangling references : Un dangling (pendouillant/errant) est un pointeur ou référence qui pointe vers une mémoire qui n'existe plus ou a été libérée.)
    // Ce qui permet une gestion sûre de la mémoire
    Option(double maturity, std::shared_ptr<Payoff> payoff);

    double maturity() const { return T_; }
    const Payoff& payoff() const { return *payoff_; }
    
    bool is_valid() const;

private:
    double T_;
    std::shared_ptr<Payoff> payoff_;  // Au lieu de const Payoff& payoff_
};