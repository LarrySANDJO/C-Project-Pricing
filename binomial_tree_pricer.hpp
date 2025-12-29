#pragma once

#include "pricer.hpp"
#include "option.hpp"
#include <vector>

/* =========================================================
   ARBRES BINOMIAUX – OPTIONS AMÉRICAINES ET EUROPÉENNES
   ========================================================= */

class BinomialTreePricer : public Pricer
{
public:
    enum class TreeType
    {
        CoxRossRubinstein,  // Arbre CRR standard
        JarrowRudd        // Arbre JR (drift matching)
    };

    BinomialTreePricer(const Option& option,
                       double spot,
                       double rate,
                       double carry,
                       double volatility,
                       std::size_t steps, // std::size_t pour les tailles non négatives et tres grandes
                       bool is_american = false,
                       TreeType type = TreeType::CoxRossRubinstein);

    double price() const override;
    double delta(double spot) const override;
    
    // Greeks par arbres binomiaux
    double gamma(double spot) const override;
    double theta() const override;
    
    // Accès aux paramètres de l'arbre (utile pour debugging)
    double get_up_factor() const { return u_; }
    double get_down_factor() const { return d_; }
    double get_risk_neutral_prob() const { return p_; }

private:
    // Calcul des paramètres de l'arbre selon le type
    void compute_tree_parameters();
    
    // CRR : Cox-Ross-Rubinstein
    void compute_crr_parameters();
    
    // JR : Jarrow-Rudd
    void compute_jr_parameters();
    
    // Construction et évaluation de l'arbre
    double evaluate_tree() const;

    const Option& option_;
    double S0_, r_, b_, sigma_;
    std::size_t N_;  // Nombre de pas
    bool is_american_;
    TreeType type_;
    
    // Paramètres de l'arbre
    double dt_;  // Pas de temps
    double u_;   // Facteur de montée
    double d_;   // Facteur de descente
    double p_;   // Probabilité risque-neutre
    double df_;  // Facteur d'actualisation
};

