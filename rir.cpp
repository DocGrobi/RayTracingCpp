#include "rir.h"
#include "math.h"

// les méthodes
std::vector<bool> toucheListener(Ray rayon, Listener listener)
{
    std::vector<bool> resultat;
    bool hypA, hypB, hypC;

    std::vector<float> posOld, dirOld, rayNew;
    posOld = rayon.getPos();
    dirOld = rayon.getDir();
    rayNew = rayon.getRay();

    int n(rayon.getNbRay());

    CoordVector L(listener.getCentre());
    float r(listener.getRayon());



    for(int i = 0; i<rayon.getNbRay() ; i=i+3)
    {

       CoordVector A(posOld[i],posOld[i+1],posOld[i+2]);
       CoordVector B(rayNew[i],rayNew[i+1],rayNew[i+2]);
       CoordVector vectDir(rayNew[n+i],rayNew[n+i+1],rayNew[n+i+2]);

       float alpha = angle(vecteur(A,B),vecteur(A,L));

       // test sur l'angle
       if ( alpha <= asin(r/norme(vecteur(A,L))))
       {
           hypA = true;
       }
       else
       {
           hypA = false;
       }

       // test sur la direction
       if (produitScalaire(vectDir,vecteur(A,B)) > 0)
       {
           hypB = true;
       }
       else
       {
           hypB = false;
       }

       // test sur la distance
       if ( norme(vecteur(A,B)) >= norme(vecteur(A,L)) * (tan(alpha) + 1) - r)
       {
           hypC = true;
       }
       else
       {
           hypC = false;
       }

       if (hypA && hypB && hypC)
       {
           resultat.push_back(true);
       }
       else
       {
           resultat.push_back(false);
       }
    }

    return resultat;

}

CoordVector sourceImage(Ray rayon)
{

}

// La classe

SourceImage::SourceImage()
{

}

SourceImage::~SourceImage()
{

}

std::vector<float> SourceImage::getSourcesImages()
{
    return m_sourcesImages;
}

std::vector<float> SourceImage::getNrgSI()
{
    return m_nrgSI;
}

void SourceImage::addSourcesImages(Ray rayon, Listener listener)
{
    std::vector<bool> touche = toucheListener(rayon,listener);
    CoordVector C; // la source image

    std::vector<float> longueurRayonTot = rayon.getDist();
    std::vector<float> longueurRayonFin = rayon.getLong();
    std::vector<float> point = rayon.getPos();
    std::vector<float> vec = rayon.getDir();
    std::vector<float> nrg = rayon.getNRG();



    for (int i = 0 ; i< touche.size() ; i++) // rayon par rayon
    {
        if (touche[i]) // si le rayon touche le listener
        {
            CoordVector A (point[3*i], point[3*i + 1], point[3*i + 2]);
            CoordVector vect(vec[3*i], vec[3*i + 1], vec[3*i + 2]);
            float norm = norme(vect);
            float longueurRay = longueurRayonTot[i] - longueurRayonFin[i];

            C.x = -longueurRay * vect.x / norm + A.x;
            C.y = -longueurRay * vect.y / norm + A.y;
            C.z = -longueurRay * vect.z / norm + A.z;

            m_sourcesImages.push_back(C.x);
            m_sourcesImages.push_back(C.y);
            m_sourcesImages.push_back(C.z);

            m_nrgSI.push_back(nrg[i]);
        }
    }
}

