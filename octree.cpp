#include "octree.h"
#include "QDebug"
#include <math.h>
#include <QMessageBox>

// classes
Octree::Octree()
{
}

Octree::Octree(MeshObj monMesh, int nbFaceFeuille)
{

    // Declaration
    m_seuil = nbFaceFeuille;
    int k(0), i(0), j(0), l(0), nbBoiteNew(0), nbBoiteOld(0);

    // création du cube racine
    std::vector<float> vert = monMesh.getVertex();

    float xMin(vert[0]), xMax(vert[0]), yMin(vert[1]), yMax(vert[1]), zMin(vert[2]), zMax(vert[2]);
    for (i = 3 ; i <vert.size() ; i=i+3)
    {
        if(vert[i]<xMin)   xMin = vert[i];
        if(vert[i]>xMax)   xMax = vert[i];
        if(vert[i+1]<yMin) yMin = vert[i+1];
        if(vert[i+1]>yMax) yMax = vert[i+1];
        if(vert[i+2]<zMin) zMin = vert[i+2];
        if(vert[i+2]>zMax) zMax = vert[i+2];
    }

    CoordVector centre((xMax + xMin)/2, (yMax + yMin)/2,(zMax + zMin)/2);

    // Recuperation de la plus grande arrete
    float arrete = (xMax - xMin);
    if ((yMax - yMin) > arrete) arrete = (yMax - yMin);
    if ((zMax - zMin) > arrete) arrete = (zMax - zMin);

    //I- Création de la boite root
    Boite boiteRacine;
    boiteRacine.m_centre = centre;
    boiteRacine.m_arrete = arrete;
    boiteRacine.m_indicePere = -1;
    boiteRacine.m_indiceBoite = 0;
    boiteRacine.m_min = CoordVector(xMin, yMin, zMin);
    boiteRacine.m_max = CoordVector(xMax, yMax, zMax);
    boiteRacine.m_coinMin = CoordVector(centre.x-arrete/2, centre.y-arrete/2, centre.z-arrete/2);
    for (k = 0 ; k < vert.size() ; k+=9)
    {
        boiteRacine.m_numElt.push_back(k);
    }

    // II- Stockage de la boite racine
    m_vectBoite.push_back(boiteRacine);

    // Tant que le nombre de boite augmente
    while (m_vectBoite.size() > nbBoiteOld)
    {
        nbBoiteNew = m_vectBoite.size() - nbBoiteOld;
        nbBoiteOld = m_vectBoite.size();

        // pour chaque boite du nouvel étage
        for(i = nbBoiteOld - nbBoiteNew ; i <nbBoiteOld ; i++ )
        {
            // III- Si la boite n'est pas une feuille : Découpage de la boite courante en huit
             etagesuivant(vert, i);
        }
    }

    // Mise à jour taille des boites
    for (i = 0 ; i< m_vectBoite.size() ; i++) // pour chaque boite
    {
        if (!m_vectBoite[i].estUneFeuille || !m_vectBoite[i].m_numElt.empty())
        {
            m_vectBoite[i].m_arrete  = m_vectBoite[i].m_max.x - m_vectBoite[i].m_min.x;
            if (m_vectBoite[i].m_arrete < m_vectBoite[i].m_max.y - m_vectBoite[i].m_min.y)
                m_vectBoite[i].m_arrete = m_vectBoite[i].m_max.y - m_vectBoite[i].m_min.y;
            if (m_vectBoite[i].m_arrete < m_vectBoite[i].m_max.z - m_vectBoite[i].m_min.z)
                m_vectBoite[i].m_arrete = m_vectBoite[i].m_max.z - m_vectBoite[i].m_min.z;

            m_vectBoite[i].m_coinMin = m_vectBoite[i].m_min - 0.001; // Pour eliminer les problèmes d'arondi
            m_vectBoite[i].m_arrete += 0.002;
        }
    }

    // Vérification
    int nbEltFeuille(0);
    for (i = 0 ; i< m_vectBoite.size() ; i++)
    {
        if (m_vectBoite[i].estUneFeuille)
            nbEltFeuille+= m_vectBoite[i].m_numElt.size();
    }
    if(vert.size()/9 != nbEltFeuille)
        QMessageBox::critical(NULL,"Erreur","Mauvaise mise en boite des faces");


}

Octree::~Octree()
{
}

Octree Octree::operator=(const Octree &oct)
{
    m_vectBoite.resize(oct.getVectBoite().size(),Boite());
    m_vectBoite = oct.getVectBoite();
    m_seuil = oct.getSeuil();
    return *this;
}


