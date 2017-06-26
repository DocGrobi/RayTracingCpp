#include "octree.h"
#include "QDebug"
#include <math.h>
//#include <algorithm>    // std::swap
#include <QMessageBox>

//methodes


// classes
Octree::Octree()
{
}

Octree::Octree(MeshObj monMesh, int nbFaceFeuille)
{

    // Declaration
    m_seuil = nbFaceFeuille;
    int k(0), i(0), j(0), nbBoiteNew(0), nbBoiteOld(0);

    // création du cube racine
    std::vector<float> vert = monMesh.getVertex();
    float xMin(0), xMax(0), yMin(0), yMax(0), zMin(0), zMax(0);
    for (int i = 0 ; i <vert.size() ; i=i+3)
    {
        if(vert[i]<xMin)
        {
            xMin = vert[i];
        }
        if(vert[i]>xMax)
        {
            xMax = vert[i];
        }

        if(vert[i+1]<yMin)
        {
            yMin = vert[i+1];
        }
        if(vert[i+1]>yMax)
        {
            yMax = vert[i+1];
        }

        if(vert[i+2]<zMin)
        {
            zMin = vert[i+2];
        }
        if(vert[i+2]>zMax)
        {
            zMax = vert[i+2];
        }
    }
    float arrete;
    CoordVector centre((xMax + xMin)/2, (yMax + yMin)/2,(zMax + zMin)/2);
    arrete = (xMax - xMin)/2;
    if ((yMax - yMin)/2 > arrete)
    {
        arrete = (yMax - yMin)/2;
    }
    if ((zMax - zMin)/2 > arrete)
    {
        arrete = (zMax - zMin)/2;
    }



    //I- Création de la boite root
    Boite boiteRacine(centre, arrete, -1);
    boiteRacine.m_indiceBoite = 0;
    boiteRacine.m_coinMin = CoordVector(xMin, yMin, zMin);
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

    // Vérification
    int nbElt(0);
    for (i = 0 ; i< m_vectBoite.size() ; i++)
    {
        nbElt+= m_vectBoite[i].m_numElt.size();
    }
    if(vert.size()/9 != nbElt)
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


std::vector<Boite> Octree::getVectBoite() const{
    return m_vectBoite;
}

int Octree::getSeuil() const{
    return m_seuil;
}

Boite::Boite()
{
    m_centre = CoordVector(0,0,0);
    m_arrete = 0;
    m_indicePere = 0;
    m_indiceBoite = 0;
}

Boite::Boite(const CoordVector &centre, float arrete, int indPere)
{
    m_centre = centre;
    m_arrete = arrete;
    m_indicePere = indPere;
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
/*
CoordVector Boite::getCentre(){
    return m_centre;
}

float Boite::getRayon(){
    return m_arrete;
}

int Boite::getIndPere(){
    return m_indicePere;
}
*/
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
    // Si la boite n'est pas une feuille
    if (m_vectBoite[indiceBoite].m_numElt.size() < m_seuil)
    {
        m_vectBoite[indiceBoite].estUneFeuille = true;
    }
    else
    {
        float r, x, y, z; // valeurs tampon
        // les valeurs tampon servent à comparer [Somme des x] à 3*centre +/-r.
        int i, k, ind;
        std::vector<int> elt;

        // V- Création des 8 boites filles à partir de la boite actuelle
        std::vector<Boite> boitesFilles = decoupage(m_vectBoite[indiceBoite]);

        //r = 3*m_vectBoite[indiceBoite].m_arrete/2; // valeur de comparaison tampon
        r = m_vectBoite[indiceBoite].m_arrete/2; // valeur de comparaison tampon

        // VI- Pour chaque nouvelle boite :
        for (i = 0 ; i<8 ; i++)
        {

            // valeurs de comparaison tampon
            x = 3*boitesFilles[i].m_centre.x;
            y = 3*boitesFilles[i].m_centre.y;
            z = 3*boitesFilles[i].m_centre.z;

            elt = m_vectBoite[indiceBoite].getVectNumElt();

            // Pour chaque face compris dans la boite père
            for (k = elt.size()-1; k >=0  ; k--)
            {
                ind = elt[k];
                /*
                if (fabs(vert[ind]  +vert[ind+3]+vert[ind+6] - x) <= r
                 && fabs(vert[ind+1]+vert[ind+4]+vert[ind+7] - y) <= r
                 && fabs(vert[ind+2]+vert[ind+5]+vert[ind+8] - z) <= r)
                 */
                if (fabs(vert[ind]   - boitesFilles[i].m_centre.x) <= r
                 && fabs(vert[ind+3] - boitesFilles[i].m_centre.x) <= r
                 && fabs(vert[ind+6] - boitesFilles[i].m_centre.x) <= r
                 && fabs(vert[ind+1] - boitesFilles[i].m_centre.y) <= r
                 && fabs(vert[ind+4] - boitesFilles[i].m_centre.y) <= r
                 && fabs(vert[ind+7] - boitesFilles[i].m_centre.y) <= r
                 && fabs(vert[ind+2] - boitesFilles[i].m_centre.z) <= r
                 && fabs(vert[ind+5] - boitesFilles[i].m_centre.z) <= r
                 && fabs(vert[ind+8] - boitesFilles[i].m_centre.z) <= r)
                {
                    boitesFilles[i].chargerElt(ind);    // Si la face est inclue dans la boite fille en test on stock son indice
                    m_vectBoite[indiceBoite].supprElt(k); // et on le retire de la boite pere
                }
            }
            boitesFilles[i].m_indiceBoite = m_vectBoite.size();
            m_vectBoite.push_back(boitesFilles[i]); // Ajout de la boite fille
        }
    }


}
/*
bool estUneFeuille(Boite boite, int seuil)
{
    if(boite.getNbElt()<seuil)
    {
        return true;
    }
    else{return false;}
}

bool Boite::estUneFeuille()
{
    if(m_numElt.size()<m_seuil)
    {
        return true;
    }
    else{return false;}
}
*/
std::vector<Boite> decoupage(Boite &boitePere)
{
    // Recuperation des données du père
    CoordVector centre = boitePere.m_centre;
    CoordVector coinMin = boitePere.m_coinMin;
    float arrete = boitePere.m_arrete;
    int indicePere = boitePere.m_indiceBoite;

    // Initialisation du vecteur de boites filles
    std::vector<Boite> boitesFilles;
    //Boite boite0(CoordVector(0,0,0),0,0);
    boitesFilles.resize(8, Boite());

    // Remplissage des boites filles (code binaire pour le centre)
    boitesFilles[0] = Boite(CoordVector(centre.x - arrete/2, centre.y - arrete/2, centre.z - arrete/2), arrete/2, indicePere);
    boitesFilles[1] = Boite(CoordVector(centre.x + arrete/2, centre.y - arrete/2, centre.z - arrete/2), arrete/2, indicePere);
    boitesFilles[2] = Boite(CoordVector(centre.x - arrete/2, centre.y + arrete/2, centre.z - arrete/2), arrete/2, indicePere);
    boitesFilles[3] = Boite(CoordVector(centre.x + arrete/2, centre.y + arrete/2, centre.z - arrete/2), arrete/2, indicePere);
    boitesFilles[4] = Boite(CoordVector(centre.x - arrete/2, centre.y - arrete/2, centre.z + arrete/2), arrete/2, indicePere);
    boitesFilles[5] = Boite(CoordVector(centre.x + arrete/2, centre.y - arrete/2, centre.z + arrete/2), arrete/2, indicePere);
    boitesFilles[6] = Boite(CoordVector(centre.x - arrete/2, centre.y + arrete/2, centre.z + arrete/2), arrete/2, indicePere);
    boitesFilles[7] = Boite(CoordVector(centre.x + arrete/2, centre.y + arrete/2, centre.z + arrete/2), arrete/2, indicePere);

    boitesFilles[0].m_coinMin = CoordVector(coinMin.x           , coinMin.y          , coinMin.z         );
    boitesFilles[1].m_coinMin = CoordVector(coinMin.x + arrete  , coinMin.y          , coinMin.z         );
    boitesFilles[2].m_coinMin = CoordVector(coinMin.x           , coinMin.y + arrete , coinMin.z         );
    boitesFilles[3].m_coinMin = CoordVector(coinMin.x + arrete  , coinMin.y + arrete , coinMin.z         );
    boitesFilles[4].m_coinMin = CoordVector(coinMin.x           , coinMin.y          , coinMin.z + arrete);
    boitesFilles[5].m_coinMin = CoordVector(coinMin.x + arrete  , coinMin.y          , coinMin.z + arrete);
    boitesFilles[6].m_coinMin = CoordVector(coinMin.x           , coinMin.y + arrete , coinMin.z + arrete);
    boitesFilles[7].m_coinMin = CoordVector(coinMin.x + arrete  , coinMin.y + arrete , coinMin.z + arrete);


    return boitesFilles;

}

void Octree::chargerRayon(std::vector<float> &orig, std::vector<float> &dir)
{

    Boite boitePere;

    int i, j, ind;
    int nbRay(0);

    // Chargement de la boite racine avec tous les indices rayons : pourra se mettre dans une fonction externe pour ne pas être répeté à chaque boucle
    for (j = 0; j<orig.size() ; j+=3)
    {
        m_vectBoite[0].m_numRayon.push_back(j);
    }

    // Pour chaque boite : chargement de l'indice des rayons qui intersectent avec elle
    for (i = 1 ; i < m_vectBoite.size() ; i++)
    {
        // boite i en test : m_vectBoite[i]

        // On supprime les indices des rayons précedemment enregistrés
        m_vectBoite[i].m_numRayon.clear();

        boitePere = m_vectBoite[m_vectBoite[i].m_indicePere];

        // Pour tous les rayons contenus dans la boite père.
        for (j = boitePere.m_numRayon.size()-1 ; j >=0  ; j--)
        {
            ind = boitePere.m_numRayon[j];

            // test intersection entre rayon ind et boite i
            if (intersecBoiteRay(m_vectBoite[i], orig, dir, ind))
            {
                m_vectBoite[i].chargerRay(ind); // Ajout des rayons à la boite courante
                // On ne supprime pas de la boite père car il se peut qu'il y est des faces stockées dedans
                //m_vectBoite[m_vectBoite[i].m_indicePere].supprRay(j); // Suppression des rayons de la boite père
                //rayonStock.push_back(ind); // Ajout au vecteur de stockage
            }
        }
/*
        // Vérification - Pour chaque étage on compte les rayons
        if (m_vectBoite[i+1].m_arrete == m_vectBoite[i].m_arrete)
        {
            nbRay+=m_vectBoite[i].m_numRayon.size();
        }
        else // Si à la boite suivante on change d'étage
        {
            qDebug() << "nombre rayons :" << nbRay;
            nbRay = 0;
        }
    }
*/

}

bool intersecBoiteRay(Boite &boite, std::vector<float>& orig, std::vector<float>& dir, int indice)
{
    CoordVector centre = boite.m_centre;
    float r = boite.m_arrete;
    float t0x, t0y, t0z, t1x, t1y, t1z;

    t0x =  centre.x - r;
    t0y =  centre.y - r;
    t0z =  centre.z - r;
    t1x =  centre.x + r;
    t1y =  centre.y + r;
    t1z =  centre.z + r;

    // implementation http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection


    float invDirx = 1/ dir[indice];
    float invDiry = 1/ dir[indice+1];
    float invDirz = 1/ dir[indice+2];

    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    if (invDirx >=0)
    {
        tmin = (t0x - orig[indice]) * invDirx;
        tmax = (t1x - orig[indice]) * invDirx;
    }
    else
    {
        tmax = (t0x - orig[indice]) * invDirx;
        tmin = (t1x - orig[indice]) * invDirx;
    }

    if (invDiry >=0)
    {
        tymin = (t0y - orig[indice+1]) * invDiry;
        tymax = (t1y - orig[indice+1]) * invDiry;
    }
    else
    {
        tymax = (t0y - orig[indice+1]) * invDiry;
        tymin = (t1y - orig[indice+1]) * invDiry;
    }

   if ((tmin > tymax) || (tymin > tmax))
   return false;

   if (tymin > tmin)
   tmin = tymin;
   if (tymax < tmax)
   tmax = tymax;

   if(invDirz >= 0)
   {
       tzmin = (t0z - orig[indice+2]) * invDirz;
       tzmax = (t1z - orig[indice+2]) * invDirz;
   }
   else
   {
       tzmax = (t0z - orig[indice+2]) * invDirz;
       tzmin = (t1z - orig[indice+2]) * invDirz;
   }

   if ((tmin > tzmax) || (tzmin > tmax))
   return false;


   return true;

}
