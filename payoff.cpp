#include "payoff.hpp"
#include <stdexcept>
#include <numeric>

/* =========================================================
   PAYOFF - IMPLÉMENTATION DE BASE
   ========================================================= */

Payoff::Payoff(double strike, OptionType type)
    : type_(type), K_(strike)
{
    // Gestion d'erreurs
    if (strike < 0.0)
        throw std::invalid_argument("Strike must be non-negative");
}

double Payoff::operator()(const std::vector<double>& path) const
{
    return payoff_spot(path.back());
}

double Payoff::payoff_spot(double spot) const
{
    if (type_ == OptionType::Call)
        return std::max(spot - K_, 0.0);
    else
        return std::max(K_ - spot, 0.0);
}

double Payoff::payoff_derivative(double spot) const
{
    if (type_ == OptionType::Call)
        return (spot > K_) ? 1.0 : 0.0;
    else
        return (spot < K_) ? -1.0 : 0.0;
}

/* =========================================================
   OPTIONS ASIATIQUES
   ========================================================= */

AsianCallPayoff::AsianCallPayoff(double strike)
    : Payoff(strike, OptionType::Call) {}

double AsianCallPayoff::operator()(const std::vector<double>& path) const
{
    double sum = std::accumulate(path.begin(), path.end(), 0.0);
    double avg = sum / static_cast<double>(path.size());
    return std::max(avg - strike(), 0.0);
}

AsianPutPayoff::AsianPutPayoff(double strike)
    : Payoff(strike, OptionType::Put) {}

double AsianPutPayoff::operator()(const std::vector<double>& path) const
{
    double sum = std::accumulate(path.begin(), path.end(), 0.0);
    double avg = sum / static_cast<double>(path.size());
    return std::max(strike() - avg, 0.0);
}

AsianGeometricCallPayoff::AsianGeometricCallPayoff(double strike)
    : Payoff(strike, OptionType::Call) {}

double AsianGeometricCallPayoff::operator()(const std::vector<double>& path) const
{
    // Moyenne géométrique 
    double log_sum = 0.0;
    for (double s : path)
    {
        if (s <= 0.0)
            throw std::runtime_error("Negative or zero price in path");
        log_sum += std::log(s);
    }
    double geometric_mean = std::exp(log_sum / static_cast<double>(path.size()));
    return std::max(geometric_mean - strike(), 0.0);
}

AsianGeometricPutPayoff::AsianGeometricPutPayoff(double strike)
    : Payoff(strike, OptionType::Put) {}

double AsianGeometricPutPayoff::operator()(const std::vector<double>& path) const
{
    double log_sum = 0.0;
    for (double s : path) {
        if (s <= 0.0)
            throw std::runtime_error("Negative or zero price in path");
        log_sum += std::log(s);
    }
    double geometric_mean = std::exp(log_sum / static_cast<double>(path.size()));
    return std::max(strike() - geometric_mean, 0.0);  // Put : K - S
}

/* =========================================================
   OPTIONS LOOKBACK
   ========================================================= */

LookbackCallPayoff::LookbackCallPayoff(double strike)
    : Payoff(strike, OptionType::Call) {}

double LookbackCallPayoff::operator()(const std::vector<double>& path) const
{
    double maxS = *std::max_element(path.begin(), path.end());
    return std::max(maxS - strike(), 0.0);
}

LookbackPutPayoff::LookbackPutPayoff(double strike)
    : Payoff(strike, OptionType::Put) {}

double LookbackPutPayoff::operator()(const std::vector<double>& path) const
{
    double minS = *std::min_element(path.begin(), path.end());
    return std::max(strike() - minS, 0.0);
}

LookbackFloatingCallPayoff::LookbackFloatingCallPayoff()
    : Payoff(0.0, OptionType::Call) {}

double LookbackFloatingCallPayoff::operator()(const std::vector<double>& path) const
{
    // Payoff : max(S_t) - S_T
    double maxS = *std::max_element(path.begin(), path.end());
    double S_T = path.back();
    return std::max(maxS - S_T, 0.0);
}

LookbackFloatingPutPayoff::LookbackFloatingPutPayoff()
    : Payoff(0.0, OptionType::Put) {}

