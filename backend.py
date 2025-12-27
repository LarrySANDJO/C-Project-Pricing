from flask import Flask, request, jsonify
from flask_cors import CORS
import option_pricer_cpp as opc
import traceback
import numpy as np

app = Flask(__name__)
CORS(app)  # Permettre les requêtes cross-origin

# Mapping des noms de l'interface vers les enums C++
STYLE_MAP = {
    'European': opc.PayoffStyle.European,
    'American': opc.PayoffStyle.European,  # On utilise le payoff européen
    'Asian': opc.PayoffStyle.Asian,
    'AsianGeometric': opc.PayoffStyle.AsianGeometric,
    'Lookback': opc.PayoffStyle.Lookback,
    'LookbackFloating': opc.PayoffStyle.LookbackFloating,
    'BarrierUpOut': opc.PayoffStyle.BarrierUpOut,
    'BarrierUpIn': opc.PayoffStyle.BarrierUpIn,
    'BarrierDownOut': opc.PayoffStyle.BarrierDownOut,
    'BarrierDownIn': opc.PayoffStyle.BarrierDownIn,
    'Digital': opc.PayoffStyle.Digital,
    'Power': opc.PayoffStyle.Power
}

TYPE_MAP = {
    'Call': opc.OptionType.Call,
    'Put': opc.OptionType.Put
}

@app.route('/api/health', methods=['GET'])
def health_check():
    """Vérifier que l'API fonctionne"""
    return jsonify({'status': 'ok', 'message': 'Backend operational'})

@app.route('/api/price', methods=['POST'])
def calculate_price():
    """
    Endpoint principal pour calculer le prix d'une option
    
    Body JSON attendu:
    {
        "spot": 100.0,
        "strike": 100.0,
        "maturity": 1.0,
        "rate": 0.05,
        "volatility": 0.20,
        "optionType": "Call",
        "optionStyle": "European",
        "pricingMethod": "BlackScholes",
        "barrier": 120.0,  // optionnel pour barrières
        "power": 2.0       // optionnel pour power options
    }
    """
    try:
        data = request.json
        
        # Extraire les paramètres
        spot = float(data['spot'])
        strike = float(data['strike'])
        maturity = float(data['maturity'])
        rate = float(data['rate'])
        volatility = float(data['volatility'])
        option_type = TYPE_MAP[data['optionType']]
        option_style_str = data['optionStyle']
        pricing_method = data['pricingMethod']
        
        # Paramètres optionnels
        barrier = float(data.get('barrier', 0.0))
        power = float(data.get('power', 2.0))
        cash_amount = float(data.get('cashAmount', 100.0))
        
        # Créer le payoff
        style = STYLE_MAP[option_style_str]
        
        # Déterminer le paramètre supplémentaire selon le style
        if 'Barrier' in option_style_str:
            param = barrier
        elif option_style_str == 'Digital':
            param = cash_amount
        elif option_style_str == 'Power':
            param = power
        else:
            param = 0.0
        
        payoff = opc.create_payoff(style, option_type, strike, param)
        
        # Créer l'option
        option = opc.Option(maturity, payoff)
        
        # Choisir le pricer
        pricer = None
        
        if pricing_method == 'BlackScholes':
            if option_style_str != 'European':
                return jsonify({
                    'error': 'Black-Scholes ne fonctionne que pour les options européennes'
                }), 400
            
            pricer = opc.BlackScholesPricer(
                option, spot, rate, rate, volatility
            )
            
        elif pricing_method == 'MonteCarlo':
            paths = int(data.get('mcPaths', 100000))
            steps = int(data.get('mcSteps', 100))
            seed = int(data.get('seed', 42))
            
            pricer = opc.MonteCarloPricer(
                option, spot, rate, rate, volatility,
                paths, steps, seed, True
            )
            
        elif pricing_method == 'BinomialTree':
            steps = int(data.get('treeSteps', 200))
            is_american = (option_style_str == 'American')
            tree_type = opc.TreeType.CoxRossRubinstein
            
            if data.get('treeType') == 'LeisenReimer':
                tree_type = opc.TreeType.LeisenReimer
            elif data.get('treeType') == 'JarrowRudd':
                tree_type = opc.TreeType.JarrowRudd
            
            pricer = opc.BinomialTreePricer(
                option, spot, rate, rate, volatility,
                steps, is_american, tree_type
            )
            
        elif pricing_method == 'FiniteDifference':
            M = int(data.get('fdM', 200))
            N = int(data.get('fdN', 200))
            scheme = opc.FDScheme.CrankNicolson
            
            pricer = opc.FiniteDifferenceAmericanPricer(
                option, spot, rate, rate, volatility,
                M, N, scheme
            )
        
        if pricer is None:
            return jsonify({'error': 'Méthode de pricing invalide'}), 400
        
        # Calculer le prix et les Greeks
        price = pricer.price()
        delta = pricer.delta(spot)
        
        # Greeks qui peuvent échouer
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
        
        # Pour Monte Carlo, obtenir l'intervalle de confiance
        confidence_interval = None
        if pricing_method == 'MonteCarlo':
            try:
                mc_result = pricer.price_with_confidence()
                confidence_interval = {
                    'lower': mc_result.ci_lower_95,
                    'upper': mc_result.ci_upper_95,
                    'std_error': mc_result.std_error
                }
            except:
                pass
        
        # Construire la réponse
        result = {
            'price': price,
            'delta': delta,
            'gamma': gamma,
            'vega': vega,
            'theta': theta,
            'rho': rho,
            'method': pricing_method,
            'confidenceInterval': confidence_interval
        }
        
        return jsonify(result)
        
    except Exception as e:
        traceback.print_exc()
        return jsonify({
            'error': str(e),
            'traceback': traceback.format_exc()
        }), 500

