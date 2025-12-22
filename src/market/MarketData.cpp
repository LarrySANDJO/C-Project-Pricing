#include "MarketData.hpp"

MarketData::MarketData(double spot, double rate, double dividend)
    : spot_(spot), rate_(rate), dividend_(dividend)
{}

double MarketData::spot() const {
    return spot_;
}

double MarketData::rate() const {
    return rate_;
}

double MarketData::dividend() const {
    return dividend_;
}
