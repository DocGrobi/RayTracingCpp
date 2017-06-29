#include "octree.h"
#include "QDebug"
#include <math.h>
//#include <algorithm>    // std::swap
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
    for (int i = 3 ; i <vert.size() ; i=i+3)
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
    //Boite boiteRacine(centre, arrete, -1);
    Boite boiteRacine;
    boiteRacine.m_centre = centre;
    boiteRacine.m_arrete = arrete;
    boiteRacine.m_indicePere = -1;
    boiteRacine.m_indiceBoite = 0;
    boiteRacine.m_coinMin = CoordVector(centre.x-arrete/2, centre.y-arrete/2, centre.z-arrete/2);
    for (k = 0 ; k < vert.size() ; k+=9)
    {
        boiteRacine.chargerElt(k);
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

    // Mise à jour taille des boites feuilles
    for (i = 0 ; i< m_vectBoite.size() ; i++) // pour chaque boite
    {
        // Si la boite est une feuille (elle comporte forcement au moins un element)
        if (m_vectBoite[i].estUneFeuille)
        {
            // initialisation des min et max
            k = m_vectBoite[i].m_numElt[0];
            xMin = vert[k];
            xMax = vert[k];
            yMin = vert[k+1];
            yMax = vert[k+1];
            zMin = vert[k+2];
            zMax = vert[k+2];

            for(j = 0 ; j < m_vectBoite[i].m_numElt.size() ; j++) // pour chaque elt contenu dans la boite
            {
                k = m_vectBoite[i].m_numElt[j];
                for (l = 0 ; l< 9 ; l+=3) // pour chaque point de l'element
                {
                    // On recupère les min et max
                    if(vert[k+l]<xMin)   xMin = vert[k+l];
                    if(vert[k+l]>xMax)   xMax = vert[k+l];
                    if(vert[k+l+1]<yMin) yMin = vert[k+l+1];
                    if(vert[k+l+1]>yMax) yMax = vert[k+l+1];
                    if(vert[k+l+2]<zMin) zMin = vert[k+l+2];
                    if(vert[k+l+2]>zMax) zMax = vert[k+l+2];
                }
            }
            centre = CoordVector((xMax + xMin)/2, (yMax + yMin)/2,(zMax + zMin)/2);
            m_vectBoite[i].m_centre = centre;
            // Recuperation de la plus grande arrete
            arrete = (xMax - xMin);
            if ((yMax - yMin) > arrete) arrete = (yMax - yMin);
            if ((zMax - zMin) > arrete) arrete = (zMax - zMin);

            m_vectBoite[i].m_arrete = arrete;
            m_vectBoite[i].m_coinMin = CoordVector(centre.x-arrete/2, centre.y-arrete/2, centre.z-arrete/2);
        }
    }

    // Vérification
    // int nbElt(0);
    int nbEltFeuille(0);
    for (i = 0 ; i< m_vectBoite.size() ; i++)
    {
        //nbElt+= m_vectBoite[i].m_numElt.size();
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
    m_centre = CoordVector(0,0,0);
    m_arrete = 0;
    m_coinMin = CoordVector(0,0,0);
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
    m_centre = boite.m_centre;
    m_coinMin = boite.m_coinMin;
    m_arrete = boite.m_arrete;
    m_indicePere = boite.m_indicePere;
    m_indiceBoite = boite.m_indiceBoite;
    m_numElt = boite.m_numElt;
    m_numRayon = boite.m_numRayon;
    estUneFeuille = boite.estUneFeuille;

    return *this;
}

int Boite::getNbElt() {
    return m_numElt.size();
}

std::vector<int> &Boite::getVectNumElt() {
    return m_numElt;
}

void Boite::chargerElt(int indice)
{
    m_numElt.push_back(indice);
}

void Boite::chargerRay(int indice)
{
    m_numRayon.push_back(indice);
}

void Boite::supprElt(int position)
{
    m_numElt.erase(m_numElt.begin()+position);
}

void Boite::supprRay(int position)
{
    m_numRayon.erase(m_numRayon.begin()+position);
}

//Methodes

void Octree::etagesuivant(std::vector<float> &vert, int indiceBoite)
{

    if (m_vectBoite[indiceBoite].m_numElt.size() < m_seuil)
    {
        if (!m_vectBoite[indiceBoite].m_numElt.empty()) // si la boite est vide on lui accorde pas le statut de feuille
                m_vectBoite[indiceBoite].estUneFeuille = true;
    }
    else // Si la boite n'est pas une feuille
    {
        int i, k, ind;
        std::vector<int> elt;

        // IV- Création des 8 boites filles à partir de la boite actuelle
        std::vector<Boite> boitesFilles;
        decoupage(m_vectBoite[indiceBoite], boitesFilles);

        // V- Pour chaque nouvelle boite :
        for (i = 0 ; i<8 ; i++)
        {
            elt = m_vectBoite[indiceBoite].getVectNumElt();

            // Pour chaque face compris dans la boite père
            for (k = elt.size()-1; k >=0  ; k--)
            {
                ind = elt[k];

                if (appartientBoite(boitesFilles[i], vert, elt[k]))
                {
                    boitesFilles[i].chargerElt(ind);    // Si le centre de la face est inclue dans la boite fille en test on stock son indice
                    m_vectBoite[indiceBoite].supprElt(k); // et on le retire de la boite pere (ancienne methode)
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
    CoordVector centre = boitePere.m_centre;
    CoordVector coinMin = boitePere.m_coinMin;
    float arrete = boitePere.m_arrete/2;
    int indicePere = boitePere.m_indiceBoite;

    // Initialisation du vecteur de boites filles
    //std::vector<Boite> boitesFilles;
    boitesFilles.resize(8, Boite());

    // Remplissage des boites filles (code binaire pour le centre)
    boitesFilles[0] = Boite(CoordVector(centre.x - arrete/2, centre.y - arrete/2, centre.z - arrete/2), arrete, indicePere);
    boitesFilles[1] = Boite(CoordVector(centre.x + arrete/2, centre.y - arrete/2, centre.z - arrete/2), arrete, indicePere);
    boitesFilles[2] = Boite(CoordVector(centre.x - arrete/2, centre.y + arrete/2, centre.z - arrete/2), arrete, indicePere);
    boitesFilles[3] = Boite(CoordVector(centre.x + arrete/2, centre.y + arrete/2, centre.z - arrete/2), arrete, indicePere);
    boitesFilles[4] = Boite(CoordVector(centre.x - arrete/2, centre.y - arrete/2, centre.z + arrete/2), arrete, indicePere);
    boitesFilles[5] = Boite(CoordVector(centre.x + arrete/2, centre.y - arrete/2, centre.z + arrete/2), arrete, indicePere);
    boitesFilles[6] = Boite(CoordVector(centre.x - arrete/2, centre.y + arrete/2, centre.z + arrete/2), arrete, indicePere);
    boitesFilles[7] = Boite(CoordVector(centre.x + arrete/2, centre.y + arrete/2, centre.z + arrete/2), arrete, indicePere);

    boitesFilles[0].m_coinMin = CoordVector(coinMin.x           , coinMin.y          , coinMin.z         );
    boitesFilles[1].m_coinMin = CoordVector(coinMin.x + arrete  , coinMin.y          , coinMin.z         );
    boitesFilles[2].m_coinMin = CoordVector(coinMin.x           , coinMin.y + arrete , coinMin.z         );
    boitesFilles[3].m_coinMin = CoordVector(coinMin.x + arrete  , coinMin.y + arrete , coinMin.z         );
    boitesFilles[4].m_coinMin = CoordVector(coinMin.x           , coinMin.y          , coinMin.z + arrete);
    boitesFilles[5].m_coinMin = CoordVector(coinMin.x + arrete  , coinMin.y          , coinMin.z + arrete);
    boitesFilles[6].m_coinMin = CoordVector(coinMin.x           , coinMin.y + arrete , coinMin.z + arrete);
    boitesFilles[7].m_coinMin = CoordVector(coinMin.x + arrete  , coinMin.y + arrete , coinMin.z + arrete);


    //return boitesFilles;

}

bool appartientBoite(Boite &boite, std::vector<float> &vert, int indice)
{
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
    for (j = 0; j<3*nbRay ; j+=3)
    {
        m_vectBoite[0].m_numRayon.push_back(j);
    }
}

void Octree::chargerRayon(std::vector<float> &orig, std::vector<float> &dir)
{

    //Boite boitePere;

    int i, j, ind;

    // Pour chaque boite : chargement de l'indice des rayons qui intersectent avec elle
    for (i = 1 ; i < m_vectBoite.size() ; i++)
    {
        // On ne s'occupe que des feuilles
        if (m_vectBoite[i].estUneFeuille)
        {
            // On supprime les indices des rayons précedemment enregistrés
            m_vectBoite[i].m_numRayon.clear();

            //boitePere = m_vectBoite[m_vectBoite[i].m_indicePere];

            // Pour tous les rayons contenus dans la boite père.
            //for (j = boitePere.m_numRayon.size()-1 ; j >=0  ; j--)
            for (ind = 0 ; ind < dir.size() ; ind+=3)
            {
                //ind = boitePere.m_numRayon[j];

                // test intersection entre rayon ind et boite i
                if (intersecBoiteRay(m_vectBoite[i], orig, dir, ind))
                //if (intersecSphereRay(m_vectBoite[i], orig, dir, ind))
                {
                    m_vectBoite[i].chargerRay(ind); // Ajout des rayons à la boite courante
                    // On ne supprime pas de la boite père car il se peut qu'il y est des faces stockées dedans
                    //m_vectBoite[m_vectBoite[i].m_indicePere].supprRay(j); // Suppression des rayons de la boite père
                }
            }
        }
    }



    // Vérification
    int nbRay(0);
    for (i = 0 ; i< m_vectBoite.size() ; i++)
    {
        nbRay+= m_vectBoite[i].m_numRayon.size();
    }
    qDebug() << "Nombre total de rayon stocké :" << nbRay;

}

bool intersecSphereRay(Boite &boite, std::vector<float>& orig, std::vector<float>& dir, int indice)
{
    //CoordVector origine(orig[indice], orig[indice+1], orig[indice+2]);
    CoordVector direction(dir[indice], dir[indice+1], dir[indice+2]);
    CoordVector u = vecteur(orig, indice, boite.m_centre);
    //float alpha = angle(direction, u);
/*
    if (norme(u) <= sqrt(2)*boite.m_arrete/2) return true; // si l'origine est dans la sphere
    else if (cos(alpha) >=0 && alpha <= asin(sqrt(2)*boite.m_arrete/2/norme(u))) return true; // si l'angle du rayon permet d'intesecter la sphere

    return false;
    */

    if (norme(u) <= sqrt(2)*boite.m_arrete/2) return true; // si l'origine est dans la sphere
    float p = produitScalaire(direction, u);
    if (p <= 0) return false; // si les directions sont opposées
    if (acos(p/(norme(direction)*norme(u))) > asin(sqrt(2)*boite.m_arrete/2/norme(u))) return false; // si l'angle du rayon permet d'intersecter la sphere
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

bool intersecBoiteRay(Boite &boite, std::vector<float>& orig, std::vector<float>& dir, int indice)
{

    //CoordVector coinMin = boite.m_coinMin;
    //float a = boite.m_arrete;

    // implementation http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection

    // Il faut calculer l'inverse avant car sinon il y a des problème de signe lors de la division par 0 et on perd des rayons
    float invDirx = 1/ dir[indice];
    float invDiry = 1/ dir[indice+1];
    float invDirz = 1/ dir[indice+2];

    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    if (invDirx >=0)
    {
        tmin = (boite.m_coinMin.x     - orig[indice]) * invDirx;
        tmax = (boite.m_coinMin.x +boite.m_arrete- orig[indice]) * invDirx;
    }
    else
    {
        tmax = (boite.m_coinMin.x     - orig[indice]) * invDirx;
        tmin = (boite.m_coinMin.x +boite.m_arrete- orig[indice]) * invDirx;
    }

    if (invDiry >=0)
    {
        tymin = (boite.m_coinMin.y     - orig[indice+1]) * invDiry;
        tymax = (boite.m_coinMin.y +boite.m_arrete- orig[indice+1]) * invDiry;
    }
    else
    {
        tymax = (boite.m_coinMin.y     - orig[indice+1]) * invDiry;
        tymin = (boite.m_coinMin.y +boite.m_arrete- orig[indice+1]) * invDiry;
    }

   if ((tmin > tymax) || (tymin > tmax)) return false;

   if (tymin > tmin)
   tmin = tymin;
   if (tymax < tmax)
   tmax = tymax;

   if(invDirz >= 0)
   {
       tzmin = (boite.m_coinMin.z     - orig[indice+2]) * invDirz;
       tzmax = (boite.m_coinMin.z +boite.m_arrete- orig[indice+2]) * invDirz;
   }
   else
   {
       tzmax = (boite.m_coinMin.z     - orig[indice+2]) * invDirz;
       tzmin = (boite.m_coinMin.z +boite.m_arrete- orig[indice+2]) * invDirz;
   }

   if ((tmin > tzmax) || (tzmin > tmax)) return false;

   return true;

/*
    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    if (dir[indice] >=0)
    {
        tmin = (coinMin.x     - orig[indice]) / dir[indice];
        tmax = (coinMin.x + a - orig[indice]) / dir[indice];
    }
    else
    {
        tmax = (coinMin.x     - orig[indice]) / dir[indice];
        tmin = (coinMin.x + a - orig[indice]) / dir[indice];
    }

    if (dir[indice+1] >=0)
    {
        tymin = (coinMin.y     - orig[indice+1]) / dir[indice+1];
        tymax = (coinMin.y + a - orig[indice+1]) / dir[indice+1];
    }
    else
    {
        tymax = (coinMin.y     - orig[indice+1]) / dir[indice+1];
        tymin = (coinMin.y + a - orig[indice+1]) / dir[indice+1];
    }

   if ((tmin > tymax) || (tymin > tmax)) return false;

   if (tymin > tmin)
   tmin = tymin;
   if (tymax < tmax)
   tmax = tymax;

   if(dir[indice+2] >= 0)
   {
       tzmin = (coinMin.z     - orig[indice+2]) / dir[indice+2];
       tzmax = (coinMin.z + a - orig[indice+2]) / dir[indice+2];
   }
   else
   {
       tzmax = (coinMin.z     - orig[indice+2]) / dir[indice+2];
       tzmin = (coinMin.z + a - orig[indice+2]) / dir[indice+2];
   }

   if ((tmin > tzmax) || (tzmin > tmax)) return false;

   return true;
*/
}
