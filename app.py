import streamlit as st
import option_pricer_cpp as opc
import pandas as pd
import numpy as np
import plotly.graph_objects as go
from plotly.subplots import make_subplots
import time

# Configuration de la page
st.set_page_config(
    page_title="Option Pricer",
    page_icon="🚀",
    layout="wide",
    initial_sidebar_state="expanded"
)

# Style CSS personnalisé
st.markdown("""
    <style>
    .main {
        background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    }
    .stMetric {
        background-color: rgba(255, 255, 255, 0.1);
        padding: 20px;
        border-radius: 10px;
    }
    </style>
    """, unsafe_allow_html=True)

# Titre principal
st.title("🚀 Option Pricer - Pricing Quantitatif")
st.markdown("**Interface complète de pricing d'options avec C++**")

# Initialiser le session state
if 'results' not in st.session_state:
    st.session_state.results = None

# Sidebar - Paramètres
st.sidebar.header("⚙️ Paramètres de Marché")

spot = st.sidebar.number_input("Spot (S₀)", value=100.0, min_value=0.01, step=1.0)
strike = st.sidebar.number_input("Strike (K)", value=100.0, min_value=0.01, step=1.0)
maturity = st.sidebar.number_input("Maturité (T) en années", value=1.0, min_value=0.01, max_value=10.0, step=0.1)
rate = st.sidebar.number_input("Taux sans risque (r)", value=0.05, min_value=0.0, max_value=1.0, step=0.01, format="%.4f")
volatility = st.sidebar.number_input("Volatilité (σ)", value=0.20, min_value=0.01, max_value=2.0, step=0.01, format="%.4f")

st.sidebar.markdown("---")
st.sidebar.header("🎯 Configuration Option")

option_type = st.sidebar.selectbox("Type d'option", ["Call", "Put"])
option_style = st.sidebar.selectbox("Style", ["European", "American", "Asian", "AsianGeometric", "Lookback"])
pricing_method = st.sidebar.selectbox("Méthode de pricing", ["BlackScholes", "MonteCarlo", "BinomialTree", "FiniteDifference"])

# Paramètres avancés selon la méthode
st.sidebar.markdown("---")
st.sidebar.header("🔧 Paramètres Avancés")

if pricing_method == "MonteCarlo":
    mc_paths = st.sidebar.slider("Nombre de paths", 1000, 500000, 100000, step=1000)
    mc_steps = st.sidebar.slider("Nombre de steps", 50, 500, 100, step=10)
elif pricing_method == "BinomialTree":
    tree_steps = st.sidebar.slider("Nombre de steps", 50, 1000, 200, step=10)
    tree_type = st.sidebar.selectbox("Type d'arbre", ["CoxRossRubinstein", "JarrowRudd"])
    is_american = (option_style == "American")
elif pricing_method == "FiniteDifference":
    fd_M = st.sidebar.slider("Points en espace (M)", 50, 500, 200, step=10)
    fd_N = st.sidebar.slider("Points en temps (N)", 50, 500, 200, step=10)

# Bouton de calcul
calculate_button = st.sidebar.button("🚀 Calculer Prix", type="primary", use_container_width=True)

# Validation des combinaisons
st.sidebar.markdown("---")
with st.sidebar.expander("ℹ️ Restrictions"):
    st.markdown("""
    **Limitations:**
    - Black-Scholes: Européennes uniquement
    - Lookback: Call uniquement pour certains types
    - American: Utilisez Tree ou FD
    """)