@app.route('/api/sensitivity', methods=['POST'])
def calculate_sensitivity():
    """
    Calculer la sensibilité du prix par rapport au spot
    """
    try:
        data = request.json
        
        # Paramètres de base
        strike = float(data['strike'])
        maturity = float(data['maturity'])
        rate = float(data['rate'])
        volatility = float(data['volatility'])
        option_type = TYPE_MAP[data['optionType']]
        option_style_str = data['optionStyle']
        pricing_method = data['pricingMethod']
        
        # Range de spots
        base_spot = float(data['spot'])
        spot_min = base_spot * 0.7
        spot_max = base_spot * 1.3
        num_points = int(data.get('numPoints', 25))
        
        spots = np.linspace(spot_min, spot_max, num_points)
        
        results = []
        
        for spot in spots:
            # Créer payoff et option
            style = STYLE_MAP[option_style_str]
            payoff = opc.create_payoff(style, option_type, strike)
            option = opc.Option(maturity, payoff)
            
            # Créer le pricer
            if pricing_method == 'BlackScholes':
                pricer = opc.BlackScholesPricer(
                    option, spot, rate, rate, volatility
                )
            elif pricing_method == 'BinomialTree':
                pricer = opc.BinomialTreePricer(
                    option, spot, rate, rate, volatility, 200, False
                )
            else:
                # Pour les autres méthodes, utiliser Black-Scholes par défaut
                pricer = opc.BlackScholesPricer(
                    option, spot, rate, rate, volatility
                )
            
            price = pricer.price()
            
            # Valeur intrinsèque
            if option_type == opc.OptionType.Call:
                intrinsic = max(0, spot - strike)
            else:
                intrinsic = max(0, strike - spot)
            
            time_value = price - intrinsic
            
            results.append({
                'spot': round(spot, 2),
                'price': round(price, 4),
                'intrinsic': round(intrinsic, 4),
                'timeValue': round(time_value, 4)
            })
        
        return jsonify(results)
        
    except Exception as e:
        traceback.print_exc()
        return jsonify({
            'error': str(e),
            'traceback': traceback.format_exc()
        }), 500

@app.route('/api/volatility_surface', methods=['POST'])
def calculate_vol_surface():
    """
    Calculer une surface de volatilité implicite (simplifié)
    """
    try:
        data = request.json
        
        spot = float(data['spot'])
        rate = float(data['rate'])
        option_type = TYPE_MAP[data['optionType']]
        
        # Grille de strikes et maturités
        strikes = np.linspace(spot * 0.8, spot * 1.2, 10)
        maturities = np.linspace(0.25, 2.0, 10)
        volatilities = np.linspace(0.1, 0.4, 10)
        
        results = []
        
        for maturity in maturities:
            for strike in strikes:
                for vol in volatilities:
                    payoff = opc.create_payoff(
                        opc.PayoffStyle.European,
                        option_type,
                        strike
                    )
                    option = opc.Option(maturity, payoff)
                    pricer = opc.BlackScholesPricer(
                        option, spot, rate, rate, vol
                    )
                    
                    price = pricer.price()
                    
                    results.append({
                        'strike': round(strike, 2),
                        'maturity': round(maturity, 2),
                        'volatility': round(vol, 2),
                        'price': round(price, 4)
                    })
        
        return jsonify(results)
        
    except Exception as e:
        traceback.print_exc()
        return jsonify({
            'error': str(e)
        }), 500

if __name__ == '__main__':
    print("=" * 60)
    print("🚀 Démarrage du backend Option Pricer")
    print("=" * 60)
    print("API disponible sur: http://localhost:5000")
    print("Endpoints:")
    print("  - GET  /api/health")
    print("  - POST /api/price")
    print("  - POST /api/sensitivity")
    print("  - POST /api/volatility_surface")
    print("=" * 60)
    
    app.run(debug=True, host='0.0.0.0', port=5000)