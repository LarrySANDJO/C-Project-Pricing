#include <iostream>
#include <memory>

#include "MarketData.hpp"
#include "Option.hpp"
#include "PayoffCall.hpp"
#include "PayoffPut.hpp"
#include "EuropeanBlackScholesPricer.hpp"

int main() {
    double spot = 100.0;
    double strike = 100.0;
    double maturity = 1.0;
    double rate = 0.05;
    double volatility = 0.2;

    MarketData market(spot, rate, volatility);

    auto call_payoff = std::make_unique<PayoffCall>(strike);
    auto put_payoff  = std::make_unique<PayoffPut>(strike);

    Option call_option(maturity, std::move(call_payoff));
    Option put_option(maturity, std::move(put_payoff));

    EuropeanBlackScholesPricer pricer;

    std::cout << "Call price: "
              << pricer.price(call_option, market) << std::endl;

    std::cout << "Put price:  "
              << pricer.price(put_option, market) << std::endl;

    return 0;
}