const std::vector<Boite> &Octree::getVectBoite() const{
    return m_vectBoite;
}

int Octree::getSeuil() const{
    return m_seuil;
}

Boite::Boite()
{
    m_centre = CoordVector();
    m_min = CoordVector();
    m_max = CoordVector();
    m_arrete = 0;
    m_coinMin = CoordVector();
    m_indicePere = 0;
    m_indiceBoite = 0;
    estUneFeuille = false;
}

Boite::Boite(const CoordVector &centre, float arrete, int indPere)
{
    m_centre = centre;
    m_arrete = arrete;
    m_indicePere = indPere;
    estUneFeuille = false;
}

Boite::~Boite()
{
}

Boite Boite::operator=(const Boite &boite)
{
    m_centre      = boite.m_centre;
    m_min         = boite.m_min;
    m_max         = boite.m_max;
    m_coinMin     = boite.m_coinMin;
    m_arrete      = boite.m_arrete;
    m_indicePere  = boite.m_indicePere;
    m_indiceBoite = boite.m_indiceBoite;
    m_numElt      = boite.m_numElt;
    m_numRayon    = boite.m_numRayon;
    estUneFeuille = boite.estUneFeuille;

    return *this;
}

int Boite::getNbElt() {
    return m_numElt.size();
}

std::vector<int> &Boite::getVectNumElt() {
    return m_numElt;
}

void Boite::chargerElt(std::vector<float> const& vert, int indice)
{
    // on garde l'indice
    m_numElt.push_back(indice);

    // Recupération du min et max
    for (int i = 0; i<9 ; i+=3)
    {
        if(vert[indice+i]   < m_min.x) m_min.x = vert[indice+i];
        if(vert[indice+i]   > m_max.x) m_max.x = vert[indice+i];
        if(vert[indice+i+1] < m_min.y) m_min.y = vert[indice+i+1];
        if(vert[indice+i+1] > m_max.y) m_max.y = vert[indice+i+1];
        if(vert[indice+i+2] < m_min.z) m_min.z = vert[indice+i+2];
        if(vert[indice+i+2] > m_max.z) m_max.z = vert[indice+i+2];
    }

}



void Boite::chargerRay(int indice) {
    m_numRayon.push_back(indice);
}

void Boite::supprElt(int position) {
    m_numElt.erase(m_numElt.begin()+position);
}

void Boite::supprRay(int position) {
    m_numRayon.erase(m_numRayon.begin()+position);
}

//Methodes

void Octree::etagesuivant(std::vector<float> &vert, int indiceBoite)
{

    if (m_vectBoite[indiceBoite].m_numElt.size() < m_seuil)
    {
        m_vectBoite[indiceBoite].estUneFeuille = true;
    }
    else // Si la boite n'est pas une feuille
    {
        int i, k, ind;
        std::vector<int> elt;
        bool premierElt;

        // IV- Création des 8 boites filles à partir de la boite actuelle
        std::vector<Boite> boitesFilles;
        decoupage(m_vectBoite[indiceBoite], boitesFilles);

        // V- Pour chaque nouvelle boite :
        for (i = 0 ; i<8 ; i++)
        {
            elt = m_vectBoite[indiceBoite].getVectNumElt();
            premierElt = true;

            // Pour chaque face compris dans la boite père
            for (k = elt.size()-1; k >=0  ; k--)
            {
                ind = elt[k];

                if (appartientBoite(boitesFilles[i], vert, ind))
                {
                    if(premierElt)
                    {
                        boitesFilles[i].m_min.x = vert[ind];
                        boitesFilles[i].m_min.y = vert[ind+1];
                        boitesFilles[i].m_min.z = vert[ind+2];
                        boitesFilles[i].m_max.x = vert[ind];
                        boitesFilles[i].m_max.y = vert[ind+1];
                        boitesFilles[i].m_max.z = vert[ind+2];
                        premierElt = false;
                    }
                    boitesFilles[i].chargerElt(vert, ind); // Si le centre de la face est inclue dans la boite fille en test on stock son indice
                    m_vectBoite[indiceBoite].supprElt(k);  // et on le retire de la boite pere (ancienne methode)
                }
            }
            boitesFilles[i].m_indiceBoite = m_vectBoite.size();
            m_vectBoite.push_back(boitesFilles[i]); // Ajout de la boite fille
        }
    }
}

