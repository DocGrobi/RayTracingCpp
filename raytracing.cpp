#include "raytracing.h"
#include "fonction.h"
#include "QDebug"
#include "QVector"
#include <QMessageBox>
#include <math.h>
#include <QElapsedTimer>
//#include <QtOpenGL>

// fonction de Möller–Trumbore
float triangle_intersection(const CoordVector &orig, const CoordVector &dir,
                            const CoordVector &v0,const CoordVector &v1,const CoordVector &v2)
{

    CoordVector e1 = vecteur(v0,v1);
    CoordVector e2 = vecteur(v0,v2);
    // calculer les vecteur normaux aux plans
    CoordVector pvec = produitVectoriel(dir,e2);
    float det = produitScalaire(e1, pvec);

    // Si le rayon est parralelle au plan
    if (det <= 0.000001) // det proche de 0 dans la version originale
    {
        return 0;
    }
    CoordVector tvec = vecteur(v0, orig);
    float u = produitScalaire(tvec, pvec)/det;
    if (u < 0 || u > 1)
    {
            return 0;
    }
    CoordVector qvec = produitVectoriel(tvec,e1);
    float v = produitScalaire(dir,qvec)/det;

    if (v < 0 || u + v > 1)
    {
            return 0;
    }
    return produitScalaire(e2,qvec)/det; // distance entre le point d'origine et le point d'intersection

}



// Méthodes

bool appartient_face(const CoordVector &point, std::vector<float>& face)
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

bool appartient_face(std::vector<float>& point, int iP, std::vector<float>& face, int iF)
{
    bool resultat = false;


    float hypoA(0), hypoB(0), hypoC(0);
    hypoA = produitScalaire(produitVectoriel(vecteur(face, iF, face, iF+3),vecteur(face, iF,point, iP)),produitVectoriel(vecteur(face, iF,point, iP),vecteur(face, iF,face, iF+6)));
    hypoB = produitScalaire(produitVectoriel(vecteur(face, iF+3,face, iF),vecteur(face, iF+3,point, iP)),produitVectoriel(vecteur(face, iF+3,point, iP),vecteur(face, iF+3,face, iF+6)));
    hypoC = produitScalaire(produitVectoriel(vecteur(face, iF+6,face, iF),vecteur(face, iF+6,point, iP)),produitVectoriel(vecteur(face, iF+6,point, iP),vecteur(face, iF+6,face, iF+3)));

    if(hypoA>=0 && hypoB>=0 && hypoC >=0) // si =0 on est sur l'arrete
    {
        resultat = true;
    }

   return resultat;
}

bool appartient_face(const CoordVector &point, const CoordVector &a, const CoordVector &b, const CoordVector &c)
{
    //bool resultat = false;

     float hypoA(0), hypoB(0), hypoC(0);
     hypoA = produitScalaire(produitVectoriel(vecteur(a,b),vecteur(a,point)),produitVectoriel(vecteur(a,point),vecteur(a,c)));
     if (hypoA>=0)
     {
         hypoB = produitScalaire(produitVectoriel(vecteur(b,a),vecteur(b,point)),produitVectoriel(vecteur(b,point),vecteur(b,c)));
         if (hypoB >=0)
         {
             hypoC = produitScalaire(produitVectoriel(vecteur(c,a),vecteur(c,point)),produitVectoriel(vecteur(c,point),vecteur(c,b)));
             if (hypoC>=0)
             {
                 return true;
             }
             else
             {
                 return false;
             }
         }
         else
         {
             return false;
         }
     }
     else
     {
         return false;
     }
}


// intersection d'une droite comprenant un point et un vecteur directeur avec un plan comprenant un vecteur normal
CoordVector intersection(const CoordVector &point_ray, const CoordVector &vect_dir, const CoordVector &vect_norm, float k)
{
    CoordVector resultat;
    float t = -(produitScalaire(point_ray,vect_norm)+k)/produitScalaire(vect_dir,vect_norm);

    resultat.x = vect_dir.x*t + point_ray.x;
    resultat.y = vect_dir.y*t + point_ray.y;
    resultat.z = vect_dir.z*t + point_ray.z;

    return resultat;
}

