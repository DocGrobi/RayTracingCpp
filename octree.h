#ifndef OCTREE_H
#define OCTREE_H

#include "objreader.h"


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
    void chargerRay(int indice);
    void supprElt(int position);
    void supprRay(int position);
    int getNbElt();
    std::vector<int>& getVectNumElt();



//private:
    float m_rayon;
    CoordVector m_centre;
    int m_indicePere;
    int m_indiceBoite;
    std::vector<int> m_numRayon;
    std::vector<int> m_numElt;
};


class Octree{
public:
    Octree();
    Octree(MeshObj monMesh, int nbFaceFeuille);
    ~Octree();
    Octree operator=(const Octree &oct);
    std::vector<Boite> getVectBoite() const;
    void chargerRayon(std::vector<float> &orig, std::vector<float> &dir);


private:
    std::vector<Boite> m_vectBoite;
};



// methodes
void etagesuivant(std::vector<float>& vert, std::vector<Boite>& vectBoite, int indicePere);
bool appartientBoite(Boite boite, float valeur);
std::vector<Boite> decoupage(Boite &boitePere);
bool estUneFeuille(Boite boite, int seuil);
bool intersecBoiteRay(Boite &boite, std::vector<float>& orig, std::vector<float>& dir, int indice);

#endif // OCTREE_H
