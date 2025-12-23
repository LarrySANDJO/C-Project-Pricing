#include <pybind11/pybind11.h>

#include "OptionPricingService.hpp"

namespace py = pybind11;

PYBIND11_MODULE(pricer, m) {
    m.doc() = "Simple option pricer (Black-Scholes)";

    py::class_<OptionPricingService>(m, "OptionPricingService")
        .def(py::init<>())
        .def(
            "price_plain_vanilla",
            &OptionPricingService::price_plain_vanilla,
            py::arg("option_type"),
            py::arg("spot"),
            py::arg("strike"),
            py::arg("maturity"),
            py::arg("rate"),
            py::arg("volatility")
        );
}
