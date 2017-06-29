#include "fonction.h"
#include "QDebug"
#include <math.h>

// Méthodes

void debugStdVect(std::vector<float>& vect)//pour le debug
{
    QVector<float> vector = QVector<float>::fromStdVector(vect);
    qDebug() << vector;
}


CoordVector sph2cart(float ro, float theta, float phi)
{
    float x,y,z;
    x = ro*cos(phi)*cos(theta);
    y = ro*cos(phi)*sin(theta);
    z = ro*sin(phi);
    CoordVector coord(x,y,z);

    return coord;
}


CoordVector vecteur(const CoordVector &a,const CoordVector &b)
{
    CoordVector resultat(b.x-a.x,b.y-a.y,b.z-a.z);
    return resultat;
}


CoordVector vecteur(std::vector<float>& a, int indA, std::vector<float>& b, int indB)
{
    CoordVector resultat;
    resultat.x = b[indB]-a[indA];
    resultat.y = b[indB+1]-a[indA+1];
    resultat.z = b[indB+2]-a[indA+2];

    return resultat;
}

CoordVector vecteur(std::vector<float>& a, int indA, const CoordVector &b)
{
    CoordVector resultat;
    resultat.x = b.x-a[indA];
    resultat.y = b.y-a[indA+1];
    resultat.z = b.z-a[indA+2];

    return resultat;
}


float produitScalaire(const CoordVector &a,const CoordVector &b)
{
    float resultat = a.x*b.x + a.y*b.y + a.z*b.z;

    if(resultat< 0.000001 && resultat > -0.000001)
    {
        return 0;
        //qDebug() << "vecteurs colinéaires";
    }

    return resultat;
}

float produitScalaire(std::vector<float>& a, int indA,const CoordVector &b)
{
    float resultat = a[indA]*b.x + a[indA+1]*b.y + a[indA+2]*b.z;
    if(resultat< 0.000001 && resultat > -0.000001)
    {
        return 0;
    }
    return resultat;
}


// produit scalaire qui prend l'adresse de gros vecteurs ansi que l'indice du x
float produitScalaire(std::vector<float>& a, int indA, std::vector<float>& b, int indB)
{

    float resultat(0);
    for (int i=0; i<3; i++)
    {
       resultat = resultat + a[indA+i]*b[indB+i];
    }
    if(resultat< 0.000001 && resultat > -0.000001)
    {
        return 0;
        //qDebug() << "vecteurs colinéaires";
    }
    return resultat;

}


CoordVector produitVectoriel(const CoordVector &a,const CoordVector &b)
{
    CoordVector resultat;
    resultat.x = a.y*b.z - a.z*b.y;
    resultat.y = a.z*b.x - a.x*b.z;
    resultat.z = a.x*b.y - a.y*b.x;

    return resultat;
}


float angle(const CoordVector &a,const CoordVector &b) // angle entre les deux vecteurs
{

    if (norme(a) == 0 || norme(b) == 0) return 0;
    else return acos(produitScalaire(a,b)/(norme(a)*norme(b)));
}

float norme(const CoordVector &a)
{
    return sqrt(produitScalaire(a,a));
}


bool proche(float a, float b)
{
    float seuil = 0.01;

    if (a/b > 1-seuil && a/b < 1+seuil)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Les classes

CoordVector::CoordVector(float px,float py,float pz):x(px),y(py),z(pz)
{
}
CoordVector::~CoordVector()
{
}
CoordVector CoordVector::operator=(const CoordVector &fv)
{
    x=fv.x;
    y=fv.y;
    z=fv.z;

    return *this;
}

QVector<float> CoordVector::CoordVector2Qvector(CoordVector coord)
{
    QVector<float> vect;
    vect.push_back(coord.x);
    vect.push_back(coord.y);
    vect.push_back(coord.z);

    return vect;
}

void CoordVector::debug()
{

    QString Mx,My,Mz;
    Mx.setNum(x);
    My.setNum(y);
    Mz.setNum(z);
    qDebug() << "(" + Mx + "," + My + "," + Mz +")";
}
