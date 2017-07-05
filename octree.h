#ifndef OCTREE_H
#define OCTREE_H

#include "objreader.h"


// classes
class Boite{
public:
    Boite();// Constructeur
    ~Boite();
    Boite operator=(const Boite &boite);
    void chargerElt(std::vector<CoordVector> const& vert, int indice);
    void chargerRay(int indice);
    void supprElt(int position);
    void supprRay(int position);
    int getNbElt();
    std::vector<int>& getVectNumElt();

//attribus
    float m_arrete;
    CoordVector m_min, m_max, m_coinMin;
    int m_indicePere, m_indiceBoite;
    std::vector<int> m_numRayon, m_numElt;
    bool estUneFeuille;

};


class Octree{
public:
    Octree();
    Octree(MeshObj monMesh, int nbFaceFeuille);
    ~Octree();
    Octree operator=(const Octree &oct);
    const std::vector<Boite>& getVectBoite() const;
    int getSeuil() const;
    void chargerRayon(std::vector<CoordVector> const& orig, std::vector<CoordVector> const& dir);
    void chargerRayonRacine(int nbRay);
    void etagesuivant(std::vector<CoordVector> const& vert, int indicePere);


private:
    std::vector<Boite> m_vectBoite;
    int m_seuil;
};



// methodes
bool appartientBoite(Boite &boite, std::vector<CoordVector> const& vert, int indice);
void decoupage(Boite &boitePere, std::vector<Boite> &boitesFilles); // 4fois plus long si on retourne une reference
bool intersecBoiteRay(const Boite &boite, const CoordVector &orig, const CoordVector &invDir);

#endif // OCTREE_H
