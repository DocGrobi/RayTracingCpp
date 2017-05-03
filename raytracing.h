#ifndef RAYTRACING_H
#define RAYTRACING_H

#include "objreader.h"


// Méthodes
CoordVector intersection(CoordVector point_ray, CoordVector vect_dir, CoordVector vect_norm, float k);
bool appartient_face(CoordVector point, std::vector<float> face);
CoordVector vecteur_reflechi(CoordVector i, CoordVector n);

// Les classes
class Ray
{
public:
    Ray(float phy, int Nray, Source S);   //Constructeur
    ~Ray();                                 //Destructeur
    std::vector<float> getRay() const;
    void rebond(MeshObj mesh, int nb_rebond);


private:
    int m_Nray;                 // Nombre de rayon
    CoordVector m_dir;          // Direction unitaire des rayons
    CoordVector m_pos;          // Position absolue des rayons
    std::vector<int> m_col;     // Donnees de collision des rayons sur le maillage [iray, ielt, vray]
    std::vector<float> m_dist;  // Distance parcourue par les rayons
    float m_dMax;               // Distance max parcourue par les rayons
    std::vector<float> m_nrg;   // Energie portee par le rayon
    CoordVector m_src;          // Position absolue de la source

    //std::vector<float> m_dirTot; // Vecteur des directions des rayons
    std::vector<float> m_ray;   // vecteur stockant les vecteur directeur des rayons

};


#endif // RAYTRACING_H
