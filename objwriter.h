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
    void display_normales(float *vertex, float *normals, int nData); //methode créant un fichier obj comprant des edge représentant toutes les normales du MeshObj stipulé

private:
    QString m_chemin;
};

/*
class NormalDisplayer
{
public:
    NormalDisplayer();
    ~NormalDisplayer();
};
*/
