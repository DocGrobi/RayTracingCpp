#ifndef OBJWRITER_H
#define OBJWRITER_H

#endif // OBJWRITER_H

#include<QString>
#include "objreader.h"

class ObjWriter
{
    public:
    ObjWriter(QString chemin); //recupere en attribue le nom de chemin de fichier specifié
    ~ObjWriter(); //destructeur
    void display_normales(std::vector<float> vertex, std::vector<float> normals, int nData); //methode créant un fichier obj comprant des edge représentant toutes les normales du MeshObj stipulé
    void display_ray(Source source, std::vector<float> ray, int nbRay, int nb_rebond);
    QString CoordVector2QString(CoordVector coord);    
    void rec_Vert(Source source, std::vector<float> ray, int nbRay,int nbRebond, int num_rebond);
    void rec_Line(int nbRay, int nbRebond);

private:
    QString m_chemin;
};


