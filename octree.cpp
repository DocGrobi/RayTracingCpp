#include "octree.h"
#include "QDebug"
#include <math.h>

//methodes


// classes
Octree::Octree(MeshObj monMesh, int nbFaceFeuille)
{

    // Declaration
    int seuil = nbFaceFeuille;
    int k(0), i(0), j(0), nbBoiteNew, nbBoiteOld(1); // on demarre à 1 boite (boite racine)



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
    float rayon;
    CoordVector centre((xMax + xMin)/2, (yMax + yMin)/2,(zMax + zMin)/2);
    rayon = (xMax - xMin)/2;
    if ((yMax - yMin)/2 > rayon)
    {
        rayon = (yMax - yMin)/2;
    }
    if ((zMax - zMin)/2 > rayon)
    {
        rayon = (zMax - zMin)/2;
    }


    //I- Création de la boite root
    Boite boitesRacine(centre, rayon, 0);
    for (k = 0 ; k < vert.size() ; k+=9)
    {
        boitesRacine.chargerElt(k);
    }

    // II- Stockage de la boite racine
    m_vectBoite.push_back(boitesRacine);

    // III- Premiere ramification (les huit nouvelles boites sont ajoutée à m_vectBoite)
    etagesuivant(vert,m_vectBoite, 0);

    // Tant que le nombre de boite augmente
    while (m_vectBoite.size() > nbBoiteOld)
    {
        nbBoiteNew = m_vectBoite.size() - nbBoiteOld;
        nbBoiteOld = m_vectBoite.size();

        // pour chaque boite du nouvel étage
        for(i = nbBoiteOld - nbBoiteNew ; i <nbBoiteOld ; i++ )
        {
            // IV- Si la boite n'est pas une feuille : Découpage de la boite courante en huit
            if (!estUneFeuille(m_vectBoite[i], seuil))
            {
                etagesuivant(vert,m_vectBoite, i);
            }
        }
    }
}

Octree::~Octree()
{
}


std::vector<Boite>& Octree::getVectBoite(){
    return m_vectBoite;
}

Boite::Boite()
{
    m_centre = CoordVector(0,0,0);
    m_rayon = 0;
    m_indicePere = 0;
}

Boite::Boite(const CoordVector &centre, float rayon, int indPere)
{
    m_centre = centre;
    m_rayon = rayon;
    m_indicePere = indPere;
}

Boite::~Boite()
{
}

Boite Boite::operator=(const Boite &boite)
{
    m_centre = boite.m_centre;
    m_rayon = boite.m_rayon;
    m_indicePere = boite.m_indicePere;
    m_numElt = boite.m_numElt;

    return *this;
}
/*
CoordVector Boite::getCentre(){
    return m_centre;
}

float Boite::getRayon(){
    return m_rayon;
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

void Boite::supprElt(int position)
{
    m_numElt.erase(m_numElt.begin()+position);
}

//Methodes

void etagesuivant(std::vector<float> &vert, std::vector<Boite> &vectBoite, int indiceBoite)
{
    float r, x, y, z; // valeurs tampon
    // les valeurs tampon servent à comparer [Somme des x] à 3*centre +/-r.
    int i, k, ind;
    std::vector<int> pos; // Positions à supprimer du père
    std::vector<int> elt;

    // V- Création des 8 boites filles à partir de la boite actuelle
    std::vector<Boite> boitesFilles = decoupage(vectBoite[indiceBoite]);
    r = 3*vectBoite[indiceBoite].m_rayon/4; // valeur de comparaison tampon

    // VI- Pour chaque nouvelle boite :
    for (i = 0 ; i<8 ; i++)
    {
        // valeurs de comparaison tampon
        x = 3*boitesFilles[i].m_centre.x;
        y = 3*boitesFilles[i].m_centre.y;
        z = 3*boitesFilles[i].m_centre.z;

        elt = vectBoite[indiceBoite].getVectNumElt();

        // Pour chaque face compris dans la boite père
        for (k = elt.size()-1; k >=0  ; k--)
        {
            ind = elt[k];
            if (fabs(vert[ind]  +vert[ind+3]+vert[ind+6] - x) <= r
             && fabs(vert[ind+1]+vert[ind+4]+vert[ind+7] - y) <= r
             && fabs(vert[ind+2]+vert[ind+5]+vert[ind+8] - z) <= r)
            {
                boitesFilles[i].chargerElt(ind); // Si la face est inclue dans la boite fille en test on stock son indice
                vectBoite[indiceBoite].supprElt(k);
                //pos.push_back(k);// On stock la position pour retirer l'indice de la boite pere
            }
        }
        vectBoite.push_back(boitesFilles[i]); // Ajout de la boite fille
    }
    /*
    // suppression des elt utilisées dans les boites filles de la boite pere
    for(int j = pos.size()-1 ; j>=0 ; j--)
    {
        vectBoite[indiceBoite].supprElt(pos[j]);
    }
    */
}

bool estUneFeuille(Boite boite, int seuil)
{
    if(boite.getNbElt()<seuil)
    {
        return true;
    }
    else{return false;}
}


std::vector<Boite> decoupage(Boite &boitePere)
{
    // Recuperation des données du père
    CoordVector centre = boitePere.m_centre;
    float rayon = boitePere.m_rayon;
    int indicePere = boitePere.m_indicePere;

    // Initialisation du vecteur de boites filles
    std::vector<Boite> boitesFilles;
    //Boite boite0(CoordVector(0,0,0),0,0);
    boitesFilles.resize(8, Boite());

    // Remplissage des boites filles (code binaire pour le centre)
    boitesFilles[0] = Boite(CoordVector(centre.x - rayon/2, centre.y - rayon/2, centre.z - rayon/2), rayon/2, indicePere);
    boitesFilles[1] = Boite(CoordVector(centre.x + rayon/2, centre.y - rayon/2, centre.z - rayon/2), rayon/2, indicePere);
    boitesFilles[2] = Boite(CoordVector(centre.x - rayon/2, centre.y + rayon/2, centre.z - rayon/2), rayon/2, indicePere);
    boitesFilles[3] = Boite(CoordVector(centre.x + rayon/2, centre.y + rayon/2, centre.z - rayon/2), rayon/2, indicePere);
    boitesFilles[4] = Boite(CoordVector(centre.x - rayon/2, centre.y - rayon/2, centre.z + rayon/2), rayon/2, indicePere);
    boitesFilles[5] = Boite(CoordVector(centre.x + rayon/2, centre.y - rayon/2, centre.z + rayon/2), rayon/2, indicePere);
    boitesFilles[6] = Boite(CoordVector(centre.x - rayon/2, centre.y + rayon/2, centre.z + rayon/2), rayon/2, indicePere);
    boitesFilles[7] = Boite(CoordVector(centre.x + rayon/2, centre.y + rayon/2, centre.z + rayon/2), rayon/2, indicePere);

    return boitesFilles;

}
