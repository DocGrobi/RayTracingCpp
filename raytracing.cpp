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


// Les classes

// Constructeur
Ray::Ray(int Nray, Source S, int nSrc, bool fibonacci)
{    
    m_src = S.getCentre(nSrc);
    double theta(0), phi(0), nor(0), N(Nray-1);
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
            theta = fmod((i*2*M_PI/OR) , (2*M_PI)); // on ajoute 0.1 pour eviter d'avoir un rayons sur l'axe 0
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
    m_rayVivantBackup.resize(m_Nray, true);
    qDebug() << "nb ray" << m_ray.size();

}

// Constructeur surchargé
Ray::Ray(const CoordVector &point, const std::vector<CoordVector>& dir){

    m_src = point;
    CoordVector vect;
    float nor;

    for (auto &a : dir)
    {
        if (!proche(m_src, a)) // on ne prend pas les sources confondues avec le listener
        {
            // creation des points initiaux
            m_ray.push_back(m_src);

            // creation des vecteurs directeurs normalisés
            vect = vecteur(m_src,a);
            nor = norme(vect);
            m_vDir.push_back(vect/nor);
        }
    }

    m_Nray = m_ray.size(); // nombre de rayon


    // initialisation des attributs
    m_dMax = 0;
    m_dist.resize(m_Nray, 0); // longueur totale de chaque rayon
    m_long.resize(m_Nray, 0); // longueur du dernier segment de rayon
    m_nrg.resize(m_Nray*8, 1); // 8 bandes de fréquence par rayons

    m_pos.resize(m_Nray, 0);
    m_dir.resize(m_Nray, 0);
    m_nbRayMort = 0;
    m_rayVivant.resize(m_Nray, true); // Tous les rayons sont vivant
m_rayVivantBackup = m_rayVivant;
}

Ray::Ray(const std::vector<CoordVector> &point, const std::vector<CoordVector>& dir){

    for (int i=0 ; i< point.size() ; i++)
    {
        // creation des points initiaux
        m_ray.push_back(point[i]);

        // creation des vecteurs directeurs normalisés
        m_vDir.push_back(dir[i]);
    }

    m_Nray = m_ray.size(); // nombre de rayon

    // initialisation des attributs
    m_dMax = 0;
    m_dist.resize(m_Nray, 0); // longueur totale de chaque rayon
    m_long.resize(m_Nray, 0); // longueur du dernier segment de rayon
    m_nrg.resize(m_Nray*8, 1); // 8 bandes de fréquence par rayons

    m_pos.resize(m_Nray, 0);
    m_dir.resize(m_Nray, 0);
    m_nbRayMort = 0;
    m_rayVivant.resize(m_Nray, true); // Tous les rayons sont vivant
m_rayVivantBackup = m_rayVivant;
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
    return m_rayVivantBackup;
}

std::vector<bool> &Ray::getRayVivant2(){
    return m_rayVivant;
}

int Ray::getRayMorts() const{
    return m_nbRayMort;
}

void Ray::killRay(int i){
    m_rayVivant[i] = false;
    m_nbRayMort++;
}

void Ray::stockage(){
    m_pos = m_ray;
    m_dir = m_vDir;
    m_nrgBackup = m_nrg;
    m_rayVivantBackup = m_rayVivant;
}


// rayons puis faces
bool Ray::rebondSansMemoire(MeshObj mesh, float seuil)
{
    // chargement du mesh
    std::vector<float> indiceMat(mesh.getIndMat());

    std::vector<CoordVector> normales(mesh.getNorm());
    std::vector<CoordVector> vertex(mesh.getVert());

    // Declarations
    //std::vector<float> absAir = absorptionAir(20);
    CoordVector point, vect_dir, vect_ref;
    CoordVector A,B,C; // trois points
    float longueur_inst(0),nor(0);
    int compteur(0),face(0);
    bool rayonsExistent = false;
    int j(0), k(0), l(0);

    *std::transform(m_dist.begin(), m_dist.end(), m_long.begin(), m_dist.begin(), std::plus<float>()); // ajout de la dernière longueur de rayon à la distance totale

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
            }

            // POUR CHAQUE NOUVEAU RAYON

            // Sécurité si problème d'intersection
            if(m_long[j] == 1000000)
            {
                qDebug() << "stop";
                QMessageBox::critical(NULL,"Erreur","Le " + QString::number(j) + "ème rayon n'a pas atteind de face", "Arreter le programme");

                return false;
            }

            //On met à jour l'énergie du nouveau rayon pour chaque bande
            compteur = 0;
            for (l=0; l<8; l++)
            {
                m_nrg[j*8 + l] *= (1-indiceMat[3*face+l+1]); // l'attenuation de l'air se fait dans les sources images
                // test si le rayon est mort
                if (m_nrg[j*8 + l] <= 0) compteur++;// Les rayons qui ont une énérgie non nul peuvent continuer de générer des sources images.
            }
            // si l'énergie sur les 8 bandes est à 0 le rayons est déclaré mort
            if (compteur == 8)
            {
                m_rayVivant[j] = false;
                //comptage des rayons morts pour la progress bar
                m_nbRayMort++;
            }
        }
        //Mise à jour des rayons vivant
        if (m_rayVivant[j])
        {
            // Mise à jour du point d'origine
            m_ray[j]+= m_vDir[j]*(m_long[j] - 1e-6); // On eloigne la point de la face de 1um pour éviter les rayons coincés dans des coins
            vect_ref = vecteur_reflechi(vect_dir,normales[face]);
            nor = norme(vect_ref);
            m_vDir[j] = vect_ref/nor;
        }
   }

    qDebug() <<  "nb ray mort" << m_nbRayMort;

    if(m_nbRayMort<m_Nray) return true;
    else return false;
}


