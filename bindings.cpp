#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

// Inclure tous vos headers
#include "option_type.hpp"
#include "payoff.hpp"
#include "option.hpp"
#include "pricer.hpp"
#include "black_scholes_pricer.hpp"
#include "monte_carlo_pricer.hpp"
#include "binomial_tree_pricer.hpp"
#include "finite_difference_pricer.hpp"
#include "replication_strategy.hpp"

namespace py = pybind11;

PYBIND11_MODULE(option_pricer_cpp, m) {
    m.doc() = "Module Python pour le pricing d'options - Binding C++";

    // =========================================================
    // ENUM : OptionType
    // =========================================================
    py::enum_<OptionType>(m, "OptionType")
        .value("Call", OptionType::Call)
        .value("Put", OptionType::Put)
        .export_values();

    // =========================================================
    // ENUM : PayoffStyle
    // =========================================================
    py::enum_<PayoffFactory::PayoffStyle>(m, "PayoffStyle")
        .value("European", PayoffFactory::PayoffStyle::European)
        .value("Asian", PayoffFactory::PayoffStyle::Asian)
        .value("AsianGeometric", PayoffFactory::PayoffStyle::AsianGeometric)
        .value("Lookback", PayoffFactory::PayoffStyle::Lookback)
        .value("LookbackFloating", PayoffFactory::PayoffStyle::LookbackFloating)
        .value("BarrierUpOut", PayoffFactory::PayoffStyle::BarrierUpOut)
        .value("BarrierUpIn", PayoffFactory::PayoffStyle::BarrierUpIn)
        .value("BarrierDownOut", PayoffFactory::PayoffStyle::BarrierDownOut)
        .value("BarrierDownIn", PayoffFactory::PayoffStyle::BarrierDownIn)
        .value("Digital", PayoffFactory::PayoffStyle::Digital)
        .value("Power", PayoffFactory::PayoffStyle::Power)
        .export_values();

    // =========================================================
    // CLASS : Payoff (base polymorphe)
    // =========================================================
    py::class_<Payoff, std::shared_ptr<Payoff>>(m, "Payoff")
        .def("payoff_spot", &Payoff::payoff_spot,
             py::arg("spot"),
             "Calculer le payoff pour un prix spot donné")
        .def("type", &Payoff::type,
             "Obtenir le type d'option (Call/Put)")
        .def("strike", &Payoff::strike,
             "Obtenir le strike");

    // =========================================================
    // FACTORY : PayoffFactory - SIGNATURE CORRECTE (4 params)
    // =========================================================
    m.def("create_payoff", 
        [](PayoffFactory::PayoffStyle style, OptionType type, double strike, 
           double param1 = 0.0) {
            return PayoffFactory::create(style, type, strike, param1);
        },
        py::arg("style"),
        py::arg("type"),
        py::arg("strike"),
        py::arg("param1") = 0.0,
        "Créer un payoff avec la factory\n\n"
        "Args:\n"
        "    style: Style de l'option (European, Asian, etc.)\n"
        "    type: Type (Call ou Put)\n"
        "    strike: Prix d'exercice\n"
        "    param1: Paramètre additionnel (barrière, cash, power, etc.)"
    );

    // =========================================================
    // CLASS : Option
    // =========================================================
    py::class_<Option>(m, "Option")
        .def(py::init<double, std::shared_ptr<Payoff>>(),
             py::arg("maturity"),
             py::arg("payoff"),
             "Créer une option\n\n"
             "Args:\n"
             "    maturity: Maturité en années\n"
             "    payoff: Objet payoff définissant le type d'option")
        .def("maturity", &Option::maturity,
             "Obtenir la maturité de l'option")
        .def("payoff", &Option::payoff, 
             py::return_value_policy::reference_internal,
             "Obtenir le payoff de l'option")
        .def("is_valid", &Option::is_valid,
             "Vérifier si l'option est valide");

    // =========================================================
    // CLASS : Pricer (interface abstraite)
    // =========================================================
    py::class_<Pricer, std::shared_ptr<Pricer>>(m, "Pricer")
        .def("price", &Pricer::price,
             "Calculer le prix de l'option")
        .def("delta", &Pricer::delta, 
             py::arg("spot"),
             "Calculer le delta (sensibilité au spot)")
        .def("gamma", &Pricer::gamma, 
             py::arg("spot"),
             "Calculer le gamma (convexité)")
        .def("vega", &Pricer::vega,
             "Calculer le vega (sensibilité à la volatilité)")
        .def("theta", &Pricer::theta,
             "Calculer le theta (décroissance temporelle)")
        .def("rho", &Pricer::rho,
             "Calculer le rho (sensibilité au taux)")
        .def("bond_position", &Pricer::bond_position, 
             py::arg("spot"),
             "Calculer la position en obligations pour réplication");

    // =========================================================
    // CLASS : BlackScholesPricer
    // =========================================================
    py::class_<BlackScholesPricer, Pricer, std::shared_ptr<BlackScholesPricer>>(m, "BlackScholesPricer")
        .def(py::init<const Option&, double, double, double, double>(),
             py::arg("option"),
             py::arg("spot"),
             py::arg("rate"),
             py::arg("carry"),
             py::arg("volatility"),
             "Créer un pricer Black-Scholes\n\n"
             "Args:\n"
             "    option: Option à pricer (européenne uniquement)\n"
             "    spot: Prix spot du sous-jacent\n"
             "    rate: Taux sans risque\n"
             "    carry: Coût de portage (= rate pour actions)\n"
             "    volatility: Volatilité (sigma)")
        .def("price", &BlackScholesPricer::price)
        .def("delta", &BlackScholesPricer::delta)
        .def("gamma", &BlackScholesPricer::gamma)
        .def("vega", &BlackScholesPricer::vega)
        .def("theta", &BlackScholesPricer::theta)
        .def("rho", &BlackScholesPricer::rho);

    // =========================================================
    // STRUCT : MCResult
    // =========================================================
    py::class_<MCResult>(m, "MCResult")
        .def(py::init<>())
        .def_readwrite("price", &MCResult::price,
                       "Prix de l'option")
        .def_readwrite("std_error", &MCResult::std_error,
                       "Erreur standard")
        .def_readwrite("ci_lower_95", &MCResult::ci_lower_95,
                       "Borne inférieure IC 95%")
        .def_readwrite("ci_upper_95", &MCResult::ci_upper_95,
                       "Borne supérieure IC 95%");

    // =========================================================
    // CLASS : MonteCarloPricer
    // =========================================================
    py::class_<MonteCarloPricer, Pricer, std::shared_ptr<MonteCarloPricer>>(m, "MonteCarloPricer")
        .def(py::init<const Option&, double, double, double, double, 
                      std::size_t, std::size_t, unsigned, bool>(),
             py::arg("option"),
             py::arg("spot"),
             py::arg("rate"),
             py::arg("carry"),
             py::arg("volatility"),
             py::arg("paths"),
             py::arg("steps"),
             py::arg("seed") = 42,
             py::arg("use_antithetic") = true,
             "Créer un pricer Monte Carlo\n\n"
             "Args:\n"
             "    option: Option à pricer (tous types)\n"
             "    spot: Prix spot\n"
             "    rate: Taux sans risque\n"
             "    carry: Coût de portage\n"
             "    volatility: Volatilité\n"
             "    paths: Nombre de simulations\n"
             "    steps: Nombre de pas de temps\n"
             "    seed: Graine aléatoire\n"
             "    use_antithetic: Utiliser variables antithétiques")
        .def("price", &MonteCarloPricer::price)
        .def("delta", &MonteCarloPricer::delta)
        .def("vega", &MonteCarloPricer::vega)
        .def("price_with_confidence", &MonteCarloPricer::price_with_confidence,
             "Calculer le prix avec intervalle de confiance")
        .def("delta_pathwise", &MonteCarloPricer::delta_pathwise,
             "Calculer le delta par méthode pathwise");

    // =========================================================
    // ENUM : TreeType
    // =========================================================
    py::enum_<BinomialTreePricer::TreeType>(m, "TreeType")
        .value("CoxRossRubinstein", BinomialTreePricer::TreeType::CoxRossRubinstein)
        .value("JarrowRudd", BinomialTreePricer::TreeType::JarrowRudd)
        .export_values();

    // =========================================================
    // CLASS : BinomialTreePricer
    // =========================================================
    py::class_<BinomialTreePricer, Pricer, std::shared_ptr<BinomialTreePricer>>(m, "BinomialTreePricer")
        .def(py::init<const Option&, double, double, double, double,
                      std::size_t, bool, BinomialTreePricer::TreeType>(),
             py::arg("option"),
             py::arg("spot"),
             py::arg("rate"),
             py::arg("carry"),
             py::arg("volatility"),
             py::arg("steps"),
             py::arg("is_american") = false,
             py::arg("tree_type") = BinomialTreePricer::TreeType::CoxRossRubinstein,
             "Créer un pricer par arbre binomial\n\n"
             "Args:\n"
             "    option: Option à pricer\n"
             "    spot: Prix spot\n"
             "    rate: Taux sans risque\n"
             "    carry: Coût de portage\n"
             "    volatility: Volatilité\n"
             "    steps: Nombre de pas de l'arbre\n"
             "    is_american: True pour option américaine\n"
             "    tree_type: Type d'arbre (CRR, JR)")
        .def("price", &BinomialTreePricer::price)
        .def("delta", &BinomialTreePricer::delta)
        .def("gamma", &BinomialTreePricer::gamma)
        .def("theta", &BinomialTreePricer::theta)
        .def("get_up_factor", &BinomialTreePricer::get_up_factor,
             "Obtenir le facteur de montée (u)")
        .def("get_down_factor", &BinomialTreePricer::get_down_factor,
             "Obtenir le facteur de descente (d)")
        .def("get_risk_neutral_prob", &BinomialTreePricer::get_risk_neutral_prob,
             "Obtenir la probabilité risque-neutre (p)");

    // =========================================================
    // ENUM : Scheme (Finite Difference)
    // =========================================================
    py::enum_<FiniteDifferenceAmericanPricer::Scheme>(m, "FDScheme")
        .value("Explicit", FiniteDifferenceAmericanPricer::Scheme::Explicit)
        .value("Implicit", FiniteDifferenceAmericanPricer::Scheme::Implicit)
        .value("CrankNicolson", FiniteDifferenceAmericanPricer::Scheme::CrankNicolson)
        .export_values();

    // =========================================================
    // CLASS : FiniteDifferenceAmericanPricer
    // =========================================================
    py::class_<FiniteDifferenceAmericanPricer, Pricer, 
               std::shared_ptr<FiniteDifferenceAmericanPricer>>(m, "FiniteDifferenceAmericanPricer")
        .def(py::init<const Option&, double, double, double, double,
                      std::size_t, std::size_t, FiniteDifferenceAmericanPricer::Scheme>(),
             py::arg("option"),
             py::arg("spot"),
             py::arg("rate"),
             py::arg("carry"),
             py::arg("volatility"),
             py::arg("M"),
             py::arg("N"),
             py::arg("scheme") = FiniteDifferenceAmericanPricer::Scheme::CrankNicolson,
             "Créer un pricer par différences finies\n\n"
             "Args:\n"
             "    option: Option à pricer (américaine)\n"
             "    spot: Prix spot\n"
             "    rate: Taux sans risque\n"
             "    carry: Coût de portage\n"
             "    volatility: Volatilité\n"
             "    M: Nombre de points en espace\n"
             "    N: Nombre de points en temps\n"
             "    scheme: Schéma numérique (Explicit, Implicit, CrankNicolson)")
        .def("price", &FiniteDifferenceAmericanPricer::price)
        .def("delta", &FiniteDifferenceAmericanPricer::delta);
}