#ifndef FONCTION_H
#define FONCTION_H

//#include "objreader.h"
#include <vector>
#include <QVector>
#include <stdio.h>


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




// Vect3f

class Vect3f{
    public:

    inline Vect3f(){x=y=z=0;}
    inline Vect3f(float a,float b, float c){x=a;y=b;z=c;}
    inline float operator[](int i)const{return ((float*)this)[i];}
    inline float& operator[](int i){return ((float*)this)[i];}


    inline void print (const char* nom) const
    {printf("%s : %f %f %f \n",nom,x,y,z);}

    inline void operator+=(Vect3f const&a){x+=a.x;y+=a.y;z+=a.z;}
    inline void operator-=(Vect3f const&a){x-=a.x;y-=a.y;z-=a.z;}
    inline void operator*=(float a){x*=a;y*=a;z*=a;}

    float x;
    float y;
    float z;
};
inline Vect3f operator+(Vect3f const&a,Vect3f const&b){Vect3f c(a);c+=b;return c;}
inline Vect3f operator-(Vect3f const&a,Vect3f const&b){Vect3f c(a);c-=b;return c;}

inline Vect3f operator-(Vect3f const& a){return Vect3f(-a.x,-a.y,-a.z);}

inline Vect3f operator*(float a,Vect3f const& b){Vect3f v(b);v*=a;return v;}
inline float operator*(Vect3f const& a,Vect3f const& b){return a.x*b.x+a.y*b.y+a.z*b.z;}

inline Vect3f vectorielle(Vect3f const& a,Vect3f const& b){
    return Vect3f(
        a.y*b.z-a.z*b.y,
        b.x*a.z-b.z*a.x,
        a.x*b.y-a.y*b.x);
}
inline Vect3f operator^(Vect3f const& a,Vect3f const& b){
    return Vect3f(
        a.y*b.z-a.z*b.y,
        b.x*a.z-b.z*a.x,
        a.x*b.y-a.y*b.x);
}
inline Vect3f doubleproduit(Vect3f const& a,Vect3f const& b,Vect3f const& c){
    return (a*c)*b-(a*b)*c;
}

#endif // FONCTION_H
