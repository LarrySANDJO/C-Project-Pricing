import streamlit as st
import pricer

st.title("Black-Scholes Option Pricer")

option_type = st.selectbox(
    "Option type",
    options=[("Call", 0), ("Put", 1)],
    format_func=lambda x: x[0]
)[1]

spot = st.number_input("Spot", value=100.0, min_value=0.01)
strike = st.number_input("Strike", value=100.0, min_value=0.01)
maturity = st.number_input("Maturity (years)", value=1.0, min_value=0.01)
rate = st.number_input("Risk-free rate", value=0.05)
volatility = st.number_input("Volatility", value=0.2, min_value=0.001)

if st.button("Compute price"):
    price = pricer.price_plain_vanilla(
        option_type,
        spot,
        strike,
        maturity,
        rate,
        volatility
    )
    st.success(f"Option price: {price:.4f}")
