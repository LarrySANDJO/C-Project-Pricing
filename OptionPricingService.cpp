#include "OptionPricingService.hpp"

#include "Option.hpp"
#include "MarketData.hpp"
#include "EuropeanBlackScholesPricer.hpp"
#include "PayoffCall.hpp"
#include "PayoffPut.hpp"

#include <memory>
#include <stdexcept>

PricingResult OptionPricingService::price(
    const OptionInput& input
) const {
    // Validation minimale
    if (input.spot <= 0.0 || input.strike <= 0.0)
        throw std::invalid_argument("Spot and strike must be positive");

    if (input.volatility <= 0.0 || input.maturity <= 0.0)
        throw std::invalid_argument("Volatility and maturity must be positive");

    // MarketData
    MarketData market(
        input.spot,
        input.rate,
        input.volatility
    );

    // Payoff
    std::unique_ptr<Payoff> payoff;

    switch (input.type) {
        case OptionType::Call:
            payoff = std::make_unique<PayoffCall>(input.strike);
            break;
        case OptionType::Put:
            payoff = std::make_unique<PayoffPut>(input.strike);
            break;
    }

    // Option
    Option option(input.maturity, std::move(payoff));

    // Pricing
    EuropeanBlackScholesPricer pricer;
    double price = pricer.price(option, market);

    return PricingResult{price};
}

double OptionPricingService::price_plain_vanilla(
    int option_type,
    double spot,
    double strike,
    double maturity,
    double rate,
    double volatility
) const {
    OptionInput input;
    input.type = (option_type == 0 ? OptionType::Call : OptionType::Put);
    input.spot = spot;
    input.strike = strike;
    input.maturity = maturity;
    input.rate = rate;
    input.volatility = volatility;

    return price(input).price;
}