void decoupage(Boite &boitePere, std::vector<Boite>& boitesFilles)
{
    // Recuperation des données du père
    CoordVector coinMin = boitePere.m_coinMin;
    float arrete = boitePere.m_arrete/2;

    // Initialisation du vecteur de boites filles
    //std::vector<Boite> boitesFilles;
    boitesFilles.resize(8, Boite());

    boitesFilles[0].m_coinMin = CoordVector(coinMin.x           , coinMin.y          , coinMin.z         );
    boitesFilles[1].m_coinMin = CoordVector(coinMin.x + arrete  , coinMin.y          , coinMin.z         );
    boitesFilles[2].m_coinMin = CoordVector(coinMin.x           , coinMin.y + arrete , coinMin.z         );
    boitesFilles[3].m_coinMin = CoordVector(coinMin.x + arrete  , coinMin.y + arrete , coinMin.z         );
    boitesFilles[4].m_coinMin = CoordVector(coinMin.x           , coinMin.y          , coinMin.z + arrete);
    boitesFilles[5].m_coinMin = CoordVector(coinMin.x + arrete  , coinMin.y          , coinMin.z + arrete);
    boitesFilles[6].m_coinMin = CoordVector(coinMin.x           , coinMin.y + arrete , coinMin.z + arrete);
    boitesFilles[7].m_coinMin = CoordVector(coinMin.x + arrete  , coinMin.y + arrete , coinMin.z + arrete);

    for (int i = 0 ; i< 8 ; i++)
    {
        boitesFilles[i].m_centre     = CoordVector(boitesFilles[i].m_coinMin.x + arrete/2, boitesFilles[i].m_coinMin.y + arrete/2, boitesFilles[i].m_coinMin.z + arrete/2);
        boitesFilles[i].m_arrete     = arrete;
        boitesFilles[i].m_indicePere = boitePere.m_indiceBoite;
    }

}

bool appartientBoite(Boite &boite, std::vector<float> &vert, int indice)
{
    // Recupération du centre
    CoordVector centreFace(0,0,0);
    for (int i = 0 ; i < 9 ; i+=3)
    {
        centreFace.x+= vert[indice+i];
        centreFace.y+= vert[indice+i+1];
        centreFace.z+= vert[indice+i+2];
    }

    if (centreFace.x/3 < boite.m_coinMin.x)                   return false;
    if (centreFace.x/3 > boite.m_coinMin.x + boite.m_arrete)  return false;
    if (centreFace.y/3 < boite.m_coinMin.y)                   return false;
    if (centreFace.y/3 > boite.m_coinMin.y + boite.m_arrete)  return false;
    if (centreFace.z/3 < boite.m_coinMin.z)                   return false;
    if (centreFace.z/3 > boite.m_coinMin.z + boite.m_arrete)  return false;

    // ANCIENNE METHODE
    /*
    // Pour chacun des trois points de la face
    for (int i = 0 ; i < 9 ; i+=3)
    {
        if (vert[indice + i] < boite.m_coinMin.x)                   return false;
        if (vert[indice + i] > boite.m_coinMin.x + boite.m_arrete)  return false;
        if (vert[indice+i+1] < boite.m_coinMin.y)                   return false;
        if (vert[indice+i+1] > boite.m_coinMin.y + boite.m_arrete)  return false;
        if (vert[indice+i+2] < boite.m_coinMin.z)                   return false;
        if (vert[indice+i+2] > boite.m_coinMin.z + boite.m_arrete)  return false;
    }
    */
    return true;
}

void Octree::chargerRayonRacine(int nbRay)
{
    int j;
    m_vectBoite[0].m_numRayon.clear();
    // Chargement de la boite racine avec tous les indices rayons
    for (j = 0; j<nbRay ; j++)
    {
        m_vectBoite[0].m_numRayon.push_back(j);
    }
}

