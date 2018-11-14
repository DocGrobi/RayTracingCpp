#ifndef PHYSIC_H
#define PHYSIC_H

#include "fonction.h"
#include <QString>

// Definition de constantes
const float VITESSE_SON = 340; // m/s

// Classes
class Material
{
    public:
        Material();
        //Constructeur
        ~Material();
         //Destructeur
        float getIndMat(int num); // renvoi l'indce du materiau
        std::vector<QString> &getNomMat(); // renvoi le nom du materiau

    private:
        std::vector<float> m_coeffFreq; // coefficient d'absorption pour toutes les bandes de fréquence de tous les matériaux
        std::vector<QString> m_nomMat; // noms de matériaux classés dans l'ordre
        std::vector<float> m_indMat; // indice des matériaux
};

// Méthodes
std::vector<float> absair(int temp, int relh);


#endif // PHYSIC_H