std::vector<float> intersection(std::vector<float>& point_ray, std::vector<float>& vect_dir,int indRay, std::vector<float>& vect_norm, int indFace, float k)
{
    //CoordVector resultat;
    std::vector<float> resultat;
    float t = -(produitScalaire(point_ray,indRay,vect_norm,indFace)+k)/produitScalaire(vect_dir,indRay,vect_norm,indFace);

    /*
    resultat.x = vect_dir[indRay]*t + point_ray[indRay];
    resultat.y = vect_dir[indRay+1]*t + point_ray[indRay+1];
    resultat.z = vect_dir[indRay+2]*t + point_ray[indRay+2];
    */

    for (int i = 0 ; i < 3 ; i ++)
    {
        resultat.push_back(vect_dir[indRay+i]*t + point_ray[indRay+i]);
    }

    return resultat;
}

// vecteur reflechi par rapport au rayon incident et au vecteur normale de la face
CoordVector vecteur_reflechi(const CoordVector &i, const CoordVector &n)
{
    CoordVector resultat;
    resultat.x = -2*produitScalaire(i,n)*n.x + i.x;
    resultat.y = -2*produitScalaire(i,n)*n.y + i.y;
    resultat.z = -2*produitScalaire(i,n)*n.z + i.z;

    return resultat;
}

std::vector<float> vecteur_reflechi(std::vector<float>& i, int ii,std::vector<float>& n, int in)
{
    std::vector<float> resultat;
    for (int j = 0 ; j<3 ; j++)
    {
        resultat.push_back(-2*produitScalaire(i,ii,n,in)*n[in+j] + i[ii+j]);
    }

    return resultat;
}


// Les classes

