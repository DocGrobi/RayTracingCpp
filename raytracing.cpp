#include "raytracing.h"
#include <math.h>
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

     if(hypoA>=0 && hypoB>=0 && hypoC >=0) // si =0 on est sur l'arrete
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

    resultat.x = vect_dir.x*t + point_ray.x;
    resultat.y = vect_dir.y*t + point_ray.y;
    resultat.z = vect_dir.z*t + point_ray.z;

    return resultat;
}

// vecteur reflechi par rapport au rayon incident et au vecteur normale de la face
CoordVector vecteur_reflechi(CoordVector i, CoordVector n)
{
    CoordVector resultat;
    resultat.x = -2*produitScalaire(i,n)*n.x + i.x;
    resultat.y = -2*produitScalaire(i,n)*n.y + i.y;
    resultat.z = -2*produitScalaire(i,n)*n.z + i.z;

    return resultat;
}


// Les classes

// Constructeur
Ray::Ray(float phy, int Nray, Source S)
{    
    m_Nray = S.vert().size();

    m_dMax = 0;
    m_src = S.centre();        
    m_dist.resize(m_Nray/3, 0); // longueur totale de chaque rayon
    m_long.resize(m_Nray/3, 0); // longueur du dernier segment de rayon
    m_nrg.resize(m_Nray/3*8, 1); // 8 bandes de fréquence par rayons

    m_pos.resize(m_Nray, 0);
    m_dir.resize(m_Nray, 0);
    //m_angle.resize(m_Nray/3, 0);

    m_nbRayMort = 0;
    m_rayVivant.resize(m_Nray/3, true); // Tous les rayons sont vivant

    m_ray.clear(); //facultatif à priori

    /*
    // OPTION 1 : Repartition uniforme des rayons (2D pour l'instant)
    float theta(0), phi(0), ro(1);;
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


    // OPTION 2 : Repartition des rayons sur chaque vertex de la sphere

    // étage 0 : source
    for (int i=0; i<m_Nray; i=i+3)
    {
    // creation des vecteurs directeur
        m_ray.push_back(m_src.x);
        m_ray.push_back(m_src.y);
        m_ray.push_back(m_src.z);
    }

    // étage 1 : coordonnées des vertex
    for (int i=0; i<m_Nray; i=i+3)
    {
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

// Destructeur
Ray::~Ray()
{
}

// Accesseur
std::vector<float> Ray::getRay() const
{
    return m_ray;
}

std::vector<float> Ray::getNRG() const
{
    return m_nrg;
}
std::vector<float> Ray::getPos() const
{
    return m_pos;
}

std::vector<float> Ray::getDir() const
{
    return m_dir;
}

int Ray::getNbRay() const
{
    return m_Nray;
}

std::vector<float> Ray::getDist() const
{
    return m_dist;
}

std::vector<float> Ray::getLong() const
{
    return m_long;
}

int Ray::getRayMorts() const
{
    return m_nbRayMort;
}

void Ray::stockage()
{
    for(int i = 0; i<m_Nray ; i++)
    {
       m_pos[i] = m_ray[i];
       m_dir[i] = m_ray[i+m_Nray]-m_ray[i];
    }

}


void Ray::rebond(MeshObj mesh, int nb_rebond)
{
    std::vector<float> normales(mesh.getNormals());
    std::vector<float> vertex(mesh.getVertex());

    for (int i=0; i<(nb_rebond*m_Nray); i=i+m_Nray)// pour chaque ordre de rebond
    {
        for(int j=0; j<m_Nray;j=j+3) // pour chaque rayon
        {
            // recuperation d'un point et du vecteur directeur
            CoordVector point(m_ray[i+j], m_ray[i+j+1] , m_ray[i+j+2]);
            CoordVector vect_dir(m_ray[i+j+m_Nray]-m_ray[i+j],m_ray[i+j+m_Nray+1]-m_ray[i+j+1] ,m_ray[i+j+m_Nray+2]-m_ray[i+j+2]);

            // Stockage de la longeur du nouveau rayon
            float longueur_ray(1000000);

            // On rajoute un etage à m_ray
            m_ray.push_back(0);
            m_ray.push_back(0);
            m_ray.push_back(0);

            for (int k=0; k < vertex.size(); k = k+9) // pour chaque face
            {
                // Recuperation du vecteur normale
                CoordVector vect_norm(normales[k], normales[k+1], normales[k+2]);

                if (produitScalaire(vect_dir,vect_norm)<0) // test non-parralelisme des vecteurs et sens opposé
                {
                    //calcul de la constante de l'équation du plan
                    float cons(0);
                    for (int ind=0;ind<3; ind++)
                    {
                        cons = cons - (vertex[k+ind]*normales[k+ind]);
                    }

                    // point d'intersection de la droite portée par vect-dir et du plan de normale vect_norm
                    CoordVector intersec = intersection(point, vect_dir, vect_norm, cons);
                    //intersec.debug();


                    // matrice prenant en colonne les coordonnées des trois vertex de la face
                    std::vector<float> mat_coordFace;
                    for (int l=0; l<9; l++)
                    {
                        mat_coordFace.push_back(vertex[k+l]);
                    }

                    // si le point d'intersection appartient à la face
                    if (appartient_face(intersec, mat_coordFace))
                    {
                        // s'il s'agit du plus petit rayon et qu'il est dans le sens du vecteur directeur
                        if(norme(vecteur(point,intersec))<longueur_ray && produitScalaire(vect_dir,vecteur(point,intersec))>0)
                        {
                            // on garde la longueur min
                            longueur_ray = norme(vecteur(point,intersec));

                            // on remplace le bout du rayon par le point d'intersection
                            m_ray[i+j+m_Nray] = intersec.x;
                            m_ray[i+j+m_Nray+1] = intersec.y;
                            m_ray[i+j+m_Nray+2] = intersec.z;

                            // étage suivant : ecriture du prochain point pour création d'un nouveau vecteur directeur
                            m_ray[i+j+2*m_Nray] = vecteur_reflechi(vect_dir,vect_norm).x+intersec.x;
                            m_ray[i+j+2*m_Nray+1] = vecteur_reflechi(vect_dir,vect_norm).y+intersec.y;
                            m_ray[i+j+2*m_Nray+2] = vecteur_reflechi(vect_dir,vect_norm).z+intersec.z;

                        }
                    }
                }
            }
        }
    }
}


bool Ray::rebondSansMemoire(MeshObj mesh, float seuil)
{
    std::vector<float> normales(mesh.getNormals());
    std::vector<float> vertex(mesh.getVertex());
    std::vector<float> indiceMat(mesh.getIndMat());

    stockage();

    bool rayonsExistent = false;
    int face(0);

        for(int j=0; j<m_Nray;j=j+3) // pour chaque rayon
        {
            //SI LE RAYON N'EST PAS MORT
            if (m_rayVivant[j/3])
            {
                // recuperation d'un point et du vecteur directeur
                CoordVector point(m_ray[j], m_ray[j+1] , m_ray[j+2]);
                CoordVector vect_dir(m_ray[j+m_Nray]-m_ray[j],m_ray[j+m_Nray+1]-m_ray[j+1] ,m_ray[j+m_Nray+2]-m_ray[j+2]);

                // Stockage de la longeur du nouveau rayon
                float longueur_ray(1000000);

                for (int k=0; k < vertex.size(); k = k+9) // pour chaque face
                {
                    // Recuperation du vecteur normale
                    CoordVector vect_norm(normales[k], normales[k+1], normales[k+2]);

                    if (produitScalaire(vect_dir,vect_norm)<0) // test non-parralelisme des vecteurs et sens opposé
                    {
                        //calcul de la constante de l'équation du plan
                        float cons(0);
                        for (int ind=0;ind<3; ind++)
                        {
                            cons = cons - (vertex[k+ind]*normales[k+ind]);
                        }

                        // point d'intersection de la droite portée par vect-dir et du plan de normale vect_norm
                        CoordVector intersec = intersection(point, vect_dir, vect_norm, cons);
                        //intersec.debug();


                        // matrice prenant en colonne les coordonnées des trois vertex de la face
                        std::vector<float> mat_coordFace;
                        for (int l=0; l<9; l++)
                        {
                            mat_coordFace.push_back(vertex[k+l]);
                        }

                        // si le point d'intersection appartient à la face
                        if (appartient_face(intersec, mat_coordFace))
                        {
                            // s'il s'agit du plus petit rayon et qu'il est dans le sens du vecteur directeur
                            if(norme(vecteur(point,intersec))<longueur_ray && produitScalaire(vect_dir,vecteur(point,intersec))>0)
                            {
                                // on garde la longueur min
                                longueur_ray = norme(vecteur(point,intersec));
                                // On sauvegarde la longueur du rayon actuel
                                m_long[j/3] = longueur_ray;

                                // on remplace le bout du rayon par le point d'intersection
                                m_ray[j] = intersec.x;
                                m_ray[j+1] = intersec.y;
                                m_ray[j+2] = intersec.z;

                                // étage suivant : ecriture du prochain point pour création d'un nouveau vecteur directeur
                                m_ray[j+m_Nray] = vecteur_reflechi(vect_dir,vect_norm).x + intersec.x;
                                m_ray[j+m_Nray+1] = vecteur_reflechi(vect_dir,vect_norm).y + intersec.y;
                                m_ray[j+m_Nray+2] = vecteur_reflechi(vect_dir,vect_norm).z + intersec.z;

                                //on sauvegarde la dernière face testée
                                face = k;
                            }
                        }
                    }
                }

                // POUR CHAQUE NOUVEAU RAYON

                // On ajoute à la longueur du nouveau rayon à la longueur totale
                m_dist[j/3] = m_dist[j/3] + longueur_ray;

                //On met à jour l'énergie du nouveau rayon pour chaque bande
                int compteur(0);
                for (int l=0; l<8; l++)
                {
                    m_nrg[j/3*8 + l] = m_nrg[j/3*8+l] * (1-indiceMat[face+l+1]) * exp(-absorptionAir(20)[l]*longueur_ray);

                    // test si le rayon est mort
                    if (m_nrg[j/3*8 + l] > seuil) // s'il existe au moins un rayon vivant
                    {
                        rayonsExistent = true;
                    }
                    else
                    {
                        compteur++;
                    }
                }
                // si l'énergie sur les 8 bandes est inférieure au seuil le rayons est déclaré mort
                if (compteur == 8)
                {
                    m_rayVivant[j/3] = false;
                    //comptage des rayons morts pour la progress bar
                    m_nbRayMort++;
                }
                compteur = 0;
            }
    }

    return rayonsExistent;
}

