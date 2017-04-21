#include "objwriter.h"
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QIODevice>
#include "QDebug"
#include <QtMath>

ObjWriter::ObjWriter(QString chemin) // recupere en attribue le nom de chemin de fichier specifié
{

    QFile fichier(chemin);
    int i=0;
    QString newName(chemin);

    /*
    while(fichier.exists()) // incrementation de version de fichier s'il existe deja
    {
        //qDebug() << fichier.fileName() << "existe !";
        QStringList nom = fichier.fileName().split(".obj");
        if (nom[0].contains("EXPORT_"))
        {
            QStringList nom2 = nom[0].split("EXPORT_");
            newName = nom2[0] + "EXPORT_" + QString::number(i) + ".obj" ;
        }
        else
        {
            newName = nom[0] + "_" + QString::number(i) + ".obj" ;
        }

        i++;
        fichier.setFileName(newName);
        //qDebug() << "newname" << newName ;
    }
    */
      m_chemin = newName; // attribution du dernier nom
}

ObjWriter::~ObjWriter()
{
}

void ObjWriter::display_normales(float *vertex, float *normals, int nData)
{
    QFile fichier(m_chemin);

    fichier.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier

    // creation d'un entete
    QString text("o Normales \n");
    fichier.write(text.toLatin1());

    // ecriture des coordonnées de vertex représentant les normales :
    for(int i=0; i<nData ; i=i+9) // incrementation par face
    {
        QString vertices("v");
        for (int j = 0; j <3 ; j++) // incrementation par coordonnée du centre de la face
        {
            float moy((vertex[i+j]+vertex[i+j+3]+vertex[i+j+6])/3); // moyenne des x, des y et des z
            vertices = vertices + " " + QString::number(moy);

        }
        vertices = vertices + "\n"; // retour la la ligne
        fichier.write(vertices.toLatin1()); // ecriture d'une ligne : v x1 y1 z1

        vertices= "v";



        // OPTION 1 - Utilisation des normales dans le fichier .obj
        /*
        for (int j = 0; j <3 ; j++) // incrementation par coordonnée du bout de la normale
        {
            float moy((vertex[i+j]+vertex[i+j+3]+vertex[i+j+6])/3); // moyenne des x, des y et des z  
            float nor(moy + normals[i+j]); // ajout de la valeur de normale

            vertices = vertices + " " + QString::number(nor);
        }
        */



        // OPTION 2 - Utilisation de l'ordre des vertices pour créer les normales
        // /*
        std::vector<float> nor(3, 0);

        nor[0] = (vertex[i+4]-vertex[i+1])*(vertex[i+8]-vertex[i+2]) - (vertex[i+5]-vertex[i+2])*(vertex[i+7]-vertex[i+1])  ;
        nor[1] = (vertex[i+5]-vertex[i+2])*(vertex[i+6]-vertex[i]) - (vertex[i+3]-vertex[i])*(vertex[i+8]-vertex[i+2]) ;
        nor[2] = (vertex[i+3]-vertex[i])*(vertex[i+7]-vertex[i+1]) - (vertex[i+4]-vertex[i+1])*(vertex[i+6]-vertex[i]) ;
        float norme = sqrt(pow(nor[0], 2) + pow(nor[1], 2) + pow(nor[2], 2));

        for (int j = 0; j <3 ; j++) // incrementation par coordonnée du bout de la normale
        {
            float moy((vertex[i+j]+vertex[i+j+3]+vertex[i+j+6])/3); // moyenne des x, des y et des z
            nor[j] = (nor[j] / norme) + moy;

            vertices = vertices + " " + QString::number(nor[j]);
        }
        // */


        vertices = vertices + "\n"; // retour la la ligne
        fichier.write(vertices.toLatin1()); // ecriture d'une ligne : v x1 y1 z1
    }
    //qDebug() << sqrt(9);

    // ecriture des lignes commençant par l pour relier les vertex
    QString ligne("");
    for(int i=0 ; i < 2*nData/9 ; i=i+2)
    {
        ligne = "l " + QString::number(i+1) + " " + QString::number(i+2) + "\n";
        fichier.write(ligne.toLatin1());
    }

    fichier.close(); // ferme le fichier
}

void ObjWriter::display_ray(Source source, float *ray, int nbRay)
{
    QFile fichier(m_chemin);

    fichier.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier

    // creation d'un entete
    QString text("o Rayons \n");
    fichier.write(text.toLatin1());

    // ecriture des vertex du centre et du vecteur de rayon
    for (int i = 0 ; i < (3*nbRay) ; i=i+3)
    {
        CoordVector rayCoord(ray[i], ray[i+1], ray[i+2]);
        text = "v " + CoordVector2QString(source.centre()) + "\n" + "v "+ CoordVector2QString(rayCoord) + "\n";
        fichier.write(text.toLatin1());

    }

    // ecriture des lignes commençant par l pour relier les vertex
    QString ligne("");
    for(int i=0 ; i < (2*nbRay) ; i=i+2)
    {
        ligne = "l " + QString::number(i+1) + " " + QString::number(i+2) + "\n";
        fichier.write(ligne.toLatin1());
    }

    fichier.close(); // ferme le fichier
}


QString ObjWriter::CoordVector2QString(CoordVector coord)
{
    QString text = QString::number(coord.x) + " " + QString::number(coord.y) + " " + QString::number(coord.z);
     return text;
}