// faces puis rayons + octree
bool Ray::rebondSansMemoire(MeshObj &mesh, float seuil, Octree &oct)
{
    // chargement du mesh
    std::vector<float> indiceMat(mesh.getIndMat());
    std::vector<Boite> vectBoite(oct.getVectBoite());

    std::vector<CoordVector> vert(mesh.getVert());
    std::vector<CoordVector> norm(mesh.getNorm());
    std::vector<CoordVector> vectFace(mesh.getVectFace());

    // Declarations
    //std::vector<float> absAir = absorptionAir(20);
    int compteur(0);
    bool rayonsExistent = false;
    int j(0), k(0), indE(0), l(0), i(0), indR(0);
    //int nbVertex = vertex.size();
    std::vector<int> face;
    face.resize(m_Nray, 0);

    *std::transform(m_dist.begin(), m_dist.end(), m_long.begin(), m_dist.begin(), std::plus<float>()); // ajout de la dernière longueur de rayon à la distance totale
    m_long.assign(m_Nray, 1000000);

    CoordVector e1, e2, pvec, tvec, qvec;
    float longueur_inst(0), det, u, v, invDet;

    stockage();

    for (i = 0 ; i< vectBoite.size() ; i ++) // pour chaque boite
    {
        if (vectBoite[i].estUneFeuille && !vectBoite[i].m_numRayon.empty()) // Si la boite est une feuille et comprend des rayons
        {
            for (indE=0; indE < vectBoite[i].m_numElt.size(); indE ++) // pour chaque face comprise dans la boite
            {
                k = vectBoite[i].m_numElt[indE];

                e1 = vectFace[2*k/3];
                e2 = vectFace[2*k/3+1];

                for(indR=0; indR<vectBoite[i].m_numRayon.size() ; indR++) // pour chaque rayon compris dans la boite
                {
                    j = vectBoite[i].m_numRayon[indR];

                    // fonction de Möller–Trumbore
                    pvec = produitVectoriel(m_dir[j], e2);
                    det = produitScalaire(e1,pvec);
                    if (det > 1e-8) {
                        invDet = 1/det;
                        tvec = vecteur(vert[k], m_pos[j]);
                        u = produitScalaire(tvec, pvec)*invDet;
                        if (u >= 0 && u <= 1) {
                            qvec = produitVectoriel(tvec,e1);
                            v = produitScalaire(m_dir[j], qvec)*invDet;
                            if (v >= 0 && u + v <= 1.00001) { // probleme d'arrondis
                                longueur_inst = produitScalaire(e2, qvec)*invDet;
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

            //Mise à jour de l'énergie des rayons pour chaque bande
            compteur = 0;
            for (l=0; l<8; l++)
            {
                m_nrg[j*8 + l] *= (1-indiceMat[3*face[j]+l+1]); // l'attenuation de l'air se fait dans les sources images

                //if (m_nrg[j*8 + l] < seuil) compteur++; // comptage des energies < seuil
                if (m_nrg[j*8 + l] <= 0) compteur++;// Les rayons qui ont une énérgie non nul peuvent continuer de générer des sources images.
            }
            // si l'énergie sur les 8 bandes est inférieure au seuil le rayons est déclaré mort
            if (compteur == 8)
            {
                m_rayVivant[j] = false;
                //comptage des rayons morts pour la progress bar
                m_nbRayMort++;
            }
            else rayonsExistent = true; // au moins un rayon est vivant
        }
       // else { m_long[j] = 0; }// A vérifier pourquoi on fait ça ...

        //Mise à jour des rayons vivant
        if (m_rayVivant[j])
        {
            // Mise à jour vecteur directeur
            m_vDir[j]  = vecteur_reflechi(m_dir[j], norm[face[j]]);

            // Mise à jour point d'origine
            m_ray[j]+= m_dir[j]*(m_long[j] - 1e-6); // On eloigne la point de la face de 1um pour éviter les rayons coincés dans des coins

        }
    }
    if(m_nbRayMort<m_Nray) return true;
    else return false;
}
