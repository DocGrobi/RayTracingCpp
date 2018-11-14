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
      m_nbligne = 0;
}

ObjWriter::~ObjWriter()
{
}


void ObjWriter::rec_Vert(Source &source, int nSrc, Ray &monRay, int num_rebond)
{
    QFile fichier(m_chemin);
    std::vector<CoordVector> ray = monRay.getRay();
    std::vector<bool> vivant = monRay.getRayVivant2();


    int nbRay = ray.size();

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

        // ecriture des vertex
        for (int i = 0; i < nbRay ; i++)
        {
            if(vivant[i])
            {
                text = "v "+ CoordVector2QString(ray[i]) + "\n";
                fichier.write(text.toLatin1());
                m_rayMort.push_back(false);
            }
            else // S'il était deja mort
            {
                m_rayMort.push_back(true);
            }
        }
    }
    fichier.close(); // ferme le fichier
}

void ObjWriter::rec_Vert_init(std::vector<CoordVector> &si)
{
    QFile fichier(m_chemin);
    if(fichier.open(QIODevice::WriteOnly | QIODevice::Text)) // ouvre le fichier
    {
        // creation d'un entete
        QString text("o Rayons \n");
       for(int i=0 ; i<si.size() ; i++)
        {
            text+= "v " + CoordVector2QString(si[i]) + "\n";
        }

        fichier.write(text.toLatin1());
    }
    fichier.close();
}

void ObjWriter::rec_Vert(Ray &monRay, int ind, CoordVector source)
{
    QFile fichier(m_chemin);
    std::vector<CoordVector> A = monRay.getPos();
    QString text;

    if(fichier.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) // ouvre le fichier
    {
        text = "v " + CoordVector2QString(A[ind]) + "\n";
        text += "v " + CoordVector2QString(source) + "\n";
        fichier.write(text.toLatin1());
        text = "l "+ QString::number(m_nblignefin+1) + " " + QString::number(m_nblignefin+2) + "\n";
        fichier.write(text.toLatin1());
        m_nbligne+=2;
        m_nblignefin+=2;
    }
    fichier.close();
}


void ObjWriter::rec_Vert(Ray &monRay)
{
    QFile fichier(m_chemin);
    std::vector<CoordVector> A = monRay.getPos();

    std::vector<bool> vivant = monRay.getRayVivant2();
    QString text;
    int nbNouvelleVert(0);

    float nb_mort(0);

    if(fichier.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) // ouvre le fichier
    {
        for (int i=0 ; i<A.size() ; i++)
        {
            if (vivant[i])
            {
                text += "v " + CoordVector2QString(A[i]) + "\n";
                m_nbligne++;
                m_buff_rayMort[i] =m_nbligne;
                nbNouvelleVert++;
            }
            else nb_mort++;
        }
        fichier.write(text.toLatin1());
        text="";

        for (int i=0 ; i<A.size() ; i++)
        {
            if (vivant[i])
            {
                text += "l "+ QString::number(m_buff_rayMort[i]-nb_mort) + " " + QString::number(m_buff_rayMort[i]+nbNouvelleVert) + "\n";
                m_buff_rayMort[i]+=m_nbligne; // passer à la prochaine série
            }
            else nb_mort--;
        }
        fichier.write(text.toLatin1());
        m_nblignefin = m_nbligne+nbNouvelleVert;
    }
    fichier.close();
}


void ObjWriter::rec_Line(int nbRay)
{
    QFile fichier(m_chemin);
    QString ligne;

    if(fichier.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) // ouvre le fichier et place le curseur à la fin
    {

        std::vector<int> dernierVertex;
        dernierVertex.resize(nbRay,0); // numero de ligne du dernier vertex du rayon

        int indiceRay(2);
      // relier les premiers points à la source
        for (int i = 0 ; i<nbRay ; i++)
        {
            if(!m_rayMort[i]){
            ligne = "l 1 " + QString::number(indiceRay) + "\n";
            fichier.write(ligne.toLatin1());
            dernierVertex[i] = indiceRay;
            indiceRay++;
            }
        }
        float nbIt = m_rayMort.size()/nbRay;
        for (int j = 1; j<nbIt; j++)
        {
            for(int i=0; i<nbRay ; i++)
            {
                if(!m_rayMort[i+j*nbRay]){
                ligne = "l " + QString::number(dernierVertex[i]) +" "+ QString::number(indiceRay) + "\n";
                fichier.write(ligne.toLatin1());
                dernierVertex[i] = indiceRay;
                indiceRay++;
                }
            }
        }
    }
    fichier.close(); // ferme le fichier
}



// pour un faisceau
void ObjWriter::display_Beam_init()
{
    QFile fichier(m_chemin);

    fichier.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier

    // creation d'un entete
    QString text("o Beams \n");
    fichier.write(text.toLatin1());
}

