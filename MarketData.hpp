#pragma once

class MarketData {
public:
    MarketData(double spot,
               double rate,
               double volatility);

    double spot() const;
    double rate() const;
    double volatility() const;

private:
    double spot_;
    double rate_;
    double volatility_;
};
