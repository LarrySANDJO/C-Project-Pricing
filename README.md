# 📊 Option Pricer - Système de Pricing Quantitatif

**Système professionnel de pricing d'options financières avec moteur C++ haute performance et interface Streamlit moderne.**

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Python](https://img.shields.io/badge/Python-3.7+-green.svg)
![Streamlit](https://img.shields.io/badge/Streamlit-1.28+-red.svg)

---

## 🎯 Vue d'Ensemble

Ce projet implémente un pricer d'options complet combinant :
- **Moteur C++** : Calculs haute performance
- **Binding Python** : Interface via pybind11
- **Interface Streamlit** : Application web interactive

### Fonctionnalités Principales

| Catégorie | Éléments Supportés |
|-----------|-------------------|
| **Types d'options** | Européennes, Américaines, Asiatiques, Lookback |
| **Méthodes** | Black-Scholes, Monte Carlo, Arbres Binomiaux, Différences Finies |
| **Greeks** | Delta, Gamma, Vega, Theta, Rho |
---

## 📁 Structure du Projet

```
C-Project-Pricing/
│
├── 📄 README.md                          # Ce fichier
├── 📄 requirements.txt                   # Dépendances Python
├── 📄 setup.py                          # Script de compilation C++
├── 📄 app.py                            # Interface Streamlit
│
├── 📂 Code Source C++ (Moteur de Calcul)
│   ├── pricer.hpp / pricer.cpp          # Interface abstraite Pricer
│   ├── option.hpp / option.cpp          # Classe Option
│   ├── payoff.hpp / payoff.cpp          # Payoffs (Européens, Asiatiques, etc.)
│   ├── option_type.hpp                  # Enum Call/Put
│   │
│   ├── black_scholes_pricer.*           # Pricing analytique Black-Scholes
│   ├── monte_carlo_pricer.*             # Simulations Monte Carlo
│   ├── binomial_tree_pricer.*           # Arbres binomiaux (CRR, JR)
│   ├── finite_difference_pricer.*       # Différences finies (Crank-Nicolson)
│   └── replication_strategy.*           # Stratégies de couverture
│
├── 📂 Binding Python
│   └── bindings.cpp                     # Exposition C++ → Python (pybind11)
│
└── 📂 Fichiers Générés (après compilation)
    ├── build/                           # Fichiers intermédiaires
    └── option_pricer_cpp.*.so           # Module Python compilé
```

### 🔍 Description des Composants

#### 1️⃣ **Couche C++ (Calculs)**

**Classes de Base :**
- `Pricer` : Interface abstraite définissant `price()`, `delta()`, `gamma()`, etc.
- `Option` : Contient maturité + payoff
- `Payoff` : Classe polymorphe pour différents types d'options

**Pricers Implémentés :**
```cpp
BlackScholesPricer       // Formule fermée pour options européennes
MonteCarloPricer         // Simulations pour options path-dependent
BinomialTreePricer       // Arbres pour options américaines
FiniteDifferenceAmericanPricer  // PDE pour options américaines
```

**Payoffs Disponibles :**
```cpp
Payoff                   // Européen Call/Put
AsianCallPayoff          // Moyenne arithmétique
AsianGeometricCallPayoff // Moyenne géométrique
LookbackCallPayoff       // Maximum du chemin
BarrierUpOutCallPayoff   // Barrière désactivante
// ... et plus
```

#### 2️⃣ **Binding Python (bindings.cpp)**

Expose toutes les classes C++ à Python via pybind11 :
```python
import option_pricer_cpp as opc

# Créer une option
payoff = opc.create_payoff(opc.PayoffStyle.European, opc.OptionType.Call, 100.0)
option = opc.Option(1.0, payoff)

# Pricer
pricer = opc.BlackScholesPricer(option, 100.0, 0.05, 0.05, 0.20)
price = pricer.price()
```

#### 3️⃣ **Interface Streamlit (app.py)**

Application web interactive avec :
- Panneau de configuration (sidebar)
- Affichage prix + Greeks
- Graphiques de sensibilité (Plotly)
- Tableau de données interactif

---

## 🚀 Installation et Utilisation

### Prérequis

- **Python** 3.7 ou supérieur
- **Compilateur C++** supportant C++17 (g++, clang++, MSVC)

---

### Installation Rapide (5 minutes)

#### Étape 1 : Cloner le projet

```bash
git clone https://github.com/LarrySANDJO/C-Project-Pricing/tree/main
```

#### Étape 2 : Créer l'environnement virtuel

```bash
# Créer l'environnement
python -m venv venv

# Activer l'environnement
# Linux/macOS :
source venv/bin/activate

# Windows :
source venv\Scripts\activate
```

#### Étape 3 : Installer les dépendances

```bash
# Installer depuis requirements.txt
pip install -r requirements.txt


#### Étape 4 : Compiler le module C++

```bash
# Compilation
python setup.py build_ext --inplace


#### Étape 6 : Lancer l'application

```bash
# Démarrer Streamlit
streamlit run app.py

# L'application s'ouvre automatiquement dans le navigateur
# URL : http://localhost:8501
```
---

## 📚 Architecture Détaillée

### Hiérarchie des Classes

```
Pricer (Interface abstraite)
├── BlackScholesPricer        (Formule fermée)
├── MonteCarloPricer          (Simulations)
├── BinomialTreePricer        (Arbre récursif)
└── FiniteDifferenceAmericanPricer (PDE)

Payoff (Base polymorphe)
├── Payoff                    (Européen standard)
├── AsianCallPayoff           (Path-dependent)
├── LookbackCallPayoff        (Extrémum)
├── BarrierUpOutCallPayoff    (Avec barrière)
└── ...

Option
└── Contient : Maturité + shared_ptr<Payoff>
```

### Flux de Calcul

```
1. Utilisateur configure paramètres (Streamlit)
         ↓
2. app.py appelle option_pricer_cpp (Python)
         ↓
3. pybind11 transmet à C++ (bindings.cpp)
         ↓
4. Moteur C++ effectue calculs
         ↓
5. Résultats retournés à Python
         ↓
6. Streamlit affiche résultats + graphiques
```

---

## 📜 Licence

Ce projet est sous licence MIT. Voir le fichier [LICENSE](LICENSE) pour plus de détails.

---

## 👨‍💻 Auteur

- **Larry SANDJO**
- **Cheick SANOGO**
- **Dunand DJAKAI**

Email: *larrysandjo337@gmail.com*