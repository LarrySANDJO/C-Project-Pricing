# pragma once // Pour que le fichier ne soit inclut qu’une seule fois par unité de compilation.
// Remplace le #ifndef ... #define...

class Payoff {
public :
    virtual ~Payoff() = default; // le destructeur doit etre virtuel pour pouvoir supprimer aussi les PayoffCall, Put
    // default pour utiliser le destructeur du compilateur

    virtual double operator()(double spot) const = 0;
    // const car la fonction ne doit pas modifier l'etat de nos objet (strike, etc.)
}