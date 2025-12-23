#pragma once

#include "OptionInput.hpp"
#include "PricingResult.hpp"

class OptionPricingService {
public:
    PricingResult price(const OptionInput& input) const;

    // Pour le future Python-friendly API
    double price_plain_vanilla(
        int option_type,   // 0 = Call, 1 = Put
        double spot,
        double strike,
        double maturity,
        double rate,
        double volatility
    ) const;
};

