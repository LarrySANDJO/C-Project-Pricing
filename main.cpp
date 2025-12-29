#include "option_type.hpp"
#include "payoff.hpp"
#include "option.hpp"
#include "black_scholes_pricer.hpp"
#include "monte_carlo_pricer.hpp"
#include "finite_difference_pricer.hpp"
#include "binomial_tree_pricer.hpp"
#include "replication_strategy.hpp"

#include <iostream>
#include <iomanip>
#include <memory>

/* =========================================================
   FONCTIONS UTILITAIRES POUR L'AFFICHAGE
   ========================================================= */

void print_separator()
{
    std::cout << std::string(75, '=') << std::endl;
}

void print_header(const std::string& title)
{
    print_separator();
    std::cout << "  " << title << std::endl;
    print_separator();
}

void print_price_result(const std::string& method, double price)
{
    std::cout << std::left << std::setw(40) << method 
              << ": " << std::fixed << std::setprecision(4) << price << std::endl;
}

void print_greeks(const std::string& label, const Pricer& pricer, double spot)
{
    std::cout << "\n" << label << " Greeks:" << std::endl;
    std::cout << "  Delta : " << std::fixed << std::setprecision(4) 
              << pricer.delta(spot) << std::endl;
    
    try {
        std::cout << "  Gamma : " << pricer.gamma(spot) << std::endl;
    } catch (const std::exception&) {
        std::cout << "  Gamma : N/A" << std::endl;
    }
    
    try {
        std::cout << "  Vega  : " << pricer.vega() << std::endl;
    } catch (const std::exception&) {
        std::cout << "  Vega  : N/A" << std::endl;
    }
    
    try {
        std::cout << "  Theta : " << pricer.theta() << std::endl;
    } catch (const std::exception&) {
        std::cout << "  Theta : N/A" << std::endl;
    }
    
    try {
        std::cout << "  Rho   : " << pricer.rho() << std::endl;
    } catch (const std::exception&) {
        std::cout << "  Rho   : N/A" << std::endl;
    }
}

/* =========================================================
   MAIN POUR LE TEST
   ========================================================= */

