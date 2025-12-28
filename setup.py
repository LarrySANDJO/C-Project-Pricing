from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension, build_ext
import sys
import os

print("=" * 70)
print("COMPILATION DU MODULE OPTION_PRICER_CPP")
print("=" * 70)

# Liste complète et ordonnée des fichiers .cpp
cpp_files = [
    'bindings.cpp',                  # Binding pybind11
    'pricer.cpp',                    # Interface Pricer (méthodes par défaut)
    'payoff.cpp',                    # Payoffs
    'option.cpp',                    # Classe Option
    'black_scholes_pricer.cpp',      # Black-Scholes
    'monte_carlo_pricer.cpp',        # Monte Carlo
    'binomial_tree_pricer.cpp',      # Arbres binomiaux
    'finite_difference_pricer.cpp',  # Différences finies
    'replication_strategy.cpp'       # Stratégies de réplication
]

# Vérifier l'existence des fichiers
existing_files = []
missing_files = []

print("\nVérification des fichiers sources:")
print("-" * 70)

for cpp_file in cpp_files:
    if os.path.exists(cpp_file):
        existing_files.append(cpp_file)
        print(f"✓ {cpp_file:<35} [TROUVÉ]")
    else:
        missing_files.append(cpp_file)
        print(f"✗ {cpp_file:<35} [MANQUANT]")

print("-" * 70)
print(f"Fichiers trouvés: {len(existing_files)}/{len(cpp_files)}")

if missing_files:
    print(f"\n⚠ ATTENTION: {len(missing_files)} fichier(s) manquant(s):")
    for f in missing_files:
        print(f"  - {f}")
    print("\nLa compilation continuera avec les fichiers disponibles.")

if not existing_files:
    print("\n❌ ERREUR: Aucun fichier source trouvé!")
    print("Vérifiez que vous êtes dans le bon répertoire.")
    sys.exit(1)

print("\n" + "=" * 70)
print("CONFIGURATION DE LA COMPILATION")
print("=" * 70)

ext_modules = [
    Pybind11Extension(
        "option_pricer_cpp",
        existing_files,
        include_dirs=['.'],
        extra_compile_args=['-std=c++17', '-O3', '-Wall'], #, '-Wno-unused-variable'
        language='c++'
    ),
]

setup(
    name="option_pricer_cpp",
    version="1.0.0",
    author="Larry",
    description="Module Python pour pricing d'options quantitatives",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.7",
)

print("\n✓ Configuration terminée")
print("=" * 70)