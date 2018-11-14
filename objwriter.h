#ifndef OBJWRITER_H
#define OBJWRITER_H



#include<QString>
#include "objreader.h"
#include "raytracing.h"
#include "rir.h"
#include "octree.h"

class ObjWriter
{
    public:
    ObjWriter(QString chemin, int nbRay); //recupere en attribue le nom de chemin de fichier specifié
    ~ObjWriter(); //destructeur
    void rec_Vert(Source &source, int nSrc, Ray &monRay, int num_rebond);
    void rec_Line(int nbRay);
    void display_sourceImages(std::vector<CoordVector> &sourcesImages);
    void display_octree(const std::vector<Boite> &oct);
    void display_coloredTriangle(std::vector<CoordVector> &point, std::vector<float> &nrg, const CoordVector &dirNormal, const CoordVector &posSource, float seuil);

    void display_Beam_init();
    void display_Beam_vert(Ray &rayon, Listener &listener);
    void display_Beam_line();

    void rec_Vert(Ray &monRay);
    void rec_Vert(Ray &monRay, int ind, CoordVector source);
    void rec_Vert_init(std::vector<CoordVector> &si);


private:
    QString m_chemin;
    std::vector<bool> m_rayMort;
    std::vector<int> m_buff_rayMort;
    int m_nbligne;
    int m_nblignefin;

};

std::vector<CoordVector> coordVertBoite(const Boite &boite);
QString HSV2RGB(float h, float s, float v);
void genererMTL(float min);
void RotateX(CoordVector &P, float ang);
void RotateY(CoordVector &P, float ang);
void Translate(CoordVector &P,CoordVector V);
void makeSplat(CoordVector &P, CoordVector pos,CoordVector nor);


#endif // OBJWRITER_H