double LookbackFloatingPutPayoff::operator()(const std::vector<double>& path) const
{
    // Payoff : S_T - min(S_t)
    double minS = *std::min_element(path.begin(), path.end());
    double S_T = path.back();
    return std::max(S_T - minS, 0.0);
}

/* =========================================================
   OPTIONS BARRIÈRES
   ========================================================= */

BarrierUpOutCallPayoff::BarrierUpOutCallPayoff(double strike, double barrier)
    : Payoff(strike, OptionType::Call), barrier_(barrier)
{
    if (barrier <= strike)
        throw std::invalid_argument("Barrier must be above strike for up-and-out call");
}

double BarrierUpOutCallPayoff::operator()(const std::vector<double>& path) const
{
    // Si le sous-jacent touche ou dépasse la barrière, l'option est désactivée
    for (double s : path)
        if (s >= barrier_)
            return 0.0;

    return payoff_spot(path.back());
}

BarrierUpOutPutPayoff::BarrierUpOutPutPayoff(double strike, double barrier)
    : Payoff(strike, OptionType::Put), barrier_(barrier)
{
    if (barrier <= strike)
        throw std::invalid_argument("Barrier must be above strike for up-and-out put");
}

double BarrierUpOutPutPayoff::operator()(const std::vector<double>& path) const
{
    // Si touche la barrière haute, désactivée
    for (double s : path)
        if (s >= barrier_)
            return 0.0;
    
    return payoff_spot(path.back());  // max(K - S_T, 0)
}

BarrierDownOutPutPayoff::BarrierDownOutPutPayoff(double strike, double barrier)
    : Payoff(strike, OptionType::Put), barrier_(barrier)
{
    if (barrier >= strike)
        throw std::invalid_argument("Barrier must be below strike for down-and-out put");
}

double BarrierDownOutPutPayoff::operator()(const std::vector<double>& path) const
{
    // Si le sous-jacent touche ou tombe sous la barrière, l'option est désactivée
    for (double s : path)
        if (s <= barrier_)
            return 0.0;

    return payoff_spot(path.back());
}

BarrierUpInCallPayoff::BarrierUpInCallPayoff(double strike, double barrier)
    : Payoff(strike, OptionType::Call), barrier_(barrier)
{
    if (barrier <= strike)
        throw std::invalid_argument("Barrier must be above strike for up-and-in call");
}

double BarrierUpInCallPayoff::operator()(const std::vector<double>& path) const
{
    // L'option s'active seulement si la barrière est touchée
    bool barrier_hit = false;
    for (double s : path)
    {
        if (s >= barrier_)
        {
            barrier_hit = true;
            break;
        }
    }

    return barrier_hit ? payoff_spot(path.back()) : 0.0;
}

BarrierDownInPutPayoff::BarrierDownInPutPayoff(double strike, double barrier)
    : Payoff(strike, OptionType::Put), barrier_(barrier)
{
    if (barrier >= strike)
        throw std::invalid_argument("Barrier must be below strike for down-and-in put");
}

double BarrierDownInPutPayoff::operator()(const std::vector<double>& path) const
{
    // L'option s'active seulement si la barrière est touchée
    bool barrier_hit = false;
    for (double s : path)
    {
        if (s <= barrier_)
        {
            barrier_hit = true;
            break;
        }
    }

    return barrier_hit ? payoff_spot(path.back()) : 0.0;
}

/* =========================================================
   OPTIONS DIGITALES (BINAIRES)
   ========================================================= */

DigitalCallPayoff::DigitalCallPayoff(double strike, double cash_amount)
    : Payoff(strike, OptionType::Call), cash_(cash_amount)
{
    if (cash_amount <= 0.0)
        throw std::invalid_argument("Cash amount must be positive");
}

double DigitalCallPayoff::operator()(const std::vector<double>& path) const
{
    // Paye cash si S_T > K, sinon 0
    double S_T = path.back();
    return (S_T > strike()) ? cash_ : 0.0;
}

DigitalPutPayoff::DigitalPutPayoff(double strike, double cash_amount)
    : Payoff(strike, OptionType::Put), cash_(cash_amount)
{
    if (cash_amount <= 0.0)
        throw std::invalid_argument("Cash amount must be positive");
}

double DigitalPutPayoff::operator()(const std::vector<double>& path) const
{
    // Paye cash si S_T < K, sinon 0
    double S_T = path.back();
    return (S_T < strike()) ? cash_ : 0.0;
}