# Fonction de calcul
def calculate_option_price():
    """Calculer le prix et les Greeks"""
    try:
        with st.spinner('Calcul en cours...'):
            # Mapper les types
            opt_type = opc.OptionType.Call if option_type == "Call" else opc.OptionType.Put
            
            # Mapper les styles avec validation
            style_map = {
                "European": opc.PayoffStyle.European,
                "American": opc.PayoffStyle.European,
                "Asian": opc.PayoffStyle.Asian,
                "AsianGeometric": opc.PayoffStyle.AsianGeometric,
                "Lookback": opc.PayoffStyle.Lookback
            }
            
            # Validation des combinaisons
            if option_style == "AsianGeometric" and option_type == "Put":
                st.error("❌ Geometric Asian Put n'est pas implémenté. Utilisez Call ou choisissez 'Asian' (arithmétique).")
                return None
            
            if pricing_method == "BlackScholes" and option_style != "European":
                st.error("❌ Black-Scholes fonctionne uniquement pour les options européennes")
                return None
            
            # Créer le payoff et l'option
            try:
                payoff = opc.create_payoff(style_map[option_style], opt_type, strike)
                option = opc.Option(maturity, payoff)
            except Exception as e:
                st.error(f"❌ Erreur lors de la création du payoff: {str(e)}")
                return None
            
            start_time = time.time()
            
            # Choisir le pricer
            if pricing_method == "BlackScholes":
                if option_style != "European":
                    st.error("❌ Black-Scholes fonctionne uniquement pour les options européennes")
                    return None
                pricer = opc.BlackScholesPricer(option, spot, rate, rate, volatility)
            
            elif pricing_method == "MonteCarlo":
                pricer = opc.MonteCarloPricer(option, spot, rate, rate, volatility, mc_paths, mc_steps, 42, True)
            
            elif pricing_method == "BinomialTree":
                tree_type_map = {
                    "CoxRossRubinstein": opc.TreeType.CoxRossRubinstein,
                    "JarrowRudd": opc.TreeType.JarrowRudd
                }
                pricer = opc.BinomialTreePricer(option, spot, rate, rate, volatility, tree_steps, is_american, tree_type_map[tree_type])
            
            elif pricing_method == "FiniteDifference":
                pricer = opc.FiniteDifferenceAmericanPricer(option, spot, rate, rate, volatility, fd_M, fd_N, opc.FDScheme.CrankNicolson)
            
            # Calculer prix et Greeks
            price = pricer.price()
            delta = pricer.delta(spot)
            
            calc_time = (time.time() - start_time) * 1000  # en ms
            
            # Greeks optionnels
            try:
                gamma = pricer.gamma(spot)
            except:
                gamma = None
            
            try:
                vega = pricer.vega()
            except:
                vega = None
            
            try:
                theta = pricer.theta()
            except:
                theta = None
            
            try:
                rho = pricer.rho()
            except:
                rho = None
            
            # Intervalle de confiance pour Monte Carlo
            ci = None
            if pricing_method == "MonteCarlo":
                try:
                    mc_result = pricer.price_with_confidence()
                    ci = (mc_result.ci_lower_95, mc_result.ci_upper_95)
                except:
                    pass
            
            return {
                'price': price,
                'delta': delta,
                'gamma': gamma,
                'vega': vega,
                'theta': theta,
                'rho': rho,
                'calc_time': calc_time,
                'confidence_interval': ci,
                'pricer': pricer
            }
    
    except Exception as e:
        st.error(f"❌ Erreur lors du calcul: {str(e)}")
        return None

# Calculer si bouton pressé
if calculate_button:
    st.session_state.results = calculate_option_price()

