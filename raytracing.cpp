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
    if (det <= 0.000001) // si det négatif, le rayon va dans le mauvais sens
    //if (det < 1e-8 && det > -1e-8)
    {
        return 0;
    }
    float invDet = 1/det;
    CoordVector tvec = vecteur(v0, orig);
    float u = produitScalaire(tvec, pvec)*invDet;
    if (u < 0 || u > 1)
    {
            return 0;
    }
    CoordVector qvec = produitVectoriel(tvec,e1);
    float v = produitScalaire(dir,qvec)*invDet;

    if (v < 0 || u + v > 1)
    {
            return 0;
    }
    return produitScalaire(e2,qvec)*invDet; // distance entre le point d'origine et le point d'intersection

}

float triangle_intersection(const Vect3f &orig, const Vect3f &dir,
                            const Vect3f &v0,const Vect3f &v1,const Vect3f &v2)
{

    Vect3f e1 = v1-v0;
    Vect3f e2 = v2-v0;
    // calculer les vecteur normaux aux plans
    Vect3f pvec = dir^e2;
    float det = e1*pvec;

    // Si le rayon est parralelle au plan
    if (det <= 0.000001) // det proche de 0 dans la version originale
    {
        return 0;
    }
    Vect3f tvec = orig-v0;
    float u = tvec*pvec/det;
    if (u < 0 || u > 1)
    {
            return 0;
    }
    Vect3f qvec = tvec^e1;
    float v = dir*qvec/det;

    if (v < 0 || u + v > 1)
    {
            return 0;
    }
    return e2*qvec/det; // distance entre le point d'origine et le point d'intersection

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
             if (hypoC>=0) return true;
             else return false;
         }
         else return false;
     }
     else return false;
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

    for (int i = 0 ; i < 3 ; i ++)
    {
        resultat.push_back(vect_dir[indRay+i]*t + point_ray[indRay+i]);
    }

    return resultat;
}

// vecteur reflechi par rapport au rayon incident et au vecteur normale de la face
//https://www.ilemaths.net/sujet-calcul-d-un-vecteur-reflechi-dans-espace-3d-32188.html
CoordVector vecteur_reflechi(const CoordVector &i, const CoordVector &n)
{
    float p= -2*(produitScalaire(i,n));
    return CoordVector(p*n.x + i.x, p*n.y + i.y, p*n.z + i.z);
}

CoordVector vecteur_reflechi(const std::vector<float> &i, int ind, const CoordVector &n)
{
    float p= 2*fabs(produitScalaire(i,ind,n));
    return CoordVector(p*n.x + i[ind], p*n.y + i[ind+1], p*n.z + i[ind+2]);
}

Vect3f vecteur_reflechi(const Vect3f &i, const Vect3f &n)
{
    float p = i*n;

    return Vect3f(-2*p*n +i);
}

std::vector<float> &vecteur_reflechi(std::vector<float>& i, int ii,std::vector<float>& n, int in)
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
Ray::Ray(int Nray, Source S, int nSrc, bool fibonacci)
{    
    m_src = S.getCentre(nSrc);
    float theta(0), phi(0), nor(0), N(Nray-1);
    CoordVector coord;

    if (fibonacci) // création des rayons sur une grille uniforme de fibonacci
    {
        float OR = (1+sqrt(5))/2;
        // étage 0 : source
        for (float i=0; i<Nray; i++)
        {
            // creation des points initiaux
            m_ray.push_back((m_src));

            // creation des vecteurs directeurs normalisés
            theta = fmod((i*2*M_PI/OR) , (2*M_PI));
            phi = asin(-1 + 2*i/(N));

            coord = sph2cart(1,theta,phi);
            nor = norme(coord);
            m_vDir.push_back(coord/nor);
        }
            m_Nray = Nray; // nombre de rayon

    }
    else // utilisation des vertex de la source blender
    {
        std::vector<float> srcVert = S.getVert();
        m_Nray = srcVert.size()/(3*S.getNbSource());

        if (S.getVert().empty()) // si pas de source chargée
        {
            QMessageBox::critical(NULL,"Erreur","Aucune source sélectionnée \nVeuillez charger une nouvelle source");
        }
        else
        {

            // étage 0 : centre source
            for (int i=m_Nray*nSrc; i<m_Nray*(nSrc+1); i++)
            {
            // creation du premier point
                m_ray.push_back(m_src);
            //  creation du vecteur directeur normalisé
                coord.x = srcVert[3*i]-m_src.x;
                coord.y = srcVert[3*i+1]-m_src.y;
                coord.z = srcVert[3*i+2]-m_src.z;
                nor = norme(coord);
                m_vDir.push_back(coord/nor);
            }
        }
    }

    // initialisation des attributs
    m_dMax = 0;
    m_dist.resize(m_Nray, 0); // longueur totale de chaque rayon
    m_long.resize(m_Nray, 0); // longueur du dernier segment de rayon
    m_nrg.resize(m_Nray*8, 1); // 8 bandes de fréquence par rayons

    m_pos.resize(m_Nray, 0);
    m_dir.resize(m_Nray, 0);
    m_nbRayMort = 0;
    m_rayVivant.resize(m_Nray, true); // Tous les rayons sont vivant

}

