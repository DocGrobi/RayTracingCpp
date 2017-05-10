#include "objwriter.h"
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QIODevice>
#include "QDebug"
#include <QtMath>
#include <QMessageBox>


QString ObjWriter::CoordVector2QString(CoordVector coord)
{
    QString text = QString::number(coord.x) + " " + QString::number(coord.y) + " " + QString::number(coord.z);
     return text;
}

ObjWriter::ObjWriter(QString chemin) // recupere en attribue le nom de chemin de fichier specifié
{

    QFile fichier(chemin);
    int i=0;
    QString newName(chemin);

    //suppression du fichier s'il existe deja
    if(!fichier.remove())
        QMessageBox::critical(NULL,"Erreur","Impossible de supprimer le fichier !");

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

void ObjWriter::display_normales(std::vector<float> vertex, std::vector<float> normals, int nData)
{
    QFile fichier(m_chemin);

    fichier.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier

    // creation d'un entete
    QString text("o Normales \n");
    fichier.write(text.toLatin1());

    std::vector<float> difference; // vérification calcul de normales

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


            difference.push_back(nor[j]-(moy + normals[i+j])); // vérification sens des normales
        }
        // */

        vertices = vertices + "\n"; // retour la la ligne
        fichier.write(vertices.toLatin1()); // ecriture d'une ligne : v x1 y1 z1
    }


    // ecriture des lignes commençant par l pour relier les vertex
    QString ligne("");
    for(int i=0 ; i < 2*nData/9 ; i=i+2)
    {
        ligne = "l " + QString::number(i+1) + " " + QString::number(i+2) + "\n";
        fichier.write(ligne.toLatin1());
    }

    fichier.close(); // ferme le fichier


    //pour le debug
    if (*std::max_element(difference.begin(),difference.end()) < 0,000001)
    {
        qDebug() << "Les normales sont dans le bon sens";
    }
    else
    {
        qDebug() << "Erreur calcul de normales";
    }

    difference.clear();
}

void ObjWriter::display_ray(Source source, std::vector<float> ray, int nbRay, int nb_rebond)
{
    QFile fichier(m_chemin);

    fichier.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier

    // creation d'un entete
    QString text("o Rayons \n");
    fichier.write(text.toLatin1());

    int nbCoord = nbRay*3; // nombre de données pour un ordre (ordre 0, ordre 1, etc)
    int ordre = nb_rebond + 1;

    // ecriture des vertex par "étage" : source, puis rebond 1, etc
    for (int i = 0; i < nbCoord*ordre ; i=i+3)
    {
        CoordVector vertCoord(ray[i], ray[i+1], ray[i+2]);
        text = "v "+ CoordVector2QString(vertCoord) + "\n";
        fichier.write(text.toLatin1());
    }

    // ecriture des lignes commençant par l pour relier les vertex
    QString ligne("");

    for(int j = 0 ; j < nb_rebond ; j++) // on décale du nb rayon pour executer la boucle suivante à l'ordre +1
    {
        for(int i = 1; i<= nbRay ; i++) // on lit les ligne de vertex dans l'ordre pour chaque rayon
        {
            ligne = "l " + QString::number(nbRay*j+i) + " " + QString::number(nbRay*(j+1)+i) + "\n";
            fichier.write(ligne.toLatin1());
        }
    }

    fichier.close(); // ferme le fichier
}





void ObjWriter::rec_Vert(Source source, std::vector<float> ray, int nbRay,int nbRebond, int num_rebond)
{
    QFile fichier(m_chemin);

    if(fichier.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) // ouvre le fichier
    {
        // creation d'un entete
        QString text("o Rayons \n");
        if (num_rebond == 0)
        {
            // coordonnées du premier point
            text = text + "v " + CoordVector2QString(source.centre()) + "\n";
            fichier.write(text.toLatin1());
        }

        // ecriture des vertex pour tous les rebonds
        for (int i = 0; i < nbRay*3 ; i=i+3) // on n'ecrit que le deuxième point
        {
            CoordVector vertCoord(ray[i], ray[i+1], ray[i+2]);
            text = "v "+ CoordVector2QString(vertCoord) + "\n";
            fichier.write(text.toLatin1());
        }

        if (num_rebond == nbRebond-1)
        {
            // ecriture des derniers vertex
            for (int i = nbRay*3; i < nbRay*6 ; i=i+3) // on n'ecrit que le deuxième point
            {
                CoordVector vertCoord(ray[i], ray[i+1], ray[i+2]);
                text = "v "+ CoordVector2QString(vertCoord) + "\n";
                fichier.write(text.toLatin1());
            }
        }
    }
    fichier.close(); // ferme le fichier
}


void ObjWriter::rec_Line(int nbRay, int nbRebond)
{
    QFile fichier(m_chemin);
    QString ligne;

    if(fichier.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) // ouvre le fichier et place le curseur à la fin
    {
      // relier les premier point à la source
        for (int i = 0 ; i<nbRay ; i++)
        {
            ligne = "l 1 " + QString::number(i+2) + "\n";
            fichier.write(ligne.toLatin1());
        }
      // relier les points suivent deux par deux
        for (int i = 0 ; i<nbRay*nbRebond ; i++)
        {
            ligne = "l " + QString::number(i+1) + " " + QString::number(nbRay+1+i) + "\n";
            fichier.write(ligne.toLatin1());
        }
    }

    fichier.close(); // ferme le fichier
}
