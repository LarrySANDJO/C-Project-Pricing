#pragma once

class BlackScholesModel {
public:
    explicit BlackScholesModel(double vol);
    double volatility() const;

private:
    double vol_;
};
