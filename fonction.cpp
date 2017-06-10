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
    /*
    if(resultat< 0.0001 && resultat > -0.0001)
    {
        return 0;
        //qDebug() << "vecteurs colinéaires";
    }
    */
    return resultat;
}

float produitScalaire(std::vector<float>& a, int indA,const CoordVector &b)
{
    float resultat = a[indA]*b.x + a[indA+1]*b.y + a[indA+2]*b.z;
    if(resultat< 0.0001 && resultat > -0.0001)
    {
        return 0;
    }
    return resultat;
}

/*
float produitScalaire(float x1,float y1,float z1,float x2,float y2,float z2)
{
    float resultat = x1*x2 + y1*y2 + z1*z2;
    if(resultat< 0.0001 && resultat > -0.0001)
    {
        resultat =0;
        //qDebug() << "vecteurs colinéaires";
    }
    return resultat;
}
*/

// produit scalaire qui prend l'adresse de gros vecteurs ansi que l'indice du x
float produitScalaire(std::vector<float>& a, int indA, std::vector<float>& b, int indB)
{

    float resultat(0);
    for (int i=0; i<3; i++)
    {
       resultat = resultat + a[indA+i]*b[indB+i];
    }
    if(resultat< 0.0001 && resultat > -0.0001)
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

    if (norme(a) == 0 || norme(b) == 0)
    {
        return 0;
    }
    else
    {
        return acos(produitScalaire(a,b)/(norme(a)*norme(b)));
    }

}

float norme(const CoordVector &a)
{
    return sqrt(produitScalaire(a,a));
}


//CoordVector coord_New_Base(CoordVector point, std::vector<float> mat) // pour matrice 3x3 pour l'instant
//{
//    // calcul déterminant
//    float determinant(0);
//    for (int i =0; i< 3; i++)
//    {
//        float produit(1);
//        i = i*3;
//        for(int ind=0; ind <mat.size(); ind=ind+4) // dependra de la taille de la matrice
//        {
//            /*ind = 4*ind;
//            if ((ind+i) > mat.size())
//            {
//                ind = ind - mat.size();
//            }
//            */
//            produit = produit*mat[(ind+i) % mat.size()];
//        }
//        determinant = determinant + produit;
//    }
//    for (int i =0; i< 3; i++)
//    {
//        float produit(1);
//        for(int ind=6; ind >1; ind=ind-2) // dependra de la taille de la matrice
//        {
//            produit = produit*mat[ind+i];
//        }
//        determinant = determinant - produit;
//    }


//    // matrice
//    /*
//    for (int i; i<mat.size(); i++)
//    {
//        float r(0);
//        for (int k = 0; k<2 ;k ++)
//        {
//            //int ind = (i + 4) % 9; //modulo
//            r = (mat[(i + 4) % mat.size()]*mat[(i + 8) % mat.size()]) - (mat[(i + 7) % mat.size()]*mat[(i + 5) % mat.size()]);
//        }

//        mat[i]= r/determinant;
//    }*/

//    // nouvelle matrice ---- utiliser des produit vectoriel
//    std::vector<float> newMat;
//    newMat.push_back(mat[4]*mat[8] - mat[5]*mat[7]);
//    newMat.push_back(mat[5]*mat[6] - mat[3]*mat[8]);
//    newMat.push_back(mat[3]*mat[7] - mat[4]*mat[6]);
//    newMat.push_back(mat[2]*mat[7] - mat[1]*mat[8]);
//    newMat.push_back(mat[0]*mat[8] - mat[2]*mat[6]);
//    newMat.push_back(mat[1]*mat[6] - mat[0]*mat[7]);
//    newMat.push_back(mat[1]*mat[5] - mat[2]*mat[4]);
//    newMat.push_back(mat[2]*mat[3] - mat[0]*mat[5]);
//    newMat.push_back(mat[3]*mat[4] - mat[1]*mat[3]);

//    //debugStdVect(mat);
//    //debugStdVect(newMat);

//    CoordVector resultat(0,0,0);
//    std::vector<float> coordpoint;
//    coordpoint.push_back(point.x);
//    coordpoint.push_back(point.y);
//    coordpoint.push_back(point.z);

//    //point.debug();

//    for (int i = 0; i<3; i++)
//    {
//        resultat.x = resultat.x + (newMat[i]*coordpoint[i]/determinant);
//        resultat.y= resultat.y + (newMat[i+3]*coordpoint[i]/determinant);
//        resultat.z= resultat.z + (newMat[i+6]*coordpoint[i]/determinant);
//        //qDebug()<< resultat.x;
//    }
//    qDebug() << "dans nouvelle base :";
//    resultat.debug();
//    return resultat;
//}

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
    //a=fv.a;

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