// Constructeur
Ray::Ray(int Nray, Source S, bool fibonacci)
{    
    m_src = S.centre();
    float theta(0), phi(0), nor(0), N(Nray-1);
    CoordVector coord;

    if (fibonacci) // création des rayons sur une grille uniforme de fibonacci
    {
        float OR = (1+sqrt(5))/2;
        // étage 0 : source
        for (float i=0; i<Nray; i++)
        {
            // creation des points initiaux
            m_ray.push_back(m_src.x);
            m_ray.push_back(m_src.y);
            m_ray.push_back(m_src.z);

            //m_test.push_back((m_src));

            // creation des vecteurs directeurs normalisés
            theta = fmod((i*2*M_PI/OR) , (2*M_PI));
            phi = asin(-1 + 2*i/(N));

            coord = sph2cart(1,theta,phi);
            nor = norme(coord);
            m_vDir.push_back(coord.x/nor);
            m_vDir.push_back(coord.y/nor);
            m_vDir.push_back(coord.z/nor);
        }


/*
        for (float i=0; i<Nray; i++)
        {
            float theta = fmod((i*2*M_PI/OR) , (2*M_PI));
            float phi = asin(-1 + 2*i/(Nray-1));

            CoordVector coord(sph2cart(1,theta,phi));

            m_ray.push_back(coord.x + m_src.x);
            m_ray.push_back(coord.y + m_src.y);
            m_ray.push_back(coord.z + m_src.z);

            m_vDir.push_back(coord.x);
            m_vDir.push_back(coord.y);
            m_vDir.push_back(coord.z);
        }
*/
            m_Nray = 3*Nray; // nombre de rayon * 3 coordonnées

    }
    else // utilisation des vertex de la source blender
    {

        m_Nray = S.vert().size();

        if (S.vert().size() == 0) // si pas de source chargée
        {
            QMessageBox::critical(NULL,"Erreur","Aucune source sélectionnée \nVeuillez charger une nouvelle source");
        }
        else
        {
            // étage 0 : source
            for (int i=0; i<m_Nray; i=i+3)
            {
            // creation du premier point
                m_ray.push_back(m_src.x);
                m_ray.push_back(m_src.y);
                m_ray.push_back(m_src.z);
            //  creation du vecteur directeur normalisé

                coord.x = S.vert()[i]-m_src.x;
                coord.y = S.vert()[i+1]-m_src.y;
                coord.z = S.vert()[i+2]-m_src.z;
                nor = norme(coord);
                m_vDir.push_back(coord.x/nor);
                m_vDir.push_back(coord.y/nor);
                m_vDir.push_back(coord.z/nor);
            }
            /*
            // étage 1 : coordonnées des vertex
            for (int i=0; i<m_Nray; i++)
            {
                m_ray.push_back(S.vert()[i]);
            }
            */
        }

    }

    // initialisation des attributs
    m_dMax = 0;
    m_dist.resize(m_Nray/3, 0); // longueur totale de chaque rayon
    m_long.resize(m_Nray/3, 0); // longueur du dernier segment de rayon
    m_nrg.resize(m_Nray/3*8, 1); // 8 bandes de fréquence par rayons

    m_pos.resize(m_Nray, 0);
    m_dir.resize(m_Nray, 0);
    m_nbRayMort = 0;
    m_rayVivant.resize(m_Nray/3, true); // Tous les rayons sont vivant

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
    /*
    for(int i = 0; i<m_Nray ; i++)
    {
       m_pos[i] = m_ray[i];
       m_dir[i] = m_ray[i+m_Nray]-m_ray[i];
    }
    */
    m_pos = m_ray;
    m_dir = m_vDir;

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


// rayons puis faces
bool Ray::rebondSansMemoire(MeshObj mesh, float seuil)
{
    // chargement du mesh
    std::vector<float> normales(mesh.getNormals());
    std::vector<float> vertex(mesh.getVertex());
    std::vector<float> indiceMat(mesh.getIndMat());

    // Declarations
    std::vector<float> absAir = absorptionAir(20);
    CoordVector point, vect_dir, vect_ref,vect_norm;
    CoordVector A,B,C; // trois points
    float longueur_inst(0),nor(0);
    int compteur(0),face(0);
    bool rayonsExistent = false;
    int j(0), k(0), l(0);
    int nbVertex = vertex.size();
    //int ind(0);
    // float cons(0);
    //float longueur_ray(1000000);
    //CoordVector intersec;

//    QElapsedTimer timer, timer2, timer3, timer4;
//    double tempsRay(0), tempsNrg(0), tempsAvIf(0), tempsApIf(0), tps2(0), tps3(0), tps4(0);

//    timer.start();
//    timer2.start();
//    timer3.start();
//    timer4.start();
    stockage();

//    qDebug() << "stockage" << timer.nsecsElapsed() << "ns";
//    timer.restart();


        for(j=0; j<m_Nray/3;j++) // pour chaque rayon
        {
            //SI LE RAYON N'EST PAS MORT
            if (m_rayVivant[j])
            {
                // recuperation d'un point et du vecteur directeur
                point.x = m_ray[3*j];
                point.y = m_ray[3*j+1];
                point.z = m_ray[3*j+2];

                //timer.restart();

                vect_dir.x = m_vDir[3*j];
                vect_dir.y = m_vDir[3*j+1];
                vect_dir.z = m_vDir[3*j+2];

                //tpsCoo = tpsCoo + timer.nsecsElapsed();
                //timer.restart();

                // Stockage de la longeur du nouveau rayon
                m_long[j] = 1000000;

                for (k=0; k < nbVertex; k = k+9) // pour chaque face
                {

                    // ALTERNATIVE

                    A.x = vertex[k];
                    A.y = vertex[k+1];
                    A.z = vertex[k+2];
                    B.x = vertex[k+3];
                    B.y = vertex[k+4];
                    B.z = vertex[k+5];
                    C.x = vertex[k+6];
                    C.y = vertex[k+7];
                    C.z = vertex[k+8];

                    // longueur du rayon depuis point de depart dans le sens du vecteur directeur et intersectant avec la face ABC
                    longueur_inst = triangle_intersection(point,vect_dir,A,B,C);
                    //longueur_inst = triangle_intersection(m_test[j],vect_dir,A,B,C);
                    if (longueur_inst > 0 && longueur_inst < m_long[j]) // Rayon dans le sens du vecteur directeur et le plus petit trouvé
                    {
                        // On sauvegrade la plus petite longueur
                        m_long[j] = longueur_inst;
                        //on sauvegarde la dernière face testée
                        face = k;
                    }


                    /*
                    // ANCIENNE VERSION
                    vect_norm.x = normales[k];
                    vect_norm.y = normales[k+1];
                    vect_norm.z = normales[k+2];

                    if (produitScalaire(vect_dir,vect_norm)<0) // test non-parralelisme des vecteurs et sens opposé
                    //if (produitScalaire(m_vDir,j,normales,k)<0)
                    {
                        //calcul de la constante de l'équation du plan
                        cons = 0;
                        for (ind=0;ind<3; ind++)
                        {
                            cons = cons - (vertex[k+ind]*normales[k+ind]);
                            // matrice de face
                        }

                        // point d'intersection de la droite portée par vect-dir et du plan de normale vect_norm
                        intersec = intersection(point, vect_dir, vect_norm, cons);
                        //intersec = intersection(m_ray, m_vDir, j, normales, k, cons);

                        // les trois points de la face k
                        A.x = vertex[k];
                        A.y = vertex[k+1];
                        A.z = vertex[k+2];
                        B.x = vertex[k+3];
                        B.y = vertex[k+4];
                        B.z = vertex[k+5];
                        C.x = vertex[k+6];
                        C.y = vertex[k+7];
                        C.z = vertex[k+8];

//                        tps2 = tps2 + timer2.nsecsElapsed();
//                        timer3.restart();

                        // si le point d'intersection appartient à la face
                        //if (appartient_face(intersec, mat_coordFace))
                        if (appartient_face(intersec, A,B,C))
                        //if (appartient_face(intersec, 0, vertex, k))
                        {
                            // s'il s'agit du plus petit rayon et qu'il est dans le sens du vecteur directeur
                            //longueur_inst = norme(vecteur(m_ray, j,intersec, 0));
                            longueur_inst = norme(vecteur(point,intersec));

//                            tps3 = tps3 + timer3.nsecsElapsed();
//                            timer4.restart();

                            if(longueur_inst<m_long[j] )
                            //if(longueur_inst<longueur_ray && produitScalaire(m_vDir,j,vecteur(m_ray, j,intersec, 0))>0)
                            {
                                if (produitScalaire(vect_dir,vecteur(point,intersec))>0)
                                {
                                    // on garde la longueur min
                                    //longueur_ray = norme(vecteur(point,intersec));
                                    m_long[j] = longueur_inst;
                                    // On sauvegarde la longueur du rayon actuel
                                    //m_long[j] = longueur_ray;

                                    // on remplace le bout du rayon par le point d'intersection
                                    m_ray[3*j] = intersec.x;
                                    m_ray[3*j+1] = intersec.y;
                                    m_ray[3*j+2] = intersec.z;

                                    // étage suivant : ecriture du prochain point pour création d'un nouveau vecteur directeur
                                    vect_ref = vecteur_reflechi(vect_dir,vect_norm);

                                    m_ray[3*j+m_Nray] = vect_ref.x + intersec.x;
                                    m_ray[3*j+m_Nray+1] = vect_ref.y + intersec.y;
                                    m_ray[3*j+m_Nray+2] = vect_ref.z + intersec.z;

                                    m_vDir[3*j] = vect_ref.x;
                                    m_vDir[3*j+1] = vect_ref.y;
                                    m_vDir[3*j+2] = vect_ref.z;

                                    //on sauvegarde la dernière face testée
                                    face = k;
                                }
                            }
                        }
                    }
                    */
                }

                // POUR CHAQUE NOUVEAU RAYON

                // on remplace le bout du rayon par le point d'intersection
                m_ray[3*j] = point.x + vect_dir.x*m_long[j];
                m_ray[3*j+1] = point.y + vect_dir.y*m_long[j];
                m_ray[3*j+2] = point.z + vect_dir.z*m_long[j];

//                m_test[j].x = m_test[j].x + vect_dir.x*m_long[j];
//                m_test[j].y = m_test[j].y + vect_dir.y*m_long[j];
//                m_test[j].z = m_test[j].z + vect_dir.z*m_long[j];

                // étage suivant : ecriture du prochain point pour création d'un nouveau vecteur directeur
                vect_norm.x = normales[face];
                vect_norm.y = normales[face+1];
                vect_norm.z = normales[face+2];

                vect_ref = vecteur_reflechi(vect_dir,vect_norm);
                nor = norme(vect_ref);
                m_vDir[3*j] = vect_ref.x/nor;
                m_vDir[3*j+1] = vect_ref.y/nor;
                m_vDir[3*j+2] = vect_ref.z/nor;


                // On ajoute à la longueur du nouveau rayon à la longueur totale
                m_dist[j] = m_dist[j] + m_long[j];

                //On met à jour l'énergie du nouveau rayon pour chaque bande
                compteur = 0;
                for (l=0; l<8; l++)
                {
                    m_nrg[j*8 + l] = m_nrg[j*8+l] * (1-indiceMat[face+l+1]) * exp(-absAir[l]*m_long[j]);
                    //m_nrg[j/3*8 + l] = m_nrg[j/3*8+l] * (1-indiceMat[face+l+1]);
                    // test si le rayon est mort
                    if (m_nrg[j*8 + l] > seuil) // s'il existe au moins un rayon vivant
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
                    m_rayVivant[j] = false;
                    //comptage des rayons morts pour la progress bar
                    m_nbRayMort++;
                }
                compteur = 0;

                //qDebug() << "gestion energie : " << timer.nsecsElapsed() << "ns";
//                tempsNrg = tempsNrg + timer.nsecsElapsed();
//                timer.restart();
            }
    }
        /*
        qDebug() << "gestion rayons avant if : " << tempsAvIf/1000000 << "ms";
        qDebug() << "gestion rayons apres if : " << tempsApIf/1000000 << "ms";
        qDebug() << "gestion energie : " << tempsNrg/1000000 << "ms";
        qDebug() << "produit scalire : " << tps2/1000000 << "ms";
        qDebug() << "appartient face : " << tps3/1000000 << "ms";
        qDebug() << "bon rayon : " << tps4/1000000 << "ms";
        */
        //qDebug() << "produit scalaire avec std vector : " << tpsSTD/1000000 << "ms";
        //qDebug() << "produit scalaire avec  Coordvector : " << tpsCoo/1000000 << "ms";

    return rayonsExistent;
}



// faces puis rayons
bool Ray::rebondSansMemoireBis(MeshObj mesh, float seuil)
{
    // chargement du mesh
    std::vector<float> normales(mesh.getNormals());
    std::vector<float> vertex(mesh.getVertex());
    std::vector<float> indiceMat(mesh.getIndMat());

    // Declarations
    std::vector<float> absAir = absorptionAir(20);
    CoordVector point, vect_dir, vect_ref, vect_norm;
    //CoordVector  intersec;
    //CoordVector A,B,C; // trois points
    //float longueur_inst(0), nor(0);
    //float cons(0);
    int compteur(0);
    bool rayonsExistent = false;
    int j(0), k(0), ind(0), l(0);
    int nbVertex = vertex.size();
    std::vector<int> face;
    face.resize(m_Nray/3, 0);
    m_long.assign(m_Nray/3, 1000000);
    CoordVector A,B,C, e1, e2, pvec, tvec, qvec; // trois points
    float longueur_inst(0),nor(0), det, u, v;

    stockage();

    for (k=0; k < nbVertex; k = k+9) // pour chaque face
    {
        vect_norm.x = normales[k];
        vect_norm.y = normales[k+1];
        vect_norm.z = normales[k+2];
        // les trois points de la face k
        A.x = vertex[k];
        A.y = vertex[k+1];
        A.z = vertex[k+2];
        B.x = vertex[k+3];
        B.y = vertex[k+4];
        B.z = vertex[k+5];
        C.x = vertex[k+6];
        C.y = vertex[k+7];
        C.z = vertex[k+8];

        e1 = vecteur(A,B);
        e2 = vecteur(A,C);

        for(j=0; j<m_Nray/3;j++) // pour chaque rayon
            {
                //SI LE RAYON N'EST PAS MORT
                if (m_rayVivant[j])
                {
                    // recuperation d'un point et du vecteur directeur
                    point.x = m_pos[3*j];
                    point.y = m_pos[3*j+1];
                    point.z = m_pos[3*j+2];

                    vect_dir.x = m_dir[3*j];
                    vect_dir.y = m_dir[3*j+1];
                    vect_dir.z = m_dir[3*j+2];

                    pvec = produitVectoriel(vect_dir,e2);
                    det = produitScalaire(e1,pvec);
                    if(det>0.000001)
                    {
                        tvec = vecteur(A, point);
                        u = produitScalaire(tvec, pvec)/det;
                        if(u>=0 && u <=1)
                        {
                            qvec = produitScalaire(tvec,e1);
                            v = produitScalaire(vect_dir,qvec)/det;
                            if(v >=0 && u+v <=1)
                            {
                                longueur_inst = produitScalaire(e2, qvec)/det;
                            }
                        }
                    }

                    //longueur_inst = triangle_intersection(point,vect_dir,A,B,C);
                    if (longueur_inst >0 && longueur_inst<m_long[j])
                    {
                        m_long[j] = longueur_inst;
/*
                        // on remplace le bout du rayon par le point d'intersection
                        m_ray[3*j] = point.x + vect_dir.x*longueur_inst;
                        m_ray[3*j+1] = point.y + vect_dir.y*longueur_inst;
                        m_ray[3*j+2] = point.z + vect_dir.z*longueur_inst;
*/
                        // étage suivant : ecriture du prochain point pour création d'un nouveau vecteur directeur
                        vect_ref = vecteur_reflechi(vect_dir,vect_norm);
                        nor = norme(vect_ref);
                        m_vDir[3*j] = vect_ref.x/nor;
                        m_vDir[3*j+1] = vect_ref.y/nor;
                        m_vDir[3*j+2] = vect_ref.z/nor;

                        //on sauvegarde la dernière face testée
                        face[j] = k;
                    }
                }
            }
    }

    // POUR CHAQUE RAYON
    for (j = 0; j<m_Nray/3; j++)
    {
        //SI LE RAYON N'EST PAS MORT
        if (m_rayVivant[j])
        {
            for (l=0; l<3; l++)
            {
                m_ray[3*j+l] = m_pos[3*j+l] + m_dir[3*j+l]*m_long[j];
            }

            // On ajoute à la longueur des nouveaux rayons à la longueur totale
            m_dist[j] = m_dist[j] + m_long[j];

            //On met à jour l'énergie des rayons pour chaque bande
            compteur = 0;
            for (l=0; l<8; l++)
            {
                m_nrg[j*8 + l] = m_nrg[j*8+l] * (1-indiceMat[face[j]+l+1]) * exp(-absAir[l]*m_long[j]);
                //m_nrg[j/3*8 + l] = m_nrg[j/3*8+l] * (1-indiceMat[face+l+1]);
                // test si le rayon est mort
                if (m_nrg[j*8 + l] > seuil) // s'il existe au moins un rayon vivant
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
                m_rayVivant[j] = false;
                //comptage des rayons morts pour la progress bar
                m_nbRayMort++;
            }
            compteur = 0;
        }
        else
        {
            m_long[j] = 0;
        }
    }


    return rayonsExistent;
}