// Destructeur
Ray::~Ray()
{
}

// Accesseur
std::vector<CoordVector> &Ray::getRay(){
    return m_ray;
}

std::vector<float> &Ray::getNRG(){
    return m_nrg;
}

std::vector<float> &Ray::getNRGbackup(){
    return m_nrgBackup;
}

std::vector<CoordVector> &Ray::getPos(){
    return m_pos;
}

std::vector<CoordVector> &Ray::getDir(){
    return m_dir;
}

std::vector<CoordVector> &Ray::getvDir(){
    return m_vDir;
}

int Ray::getNbRay() const{
    return m_Nray;
}

std::vector<float> &Ray::getDist(){
    return m_dist;
}

std::vector<float> &Ray::getLong(){
    return m_long;
}

std::vector<bool> &Ray::getRayVivant(){
    return m_rayVivant;
}

int Ray::getRayMorts() const{
    return m_nbRayMort;
}

void Ray::stockage(){
    m_pos = m_ray;
    m_dir = m_vDir;
    m_nrgBackup = m_nrg;
}


// rayons puis faces
bool Ray::rebondSansMemoire(MeshObj mesh, float seuil)
{
    // chargement du mesh
    //std::vector<float> normales(mesh.getNormals());
    //std::vector<float> vertex(mesh.getVertex());
    std::vector<float> indiceMat(mesh.getIndMat());

    std::vector<CoordVector> normales(mesh.getNorm());
    std::vector<CoordVector> vertex(mesh.getVert());

    // Declarations
    std::vector<float> absAir = absorptionAir(20);
    CoordVector point, vect_dir, vect_ref,vect_norm;
    CoordVector A,B,C; // trois points
    float longueur_inst(0),nor(0);
    int compteur(0),face(0);
    bool rayonsExistent = false;
    int j(0), k(0), l(0);
   // int nbVertex = vertex.size();

    stockage();

        for(j=0; j<m_Nray;j++) // pour chaque rayon
        {
            //SI LE RAYON N'EST PAS MORT
            if (m_rayVivant[j])
            {
                // recuperation d'un point et du vecteur directeur
                point = m_ray[j];
                vect_dir = m_vDir[j];

                // Stockage de la longeur du nouveau rayon
                m_long[j] = 1000000;

                for (k=0; k < vertex.size(); k+=3) // pour chaque face
                {
                    /*
                    A.x = vertex[k];
                    A.y = vertex[k+1];
                    A.z = vertex[k+2];
                    B.x = vertex[k+3];
                    B.y = vertex[k+4];
                    B.z = vertex[k+5];
                    C.x = vertex[k+6];
                    C.y = vertex[k+7];
                    C.z = vertex[k+8];
                    */

                    A=vertex[k];
                    B=vertex[k+1];
                    C=vertex[k+2];

                    // longueur du rayon depuis point de depart dans le sens du vecteur directeur et intersectant avec la face ABC
                    longueur_inst = triangle_intersection(point,vect_dir,A,B,C);


                    if (longueur_inst > 0 && longueur_inst < m_long[j]) // Rayon dans le sens du vecteur directeur et le plus petit trouvé
                    {
                        m_long[j] = longueur_inst; // On sauvegrade la plus petite longueur
                        face = k;                  //on sauvegarde la dernière face testée
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

                // Sécurité si problème d'intersection
                if(m_long[j] == 1000000)
                {
                    QMessageBox::critical(NULL,"Erreur","Le " + QString::number(j) + "ème rayon n'a pas atteind de face", "Arreter le programme");
                    return false;
                }

                // Mise à jour du point d'origine
                m_ray[j]+= m_vDir[j]*m_long[j];

                // Mise à jour du vecteur directeur
                /*
                vect_norm.x = normales[face];
                vect_norm.y = normales[face+1];
                vect_norm.z = normales[face+2];
                vect_ref = vecteur_reflechi(vect_dir,vect_norm);
                */

                vect_ref = vecteur_reflechi(vect_dir,normales[face]);
                nor = norme(vect_ref);
                m_vDir[j] = vect_ref/nor;

                // On ajoute la longueur du nouveau rayon à la longueur totale
                m_dist[j] += m_long[j];

                //On met à jour l'énergie du nouveau rayon pour chaque bande
                compteur = 0;
                for (l=0; l<8; l++)
                {
                    m_nrg[j*8 + l] = m_nrg[j*8+l] * (1-indiceMat[3*face+l+1]) * exp(-absAir[l]*m_long[j]);
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
    }

    return rayonsExistent;
}


// faces puis rayons + octree
bool Ray::rebondSansMemoire(MeshObj &mesh, float seuil, Octree &oct)
{
    // chargement du mesh
    //std::vector<float> normales(mesh.getNormals());
    //std::vector<float> vertex(mesh.getVertex());
    std::vector<float> indiceMat(mesh.getIndMat());
    std::vector<Boite> vectBoite(oct.getVectBoite());

    std::vector<CoordVector> vert(mesh.getVert());
    std::vector<CoordVector> norm(mesh.getNorm());
    std::vector<CoordVector> vectFace(mesh.getVectFace());

    // Declarations
    std::vector<float> absAir = absorptionAir(20);
    CoordVector vect_ref;
    int compteur(0);
    bool rayonsExistent = false;
    int j(0), k(0), indE(0), l(0), i(0), indR(0);
    //int nbVertex = vertex.size();
    std::vector<int> face;
    face.resize(m_Nray, 0);
    m_long.assign(m_Nray, 1000000);
    CoordVector A,B,C, e1, e2, pvec, tvec, qvec;
    float longueur_inst(0), det, u, v, invDet;

    stockage();

    for (i = 0 ; i< vectBoite.size() ; i ++) // pour chaque boite
    {
        if (vectBoite[i].estUneFeuille && !vectBoite[i].m_numRayon.empty()) // Si la boite est une feuille et comprend des rayons
        {
            for (indE=0; indE < vectBoite[i].m_numElt.size(); indE ++) // pour chaque face comprise dans la boite
            {
                //k = vectBoite[i].m_numElt[indE]/3;
                k = vectBoite[i].m_numElt[indE];

                /*
                e1 = vecteur(vert[k],vert[k+1]);
                e2 = vecteur(vert[k],vert[k+2]);
                */
                e1 = vectFace[2*k/3];
                e2 = vectFace[2*k/3+1];

                for(indR=0; indR<vectBoite[i].m_numRayon.size() ; indR++) // pour chaque rayon compris dans la boite
                //for (j = 0 ; j < m_Nray ; j++)
                {
                    j = vectBoite[i].m_numRayon[indR];

                    // fonction de Möller–Trumbore
                    //pvec = produitVectoriel(m_dir[j],vectFace[2*k/3+1]);
                    pvec = produitVectoriel(m_dir[j], e2);
                    det = produitScalaire(e1,pvec);
                    //det = produitScalaire(vectFace[2*k/3],pvec);
                    if (det > 1e-8) {
                        invDet = 1/det;
                        //tvec = vecteur(A, point);
                        tvec = vecteur(vert[k], m_pos[j]);
                        u = produitScalaire(tvec, pvec)*invDet;
                        if (u >= 0 && u <= 1) {
                            qvec = produitVectoriel(tvec,e1);
                            //qvec = produitVectoriel(tvec,vectFace[2*k/3]);
                            //v = produitScalaire(vect_dir,qvec)*invDet;
                            v = produitScalaire(m_dir[j], qvec)*invDet;
                            if (v >= 0 && u + v <= 1.00001) { // probleme d'arrondis
                                longueur_inst = produitScalaire(e2, qvec)*invDet;
                                //longueur_inst = produitScalaire(vectFace[2*k/3+1], qvec)*invDet;
                                if (longueur_inst >0 && longueur_inst<m_long[j]){
                                    m_long[j] = longueur_inst; // On sauvegarde la longueur
                                    face[j] = k;               //on sauvegarde la dernière face testée
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // POUR CHAQUE RAYON
    for (j = 0; j<m_Nray; j++)
    {
        //SI LE RAYON N'EST PAS MORT
        if (m_rayVivant[j])
        {
            // Sécurité si problème d'intersection
            if(m_long[j] == 1000000)
            {
                QMessageBox::critical(NULL,"Erreur","Le " + QString::number(j) + "ème rayon n'a pas atteind de face", "Arreter le programme");
                return false;
            }

            // Mise à jour vecteur directeur
            vect_ref  = vecteur_reflechi(m_dir[j], norm[face[j]]);
            m_vDir[j] = vect_ref/norme(vect_ref);

            // Mise à jour point d'origine
            m_ray[j]+= m_dir[j]*m_long[j];

            // On ajoute à la longueur des nouveaux rayons à la longueur totale
            m_dist[j] += m_long[j];

            //Mise à jour de l'énergie des rayons pour chaque bande
            compteur = 0;
            for (l=0; l<8; l++)
            {
                m_nrg[j*8 + l] = m_nrg[j*8+l] * (1-indiceMat[3*face[j]+l+1]) * exp(-absAir[l]*m_long[j]);
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
            m_long[j] = 0; // A vérifier pourquoi on fait ça ...
        }
    }
    return rayonsExistent;
}