/* =========================================================
   OPTIONS POWER
   ========================================================= */

PowerCallPayoff::PowerCallPayoff(double strike, double power)
    : Payoff(strike, OptionType::Call), power_(power)
{
    if (power <= 0.0)
        throw std::invalid_argument("Power must be positive");
}

double PowerCallPayoff::operator()(const std::vector<double>& path) const
{
    // Payoff : max((S_T)^α - K, 0) ou max((S_T - K)^α, 0)
    double S_T = path.back();
    if (S_T > strike())
    {
        double intrinsic = S_T - strike();
        return std::pow(intrinsic, power_);
    }
    return 0.0;
}

PowerPutPayoff::PowerPutPayoff(double strike, double power)
    : Payoff(strike, OptionType::Put), power_(power)
{
    if (power <= 0.0)
        throw std::invalid_argument("Power must be positive");
}

double PowerPutPayoff::operator()(const std::vector<double>& path) const
{
    double S_T = path.back();
    if (S_T < strike()) {
        double intrinsic = strike() - S_T;
        return std::pow(intrinsic, power_);
    }
    return 0.0;
}

/* =========================================================
   PAYOFF FACTORY
   ========================================================= */

std::shared_ptr<Payoff> PayoffFactory::create(
    PayoffStyle style,
    OptionType type,
    double strike,
    double param1
    // double param2
    )
{
    switch (style)
    {
    case PayoffStyle::European:
        return std::make_shared<Payoff>(strike, type);

    case PayoffStyle::Asian:
        if (type == OptionType::Call)
            return std::make_shared<AsianCallPayoff>(strike);
        else
            return std::make_shared<AsianPutPayoff>(strike);

    case PayoffStyle::AsianGeometric:
        if (type == OptionType::Call)
            return std::make_shared<AsianGeometricCallPayoff>(strike);
        else
            return std::make_shared<AsianGeometricPutPayoff>(strike);

    case PayoffStyle::Lookback:
        if (type == OptionType::Call)
            return std::make_shared<LookbackCallPayoff>(strike);
        else
            return std::make_shared<LookbackPutPayoff>(strike);

    case PayoffStyle::LookbackFloating:
        return std::make_shared<LookbackFloatingCallPayoff>();

    case PayoffStyle::BarrierUpOut:
        if (param1 == 0.0)
            throw std::invalid_argument("Barrier level required");
        if (type == OptionType::Call)
            return std::make_shared<BarrierUpOutCallPayoff>(strike, param1);
        else
            return std::make_shared<BarrierUpOutPutPayoff>(strike, param1);

    case PayoffStyle::BarrierDownOut:
        if (param1 == 0.0)
            throw std::invalid_argument("Barrier level required");
        if (type == OptionType::Put)
            return std::make_shared<BarrierDownOutPutPayoff>(strike, param1);
        else
            throw std::invalid_argument("Down-Out barrier only for puts");

    case PayoffStyle::BarrierUpIn:
        if (param1 == 0.0)
            throw std::invalid_argument("Barrier level required");
        if (type == OptionType::Call)
            return std::make_shared<BarrierUpInCallPayoff>(strike, param1);
        else
            throw std::invalid_argument("Up-In barrier only for calls");

    case PayoffStyle::BarrierDownIn:
        if (param1 == 0.0)
            throw std::invalid_argument("Barrier level required");
        if (type == OptionType::Put)
            return std::make_shared<BarrierDownInPutPayoff>(strike, param1);
        else
            throw std::invalid_argument("Down-In barrier only for puts");

    case PayoffStyle::Digital:
        if (param1 == 0.0)
            throw std::invalid_argument("Cash amount required for digital option");
        if (type == OptionType::Call)
            return std::make_shared<DigitalCallPayoff>(strike, param1);
        else
            return std::make_shared<DigitalPutPayoff>(strike, param1);

    case PayoffStyle::Power:
        if (param1 == 0.0)
            throw std::invalid_argument("Power parameter required");
        if (type == OptionType::Call)
            return std::make_shared<PowerCallPayoff>(strike, param1);
        else
            return std::make_shared<PowerPutPayoff>(strike, param1);

    default:
        throw std::invalid_argument("Unknown payoff style");
    }
}