#pragma once

#include "/home/onyxia/C-Project-Pricing/include/option/Option.hpp"
#include "/home/onyxia/C-Project-Pricing/include/market/MarketData.hpp"

class EuropeanBlackScholesPricer {
public:
    double price(const Option& option,
                 const MarketData& market) const;
        // le const sur les arguments empeche la modification de ceux ci
};

// Aucun element prive car le pricer n doit garder aucune memoire interne, il ne donne que le prix