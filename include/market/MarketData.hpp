#pragma once

class MarketData {
public:
    MarketData(double spot, double rate, double dividend = 0.0);

    double spot() const;
    double rate() const;
    double dividend() const;

private:
    double spot_;
    double rate_;
    double dividend_;
};
