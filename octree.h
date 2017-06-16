#ifndef OCTREE_H
#define OCTREE_H

#include "objreader.h"
#include "raytracing.h"

// classes
class Octree{
public:
    Octree(Ray monRay, MeshObj monMesh);
    ~Octree();
    std::vector<int>& getindCube();

private:
    std::vector<int> indCube;
};
/*
class Noeud{
public:
    Noeud(float centre, float rayon);
    ~Noeud;
    float getCentre();
    float getRayon();

private:
    float m_rayons;
};
*/
// methodes



#endif // OCTREE_H
