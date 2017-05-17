#ifndef OBJWRITER_H
#define OBJWRITER_H

#endif // OBJWRITER_H

#include<QString>
#include "objreader.h"
#include "raytracing.h"

class ObjWriter
{
    public:
    ObjWriter(QString chemin, int nbRay); //recupere en attribue le nom de chemin de fichier specifié
    ~ObjWriter(); //destructeur
    void display_normales(std::vector<float> vertex, std::vector<float> normals, int nData); //methode créant un fichier obj comprant des edge représentant toutes les normales du MeshObj stipulé
    void display_ray(Source source, std::vector<float> ray, int nbRay, int nb_rebond);
    QString CoordVector2QString(CoordVector coord);    
    void rec_Vert(Source source, Ray monRay, int nbRay, int num_rebond, float seuil);
    void rec_Line(int nbRay, int nbRebond);

private:
    QString m_chemin;
    std::vector<int> m_rayMort;
    std::vector<int> m_buff_rayMort;

};