# Affichage des résultats
if st.session_state.results:
    results = st.session_state.results
    
    st.markdown("---")
    st.header("💰 Résultats du Pricing")
    
    # Métriques principales
    col1, col2, col3, col4 = st.columns(4)
    
    with col1:
        st.metric("Prix de l'Option", f"{results['price']:.4f}")
    with col2:
        st.metric("Méthode", pricing_method)
    with col3:
        st.metric("Temps de calcul", f"{results['calc_time']:.2f} ms")
    with col4:
        if results['confidence_interval']:
            ci_width = results['confidence_interval'][1] - results['confidence_interval'][0]
            st.metric("IC 95% largeur", f"{ci_width:.4f}")
        else:
            st.metric("IC 95%", "N/A")
    
    # Greeks
    st.markdown("---")
    st.subheader("📈 Greeks (Sensibilités)")
    
    col1, col2, col3, col4, col5 = st.columns(5)
    
    with col1:
        st.metric("Delta (Δ)", f"{results['delta']:.4f}" if results['delta'] is not None else "N/A")
    with col2:
        st.metric("Gamma (Γ)", f"{results['gamma']:.6f}" if results['gamma'] is not None else "N/A")
    with col3:
        st.metric("Vega (ν)", f"{results['vega']:.4f}" if results['vega'] is not None else "N/A")
    with col4:
        st.metric("Theta (Θ)", f"{results['theta']:.4f}" if results['theta'] is not None else "N/A")
    with col5:
        st.metric("Rho (ρ)", f"{results['rho']:.4f}" if results['rho'] is not None else "N/A")
    
    # Intervalle de confiance détaillé
    if results['confidence_interval']:
        st.info(f"📊 Intervalle de confiance 95% : [{results['confidence_interval'][0]:.4f}, {results['confidence_interval'][1]:.4f}]")
    
    # Graphiques de sensibilité
    st.markdown("---")
    st.subheader("📊 Analyse de Sensibilité")
    
    # Calculer la sensibilité au spot
    with st.spinner('Calcul des sensibilités...'):
        spots = np.linspace(spot * 0.7, spot * 1.3, 30)
        prices = []
        deltas = []
        intrinsics = []
        
        opt_type_enum = opc.OptionType.Call if option_type == "Call" else opc.OptionType.Put
        
        # Utiliser uniquement des options européennes pour les graphiques de sensibilité
        # pour éviter les erreurs avec les options exotiques non implémentées
        for s in spots:
            try:
                # Toujours utiliser une option européenne pour la sensibilité
                payoff_temp = opc.create_payoff(opc.PayoffStyle.European, opt_type_enum, strike)
                option_temp = opc.Option(maturity, payoff_temp)
                
                if pricing_method == "BlackScholes":
                    pricer_temp = opc.BlackScholesPricer(option_temp, s, rate, rate, volatility)
                else:
                    # Utiliser arbre binomial pour les autres méthodes
                    pricer_temp = opc.BinomialTreePricer(option_temp, s, rate, rate, volatility, 100, False)
                
                prices.append(pricer_temp.price())
                deltas.append(pricer_temp.delta(s))
                
                # Valeur intrinsèque
                if option_type == "Call":
                    intrinsics.append(max(0, s - strike))
                else:
                    intrinsics.append(max(0, strike - s))
            except Exception as e:
                st.warning(f"⚠️ Erreur lors du calcul de sensibilité pour spot={s:.2f}: {str(e)}")
                # Utiliser des valeurs par défaut en cas d'erreur
                prices.append(np.nan)
                deltas.append(np.nan)
                intrinsics.append(np.nan)
    
    # Créer les graphiques
    fig = make_subplots(
        rows=1, cols=2,
        subplot_titles=("Prix vs Spot", "Delta vs Spot")
    )
    
    # Graphique 1 : Prix
    fig.add_trace(
        go.Scatter(x=spots, y=prices, mode='lines', name='Prix Option', line=dict(color='blue', width=3)),
        row=1, col=1
    )
    fig.add_trace(
        go.Scatter(x=spots, y=intrinsics, mode='lines', name='Valeur Intrinsèque', 
                   line=dict(color='green', width=2, dash='dash')),
        row=1, col=1
    )
    fig.add_vline(x=spot, line_dash="dot", line_color="red", annotation_text="Spot actuel", row=1, col=1)
    
    # Graphique 2 : Delta
    fig.add_trace(
        go.Scatter(x=spots, y=deltas, mode='lines', name='Delta', line=dict(color='orange', width=3)),
        row=1, col=2
    )
    fig.add_vline(x=spot, line_dash="dot", line_color="red", annotation_text="Spot actuel", row=1, col=2)
    
    fig.update_xaxes(title_text="Spot", row=1, col=1)
    fig.update_xaxes(title_text="Spot", row=1, col=2)
    fig.update_yaxes(title_text="Prix", row=1, col=1)
    fig.update_yaxes(title_text="Delta", row=1, col=2)
    
    fig.update_layout(height=500, showlegend=True)
    st.plotly_chart(fig, use_container_width=True)
    
    # Tableau de données
    st.markdown("---")
    st.subheader("📋 Tableau de Sensibilité")
    
    df = pd.DataFrame({
        'Spot': spots,
        'Prix': prices,
        'Delta': deltas,
        'Valeur Intrinsèque': intrinsics,
        'Valeur Temps': [p - i for p, i in zip(prices, intrinsics)]
    })
    
    st.dataframe(df.style.format({
        'Spot': '{:.2f}',
        'Prix': '{:.4f}',
        'Delta': '{:.4f}',
        'Valeur Intrinsèque': '{:.4f}',
        'Valeur Temps': '{:.4f}'
    }), use_container_width=True)

else:
    # Message d'accueil
    st.info("👈 Configurez les paramètres dans la barre latérale et cliquez sur **Calculer Prix**")
    
    st.markdown("---")
    st.subheader("✨ Fonctionnalités")
    
    col1, col2, col3 = st.columns(3)
    
    with col1:
        st.markdown("""
        **Méthodes de Pricing**
        - ✅ Black-Scholes
        - ✅ Monte Carlo
        - ✅ Arbres Binomiaux
        - ✅ Différences Finies
        """)
    
    with col2:
        st.markdown("""
        **Types d'Options**
        - 📈 Européennes
        - 📉 Américaines
        - 🌏 Asiatiques
        - 👀 Lookback
        """)
    
    with col3:
        st.markdown("""
        **Greeks Calculés**
        - Δ Delta
        - Γ Gamma
        - ν Vega
        - Θ Theta
        - ρ Rho
        """)

# Footer
st.markdown("---")
st.markdown("""
<div style='text-align: center; color: #888;'>
    <p>Option Pricer v1.0 | Powered by C++ & Streamlit</p>
</div>
""", unsafe_allow_html=True)