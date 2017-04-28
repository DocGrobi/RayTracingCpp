#include "raytracing.h"
#include <math.h>
#include <vector>
#include "QDebug"
#include "QVector"


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

float angle(float prdScalaire, CoordVector a, CoordVector b) // cos de l'angle entre les deux vecteurs
{
    float resultat = prdScalaire/(norme(a)*norme(b));
    return resultat;
}

float norme(CoordVector a)
{
    float resultat = sqrt(produitScalaire(a,a));
    return resultat;
}

// intersection d'une droite comprenant un point et un vecteur directeur avec un plan comprenant un vecteur normal
CoordVector intersection(CoordVector point_ray, CoordVector vect_dir, CoordVector vect_norm, float k)
{
    CoordVector resultat;
    float t = -(produitScalaire(point_ray,vect_norm)+k)/produitScalaire(vect_dir,vect_norm);
    //qDebug() << "t : ";
    //qDebug() << produitScalaire(point_ray,vect_norm);
    //qDebug() << produitScalaire(vect_dir,vect_norm);
    resultat.x = vect_dir.x*t + point_ray.x;
    resultat.y = vect_dir.y*t + point_ray.y;
    resultat.z = vect_dir.z*t + point_ray.z;

    //qDebug() << "point d'intersection: ";
    //resultat.debug();

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

bool appartient_face(CoordVector point, std::vector<float> face)
{
    bool resultat = false;

    /* Methode de changement de base (ne marche pas)
     * point = coord_New_Base(point,face);
    if(point.x >0 && point.x < 1 && point.y > 0 && point.y < 1)
    {
        resultat = true;
    }
    */

     CoordVector a(face[0],face[1],face[2]);
     CoordVector b(face[3],face[4],face[5]);
     CoordVector c(face[6],face[7],face[8]);

     float hypoA(0), hypoB(0), hypoC(0);
     hypoA = produitScalaire(produitVectoriel(vecteur(a,b),vecteur(a,point)),produitVectoriel(vecteur(a,point),vecteur(a,c)));
     hypoB = produitScalaire(produitVectoriel(vecteur(b,a),vecteur(b,point)),produitVectoriel(vecteur(b,point),vecteur(b,c)));
     hypoC = produitScalaire(produitVectoriel(vecteur(c,a),vecteur(c,point)),produitVectoriel(vecteur(c,point),vecteur(c,b)));

     if(hypoA>0 && hypoB>0 && hypoC >0)
     {
         resultat = true;
     }

    return resultat;
}


// Les classes
Ray::Ray(float phy, int Nray, Source S)
{
    m_Nray = S.vert().size();
    //m_col(0);
    //m_dist(0)
    m_dMax = 0;
    m_src = S.centre();

    float ro(1);

    m_ray.clear();

    /*
    // OPTION 1 : Repartition uniforme des rayons (2D pour l'instant)
    float theta(0), phi(0);
    for (int i = 0; i<Nray ; i++)
    {
        // creation des vecteurs directeur
        theta = theta + 2*M_PI/Nray;
        CoordVector coord(sph2cart(ro,theta,phi));

        m_ray.push_back(m_src.x);
        m_ray.push_back(m_src.y);
        m_ray.push_back(m_src.z);

        m_ray.push_back(coord.x + m_src.x);
        m_ray.push_back(coord.y + m_src.y);
        m_ray.push_back(coord.z + m_src.z);
    }

    */

 qDebug() << S.vert().size();
    // OPTION 2 : Repartition des rayons sur chaque vertex de la sphere
    for (int i=0; i<(m_Nray-3); i=i+3)
    {
    // creation des vecteurs directeur
        m_ray.push_back(m_src.x);
        m_ray.push_back(m_src.y);
        m_ray.push_back(m_src.z);

        for(int j=0;j<3;j++)
        {
            m_ray.push_back(S.vert()[i+j]);
        }
    }



    // OPTION 3 : Discretisation des rayons sur une grille uniforme de Fibonacci
    /*
    float OR = (1+sqrt(5))/2;

    std::vector<float> theta(0);
    for (int i; i<N; i++)
    {

        //float mod = (i*2*M_PI/OR) % (2*M_PI); // a inverser et comprendre
        float mod = (i*2*M_PI/OR) - (2*M_PI)*(floor((i*2*M_PI/OR)/(2*M_PI)));
        theta.push_back(mod);
    }

    std::vector<float> phi(0);
    for (int i; i<N; i++)
    {
        float angle = asin(-1 + 2/(N-1)*i);
        phi.push_back(angle);
    }
    */
}

Ray::~Ray()
{
    //free(m_ray);
}

std::vector<float> Ray::getRay() const // Accesseur au vecteur de rayons
{
    return m_ray;
}



void Ray::rebond(MeshObj mesh, int nb_rebond)
{
    std::vector<float> normales(mesh.getNormals());
    std::vector<float> vertex(mesh.getVertex());

    for (int i=0; i<nb_rebond; i++)// pour chaque ordre de rebond
    {
        for(int j=0; j<m_Nray*6;j=j+6) // pour chaque rayon
        {
            // pour l'ordre deux on prendra j+4 pour le vecteur directeur et j pour le point
            CoordVector point(m_ray[j], m_ray[j+1] , m_ray[j+2]);
            CoordVector vect_dir(m_ray[j+3]-m_ray[j],m_ray[j+4]-m_ray[j+1] ,m_ray[j+5]-m_ray[j+2]);
            //vect_dir.debug();
            bool cherche_face = true;


            for (int k=0; k < vertex.size(); k = k+9) // pour chaque face
            {
                if(cherche_face)
                {
                    CoordVector vect_norm(normales[k], normales[k+1], normales[k+2]);
                    //qDebug() << "vecteur normal: ";
                    //vect_norm.debug();

                    if (produitScalaire(vect_dir,vect_norm)<0) // test non-parralelisme des vecteurs et sens opposé
                    {

                        //calcul de la constante de l'équation du plan
                        float cons(0);
                        for (int ind=0;ind<3; ind++)
                        {
                            cons = cons - (vertex[k+ind]*normales[k+ind]);
                        }

                        // point d'intersection de la droite portée par vect-dir et du plan de normal vect_norm
                        CoordVector intersec = intersection(point, vect_dir, vect_norm, cons);
                        //intersec.debug();

                        // matrice prenant en colonne les coordonnées des trois vertex de la face
                        std::vector<float> mat_coordFace;
                        for (int l=0; l<9; l++)
                        {
                            mat_coordFace.push_back(vertex[k+l]);
                        }

                        // si le point est appartient à la face donc si x et y (dans la nouvelle base) sont compris entre 0 et 1
                        if (appartient_face(intersec, mat_coordFace))
                        {
                           //m_ray.push_back(intersec.x);m_ray.push_back(intersec.y);m_ray.push_back(intersec.z);

                            m_ray[j+3] = intersec.x;
                            m_ray[j+4] = intersec.y;
                            m_ray[j+5] = intersec.z;

                            cherche_face = false;
                        }
                    }
                }
            }
        }
    }
}

