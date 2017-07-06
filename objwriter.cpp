#include "objwriter.h"
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QIODevice>
#include "QDebug"
#include <QtMath>
#include <QMessageBox>


QString ObjWriter::CoordVector2QString(const CoordVector &coord)
{
    QString text = QString::number(coord.x) + " " + QString::number(coord.y) + " " + QString::number(coord.z);
     return text;
}

ObjWriter::ObjWriter(QString chemin, int nbRay) // recupere en attribue le nom de chemin de fichier specifié
{

    QFile fichier(chemin);

    QString newName(chemin);
    m_buff_rayMort.resize(nbRay, 0); // 0 = rayon vivant

    //suppression du fichier s'il existe deja
    if(!fichier.remove())
        QMessageBox::critical(NULL,"Erreur","Impossible de supprimer le fichier !");

    // A CONSERVER : INCREMENTATION DES FICHIERS
/*
    int i=0;
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

void ObjWriter::display_normales(std::vector<float> &vertex, std::vector<float> &normals, int nData)
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

void ObjWriter::display_normales(std::vector<CoordVector> &vertex)
{
    QFile fichier(m_chemin);

    fichier.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier

    // creation d'un entete
    QString text("o Normales \n");
    fichier.write(text.toLatin1());

    QString vertices;

    CoordVector centre;

    // ecriture des coordonnées de vertex représentant les normales :
    for(int i=0; i<vertex.size() ; i+=3) // incrementation par face
    {
        centre = (vertex[i]+vertex[i+1]+vertex[i+2])/3;
        vertices += "v " + CoordVector2QString(centre)+ "\n"; // retour la la ligne

        CoordVector nor = produitVectoriel(vecteur(vertex[i],vertex[i+1]), vecteur(vertex[i],vertex[i+2])); // normale à la face
        nor = nor/norme(nor);

        vertices += "v " + CoordVector2QString(centre+nor)+ "\n"; // retour la la ligne

    }

    fichier.write(vertices.toLatin1()); // ecriture d'une ligne : v x1 y1 z1

    // ecriture des lignes commençant par l pour relier les vertex
    QString ligne("");
    for(int i=0 ; i < 2*vertex.size()/3 ; i+=2)
    {
        ligne = "l " + QString::number(i+1) + " " + QString::number(i+2) + "\n";
        fichier.write(ligne.toLatin1());
    }

    fichier.close(); // ferme le fichier
}

void ObjWriter::display_ray(Source &source, std::vector<float> &ray, int nbRay, int nb_rebond)
{
    QFile fichier(m_chemin);

    fichier.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier

    // creation d'un entete
    QString text("o Rayons \n");
    fichier.write(text.toLatin1());

    int nbCoord = nbRay; // nombre de données pour un ordre (ordre 0, ordre 1, etc)
    int ordre = nb_rebond + 1;

    // ecriture des vertex par "étage" : source, puis rebond 1, etc
    for (int i = 0; i < nbCoord*ordre ; i++)
    {
        //CoordVector vertCoord(ray[i], ray[i+1], ray[i+2]);

        text = "v "+ CoordVector2QString(ray[i]) + "\n";
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


void ObjWriter::rec_Vert(Source &source, Ray &monRay, int nbRay, int num_rebond, float seuil)
{
    QFile fichier(m_chemin);
    std::vector<CoordVector> ray = monRay.getRay();
    std::vector<float> nrg = monRay.getNRG();

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

        // ecriture des vertex pour tous les rayons
        for (int i = 0; i < nbRay ; i++) // on n'ecrit que le premier point
        {
            if (m_buff_rayMort[i] == 0) // si le rayon est toujours vivant
            {
                // si l'énergie sur au moins une bande est au dessus du seuil le rayon reste vivant
                bool rayVivant = false;
                for (int l=0; l<8; l++)
                {
                    if (nrg[i*8+l] > seuil)
                    {
                        rayVivant = true;
                    }
                }
                //CoordVector vertCoord(ray[i], ray[i+1], ray[i+2]);
                text = "v "+ CoordVector2QString(ray[i]) + "\n";
                fichier.write(text.toLatin1());

                if (rayVivant) // S'il reste vivant
                {
                    m_rayMort.push_back(0);
                }
                else // S'il meurt
                {
                    m_rayMort.push_back(1);
                    m_buff_rayMort[i] = 1;
                }
            }
            else // S'il était deja mort
            {
                m_rayMort.push_back(1);
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

        std::vector<int> dernierVertex;
        dernierVertex.resize(nbRay,0); // numero de ligne du dernier vertex du rayon

      // relier les premier point à la source
        for (int i = 0 ; i<nbRay ; i++)
        {
            ligne = "l 1 " + QString::number(i+2) + "\n";
            fichier.write(ligne.toLatin1());

            dernierVertex[i] = i+2;
        }

        // relier les points suivent deux par deux

        if (nbRebond >= 1) // MODE NOMBRE DE REBONDS FIXE
        {
            for (int i = 0 ; i < nbRay*(nbRebond-1) ; i++)
            {
                ligne = "l " + QString::number(i+2) + " " + QString::number(nbRay+i+2) + "\n";
                fichier.write(ligne.toLatin1());
            }
        }
        else // MODE ATTENUATION
        {
            int raymort = m_rayMort[0];
            int j (0), k(0);

            while (raymort < nbRay) // tant que tous les rayons ne sont pas morts
            {
                raymort = 0;
                for (int i = 0; i<nbRay ; i++) // pour chaque rayon
                {
                    raymort = raymort + m_rayMort[j*nbRay +i]; // on ajoute 1 si le i-eme rayon meurt

                    if (m_rayMort[j*nbRay +i] == 0) // Si le i-eme rayon n'est pas mort
                    {
                        // On l'ecrit
                        ligne = "l " + QString::number(dernierVertex[i]) + " " + QString::number(nbRay+2+k) + "\n";
                        fichier.write(ligne.toLatin1());

                        // on remplace le numero du dernier vertex (que si le rayon n'est pas mort)
                        dernierVertex[i] = nbRay+2+k;

                        k++; // on n'augmente l'increment (que si le rayon n'est pas mort)
                    }
                }
                j++; // compteur general de rebond
            }
        }
    }
    fichier.close(); // ferme le fichier
}


void ObjWriter::display_sourceImages(SourceImage &srcImg, float seuil)
{
    QFile fichier(m_chemin);

    std::vector<CoordVector> sourcesImages = srcImg.getSourcesImages();
    std::vector<float> nrg = srcImg.getNrgSI();

    fichier.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier

    // creation d'un entete
    QString text("o SourcesImages \n");
    fichier.write(text.toLatin1());

    // ecriture des vertex représentant les posotions de sources images
    for (int i = 0; i < sourcesImages.size() ; i++)
    {
        if (nrg[i] > seuil) // On n'ecrit que les sources images dont l'energie est supérieure qu seuil
        {
            //CoordVector vertCoord(sourcesImages[i], sourcesImages[i+1], sourcesImages[i+2]);
            text = "v "+ CoordVector2QString(sourcesImages[i]) + "\n";
            fichier.write(text.toLatin1());
        }
    }

    fichier.close(); // ferme le fichier
}

void ObjWriter::display_octree(const std::vector<Boite> &oct)
{
    QFile fichier(m_chemin);
    int i, j;
    int nbFeuille(0);

    std::vector<CoordVector> coordVertexBoite;

    fichier.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier

    // creation d'un entete
    QString text("o Octree \n");
    fichier.write(text.toLatin1());

    // Pour chaque boite
    for ( i = 0 ; i<oct.size() ; i++)
    {
        if (!oct[i].m_numElt.empty()) // Affichage des feuilles non vide uniquement
        {
            coordVertexBoite = coordVertBoite(oct[i]);
            // Ecriture des huit vertex ligne par ligne
            for (j = 0 ; j < coordVertexBoite.size() ; j++)
            {
                text = "v "+ CoordVector2QString(coordVertexBoite[j]) + "\n";
                fichier.write(text.toLatin1());
            }
            nbFeuille++;
        }
    }

    // On relie les points
    std::vector<QString> ligne;
    ligne.resize(12,"");
    // Pour chaque boite    
    for ( i = 0 ; i<nbFeuille ; i++)
    {
        ligne[0]  = "l " + QString::number(8*i +1) + " " + QString::number(8*i+2) + "\n";
        ligne[1]  = "l " + QString::number(8*i +1) + " " + QString::number(8*i+3) + "\n";
        ligne[2]  = "l " + QString::number(8*i +1) + " " + QString::number(8*i+5) + "\n";
        ligne[3]  = "l " + QString::number(8*i +2) + " " + QString::number(8*i+4) + "\n";
        ligne[4]  = "l " + QString::number(8*i +2) + " " + QString::number(8*i+6) + "\n";
        ligne[5]  = "l " + QString::number(8*i +3) + " " + QString::number(8*i+4) + "\n";
        ligne[6]  = "l " + QString::number(8*i +3) + " " + QString::number(8*i+7) + "\n";
        ligne[7]  = "l " + QString::number(8*i +4) + " " + QString::number(8*i+8) + "\n";
        ligne[8]  = "l " + QString::number(8*i +5) + " " + QString::number(8*i+6) + "\n";
        ligne[9]  = "l " + QString::number(8*i +5) + " " + QString::number(8*i+7) + "\n";
        ligne[10] = "l " + QString::number(8*i +6) + " " + QString::number(8*i+8) + "\n";
        ligne[11] = "l " + QString::number(8*i +7) + " " + QString::number(8*i+8) + "\n";

        for (j = 0 ; j < ligne.size() ; j ++)
        {
            fichier.write(ligne[j].toLatin1());
        }
    }

    fichier.close(); // ferme le fichier

}

// Méthodes

std::vector<CoordVector> coordVertBoite(const Boite &boite)
{
    CoordVector coinMin = boite.m_coinMin;
    float arrete = boite.m_arrete;
    std::vector<CoordVector> coordVert;
    coordVert.resize(8,CoordVector(0,0,0));

    coordVert[0] = CoordVector(coinMin.x           , coinMin.y          , coinMin.z         );
    coordVert[1] = CoordVector(coinMin.x + arrete  , coinMin.y          , coinMin.z         );
    coordVert[2] = CoordVector(coinMin.x           , coinMin.y + arrete , coinMin.z         );
    coordVert[3] = CoordVector(coinMin.x + arrete  , coinMin.y + arrete , coinMin.z         );
    coordVert[4] = CoordVector(coinMin.x           , coinMin.y          , coinMin.z + arrete);
    coordVert[5] = CoordVector(coinMin.x + arrete  , coinMin.y          , coinMin.z + arrete);
    coordVert[6] = CoordVector(coinMin.x           , coinMin.y + arrete , coinMin.z + arrete);
    coordVert[7] = CoordVector(coinMin.x + arrete  , coinMin.y + arrete , coinMin.z + arrete);

    return coordVert;

}
