#pragma once

#include "pricer.hpp"
#include <vector>
#include <string>
#include <memory>

/* =========================================================
   STRUCTURE POUR UN POINT DE REBALANCEMENT
   ========================================================= */
struct RebalancingPoint
{
    double time;           // Temps t
    double spot;           // Prix du sous-jacent S_t
    double delta;          // Position en actions delta_t
    double bond;           // Position en obligations B_t
    double option_value;   // Valeur de l'option V_t
    double portfolio_value;// Valeur du portefeuille
    double pnl;            // P&L du rebalancement
};

/* =========================================================
   STRATÉGIE DE RÉPLICATION
   ========================================================= */
class ReplicationStrategy
{
public:
    ReplicationStrategy(const Pricer& pricer,
                       double initial_spot,
                       double rate,
                       std::size_t rebalancing_frequency = 100);

    // Simuler la stratégie sur une trajectoire donnée
    std::vector<RebalancingPoint> simulate_on_path(
        const std::vector<double>& price_path,
        const std::vector<double>& time_points) const;

    // Simuler avec trajectoire brownienne
    std::vector<RebalancingPoint> simulate_with_brownian(
        double volatility,
        double carry,
        double maturity,
        unsigned int seed = 42) const;

    // Statistiques de la stratégie
    struct StrategyStats
    {
        double total_pnl;
        double avg_rebalancing_cost;
        double max_drawdown;
        double final_error;  // Erreur de réplication finale
        std::size_t num_rebalances;
    };

    StrategyStats compute_statistics(
        const std::vector<RebalancingPoint>& path) const;

    // Génération de rapport
    std::string generate_report(
        const std::vector<RebalancingPoint>& path) const;

    // Exportation vers CSV
    void export_to_csv(
        const std::vector<RebalancingPoint>& path,
        const std::string& filename) const;

private:
    const Pricer& pricer_;
    double S0_;
    double r_;
    std::size_t rebalancing_freq_;
};

/* =========================================================
   ANALYSEUR DE HEDGING ERROR
   ========================================================= */
class HedgingErrorAnalyzer
{
public:
    struct ErrorAnalysis
    {
        double mean_error;
        double std_error;
        double max_error;
        double rmse;  // Root Mean Square Error
        std::vector<double> error_distribution;
    };

    static ErrorAnalysis analyze_hedging_performance(
        const Pricer& pricer,
        double spot,
        double rate,
        double carry,
        double volatility,
        double maturity,
        std::size_t num_simulations,
        std::size_t rebalancing_freq);

private:
    static double simulate_one_path_error(
        const Pricer& pricer,
        double spot,
        double rate,
        double carry,
        double volatility,
        double maturity,
        std::size_t rebalancing_freq,
        unsigned int seed);
};