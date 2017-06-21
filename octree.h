#ifndef OCTREE_H
#define OCTREE_H

#include "objreader.h"
#include "raytracing.h"

// classes
class Boite{
public:
    Boite();// Constructeur de base
    Boite(const CoordVector &centre, float rayon, int indPere); // Constructeur surchargé
    ~Boite();
    Boite operator=(const Boite &boite);
    /*
    CoordVector getCentre();
    float getRayon();
    int getIndPere();
    */
    void chargerElt(int indice);
    void supprElt(int position);
    int getNbElt();
    std::vector<int>& getVectNumElt();


//private:
    float m_rayon;
    CoordVector m_centre;
    int m_indicePere;
    std::vector<int> m_numElt;
};


class Octree{
public:
    Octree(MeshObj monMesh);
    ~Octree();
    std::vector<Boite>& getVectBoite();


private:
    std::vector<Boite> m_vectBoite;
};



// methodes
void etagesuivant(std::vector<float>& vert, std::vector<Boite>& vectBoite, int indicePere);
bool appartientBoite(Boite boite, float valeur);
std::vector<Boite> decoupage(Boite &boitePere);
bool estUneFeuille(Boite boite, int seuil);

#endif // OCTREE_H
