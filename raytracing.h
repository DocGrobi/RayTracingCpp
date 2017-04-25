#ifndef RAYTRACING_H
#define RAYTRACING_H



#include "objreader.h"
#include <vector>


// Méthodes
CoordVector sph2cart(float ro, float theta, float phi); // convertion coordonnée spherique à cartesiens

// Les classes
class Ray
{
public:
    Ray(float phy, int N, CoordVector S);   //Constructeur
    ~Ray();                                 //Destructeur
    std::vector<float> getRay() const;

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
    std::vector<float> m_ray;                // Poiteur vers le tableau de stockage des vecteurs rayons

};


#endif // RAYTRACING_H
