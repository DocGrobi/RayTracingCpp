#ifndef RIR_H
#define RIR_H

#include "fonction.h"
#include "objreader.h"
#include "raytracing.h"

// les méthodes
std::vector<bool> toucheListener(Ray &rayon, Listener &listener);

// Les classes

class SourceImage
{
    public:
        SourceImage();
        ~SourceImage();
        std::vector<CoordVector> &getSourcesImages();
        std::vector<float> &getNrgSI();
        std::vector<float> &getX();
        std::vector<float> &getY();
        void addSourcesImages(Ray &rayon, Listener &listener, float longueurMax, bool rayAuto, const std::vector<float> &absAir);
        void filtrerSourceImages();
        bool calculerRIR(int f_ech);
        int redimentionnement(int taille);
        void partitionnage(int taille);
        std::vector< std::vector<float> >&getFIR();
        std::vector< std::vector<float> >&getFirPart();

    private:
        std::vector<CoordVector> m_sourcesImages;
        std::vector<float> m_nrgSI;
        std::vector<int> m_nbSI;
        std::vector<float> m_sourcesImages_Filtrees;
        std::vector<float> m_nrgSI_Filtrees;
        std::vector<float> m_x;
        std::vector<float> m_y;
        std::vector<float> m_sourcesImages_Tps;
        float m_xMax;
        std::vector< std::vector<float> > m_FIR;
        std::vector< std::vector<float> > m_firPart;
};

#endif // RIR_H
