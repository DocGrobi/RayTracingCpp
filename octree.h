#ifndef OCTREE_H
#define OCTREE_H

#include "objreader.h"


// classes
class Boite{
public:
    Boite();// Constructeur de base
    Boite(const CoordVector &centre, float arrete, int indPere); // Constructeur surchargé
    ~Boite();
    Boite operator=(const Boite &boite);
    /*
    CoordVector getCentre();
    float getRayon();
    int getIndPere();
    */
    void chargerElt(const std::vector<float> &vert, int indice);
    void chargerRay(int indice);
    void supprElt(int position);
    void supprRay(int position);
    int getNbElt();
    //bool estUneFeuille();
    std::vector<int>& getVectNumElt();



//private:
    float m_arrete;
    CoordVector m_centre, m_min, m_max;
    CoordVector m_coinMin;
    int m_indicePere;
    int m_indiceBoite;
    std::vector<int> m_numRayon;
    std::vector<int> m_numElt;
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
    void chargerRayon(std::vector<CoordVector> &orig, std::vector<CoordVector> &dir);
    void chargerRayonRacine(int nbRay);
    void etagesuivant(std::vector<float>& vert, int indicePere);


private:
    std::vector<Boite> m_vectBoite;
    int m_seuil;
};



// methodes
bool appartientBoite(Boite &boite, std::vector<float> &vert, int indice);
void decoupage(Boite &boitePere, std::vector<Boite> &boitesFilles); // 4fois plus long si on retourne une reference
//bool estUneFeuille(Boite boite, int seuil);
bool intersecBoiteRay(const Boite &boite, const CoordVector &orig, const CoordVector &invDir);
bool intersecSphereRay(Boite &boite, const CoordVector &orig, const CoordVector &dir);

#endif // OCTREE_H
