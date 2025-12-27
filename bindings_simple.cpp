#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// Headers minimaux
#include "option_type.hpp"
#include "payoff.hpp"
#include "option.hpp"
#include "black_scholes_pricer.hpp"

namespace py = pybind11;

PYBIND11_MODULE(option_pricer_cpp, m) {
    m.doc() = "Module Python pour le pricing d'options";

    // =========================================================
    // ENUM : OptionType
    // =========================================================
    py::enum_<OptionType>(m, "OptionType")
        .value("Call", OptionType::Call)
        .value("Put", OptionType::Put)
        .export_values();

    // =========================================================
    // CLASS : Payoff (sans exposer l'interface abstraite)
    // =========================================================
    // On expose seulement PayoffEuropean directement
    py::class_<PayoffEuropean, std::shared_ptr<PayoffEuropean>>(m, "PayoffEuropean")
        .def(py::init<OptionType, double>(),
             py::arg("type"),
             py::arg("strike"),
             "Créer un payoff européen Call ou Put")
        .def("payoff_spot", &PayoffEuropean::payoff_spot,
             py::arg("spot"),
             "Calculer le payoff pour un prix spot donné")
        .def("type", &PayoffEuropean::type,
             "Obtenir le type d'option (Call/Put)")
        .def("strike", &PayoffEuropean::strike,
             "Obtenir le strike");

    // =========================================================
    // CLASS : Option
    // =========================================================
    py::class_<Option>(m, "Option")
        .def(py::init<double, std::shared_ptr<PayoffEuropean>>(),
             py::arg("maturity"),
             py::arg("payoff"),
             "Créer une option avec maturité et payoff")
        .def("maturity", &Option::maturity,
             "Obtenir la maturité")
        .def("is_valid", &Option::is_valid,
             "Vérifier si l'option est valide");

    // =========================================================
    // CLASS : BlackScholesPricer (directement, sans interface Pricer)
    // =========================================================
    py::class_<BlackScholesPricer>(m, "BlackScholesPricer")
        .def(py::init<const Option&, double, double, double, double>(),
             py::arg("option"),
             py::arg("spot"),
             py::arg("rate"),
             py::arg("carry"),
             py::arg("volatility"),
             "Créer un pricer Black-Scholes\n"
             "Args:\n"
             "    option: L'option à pricer\n"
             "    spot: Prix spot du sous-jacent\n"
             "    rate: Taux sans risque\n"
             "    carry: Coût de portage (généralement = rate)\n"
             "    volatility: Volatilité du sous-jacent")
        .def("price", &BlackScholesPricer::price,
             "Calculer le prix de l'option")
        .def("delta", &BlackScholesPricer::delta,
             py::arg("spot"),
             "Calculer le delta (sensibilité au spot)")
        .def("gamma", &BlackScholesPricer::gamma,
             py::arg("spot"),
             "Calculer le gamma (convexité)")
        .def("vega", &BlackScholesPricer::vega,
             "Calculer le vega (sensibilité à la volatilité)")
        .def("theta", &BlackScholesPricer::theta,
             "Calculer le theta (décroissance temporelle)")
        .def("rho", &BlackScholesPricer::rho,
             "Calculer le rho (sensibilité au taux)");

    // Helper pour créer rapidement une option
    m.def("create_european_option",
        [](OptionType type, double strike, double maturity) {
            auto payoff = std::make_shared<PayoffEuropean>(type, strike);
            return Option(maturity, payoff);
        },
        py::arg("type"),
        py::arg("strike"),
        py::arg("maturity"),
        "Helper pour créer rapidement une option européenne"
    );
}