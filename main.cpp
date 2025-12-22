#include <iostream>
#include <memory>

#include "Option.hpp"
#include "MarketData.hpp"
#include "EuropeanBlackScholesPricer.hpp"
#include "PayoffCall.hpp"
#include "PayoffPut.hpp"

int main() {
    MarketData market(100.0, 0.05, 0.20);

    Option call(
        1.0,
        std::make_unique<PayoffCall>(100.0)
    );

    Option put(
        1.0,
        std::make_unique<PayoffPut>(100.0)
    );

    EuropeanBlackScholesPricer pricer;

    std::cout << "Call price: "
              << pricer.price(call, market) << '\n';

    std::cout << "Put price : "
              << pricer.price(put, market) << '\n';

    return 0;
}