void ObjWriter::display_Beam_vert(Ray &rayon, Listener &listener)
{

    QFile fichier(m_chemin);
    QString text;


    if(fichier.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) // ouvre le fichier et place le curseur à la fin
    {
        std::vector<float> touche = toucheListener2(rayon,listener);
        std::vector<float> longueurRayonTot = rayon.getDist(); // Distance parcourue avant le dernier rebond
        std::vector<CoordVector> point = rayon.getPos();
        std::vector<CoordVector> vec = rayon.getDir();
        CoordVector A, C, vect;

        for( int i =0 ; i< touche.size() ; i++)
        {
            if (touche[i]>=0) // si le rayon touche le listener
            {
                A = point[i];
                vect = vec[i];
               C = A - vect*longueurRayonTot[i];

                longueurRayonTot[i]+=touche[i]; // ajout de la distance au listener à la longueur totale

                text = "v "+ CoordVector2QString(C) + "\n";
                text+= "v "+ CoordVector2QString(C+(vect*longueurRayonTot[i])) + "\n";
                fichier.write(text.toLatin1());
                m_nbligne+=2;
            }
        }
    }
    fichier.close(); // ferme le fichier
}

void ObjWriter::display_Beam_line()
{

    QFile fichier(m_chemin);
    QString ligne;

    if(fichier.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) // ouvre le fichier et place le curseur à la fin
    {
        if (m_nbligne>0)
        {
            for (int i = 1; i<= m_nbligne ; i+=2)
            {
                ligne = "l " + QString::number(i) + " " + QString::number(i+1) + "\n";
                fichier.write(ligne.toLatin1());
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

void ObjWriter::display_coloredTriangle(std::vector<CoordVector> &point, std::vector<float> &nrg, const CoordVector &dirNormal, const CoordVector &posSource, float seuil)
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


    //ECRITURE SOURCE
    vect    = vecteur(posSource,dirNormal);
    normVec = norme(vect);
    if(normVec==0) // source et recepteur confonduq
    {
        vect.x=1;
    }
    else vect    = vect/normVec;
    //point[i]+= vect*0.1;// on reduit la distance au listener
    CoordVector A(a),B(b),C(c),D(d); // initialisation
    makeSplat(A,posSource,vect);
    makeSplat(B,posSource,vect);
    makeSplat(C,posSource,vect);
    makeSplat(D,posSource,vect);
    // ecriture des carrés
    text  = "v "+ CoordVector2QString(A) + "\n";
    text += "v "+ CoordVector2QString(B) + "\n";
    text += "v "+ CoordVector2QString(C) + "\n";
    text += "v "+ CoordVector2QString(D) + "\n";
    fichier.write(text.toLatin1());



    // ecriture des vertex représentant les positions de sources images
    for (int i = 1; i < nbpoint ; i++)
    {
        vect    = vecteur(point[i],dirNormal);
        normVec = norme(vect);
        vect    = vect/normVec;
        //point[i]+= vect*0.1;// on reduit la distance au listener
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

    //max = *std::max_element(nrg.begin(), nrg.end());
    //for(auto &a : nrg) a/=max; // normalisation
    for(auto &a : nrgMoy) a/=max; // normalisation
    float min = *std::min_element(nrgMoy.begin(), nrgMoy.end());
    //float min = *std::max_element(nrg.begin(), nrg.end())*seuil;
    if (min > seuil) min = seuil;
    genererMTL(min);

    //float max2 = *std::max_element(nrg.begin(), nrg.end());
    //float min2 = *std::min_element(nrg.begin(), nrg.end());

    for (int i = 0; i < nbpoint ; i++)
    {
        //text = "usemtl " + QString::number(round(99*(nrgMoy[i]-min)/(max-min))) + "\n"; // energie moyenne vaut 99 pour le max et 0 pour le min
        //text = "usemtl " + QString::number(round(240*log10(9*(nrgMoy[i]-min)/(max-min)+1))) + "\n";
        //text = "usemtl " + QString::number(round(6-log10(nrg[8*i]/max2)*40)) + "\n"; //energie basse frequence
        //text = "usemtl " + QString::number(round(log10(min/nrgMoy[i])*240/log10(min/max))) + "\n";
        //text = "usemtl " + QString::number(round(10*log10(nrg[i+4]))) + "dB\n"; //A 1000Hz
        text = "usemtl " + QString::number(round(10*log10(nrgMoy[i]))) + "dB\n";
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

void genererMTL(float min)
{
    QFile fichier(QCoreApplication::applicationDirPath() + "/materiaux.mtl");

    fichier.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier

    // creation d'un entete
    QString text;
    fichier.write(text.toLatin1());
    min = -10*log10(min);

    // ecriture des vertex représentant les posotions de sources images
    for (float i = 0; i <= min ; i++)
    {
        //text  = "newmtl " + QString::number(i) + "\n";  // nom du materiaux
        if (i==0) text  = "newmtl " + QString::number(i) + "dB\n";  // nom du materiaux
        else text  = "newmtl -" + QString::number(i) + "dB\n";  // nom du materiaux
        text += "Ka 1 1 1\n"; // couleur ambiante
        //text += "Kd " + HSV2RGB(240-i, 1, 1) + "\n"; // couleur diffuse RGB (Hue de 240 à 0)
        text += "Kd " + HSV2RGB(round(240*i/min), 1, 1) + "\n"; // couleur diffuse RGB (Hue de 240 à 0)
        text += "Ks 0 0 0\n"; // specular
        text += "Ni 1\n"; // densité
        text += "d 0.5\n"; // transparence
        text += "illum 2\n"; // lumière
        text += "\n";

        fichier.write(text.toLatin1());
    }

    fichier.close(); // ferme le fichier

}

/// Pour afficher l'octree
/*
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
*/

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
