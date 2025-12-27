#include "replication_strategy.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>

/* =========================================================
   STRATÉGIE DE RÉPLICATION - IMPLÉMENTATION
   ========================================================= */

ReplicationStrategy::ReplicationStrategy(const Pricer& pricer,
                                        double initial_spot,
                                        double rate,
                                        std::size_t rebalancing_frequency)
    : pricer_(pricer),
      S0_(initial_spot),
      r_(rate),
      rebalancing_freq_(rebalancing_frequency)
{
    if (initial_spot <= 0.0)
        throw std::invalid_argument("Initial spot must be positive");
    if (rebalancing_frequency == 0)
        throw std::invalid_argument("Rebalancing frequency must be positive");
}

std::vector<RebalancingPoint> ReplicationStrategy::simulate_on_path(
    const std::vector<double>& price_path,
    const std::vector<double>& time_points) const
{
    if (price_path.size() != time_points.size())
        throw std::invalid_argument("Price path and time points must have same size");

    std::vector<RebalancingPoint> strategy;
    strategy.reserve(price_path.size());

    // Point initial
    RebalancingPoint initial;
    initial.time = time_points[0];
    initial.spot = price_path[0];
    initial.option_value = pricer_.price();
    initial.delta = pricer_.delta(initial.spot);
    initial.bond = initial.option_value - initial.delta * initial.spot;
    initial.portfolio_value = initial.option_value;
    initial.pnl = 0.0;

    strategy.push_back(initial);

    // Rebalancement aux points suivants
    for (std::size_t i = 1; i < price_path.size(); ++i)
    {
        RebalancingPoint point;
        point.time = time_points[i];
        point.spot = price_path[i];

        // Position précédente
        const auto& prev = strategy.back();
        double dt = point.time - prev.time;

        // Croissance de la position obligataire
        double bond_growth = prev.bond * std::exp(r_ * dt);

        // Valeur du portefeuille avant rebalancement
        double portfolio_before = prev.delta * point.spot + bond_growth;

        // Nouveau delta (rebalancement)
        point.delta = pricer_.delta(point.spot);

        // Coût de transaction (pour information) - variable inutilisée, donc supprimée
        // double delta_change = point.delta - prev.delta;
        // double transaction_cost = std::abs(delta_change) * point.spot;

        // Nouvelle position obligataire
        point.bond = portfolio_before - point.delta * point.spot;

        // Valeur du portefeuille
        point.portfolio_value = point.delta * point.spot + point.bond;

        // Valeur théorique de l'option (pour comparaison)
        point.option_value = pricer_.price();  // Devrait tenir compte du temps

        // P&L du rebalancement
        point.pnl = point.portfolio_value - portfolio_before;

        strategy.push_back(point);
    }

    return strategy;
}

std::vector<RebalancingPoint> ReplicationStrategy::simulate_with_brownian(
    double volatility,
    double carry,
    double maturity,
    unsigned int seed) const
{
    std::mt19937 gen(seed);
    std::normal_distribution<> N(0.0, 1.0);

    // Générer une trajectoire
    double dt = maturity / static_cast<double>(rebalancing_freq_);
    std::vector<double> price_path(rebalancing_freq_ + 1);
    std::vector<double> time_points(rebalancing_freq_ + 1);

    price_path[0] = S0_;
    time_points[0] = 0.0;

    for (std::size_t i = 1; i <= rebalancing_freq_; ++i)
    {
        double Z = N(gen);
        price_path[i] = price_path[i-1] * std::exp(
            (carry - 0.5 * volatility * volatility) * dt
            + volatility * std::sqrt(dt) * Z
        );
        time_points[i] = static_cast<double>(i) * dt;
    }

    return simulate_on_path(price_path, time_points);
}

