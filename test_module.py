# test_module.py
import option_pricer_cpp as opc

# Créer un payoff Call européen
payoff = opc.create_payoff(
    opc.PayoffStyle.European,
    opc.OptionType.Call,
    100.0  # strike
)

# Créer une option
option = opc.Option(1.0, payoff)  # maturité 1 an

# Créer un pricer Black-Scholes
pricer = opc.BlackScholesPricer(
    option,
    100.0,  # spot
    0.05,   # rate
    0.05,   # carry
    0.20    # volatility
)

# Calculer le prix
print(f"Prix: {pricer.price():.4f}")
print(f"Delta: {pricer.delta(100.0):.4f}")
print(f"Gamma: {pricer.gamma(100.0):.6f}")
print(f"Vega: {pricer.vega():.4f}")