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
CoordVector vecteur(const CoordVector &a,const CoordVector &b);
CoordVector vecteur(std::vector<float>& a, int indA, std::vector<float>& b, int indB);
CoordVector vecteur(std::vector<float>& a, int indA, const CoordVector &b);
float produitScalaire(const CoordVector &a,const CoordVector &b);
float produitScalaire(std::vector<float>& a, int indA,const CoordVector &b);
//float produitScalaire(float x1,float y1,float z1,float x2,float y2,float z2);
float produitScalaire(std::vector<float>& a, int indA, std::vector<float>& b, int indB);
CoordVector produitVectoriel(const CoordVector &a,const CoordVector &b);
float norme(const CoordVector &a);
float angle(const CoordVector &a,const CoordVector &b);
void debugStdVect(std::vector<float>& vect); // permet d'afficher un std::vector dans la console
//CoordVector coord_New_Base(const CoordVector &point, std::vector<float> &mat );
bool proche(float a, float b);

#endif // FONCTION_H
