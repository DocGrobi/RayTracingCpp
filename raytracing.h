#ifndef RAYTRACING_H
#define RAYTRACING_H

#include "objreader.h"


// Méthodes
CoordVector intersection(const CoordVector &point_ray, const CoordVector &vect_dir, const CoordVector &vect_norm, float k);
std::vector<float> intersection(std::vector<float>& point_ray, std::vector<float>& vect_dir,int indRay, std::vector<float>& vect_norm, int indFace, float k);
bool appartient_face(const CoordVector &point, std::vector<float>& face);
bool appartient_face(std::vector<float>& point, int iP, std::vector<float>& face, int iF);
bool appartient_face(const CoordVector &point, const CoordVector &a, const CoordVector &b, const CoordVector &c);
std::vector<float> vecteur_reflechi(std::vector<float> &i, int ii, std::vector<float> &n, int in);
CoordVector vecteur_reflechi(const CoordVector &i, const CoordVector &n);

float triangle_intersection(const CoordVector &orig, const CoordVector &dir,
                            const CoordVector &v0, const CoordVector &v1, const CoordVector &v2);
float triangle_intersection(const Vect3f &orig, const Vect3f &dir,
                            const Vect3f &v0, const Vect3f &v1, const Vect3f &v2);

Vect3f vecteur_reflechi(const Vect3f &i, const Vect3f &n);


// Les classes
class Ray
{
public:
    Ray(int Nray, Source S, bool fibonacci);   //Constructeur
    ~Ray();                               //Destructeur
    std::vector<float>& getRay() ;
    std::vector<float>& getNRG() ;
    std::vector<float>& getNRGbackup() ;
    std::vector<float>& getPos() ;
    std::vector<float>& getDir() ;
    std::vector<float>& getDist() ;
    std::vector<float>& getLong() ;

    int getNbRay() const;
    int getRayMorts() const;

    void rebond(MeshObj mesh, int nb_rebond);
    bool rebondSansMemoire(MeshObj mesh, float seuil);
    bool rebondSansMemoireBis(MeshObj mesh, float seuil);

    void stockage();


private:    

    std::vector<int> m_col;     // Donnees de collision des rayons sur le maillage [iray, ielt, vray]    
    float m_dMax;               // Distance max parcourue par les rayons

    // Utilisés
    int m_Nray;                 // Nombre de rayon * 3 (car trois coordonnées)
    CoordVector m_src;          // Position absolue de la source
    std::vector<float> m_nrg;   // Energie portee par le rayon au moment t
    std::vector<float> m_nrgBackup;   // Energie portee par le rayon

    std::vector<float> m_pos;   // Position du point de départ du rayon stockée
    std::vector<float> m_ray;   // Position du point de départ du rayon au moment t
    std::vector<float> m_dir;   // Direction des rayons stockée
    std::vector<float> m_vDir;  // vecteur directeur des rayons au moment t
    std::vector<float> m_long;  // longueur du dernier segment de rayon
    std::vector<float> m_dist;  // Distance totale parcourue par les rayons au moment t

    std::vector<bool> m_rayVivant;  // longueur du dernier segment de rayon
    int m_nbRayMort;

    //std::vector<CoordVector> m_test;

};


#endif // RAYTRACING_H
