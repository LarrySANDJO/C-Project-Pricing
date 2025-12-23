#include "Cli.hpp"
#include "OptionPricingService.hpp"

#include <iostream>
#include <limits>

namespace {

double read_double(const std::string& prompt) {
    double value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value)
            return value;

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input, try again.\n";
    }
}

OptionType read_option_type() {
    int choice;
    while (true) {
        std::cout << "Choose option type:\n";
        std::cout << "1 - Call\n";
        std::cout << "2 - Put\n> ";

        if (std::cin >> choice && (choice == 1 || choice == 2))
            return choice == 1 ? OptionType::Call : OptionType::Put;

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid choice, try again.\n";
    }
}

} // anonymous namespace
void Cli::run() {
    OptionPricingService service;

    std::cout << "=== Black-Scholes Option Pricer ===\n\n";

    OptionInput input;
    input.type = read_option_type();
    input.spot = read_double("Enter spot: ");
    input.strike = read_double("Enter strike: ");
    input.maturity = read_double("Enter maturity (years): ");
    input.rate = read_double("Enter risk-free rate: ");
    input.volatility = read_double("Enter volatility: ");

    try {
        PricingResult result = service.price(input);
        std::cout << "\nOption price: " << result.price << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
}
