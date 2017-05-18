#ifndef RIR_H
#define RIR_H

#include "fonction.h"
#include "objreader.h"
#include "raytracing.h"

// les méthodes
std::vector<bool> toucheListener(Ray rayon, Listener listener);

// Les classes

class SourceImage
{
    public:
        SourceImage();
        ~SourceImage();
        std::vector<float> getSourcesImages();
        std::vector<float> getNrgSI();
        void addSourcesImages(Ray rayon, Listener listener);

    private:
        std::vector<float> m_sourcesImages;
        std::vector<float> m_nrgSI;
};

#endif // RIR_H
