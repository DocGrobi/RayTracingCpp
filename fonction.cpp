#include "fonction.h"
#include "QDebug"
#include <math.h>

// Méthodes

void debugStdVect(std::vector<float> vect)//pour le debug
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

CoordVector vecteur(CoordVector a, CoordVector b)
{
    CoordVector resultat(b.x-a.x,b.y-a.y,b.z-a.z);
    return resultat;
}

float produitScalaire(CoordVector a, CoordVector b)
{
    float resultat = a.x*b.x + a.y*b.y + a.z*b.z;
    if(resultat< 0.0001 && resultat > -0.0001)
    {
        resultat =0;
        //qDebug() << "vecteurs colinéaires";
    }
    return resultat;
}

CoordVector produitVectoriel(CoordVector a, CoordVector b)
{
    CoordVector resultat;
    resultat.x = a.y*b.z - a.z*b.y;
    resultat.y = a.z*b.x - a.x*b.z;
    resultat.z = a.x*b.y - a.y*b.x;

    return resultat;

}

float angle(CoordVector a, CoordVector b) // angle entre les deux vecteurs
{
    float resultat = acos(produitScalaire(a,b)/(norme(a)*norme(b)));
    return resultat;
}

float norme(CoordVector a)
{
    float resultat = sqrt(produitScalaire(a,a));
    return resultat;
}


CoordVector coord_New_Base(CoordVector point, std::vector<float> mat) // pour matrice 3x3 pour l'instant
{
    // calcul déterminant
    float determinant(0);
    for (int i =0; i< 3; i++)
    {
        float produit(1);
        i = i*3;
        for(int ind=0; ind <mat.size(); ind=ind+4) // dependra de la taille de la matrice
        {
            /*ind = 4*ind;
            if ((ind+i) > mat.size())
            {
                ind = ind - mat.size();
            }
            */
            produit = produit*mat[(ind+i) % mat.size()];
        }
        determinant = determinant + produit;
    }
    for (int i =0; i< 3; i++)
    {
        float produit(1);
        for(int ind=6; ind >1; ind=ind-2) // dependra de la taille de la matrice
        {
            produit = produit*mat[ind+i];
        }
        determinant = determinant - produit;
    }


    // matrice
    /*
    for (int i; i<mat.size(); i++)
    {
        float r(0);
        for (int k = 0; k<2 ;k ++)
        {
            //int ind = (i + 4) % 9; //modulo
            r = (mat[(i + 4) % mat.size()]*mat[(i + 8) % mat.size()]) - (mat[(i + 7) % mat.size()]*mat[(i + 5) % mat.size()]);
        }

        mat[i]= r/determinant;
    }*/

    // nouvelle matrice ---- utiliser des produit vectoriel
    std::vector<float> newMat;
    newMat.push_back(mat[4]*mat[8] - mat[5]*mat[7]);
    newMat.push_back(mat[5]*mat[6] - mat[3]*mat[8]);
    newMat.push_back(mat[3]*mat[7] - mat[4]*mat[6]);
    newMat.push_back(mat[2]*mat[7] - mat[1]*mat[8]);
    newMat.push_back(mat[0]*mat[8] - mat[2]*mat[6]);
    newMat.push_back(mat[1]*mat[6] - mat[0]*mat[7]);
    newMat.push_back(mat[1]*mat[5] - mat[2]*mat[4]);
    newMat.push_back(mat[2]*mat[3] - mat[0]*mat[5]);
    newMat.push_back(mat[3]*mat[4] - mat[1]*mat[3]);

    //debugStdVect(mat);
    //debugStdVect(newMat);

    CoordVector resultat(0,0,0);
    std::vector<float> coordpoint;
    coordpoint.push_back(point.x);
    coordpoint.push_back(point.y);
    coordpoint.push_back(point.z);

    //point.debug();

    for (int i = 0; i<3; i++)
    {
        resultat.x = resultat.x + (newMat[i]*coordpoint[i]/determinant);
        resultat.y= resultat.y + (newMat[i+3]*coordpoint[i]/determinant);
        resultat.z= resultat.z + (newMat[i+6]*coordpoint[i]/determinant);
        //qDebug()<< resultat.x;
    }
    qDebug() << "dans nouvelle base :";
    resultat.debug();
    return resultat;
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
