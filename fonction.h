#ifndef FONCTION_H
#define FONCTION_H

//#include "objreader.h"
#include <vector>
#include <QVector>


// Les classes
class CoordVector
{
    /*
    Classe CoordVector : simple vecteur de coordonnées XYZ
    */
    public:
        CoordVector(float px=0,float py=0,float pz=0); // Constructeur
        ~CoordVector();// Destructeur
        CoordVector operator=(const CoordVector &fv);
        QVector<float> CoordVector2Qvector(CoordVector coord);
        void debug();
        /*
           Affecte au vecteur courant le contenu du vecteur passé en argument.
           Retourne le vecteur courant ainsi modifié.
        */

        float x,y,z;
};

// Méthodes
CoordVector sph2cart(float ro, float theta, float phi); // convertion coordonnée spherique à cartesiens
CoordVector vecteur(CoordVector a, CoordVector b);
float produitScalaire(CoordVector a, CoordVector b);
CoordVector produitVectoriel(CoordVector a, CoordVector b);
float norme(CoordVector a);
float angle(CoordVector a, CoordVector b);
void debugStdVect(std::vector<float> vect); // permet d'afficher un std::vector dans la console
CoordVector coord_New_Base(CoordVector point, std::vector<float> mat );

#endif // FONCTION_H
