#pragma once

enum class OptionType {
    Call,
    Put
};

struct OptionInput {
    OptionType type;
    double spot;
    double strike;
    double maturity;
    double rate;
    double volatility;
};
