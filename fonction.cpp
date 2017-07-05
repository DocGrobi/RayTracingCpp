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

    return CoordVector(x,y,z);
}

CoordVector vecteur(const CoordVector &a,const CoordVector &b)
{
    return CoordVector(b.x-a.x,b.y-a.y,b.z-a.z);
}

CoordVector vecteur(std::vector<float>& a, int indA, std::vector<float>& b, int indB)
{
    return CoordVector(b[indB]-a[indA], b[indB+1]-a[indA+1], b[indB+2]-a[indA+2]);
}

CoordVector vecteur(std::vector<float>& a, int indA, const CoordVector &b)
{
    return CoordVector(b.x-a[indA], b.y-a[indA+1], b.z-a[indA+2]);
}

CoordVector vecteur(const CoordVector &a, std::vector<float>& b, int indB)
{
    return CoordVector(b[indB]-a.x, b[indB+1]-a.y, b[indB+2]-a.z);
}

float produitScalaire(const CoordVector &a,const CoordVector &b)
{
    float resultat = a.x*b.x + a.y*b.y + a.z*b.z;

    if(resultat< 1e-8 && resultat > -1e-8)
    {
        return 0;
        //qDebug() << "vecteurs colinéaires";
    }

    return resultat;

    //return (a.x*b.x + a.y*b.y + a.z*b.z);
}

float produitScalaire(const std::vector<float>& a, int indA,const CoordVector &b)
{
    /*
    float resultat = a[indA]*b.x + a[indA+1]*b.y + a[indA+2]*b.z;
    if(resultat< 0.000001 && resultat > -0.000001)
    {
        return 0;
    }
    return resultat;
    */
    return (a[indA]*b.x + a[indA+1]*b.y + a[indA+2]*b.z);
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
    /*
    CoordVector resultat;
    resultat.x = a.y*b.z - a.z*b.y;
    resultat.y = a.z*b.x - a.x*b.z;
    resultat.z = a.x*b.y - a.y*b.x;

    return resultat;
    */
    return CoordVector(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

CoordVector produitVectoriel(const std::vector<float> &a,int i, const CoordVector &b)
{
    /*
    CoordVector resultat;
    resultat.x = a.y*b.z - a.z*b.y;
    resultat.y = a.z*b.x - a.x*b.z;
    resultat.z = a.x*b.y - a.y*b.x;

    return resultat;
    */
    return CoordVector(a[i+1]*b.z - a[i+2]*b.y, a[i+2]*b.x - a[i]*b.z, a[i]*b.y - a[i+1]*b.x);
}


float angle(const CoordVector &a,const CoordVector &b) // angle entre les deux vecteurs
{

    if (norme(a) < 1e-8 || norme(b) < 1e-8) return 0;
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

CoordVector CoordVector::operator+=(const CoordVector &fv)
{
    x+=fv.x;
    y+=fv.y;
    z+=fv.z;

    return *this;
}
CoordVector CoordVector::operator-=(const CoordVector &a)
{
    x-=a.x;
    y-=a.y;
    z-=a.z;

    return *this;
}


CoordVector operator+(const CoordVector &a, const CoordVector &b)
{
    CoordVector c(a);
    c+=b;
    return c;
}

CoordVector operator+(const CoordVector &a, float b) {
    return CoordVector (a.x+b, a.y+b, a.z+b);
}
CoordVector operator-(const CoordVector &a, float b){
    return a+(-b);
}

CoordVector operator*(const CoordVector &a, const CoordVector &b) {
    return CoordVector (a.x*b.x, a.y*b.y, a.z*b.z);
}

CoordVector operator*(const CoordVector &a, float b) {
    return CoordVector (a.x*b, a.y*b, a.z*b);
}

CoordVector operator/(const CoordVector &a, float b) {
    return CoordVector (a.x/b, a.y/b, a.z/b);
}


CoordVector inverse(CoordVector const& a)
{
    return CoordVector(1/a.x, 1/a.y, 1/a.z);
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