ReplicationStrategy::StrategyStats ReplicationStrategy::compute_statistics(
    const std::vector<RebalancingPoint>& path) const
{
    StrategyStats stats;
    stats.num_rebalances = path.size() - 1;

    if (path.empty())
    {
        stats.total_pnl = 0.0;
        stats.avg_rebalancing_cost = 0.0;
        stats.max_drawdown = 0.0;
        stats.final_error = 0.0;
        return stats;
    }

    // Total P&L
    stats.total_pnl = 0.0;
    for (std::size_t i = 1; i < path.size(); ++i)
        stats.total_pnl += path[i].pnl;

    // Coût moyen de rebalancement
    double total_cost = 0.0;
    for (std::size_t i = 1; i < path.size(); ++i)
    {
        double delta_change = std::abs(path[i].delta - path[i-1].delta);
        total_cost += delta_change * path[i].spot;
    }
    stats.avg_rebalancing_cost = total_cost / static_cast<double>(stats.num_rebalances);

    // Maximum drawdown
    double peak = path[0].portfolio_value;
    stats.max_drawdown = 0.0;
    for (const auto& point : path)
    {
        if (point.portfolio_value > peak)
            peak = point.portfolio_value;
        
        double drawdown = (peak - point.portfolio_value) / peak;
        if (drawdown > stats.max_drawdown)
            stats.max_drawdown = drawdown;
    }

    // Erreur finale de réplication
    const auto& final_point = path.back();
    stats.final_error = std::abs(final_point.portfolio_value - final_point.option_value);

    return stats;
}

std::string ReplicationStrategy::generate_report(
    const std::vector<RebalancingPoint>& path) const
{
    auto stats = compute_statistics(path);

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4);
    oss << "========== REPLICATION STRATEGY REPORT ==========\n";
    oss << "Number of rebalances    : " << stats.num_rebalances << "\n";
    oss << "Total P&L               : " << stats.total_pnl << "\n";
    oss << "Avg rebalancing cost    : " << stats.avg_rebalancing_cost << "\n";
    oss << "Maximum drawdown        : " << (stats.max_drawdown * 100) << "%\n";
    oss << "Final replication error : " << stats.final_error << "\n";
    oss << "================================================\n";

    return oss.str();
}

void ReplicationStrategy::export_to_csv(
    const std::vector<RebalancingPoint>& path,
    const std::string& filename) const
{
    std::ofstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Cannot open file: " + filename);

    // En-tête
    file << "Time,Spot,Delta,Bond,OptionValue,PortfolioValue,PnL\n";

    // Données
    file << std::fixed << std::setprecision(6);
    for (const auto& point : path)
    {
        file << point.time << ","
             << point.spot << ","
             << point.delta << ","
             << point.bond << ","
             << point.option_value << ","
             << point.portfolio_value << ","
             << point.pnl << "\n";
    }

    file.close();
}

/* =========================================================
   ANALYSEUR D'ERREUR DE HEDGING
   ========================================================= */

HedgingErrorAnalyzer::ErrorAnalysis HedgingErrorAnalyzer::analyze_hedging_performance(
    const Pricer& pricer,
    double spot,
    double rate,
    double carry,
    double volatility,
    double maturity,
    std::size_t num_simulations,
    std::size_t rebalancing_freq)
{
    ErrorAnalysis analysis;
    analysis.error_distribution.resize(num_simulations);

    // Simuler plusieurs trajectoires
    for (std::size_t i = 0; i < num_simulations; ++i)
    {
        double error = simulate_one_path_error(
            pricer, spot, rate, carry, volatility, maturity,
            rebalancing_freq, static_cast<unsigned int>(i)
        );
        analysis.error_distribution[i] = error;
    }

    // Statistiques
    analysis.mean_error = std::accumulate(
        analysis.error_distribution.begin(),
        analysis.error_distribution.end(),
        0.0
    ) / static_cast<double>(num_simulations);

    // Variance et écart-type
    double variance = 0.0;
    for (double error : analysis.error_distribution)
        variance += (error - analysis.mean_error) * (error - analysis.mean_error);
    variance /= static_cast<double>(num_simulations);
    analysis.std_error = std::sqrt(variance);

    // RMSE
    double sum_squared = 0.0;
    for (double error : analysis.error_distribution)
        sum_squared += error * error;
    analysis.rmse = std::sqrt(sum_squared / static_cast<double>(num_simulations));

    // Max error
    analysis.max_error = *std::max_element(
        analysis.error_distribution.begin(),
        analysis.error_distribution.end()
    );

    return analysis;
}

double HedgingErrorAnalyzer::simulate_one_path_error(
    const Pricer& pricer,
    double spot,
    double rate,
    double carry,
    double volatility,
    double maturity,
    std::size_t rebalancing_freq,
    unsigned int seed)
{
    ReplicationStrategy strategy(pricer, spot, rate, rebalancing_freq);
    auto path = strategy.simulate_with_brownian(volatility, carry, maturity, seed);
    auto stats = strategy.compute_statistics(path);
    
    return stats.final_error;
}