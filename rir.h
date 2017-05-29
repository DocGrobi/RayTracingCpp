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
        std::vector<float> getX();
        std::vector<float> getY();
        void addSourcesImages(Ray rayon, Listener listener);
        void filtrerSourceImages();
        void calculerRIR(int f_ech);

    private:
        std::vector<float> m_sourcesImages;
        std::vector<float> m_nrgSI;
        std::vector<int> m_nbSI;
        std::vector<float> m_sourcesImages_Filtrees;
        std::vector<float> m_nrgSI_Filtrees;
        std::vector<float> m_x;
        std::vector<float> m_y;
        std::vector<float> m_sourcesImages_Tps;
        float m_xMax;
};

#endif // RIR_H