int main()
{
    // Paramètres de marché
    double S0 = 100.0;     // Spot
    double K = 100.0;      // Strike
    double T = 1.0;        // Maturité (1 an)
    double r = 0.05;       // Taux sans risque
    double b = r;          // Cost of carry
    double sigma = 0.2;    // Volatilité

    // Paramètres numériques
    std::size_t mc_paths = 100000;
    std::size_t mc_steps = 100;
    std::size_t tree_steps = 200;
    std::size_t fd_M = 200;
    std::size_t fd_N = 200;

    std::cout << std::fixed << std::setprecision(4);

    /* =================================================================
       PARTIE 1 : OPTIONS EUROPÉENNES VANILLE
       ================================================================= */
    print_header("PARTIE 1 : OPTIONS EUROPÉENNES VANILLE");

    auto callPayoff = PayoffFactory::create(
        PayoffFactory::PayoffStyle::European,
        OptionType::Call, K
    );
    Option europeanCall(T, callPayoff);

    // Black-Scholes
    BlackScholesPricer bs(europeanCall, S0, r, b, sigma);
    print_price_result("Black-Scholes (Analytique)", bs.price());
    print_greeks("Black-Scholes", bs, S0);

    // Monte Carlo avec variables antithétiques
    MonteCarloPricer mc(europeanCall, S0, r, b, sigma, mc_paths, mc_steps, 42, true);
    print_price_result("Monte Carlo (Var. antithétiques)", mc.price());
    
    MCResult mcResult = mc.price_with_confidence();
    std::cout << "  IC 95% : [" << mcResult.ci_lower_95 << ", " 
              << mcResult.ci_upper_95 << "]" << std::endl;

    // Arbre binomial CRR
    BinomialTreePricer tree_crr(europeanCall, S0, r, b, sigma, tree_steps, false,
                                BinomialTreePricer::TreeType::CoxRossRubinstein);
    print_price_result("Arbre Binomial CRR", tree_crr.price());

    /* =================================================================
       PARTIE 2 : OPTIONS AMÉRICAINES
       ================================================================= */
    print_header("PARTIE 2 : OPTIONS AMÉRICAINES PUT");

    auto putPayoff = PayoffFactory::create(
        PayoffFactory::PayoffStyle::European,
        OptionType::Put, K
    );
    Option americanPut(T, putPayoff);

    // Arbre binomial (américaine)
    BinomialTreePricer tree_am(americanPut, S0, r, b, sigma, tree_steps, true);
    print_price_result("Arbre Binomial (Américain)", tree_am.price());
    print_greeks("Arbre Binomial", tree_am, S0);

    // Différences finies Crank-Nicolson
    FiniteDifferenceAmericanPricer fd_cn(
        americanPut, S0, r, b, sigma, fd_M, fd_N,
        FiniteDifferenceAmericanPricer::Scheme::CrankNicolson
    );
    print_price_result("Différences Finies (Crank-Nicolson)", fd_cn.price());

    // Comparaison européenne vs américaine
    BinomialTreePricer tree_eu_put(americanPut, S0, r, b, sigma, tree_steps, false);
    print_price_result("Même Put Européen (référence)", tree_eu_put.price());
    std::cout << "Prime d'exercice anticipé : " 
              << (tree_am.price() - tree_eu_put.price()) << std::endl;

    /* =================================================================
       PARTIE 3 : OPTIONS EXOTIQUES - ASIATIQUES
       ================================================================= */
    print_header("PARTIE 3 : OPTIONS ASIATIQUES");

    // Asiatique arithmétique
    auto asianCall = PayoffFactory::create(
        PayoffFactory::PayoffStyle::Asian,
        OptionType::Call, K
    );
    Option asianOpt(T, asianCall);
    MonteCarloPricer mcAsian(asianOpt, S0, r, b, sigma, mc_paths, mc_steps, 42, true);
    print_price_result("Asian Call (Moyenne arithmétique)", mcAsian.price());

    // Asiatique géométrique
    auto asianGeoCall = PayoffFactory::create(
        PayoffFactory::PayoffStyle::AsianGeometric,
        OptionType::Call, K
    );
    Option asianGeoOpt(T, asianGeoCall);
    MonteCarloPricer mcAsianGeo(asianGeoOpt, S0, r, b, sigma, mc_paths, mc_steps, 42, true);
    print_price_result("Asian Call (Moyenne géométrique)", mcAsianGeo.price());

    /* =================================================================
       PARTIE 4 : OPTIONS LOOKBACK
       ================================================================= */
    print_header("PARTIE 4 : OPTIONS LOOKBACK");

    // Lookback fixe
    auto lookbackCall = PayoffFactory::create(
        PayoffFactory::PayoffStyle::Lookback,
        OptionType::Call, K
    );
    Option lookbackOpt(T, lookbackCall);
    MonteCarloPricer mcLookback(lookbackOpt, S0, r, b, sigma, mc_paths, mc_steps, 42, true);
    print_price_result("Lookback Call (Strike fixe)", mcLookback.price());

    // Lookback flottant
    auto lookbackFloat = PayoffFactory::create(
        PayoffFactory::PayoffStyle::LookbackFloating,
        OptionType::Call, 0.0
    );
    Option lookbackFloatOpt(T, lookbackFloat);
    MonteCarloPricer mcLookbackFloat(lookbackFloatOpt, S0, r, b, sigma, mc_paths, mc_steps, 42, true);
    print_price_result("Lookback Call (Strike flottant)", mcLookbackFloat.price());

    /* =================================================================
       PARTIE 5 : OPTIONS BARRIÈRES
       ================================================================= */
    print_header("PARTIE 5 : OPTIONS BARRIÈRES");

    double barrier_up = 130.0;

    // Up-and-Out Call
    auto barrierUpOut = PayoffFactory::create(
        PayoffFactory::PayoffStyle::BarrierUpOut,
        OptionType::Call, K, barrier_up
    );
    Option barrierUpOutOpt(T, barrierUpOut);
    MonteCarloPricer mcBarrierUpOut(barrierUpOutOpt, S0, r, b, sigma, mc_paths, mc_steps, 42, true);
    print_price_result("Barrier Up-and-Out Call (B=130)", mcBarrierUpOut.price());

    // Up-and-In Call
    auto barrierUpIn = PayoffFactory::create(
        PayoffFactory::PayoffStyle::BarrierUpIn,
        OptionType::Call, K, barrier_up
    );
    Option barrierUpInOpt(T, barrierUpIn);
    MonteCarloPricer mcBarrierUpIn(barrierUpInOpt, S0, r, b, sigma, mc_paths, mc_steps, 42, true);
    print_price_result("Barrier Up-and-In Call (B=130)", mcBarrierUpIn.price());

    // Vérification
    std::cout << "\nVérification : Up-Out + Up-In = " 
              << (mcBarrierUpOut.price() + mcBarrierUpIn.price())
              << " (Vanille = " << mc.price() << ")" << std::endl;

    /* =================================================================
       PARTIE 6 : OPTIONS DIGITALES
       ================================================================= */
    print_header("PARTIE 6 : OPTIONS DIGITALES");

    double cash_amount = 100.0;

    auto digitalCall = PayoffFactory::create(
        PayoffFactory::PayoffStyle::Digital,
        OptionType::Call, K, cash_amount
    );
    Option digitalCallOpt(T, digitalCall);
    MonteCarloPricer mcDigital(digitalCallOpt, S0, r, b, sigma, mc_paths, mc_steps, 42, true);
    print_price_result("Digital Call (Cash=100)", mcDigital.price());

    /* =================================================================
       PARTIE 7 : OPTIONS POWER
       ================================================================= */
    print_header("PARTIE 7 : OPTIONS POWER");

    double power = 2.0;

    auto powerCall = PayoffFactory::create(
        PayoffFactory::PayoffStyle::Power,
        OptionType::Call, K, power
    );
    Option powerCallOpt(T, powerCall);
    MonteCarloPricer mcPower(powerCallOpt, S0, r, b, sigma, mc_paths, mc_steps, 42, true);
    print_price_result("Power Call", mcPower.price());

    /* =================================================================
       PARTIE 8 : STRATÉGIE DE RÉPLICATION
       ================================================================= */
    print_header("PARTIE 8 : STRATÉGIE DE RÉPLICATION");

    std::cout << "\nPosition initiale de réplication (Call européen) :" << std::endl;
    std::cout << "  Prix de l'option  V₀ = " << bs.price() << std::endl;
    std::cout << "  Position en action Δ₀ = " << bs.delta(S0) << std::endl;
    std::cout << "  Position en bond   B₀ = " << bs.bond_position(S0) << std::endl;
    std::cout << "\nVérification : Δ₀·S₀ + B₀ = " 
              << (bs.delta(S0) * S0 + bs.bond_position(S0))
              << " = V₀" << std::endl;

    // Simulation de la stratégie
    std::cout << "\nSimulation de la stratégie de delta-hedging..." << std::endl;
    
    ReplicationStrategy strategy(bs, S0, r, 50);  // 50 rebalancements
    auto replication_path = strategy.simulate_with_brownian(sigma, b, T, 12345);
    
    std::cout << strategy.generate_report(replication_path);

    // Export vers CSV
    strategy.export_to_csv(replication_path, "replication_strategy.csv");
    std::cout << "Stratégie exportée vers 'replication_strategy.csv'" << std::endl;

    /* =================================================================
       PARTIE 9 : ANALYSE D'ERREUR DE HEDGING
       ================================================================= */
    print_header("PARTIE 9 : ANALYSE D'ERREUR DE HEDGING");

    std::cout << "Analyse sur 1000 simulations avec différentes fréquences..." << std::endl;

    for (int freq : {10, 25, 50, 100})
    {
        auto error_analysis = HedgingErrorAnalyzer::analyze_hedging_performance(
            bs, S0, r, b, sigma, T, 1000, static_cast<long unsigned int>(freq)
        );

        std::cout << "\nFréquence de rebalancement : " << static_cast<int>(freq) << " fois" << std::endl;
        std::cout << "  Erreur moyenne    : " << error_analysis.mean_error << std::endl;
        std::cout << "  Écart-type        : " << error_analysis.std_error << std::endl;
        std::cout << "  RMSE              : " << error_analysis.rmse << std::endl;
        std::cout << "  Erreur maximale   : " << error_analysis.max_error << std::endl;
    }

    /* =================================================================
       PARTIE 10 : ANALYSE DE SENSIBILITÉ
       ================================================================= */
    print_header("PARTIE 10 : ANALYSE DE SENSIBILITÉ");

    std::cout << "\nImpact de la volatilité sur le prix (Call européen) :" << std::endl;
    for (double vol : {0.10, 0.15, 0.20, 0.25, 0.30, 0.40})
    {
        BlackScholesPricer bsVol(europeanCall, S0, r, b, vol);
        std::cout << "  σ = " << vol << " : Prix = " << std::setw(8) << bsVol.price()
                  << ", Vega = " << std::setw(8) << bsVol.vega() << std::endl;
    }

    std::cout << "\nImpact du spot sur le prix et le delta :" << std::endl;
    for (double spot : {80.0, 90.0, 100.0, 110.0, 120.0})
    {
        BlackScholesPricer bsSpot(europeanCall, spot, r, b, sigma);
        std::cout << "  S = " << std::setw(6) << spot 
                  << " : Prix = " << std::setw(8) << bsSpot.price()
                  << ", Delta = " << std::setw(8) << bsSpot.delta(spot)
                  << ", Gamma = " << std::setw(8) << bsSpot.gamma(spot) << std::endl;
    }

    /* =================================================================
       PARTIE 11 : COMPARAISON DES MÉTHODES
       ================================================================= */
    print_header("PARTIE 11 : COMPARAISON DES MÉTHODES NUMÉRIQUES");

    std::cout << "\nConvergence des arbres binomiaux (Call européen) :" << std::endl;
    double bs_reference = bs.price();
    
    for (int steps : {50, 100, 200, 500, 1000})
    {
        BinomialTreePricer tree_conv(europeanCall, S0, r, b, sigma, static_cast<long unsigned int>(steps), false);
        double error = std::abs(tree_conv.price() - bs_reference);
        std::cout << "  N = " << std::setw(4) << steps 
                  << " : Prix = " << std::setw(8) << tree_conv.price()
                  << ", Erreur = " << std::setw(10) << error << std::endl;
    }

    return 0;
}