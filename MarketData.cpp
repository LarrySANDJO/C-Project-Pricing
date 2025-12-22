#include "MarketData.hpp"

MarketData::MarketData(double spot,
                       double rate,
                       double volatility)
    : spot_(spot),
      rate_(rate),
      volatility_(volatility)
{}

double MarketData::spot() const {
    return spot_;
}

double MarketData::rate() const {
    return rate_;
}

double MarketData::volatility() const {
    return volatility_;
}
