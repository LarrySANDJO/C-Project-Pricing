#include <iostream>
#include <memory>

#include "include/option/Option.hpp"
#include "include/market/MarketData.hpp"
#include "include/EuropeanBlackScholesPricer.hpp"

#include "include/payoff/PayoffCall.hpp"
#include "include/payoff/PayoffPut.hpp"

int main() {
    // Données de marché
    MarketData market(
        100.0,   // spot
        0.05,    // rate
        0.20     // volatility
    );

    // Option Call européenne
    Option call_option(
        1.0, // maturity
        std::make_unique<PayoffCall>(100.0)
    );

    // Option Put européenne
    Option put_option(
        1.0,
        std::make_unique<PayoffPut>(100.0)
    );

    EuropeanBlackScholesPricer pricer;

    double call_price = pricer.price(call_option, market);
    double put_price  = pricer.price(put_option, market);

    std::cout << "Call price: " << call_price << std::endl;
    std::cout << "Put price : " << put_price  << std::endl;

    return 0;
}
