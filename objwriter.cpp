#include "objwriter.h"
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QIODevice>
#include "QDebug"
#include <QtMath>
#include <QMessageBox>
#include <QCoreApplication>


ObjWriter::ObjWriter(QString chemin, int nbRay) // recupere en attribue le nom de chemin de fichier specifié
{

    QFile fichier(chemin);

    QString newName(chemin);
    m_buff_rayMort.resize(nbRay, 0); // 0 = rayon vivant

    // A CONSERVER : INCREMENTATION DES FICHIERS

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


void ObjWriter::rec_Vert(Source &source, int nSrc, Ray &monRay, int nbRay, int num_rebond, float seuil)
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
            text = text + "v " + CoordVector2QString(source.getCentre(nSrc)) + "\n";
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


void ObjWriter::display_sourceImages(std::vector<CoordVector> &sourcesImages)
{
    QFile fichier(m_chemin);

    fichier.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier

    // creation d'un entete
    QString text("o Sourcesimages \n");
    fichier.write(text.toLatin1());

    // ecriture des vertex représentant les posotions de sources images
    for (int i = 0; i < sourcesImages.size() ; i++)
    {
        text = "v "+ CoordVector2QString(sourcesImages[i]) + "\n";
        fichier.write(text.toLatin1());
    }

    fichier.close(); // ferme le fichier
}

void ObjWriter::display_coloredTriangle(std::vector<CoordVector> &point, std::vector<float> &nrg, const CoordVector &dirNormal)
{
    QFile fichier(m_chemin);

    fichier.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier

    // creation d'un entete
    QString text("mtllib materiaux.mtl\n");
    text += "o SI_Projected \n";
    fichier.write(text.toLatin1());

    CoordVector vect;

    float normVec, nbpoint(point.size());
    std::vector<float> nrgMoy;
    nrgMoy.resize(nbpoint, 0);


    int j;

    // pour faire un carré
    float splatsize = 0.5;
    CoordVector a( splatsize/2, splatsize/2,0);
    CoordVector b( splatsize/2,-splatsize/2,0);
    CoordVector c(-splatsize/2,-splatsize/2,0);
    CoordVector d(-splatsize/2, splatsize/2,0);


    // ecriture des vertex représentant les posotions de sources images
    for (int i = 0; i < nbpoint ; i++)
    {
        vect    = vecteur(point[i],dirNormal);
        normVec = norme(vect);
        vect    = vect/normVec;
        point[i]+= vect*0.1;// on reduit la distance au listener
        CoordVector A(a),B(b),C(c),D(d); // initialisation

        makeSplat(A,point[i],vect);
        makeSplat(B,point[i],vect);
        makeSplat(C,point[i],vect);
        makeSplat(D,point[i],vect);

        //point[i].debug();
        //vect.debug();

        // ecriture des carrés
        text  = "v "+ CoordVector2QString(A) + "\n";
        text += "v "+ CoordVector2QString(B) + "\n";
        text += "v "+ CoordVector2QString(C) + "\n";
        text += "v "+ CoordVector2QString(D) + "\n";
        fichier.write(text.toLatin1());
    }

    // ecriture de la normale
    for (int i = 0; i < nbpoint ; i++)
    {
        vect = vecteur(point[i],dirNormal);
        normVec = norme(vect);

        text = "vn "+ CoordVector2QString(vect/normVec) + "\n";
        fichier.write(text.toLatin1());

        // vecteur energie
        for (j=0 ; j<8 ; j++) nrgMoy[i] += nrg[8*i+j];
        nrgMoy[i] /= 8;
    }

    float max = *std::max_element(nrgMoy.begin(), nrgMoy.end());
    float min = *std::min_element(nrgMoy.begin(), nrgMoy.end());

    for (int i = 0; i < nbpoint ; i++)
    {
        //text = "usemtl " + QString::number(round(99*(nrgMoy[i]-min)/(max-min))) + "\n"; // energie moyenne vaut 99 pour le max et 0 pour le min
        text = "usemtl " + QString::number(round(99*log10(9*(nrgMoy[i]-min)/(max-min)+1))) + "\n";
        text += "s off\n";
        text += "f ";
        for (j=1 ; j< 5 ;j++)
        {
            text += QString::number(4*i+j) + "//" + QString::number(i+1) + " "; // ecriture des vertex et leur normale
        }
        text += "\n";
        fichier.write(text.toLatin1());
    }


    fichier.close(); // ferme le fichier
}

//https://visheshvision.wordpress.com/2014/04/28/rendering-a-colored-point-cloud-in-blender/
void RotateX(CoordVector &P, float ang)
{
    float y= P.y*cos(ang) - P.z*sin(ang);
    float z= P.y*sin(ang) + P.z*cos(ang);
    P.y=y;
    P.z=z;
}

void RotateY(CoordVector &P, float ang)
{
    float x= P.x*cos(ang) + P.z*sin(ang);
    float z= -P.x*sin(ang) + P.z*cos(ang);
    P.x=x;
    P.z=z;
}

void Translate(CoordVector &P,CoordVector V)
{
    P.x+=V.x;
    P.y+=V.y;
    P.z+=V.z;
}

void makeSplat(CoordVector &P, CoordVector pos,CoordVector nor)
{
    // enter the rotation and translation code here
    //float magNormal= sqrt(nor.x*nor.x + nor.y*nor.y + nor.z * nor.z);
    //float theta = asin(nor.y/magNormal);
    float theta = asin(nor.y); // OK !
    float phi = atan(nor.x/nor.z);
    if (nor.z >0) phi+= M_PI;

    //qDebug() << "theta = " << theta << " ; phi = " << phi;

    RotateX(P,theta);
    RotateY(P,phi);
    Translate(P,pos);
}

void genererMLT()
{
    QFile fichier(QCoreApplication::applicationDirPath() + "/materiaux.mtl");

    fichier.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier

    // creation d'un entete
    QString text;
    fichier.write(text.toLatin1());

    // ecriture des vertex représentant les posotions de sources images
    for (float i = 0; i < 100 ; i++)
    {
        text  = "newmlt " + QString::number(i) + "\n";  // nom du materiaux
        text += "Ka 1.000000 1.000000 1.000000\n"; // couleur ambiante
        text += "Kd " + HSV2RGB(240-(i*2.4), 1, 1) + "\n"; // couleur diffuse RGB (Hue de 240 à 0)
        text += "Ks 1.000000 1.000000 1.000000\n"; // specular
        text += "Ni 1.000000\n"; // densité
        text += "d 1.000000\n"; // transparence
        text += "illum 2\n"; // lumière
        text += "\n";

        fichier.write(text.toLatin1());
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
    std::vector<CoordVector> coordVert;
    coordVert.resize(8,CoordVector(0,0,0));

    // Version boite cubique
    /*
    CoordVector coinMin = boite.m_coinMin;
    float arrete = boite.m_arrete;

    coordVert[0] = CoordVector(coinMin.x           , coinMin.y          , coinMin.z         );
    coordVert[1] = CoordVector(coinMin.x + arrete  , coinMin.y          , coinMin.z         );
    coordVert[2] = CoordVector(coinMin.x           , coinMin.y + arrete , coinMin.z         );
    coordVert[3] = CoordVector(coinMin.x + arrete  , coinMin.y + arrete , coinMin.z         );
    coordVert[4] = CoordVector(coinMin.x           , coinMin.y          , coinMin.z + arrete);
    coordVert[5] = CoordVector(coinMin.x + arrete  , coinMin.y          , coinMin.z + arrete);
    coordVert[6] = CoordVector(coinMin.x           , coinMin.y + arrete , coinMin.z + arrete);
    coordVert[7] = CoordVector(coinMin.x + arrete  , coinMin.y + arrete , coinMin.z + arrete);
    */

    // Version boites englobantes
    coordVert[0] = CoordVector(boite.m_min.x, boite.m_min.y, boite.m_min.z);
    coordVert[1] = CoordVector(boite.m_max.x, boite.m_min.y, boite.m_min.z);
    coordVert[2] = CoordVector(boite.m_min.x, boite.m_max.y, boite.m_min.z);
    coordVert[3] = CoordVector(boite.m_max.x, boite.m_max.y, boite.m_min.z);
    coordVert[4] = CoordVector(boite.m_min.x, boite.m_min.y, boite.m_max.z);
    coordVert[5] = CoordVector(boite.m_max.x, boite.m_min.y, boite.m_max.z);
    coordVert[6] = CoordVector(boite.m_min.x, boite.m_max.y, boite.m_max.z);
    coordVert[7] = CoordVector(boite.m_max.x, boite.m_max.y, boite.m_max.z);


    return coordVert;

}

QString HSV2RGB(float h, float s, float v)
{
    float r = 0, g = 0, b = 0;

    int i;
    float f, p, q, t;

    if (h == 360)
        h = 0;
    else
        h = h / 60;

    i = (int)trunc(h);
    f = h - i;

    p = v * (1.0 - s);
    q = v * (1.0 - (s * f));
    t = v * (1.0 - (s * (1.0 - f)));

    switch (i)
    {
    case 0:
        r = v;
        g = t;
        b = p;
        break;

    case 1:
        r = q;
        g = v;
        b = p;
        break;

    case 2:
        r = p;
        g = v;
        b = t;
        break;

    case 3:
        r = p;
        g = q;
        b = v;
        break;

    case 4:
        r = t;
        g = p;
        b = v;
        break;

    default:
        r = v;
        g = p;
        b = q;
        break;
    }

    return QString::number(r) + " " + QString::number(g) + " " + QString::number(b);

}
