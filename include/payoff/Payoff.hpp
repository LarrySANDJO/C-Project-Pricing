# pragma once

class Payoff {
public :
    virtual ~Payoff() = default;
    virtual double operator()(double spot) const = 0;
}