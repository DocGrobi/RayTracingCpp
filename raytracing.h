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
    std::vector<float> getNRG() const;
    std::vector<float> getPos() const;
    std::vector<float> getDir() const;
    std::vector<float> getDist() const;
    std::vector<float> getLong() const;

    int getNbRay() const;

    void rebond(MeshObj mesh, int nb_rebond);
    bool rebondSansMemoire(MeshObj mesh, float seuil);

    void stockage();


private:    

    std::vector<int> m_col;     // Donnees de collision des rayons sur le maillage [iray, ielt, vray]    
    float m_dMax;               // Distance max parcourue par les rayons

    // Utilisés
    int m_Nray;                 // Nombre de rayon * 3 (car trois coordonnées)
    CoordVector m_src;          // Position absolue de la source
    std::vector<float> m_ray;   // vecteur stockant les vecteur directeur des rayons
    std::vector<float> m_dist;  // Distance parcourue par les rayons au moment t
    std::vector<float> m_nrg;   // Energie portee par le rayon

    std::vector<float> m_dir;   // Direction unitaire des rayons stockée
    std::vector<float> m_pos;   // Position du point de départ du rayons stockée
    std::vector<float> m_angle;
    std::vector<float> m_long;  // longueur du dernier segment de rayon

};


#endif // RAYTRACING_H
