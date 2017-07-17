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
    void display_normales(std::vector<float> &vertex, std::vector<float> &normals, int nData); //methode créant un fichier obj comprant des edge représentant toutes les normales du MeshObj stipulé
    void display_normales(std::vector<CoordVector> &vertex); //methode créant un fichier obj comprant des edge représentant toutes les normales du MeshObj stipulé
    void display_ray(Source &source, std::vector<float> &ray, int nbRay, int nb_rebond);
    void rec_Vert(Source &source, int nSrc, Ray &monRay, int nbRay, int num_rebond, float seuil);
    void rec_Line(int nbRay, int nbRebond);
    void display_sourceImages(SourceImage &srcImg,float seuil);
    void display_octree(const std::vector<Boite> &oct);

private:
    QString m_chemin;
    std::vector<int> m_rayMort;
    std::vector<int> m_buff_rayMort;

};

std::vector<CoordVector> coordVertBoite(const Boite &boite);

#endif // OBJWRITER_H
