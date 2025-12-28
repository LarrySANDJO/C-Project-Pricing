
# Option Pricer - Système Complet de Pricing d'Options

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Python](https://img.shields.io/badge/Python-3.7+-green.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

Système professionnel de pricing d'options financières avec backend C++ haute performance et interface web React moderne.

Création d'un programme pour déterminer le prix d’une option financière, ainsi que la stratégie de réplication dans le modèle de Black-Scholes Merton. Dans la plupart des cas où les formules explicites ne s’appliquent pas, on calculera ces prix par méthode de Monte-Carlo.

---

## Table des Matières

- [Vue d'ensemble](#vue-densemble)
- [Fonctionnalités](#fonctionnalités)
- [Architecture](#architecture)
- [Installation](#installation)
- [Auteur](#auteur)

---

## Vue d'ensemble

Ce projet implémente un système complet de pricing d'options financières combinant :

- **Backend C++** : Calculs haute performance pour le pricing et les Greeks
- **Binding Python** : Exposition du code C++ via **pybind11**
- **API REST Flask** : Interface HTTP pour le pricing
- **Interface Web React** : Interface utilisateur moderne et interactive

### Pourquoi ce projet ?

- **Performance** : Calculs en C++ pour vitesse maximale
- **Flexibilité** : Multiple méthodes de pricing (analytique, numérique, simulation)
- **Modernité** : Interface web immersive avec graphiques interactifs
- **Pédagogique** : Code clair et bien documenté pour l'apprentissage
- **Production-ready** : Validation rigoureuse et gestion d'erreurs

---

## Fonctionnalités

### Types d'Options Supportés

| Type | Description | Méthodes disponibles |
|------|-------------|---------------------|
| **Européennes** | Exercice uniquement à maturité | BS, MC, Tree, FD |
| **Américaines** | Exercice anticipé possible | Tree, FD |
| **Asiatiques** | Moyenne arithmétique/géométrique | MC |
| **Lookback** | Sur maximum/minimum | MC |
| **Barrières** | Up/Down, In/Out | MC |
| **Digitales** | Paiement binaire | MC |
| **Power** | Payoff avec exposant | MC |

### Méthodes de Pricing

| Méthode | Temps | Précision | Options supportées |
|---------|-------|-----------|-------------------|
| **Black-Scholes** | < 1ms | Analytique | Européennes uniquement |
| **Monte Carlo** | ~100ms | Configurable | Toutes |
| **Arbres Binomiaux** | ~50ms | Bonne | Européennes, Américaines |
| **Différences Finies** | ~100ms | Très bonne | Américaines |

### Greeks Calculés

- **Delta (Δ)** : Sensibilité au prix du sous-jacent
- **Gamma (Γ)** : Convexité (dérivée seconde par rapport au spot)
- **Vega (ν)** : Sensibilité à la volatilité
- **Theta (Θ)** : Décroissance temporelle (time decay)
- **Rho (ρ)** : Sensibilité au taux d'intérêt

### Fonctionnalités Avancées

- **Stratégies de réplication** : Delta-hedging avec analyse P&L
- **Analyse de sensibilité** : Graphiques de prix vs spot
- **Variables antithétiques** : Réduction de variance Monte Carlo et optimisation du code
- **Arbres avancés** : CRR, Jarrow-Rudd, Leisen-Reimer (arbres binomiaux)
- **Intervalles de confiance** : Pour Monte-Carlo

---

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     INTERFACE WEB (React)                    │
│  • Paramètres interactifs                                   │
│  • Graphiques en temps réel                                 │
│  • Affichage des Greeks                                     │
└──────────────────────┬──────────────────────────────────────┘
                       │ HTTP/JSON
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                    API REST (Flask)                          │
│  • Endpoints de pricing                                     │
│  • Gestion des erreurs                                      │
│  • Validation des paramètres                                │
└──────────────────────┬──────────────────────────────────────┘
                       │ Python
                       ▼
┌─────────────────────────────────────────────────────────────┐
│              MODULE PYTHON (pybind11)                        │
│  • Binding C++ → Python                                     │
│  • Conversion des types                                     │
└──────────────────────┬──────────────────────────────────────┘
                       │ C++
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                 MOTEUR DE CALCUL (C++)                       │
│  • Black-Scholes analytique                                 │
│  • Monte Carlo avec réduction de variance                   │
│  • Arbres binomiaux (CRR, JR, LR)                          │
│  • Différences finies (Crank-Nicolson)                     │
│  • Calcul des Greeks                                        │
└─────────────────────────────────────────────────────────────┘
```

---

## Installation

### Prérequis

#### Système d'exploitation
- **Linux** : Ubuntu 20.04+, Debian 11+, CentOS 8+
- **macOS** : 10.15+ (Catalina ou supérieur)
- **Windows** : 10/11 avec WSL2 ou MinGW

#### Logiciels requis

| Logiciel | Version minimale | Commande de vérification |
|----------|------------------|-------------------------|
| **Python** | 3.7+ | `python --version` |
| **pip** | 20.0+ | `pip --version` |
| **g++** ou **clang++** | Support C++17 | `g++ --version` |
| **git** | 2.0+ | `git --version` |

#### Installation des dépendances système

**Ubuntu/Debian :**
```bash
sudo apt update
sudo apt install -y python3 python3-pip python3-dev build-essential git
```

**macOS :**
```bash
# Installer Homebrew si nécessaire
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Installer les outils
brew install python gcc git
```

**Windows (WSL2) :**
```powershell
# Installer WSL2 et Ubuntu depuis le Microsoft Store
wsl --install

# Puis dans le terminal Ubuntu :
sudo apt update
sudo apt install -y python3 python3-pip python3-dev build-essential git
```

---

### Installation

#### Étape 1 : Cloner le projet

```bash
# Cloner le dépôt
git clone https://github.com/votre-username/option-pricer.git
cd option-pricer

# Vérifier que tous les fichiers sont présents
ls -la *.cpp *.hpp
```

**Fichiers attendus :**
```
bindings.cpp
pricer.cpp
pricer.hpp
payoff.cpp
payoff.hpp
option.cpp
option.hpp
option_type.hpp
black_scholes_pricer.cpp
black_scholes_pricer.hpp
monte_carlo_pricer.cpp
monte_carlo_pricer.hpp
binomial_tree_pricer.cpp
binomial_tree_pricer.hpp
finite_difference_pricer.cpp
finite_difference_pricer.hpp
replication_strategy.cpp
replication_strategy.hpp
setup.py
backend.py
```

#### Étape 2 : Créer un environnement virtuel (facultatif mais recommandé)

```bash
# Créer l'environnement virtuel
python3 -m venv venv

# Activer l'environnement
# Linux/macOS :
source venv/bin/activate

# Windows (WSL) :
source venv/bin/activate

# Windows (PowerShell) :
.\venv\Scripts\Activate.ps1
```

#### Étape 3 : Installer les dépendances Python

```bash
# Mettre à jour pip
pip install --upgrade pip

# Installer les dépendances
pip install pybind11 flask flask-cors numpy

# Vérifier l'installation
pip list | grep -E "pybind11|flask|numpy"

```
**Sortie attendue :**
```
Flask            3.0.0
Flask-Cors       4.0.0
numpy            1.24.3
pybind11         2.11.1
```

#### Étape 4 : Compiler le module C++

```bash
# Nettoyer les compilations précédentes (important)
rm -rf build/
rm -f option_pricer_cpp*.so option_pricer_cpp*.pyd

# Compiler le module
python setup.py build_ext --inplace
```

**Sortie attendue :**
```
======================================================================
COMPILATION DU MODULE OPTION_PRICER_CPP
======================================================================

Vérification des fichiers sources:
----------------------------------------------------------------------
✓ bindings.cpp                      [TROUVÉ]
✓ pricer.cpp                        [TROUVÉ]
✓ payoff.cpp                        [TROUVÉ]
✓ option.cpp                        [TROUVÉ]
✓ black_scholes_pricer.cpp          [TROUVÉ]
✓ monte_carlo_pricer.cpp            [TROUVÉ]
✓ binomial_tree_pricer.cpp          [TROUVÉ]
✓ finite_difference_pricer.cpp      [TROUVÉ]
✓ replication_strategy.cpp          [TROUVÉ]
----------------------------------------------------------------------
Fichiers trouvés: 9/9

building 'option_pricer_cpp' extension
...
copying build/lib.*/option_pricer_cpp*.so -> .
```

**Vérification :**
```bash
# Le fichier .so (Linux/Mac) ou .pyd (Windows) doit exister
ls -lh option_pricer_cpp*.so
# ou
ls -lh option_pricer_cpp*.pyd
```

#### Étape 5 : Tester le module

```bash
# Test rapide
python -c "import option_pricer_cpp as opc; print('✓ Module chargé avec succès')"

# Test complet
python test_simple.py
```

#### Étape 6 : Lancer le backend Flask

```bash
# Lancer le serveur
python backend.py
```

#### Étape 7 : Accéder à l'interface web

Ouvrez un **nouveau terminal** et testez l'API :

```bash
# Test de santé
curl http://localhost:5000/api/health

# Sortie attendue :
# {"message":"Backend operational","status":"ok"}
```

**Pour accéder à l'interface graphique :**

1. L'interface React est disponible dans l'artefact Claude ci-dessus
2. Ouvrez votre navigateur sur la page Claude
3. L'interface se connecte automatiquement à `http://localhost:5000`

**Ou** pour un déploiement local complet :

```bash
# Dans un nouveau terminal
# (l'interface sera disponible sur http://localhost:3000)
# Instructions dans la section "Déploiement Production" ci-dessous
```

---

## Auteurs

Ce projet a été conçu par :

- Larry SANDJO
- Cheihk SANOGO
- Dunand DJAKAI

Tous Étudiants en deuxième année à l'ENSAE Paris.

Mail : *larrysandjo337@gmail.com*.