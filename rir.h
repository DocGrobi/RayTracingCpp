#ifndef RIR_H
#define RIR_H

#include "fonction.h"
#include "objreader.h"
#include "raytracing.h"

// les méthodes
std::vector<float> toucheListener(Ray &rayon, Listener &listener);
std::vector<float> toucheListener2(Ray &rayon, Listener &listener);
void partitionnage(std::vector<std::vector<float> > &fir, std::vector<std::vector<float> > &firPart, int taille);
void unite(std::vector< CoordVector> &si_in, std::vector< CoordVector> &si_out, std::vector<float> &nrg_in, std::vector<float> &nrg_out, float distance);


// Les classes

class SourceImage
{
    public:
        SourceImage();
        ~SourceImage();
        std::vector<CoordVector> &getSourcesImages();
        std::vector<float> &getNrgSI();
        std::vector<float> &getSI_Tps();
        std::vector<float> &getX();
        std::vector<std::vector<float> > &getY();
        bool addSourcesImages(Ray &rayon, Listener &listener, float longueurMax, const std::vector<float> &absAir, float seuil);
        bool calculerRIR(int f_ech);
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
        std::vector<std::vector<float> > m_y;
        std::vector<float> m_sourcesImages_Tps;
        float m_xMax;
        std::vector< std::vector<float> > m_FIR;
        std::vector< std::vector<float> > m_firPart;
};

#endif // RIR_H
