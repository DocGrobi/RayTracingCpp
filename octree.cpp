#include "octree.h"
#include "QDebug"

//methodes


// classes
Octree::Octree(Ray monRay, MeshObj monMesh)
{

    // création du cube racine
    std::vector<float> vertex = monMesh.getVertex();
    float xMin(0), xMax(0), yMin(0), yMax(0), zMin(0), zMax(0);
    for (int i = 0 ; i <vertex.size() ; i=i+3)
    {
        if(vertex[i]<xMin)
        {
            xMin = vertex[i];
        }
        if(vertex[i]>xMax)
        {
            xMax = vertex[i];
        }

        if(vertex[i+1]<yMin)
        {
            yMin = vertex[i+1];
        }
        if(vertex[i+1]>yMax)
        {
            yMax = vertex[i+1];
        }

        if(vertex[i+2]<zMin)
        {
            zMin = vertex[i+2];
        }
        if(vertex[i+2]>zMax)
        {
            zMax = vertex[i+2];
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

    Noeud root(centre, rayon);


    qDebug() << rayon;

}

Octree::~Octree()
{

}

Noeud::Noeud(CoordVector centre, float rayon)
{

}

Noeud::~Noeud()
{

}
CoordVector Noeud::getCentre(){
    return m_centre;
}

float Noeud::getRayon(){
    return m_rayon;
}
