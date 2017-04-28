#include "raytracing.h"
#include <math.h>
//#include <vector>
#include "QDebug"
#include "QVector"


// Méthodes

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