void Octree::chargerRayon(std::vector<CoordVector> &orig, std::vector<CoordVector> &dir)
{
    int i, j, ind;

    // Calcul d'inverse pour eviter problème de signe lors de divisions par 0
    std::vector<CoordVector> invDir;
    for (i = 0 ;  i < dir.size() ; i++)
    {
        invDir.push_back(inverse(dir[i]));
    }

    int numPere;

    // Pour chaque boite : chargement de l'indice des rayons qui intersectent avec elle
    for (i = 1 ; i < m_vectBoite.size() ; i++)
    {
        // On ne s'occupe que des feuilles avec des elts ou des non-feuilles
        if (!m_vectBoite[i].m_numElt.empty() || !m_vectBoite[i].estUneFeuille)
        {
            // On supprime les indices des rayons précedemment enregistrés
            m_vectBoite[i].m_numRayon.clear();

            numPere = m_vectBoite[i].m_indicePere;

            //for (ind=0 ; ind<dir.size() ; ind++) // tous les rayons
            for (j = 0 ; j < m_vectBoite[numPere].m_numRayon.size() ; j++) // Pour tous les rayons contenus dans la boite père
            {
                ind = m_vectBoite[numPere].m_numRayon[j];

                // test intersection entre rayon ind et boite i
                if (intersecBoiteRay(m_vectBoite[i], orig[ind], invDir[ind]))
                //if (intersecSphereRay(m_vectBoite[i], orig[ind], dir[ind]))
                {
                    m_vectBoite[i].chargerRay(ind); // Ajout des rayons à la boite courante
                }
            }
        }
    }
    /*
    // Vérification
    int nbRay(0);
    for (i = 0 ; i< m_vectBoite.size() ; i++)
    {
        nbRay+= m_vectBoite[i].m_numRayon.size();
    }
    qDebug() << "Nombre total de rayon stocké :" << nbRay;
    */
}


bool intersecBoiteRay(const Boite &boite, const CoordVector &orig, const CoordVector &invDir)
{
    // Ref : http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection

    float tmin, tmax, tymin, tymax;

    if (invDir.x >=0) {
        tmin = (boite.m_coinMin.x                  - orig.x) * invDir.x;
        tmax = (boite.m_coinMin.x + boite.m_arrete - orig.x) * invDir.x;
    }
    else {
        tmax = (boite.m_coinMin.x                  - orig.x) * invDir.x;
        tmin = (boite.m_coinMin.x + boite.m_arrete - orig.x) * invDir.x;
    }

    if (invDir.y >=0) {
        tymin = (boite.m_coinMin.y                  - orig.y) * invDir.y;
        tymax = (boite.m_coinMin.y + boite.m_arrete - orig.y) * invDir.y;
    }
    else {
        tymax = (boite.m_coinMin.y                  - orig.y) * invDir.y;
        tymin = (boite.m_coinMin.y + boite.m_arrete - orig.y) * invDir.y;
    }

   if ((tmin > tymax) || (tymin > tmax)) return false;

   if (tymin > tmin) tmin = tymin;
   if (tymax < tmax) tmax = tymax;

   float tzmin, tzmax;

   if(invDir.z >= 0) {
       tzmin = (boite.m_coinMin.z                   - orig.z) * invDir.z;
       tzmax = (boite.m_coinMin.z + boite.m_arrete  - orig.z) * invDir.z;
   }
   else {
       tzmax = (boite.m_coinMin.z                   - orig.z) * invDir.z;
       tzmin = (boite.m_coinMin.z + boite.m_arrete  - orig.z) * invDir.z;
   }

   if ((tmin > tzmax) || (tzmin > tmax)) return false;

   return true;
}

bool intersecSphereRay(Boite &boite, const CoordVector &orig, const CoordVector &dir)
{
    //CoordVector origine(orig[indice], orig[indice+1], orig[indice+2]);
    //CoordVector direction(dir[indice], dir[indice+1], dir[indice+2]);
    CoordVector u = vecteur(orig, boite.m_centre);

    //float alpha = angle(direction, u);
/*
    if (norme(u) <= sqrt(2)*boite.m_arrete/2) return true; // si l'origine est dans la sphere
    else if (cos(alpha) >=0 && alpha <= asin(sqrt(2)*boite.m_arrete/2/norme(u))) return true; // si l'angle du rayon permet d'intesecter la sphere

    return false;
    */

    if (norme(u) <= sqrt(2)*boite.m_arrete/2) return true; // si l'origine est dans la sphere
    float p = produitScalaire(dir, u);
    if (p <= 0) return false; // si les directions sont opposées
    if (acos(p/(norme(dir)*norme(u))) > asin(sqrt(2)*boite.m_arrete/2/norme(u))) return false; // si l'angle du rayon permet d'intersecter la sphere
    //float invNomreU = 1/norme(u);
    //if (norme(u) > sqrt(2)*boite.m_arrete/2 && acos(p*invNomreU/norme(direction)) > asin(sqrt(2)*invNomreU*boite.m_arrete/2)) return false;

    return true;

    /*
    // méthode du discriminant
    float discriminant = pow(2*produitScalaire(direction, u), 2) - 4*produitScalaire(direction,direction)*(produitScalaire(u,u)-pow(boite.m_arrete,2)/2);
    if (discriminant < 0)   return false;
    else                    return true;
    */
}
