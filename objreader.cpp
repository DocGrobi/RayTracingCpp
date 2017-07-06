#include "objreader.h"
#include "QFile"
#include "QIODevice"
#include "QTextStream"
#include "QDebug"


Source::Source()
{
    m_centreSource = (0,0,0);

}
Source::~Source()
{
}

void Source::chargerSource(const CoordVector &cs)
{
    m_centreSource = cs;
    //qDebug() << cs.x << cs.y << cs.z;
}

void Source::chargerVertSource(float coord)
{
    m_vertSource.push_back(coord);
}

CoordVector Source::centre()
{
    return m_centreSource;
}

std::vector<float> Source::vert()
{

    return m_vertSource;
}

QString Source::afficher() const
{
    QString x,y,z;
    x.setNum(m_centreSource.x);
    y.setNum(m_centreSource.y);
    z.setNum(m_centreSource.z);

    QString centre = "Centre de la source : x = " + x + ", y = " + y + ", z = " + z;

    return centre;

}

Source MeshObj::getSource() const //accesseur aux parametres de source
{
    return m_source;
}


Listener::Listener()
{
    m_centreListener = (0,0,0);
    m_rayon = 1;
}

Listener::~Listener()
{
}

void Listener::chargerListener(const CoordVector &cs, float r)
{
    m_centreListener = cs;
    m_rayon =r;
}

QString Listener::afficher()
{    
    QString x,y,z,ray;
    x.setNum(m_centreListener.x);
    y.setNum(m_centreListener.y);
    z.setNum(m_centreListener.z);
    ray.setNum(m_rayon);

    QString info = "Centre du listener : x = " + x + ", y = " + y + ", z = " + z + "\n" + "Rayon : " + ray;

    return info;

}

CoordVector Listener::getCentre()
{
    return m_centreListener;
}

float Listener::getRayon()
{
    return m_rayon;
}

Listener MeshObj::getListener() const //accesseur aux parametres du listener
{
    return m_listener;
}

MeshObj::MeshObj(QString s) //: m_nbData(1)
{
    charger_obj(s);
}

MeshObj::~MeshObj()
{
}

/*
std::vector<float> &MeshObj::getVertex()  {
    return m_vert;
}
std::vector<float> &MeshObj::getNormals()  {
    return m_norm;
}
int MeshObj::getNb_data() const {
    return m_nbData;
}
*/

std::vector<CoordVector> &MeshObj::getVert()  {
    return m_vertex;
}

std::vector<CoordVector> &MeshObj::getNorm()  {
    return m_normales;
}
std::vector<CoordVector> &MeshObj::getVectFace()  {
    return m_vecteurFace;
}

std::vector<float> &MeshObj::getIndMat()  {
    return m_indMat;
}

CoordVector &MeshObj::getMin(){
    return m_min;
}

CoordVector &MeshObj::getMax(){
    return m_max;
}



void MeshObj::charger_obj(QString file_obj)
{
    std::vector<CoordVector> ver, nor; // vecteurs de coordonnees
    std::vector<unsigned int> iv, in; // indice des points à assembler
    std::vector<float> imat;
    //float indiceMat = 0, indiceMat_Curr = 0; // Indice du materiaux
    bool lecture_source = false, lecture_listener = false;
    CoordVector coordFloat (0,0,0);
    int nb_ver = 0, nb_verSource = 0, nb_verListener = 0, nb_norSource = 0, nb_norListener = 0, i, j;
    float x_max = -10000000, rayon = 0;

    Material matOdeon; // chargement des materiaux Odéons
    std::vector<QString> matOdeon_vect = matOdeon.getNomMat();

    int rangCoeff = 7; // par default c'est le materiaux 50% absorbant
    QString nomMat;

    QFile fichier(file_obj); // fichier .obj

    if(fichier.open(QIODevice::ReadOnly | QIODevice::Text)) // Si on peut ouvrir le fichier
    {
        QTextStream flux(&fichier);
        while(!flux.atEnd())
        {
            QString ligne = flux.readLine(); // Lecture ligne par ligne du fichier

            if(lecture_source || lecture_listener) // Si l'objet lu est la source ou le listener
            {

                if(ligne[0]=='v' && ligne[1]==' ') //Vertex
                {
                    QStringList coord = ligne.split(" ");

                    if(lecture_source)
                    {
                        for(int i=0; i<3;i++)
                        {
                            m_source.chargerVertSource(coord[i+1].toFloat());
                        }
                    }

                    coordFloat.x += coord[1].toFloat();
                    coordFloat.y += coord[2].toFloat();
                    coordFloat.z += coord[3].toFloat();
                    nb_ver++; // incrementation du nombre d'element dans la source

                    if (lecture_listener) // mesure de x max pour calcul du rayon
                    {
                        if(x_max < coord[1].toFloat()) x_max = coord[1].toFloat();
                    }
                }
                else if(ligne[1]=='n') // comptage des normales
                {
                    if(lecture_source) nb_norSource++;

                    if(lecture_listener) nb_norListener++;
                }                                
                else if(ligne[0]=='o') // fin des vertices et des normales
                {
                    // le centre de la source et la moyenne de ses coordonnées
                    coordFloat = coordFloat/nb_ver;

                    if(lecture_source)
                    {
                        m_source.chargerSource(coordFloat);// on affecte à la source les coordonnées du centre recupérés
                        lecture_source = false; // on sort du mode lecture source
                        nb_verSource = nb_ver; // ici il faut enlever le nombre d'elements de source aux indices car dans les vecteurs i.. on n'enregistre pas les element de source

                    }
                    if(lecture_listener)
                    {
                        rayon = x_max - coordFloat.x; // VALABLE QUE POUR UNE SPHERE - calcul du rayon par coordonnee x max moins centre sur x
                        m_listener.chargerListener(coordFloat, rayon);// on affecte au listener les coordonnées du centre recupérés
                        lecture_listener = false; // on sort du mode lecture listener
                        nb_verListener = nb_ver;
                    }

                    nb_ver = 0; //remsie à 0 du compteur de vertex
                    coordFloat = (0,0,0); //remsie à 0 de la somme des coordonnees
                }
            }

            else // mode chargement du mesh normal
            {
                //Coordonnees de points (vertex, texture et normale)
                if(ligne[0]=='v')
                {
                    if(ligne[1]==' ') //Vertex
                    {
                        QStringList coord = ligne.split(" ");
                        float x,y,z;
                        x = coord[1].toFloat();
                        y = coord[2].toFloat();
                        z = coord[3].toFloat();

                        ver.push_back(CoordVector(x,y,z)); // C'est un std::vector rempli avec les CoordVector de coordonnees des vertex

                    }

                    else if(ligne[1]=='n') //Normales
                    {
                        QStringList coord = ligne.split(" ");
                        float x,y,z;
                        x = coord[1].toFloat();
                        y = coord[2].toFloat();
                        z = coord[3].toFloat();

                        nor.push_back(CoordVector(x,y,z)); // C'est un std::vector rempli avec les CoordVector de coordonnees de normales
                    }
                }

                // Les matériaux
                if(ligne[0]=='u')
                {
                    QStringList materiau = ligne.split(" ");
                    QStringList indice = materiau[1].split("_"); // ne prend que le premier "mot" du nom du materiau

                    rangCoeff = 7; // valeur par default de l'absorption du materiau correspond à 50% sur toutes les bandes
                    for(int i=0 ; i < matOdeon_vect.size(); i++ )
                    {
                        if (matOdeon_vect[i] == indice[0]) // si le materiau existe dans le tableau odeon
                        {
                            nomMat = indice[0];
                            rangCoeff = i;
                        }
                    }
                }

                //Les faces : f V1/T1/N1 V2/T2/N2 V3/T3/N3
                if(ligne[0]=='f')
                {
                    if(ligne.contains("//")) // pour les faces sans texture
                    {
                        ligne = doubleSlash(ligne); // on donne 1 comme numero de texture
                    }
                    ligne = supprimeSlash(ligne); // on supprime les slashs pour pourvoir ecrire les vecteurs :f V1 T1 N1 V2 T2 N2 V3 T3 N3

                    QStringList indice = ligne.split(" ");

                    int nbDonnees = indice.size()/3; //nombre de donnees V1/T1/N1 par face (dans le cas de faces triangles) : 10/3

                    for(i= 1; i<=nbDonnees;i++)
                    {
                        unsigned int v,n;
                        v=indice[i*3-2].toInt() - (nb_verSource + nb_verListener) -1; // on enleve le nombre de vertex de source et listener s'il y en a car ils ne s'enregistrent pas dans les vecteurs de coordonnées et 1 pour que indice 1 corresponde à 0
                        n=indice[i*3].toInt() - (nb_norSource + nb_norListener) -1;

                        iv.push_back(v); // indice des vertex
                        in.push_back(n); // indice des normales
                    }
                    // pour chaque face on rempli le vecteur materiau avec son nom et ses 8 coefficients d'absorption
                    m_indMat.push_back(nomMat.toFloat());
                    for (i = 0; i< 8; i++)
                    {
                        m_indMat.push_back(matOdeon.getIndMat(rangCoeff*8+i));
                    }

                }
            }

            // condition qui se place apres les actions de ligne pour que l'activation du mode agisse à partir de la ligne suivante
            if(ligne.contains("source"))
            {
                lecture_source = true; // on est en mode lecture de source
                //qDebug() << lecture_source;
            }

            // Listener
            if(ligne.contains("listener"))
            {
                lecture_listener = true; // on est en mode lecture de source
            }

        }
        fichier.close();

        //si on n'est pas sortie du mode source/listener car il n'y a que des sources ou des listeners
        if(lecture_source || lecture_listener)
        {
            // le centre de la source et la moyenne de ses coordonnées
            coordFloat = coordFloat/nb_ver;

            if(lecture_source)
            {
                m_source.chargerSource(coordFloat);// on affecte à la source les coordonnées du centre recupérés
                lecture_source = false; // on sort du mode lecture source
                nb_verSource = nb_ver; // ici il faut enlever le nombre d'elements de source aux indices car dans les vecteurs i.. on n'enregistre pas les element de source
            }
            if(lecture_listener)
            {
                rayon = x_max - coordFloat.x; // VALABLE QUE POUR UNE SPHERE - calcul du rayon par coordonnee x max moins centre sur x
                m_listener.chargerListener(coordFloat, rayon);// on affecte au listener les coordonnées du centre recupérés
                lecture_listener = false; // on sort du mode lecture listener
                nb_verListener = nb_ver;
            }

            nb_ver = 0; //remsie à 0 du compteur de vertex
            coordFloat = (0,0,0); //remsie à 0 de la somme des coordonnees

        }
    }

    m_min = ver[0];
    m_max = ver[0];
    // classement des coordonnées face par face
    for(i=0; i<iv.size(); i++) // vertex
    {
        if(iv[i]<ver.size())
        {   /*
            m_vert.push_back(ver[iv[i]].x);
            m_vert.push_back(ver[iv[i]].y);
            m_vert.push_back(ver[iv[i]].z);
            */
            m_vertex.push_back(ver[iv[i]]);
            for (j = 0 ; j<3 ; j++)
            {
                if(ver[iv[i]][j]<m_min[j])   m_min[j] = ver[iv[i]][j];
                if(ver[iv[i]][j]>m_max[j])   m_max[j] = ver[iv[i]][j];
            }

        }
    }
    for(i=0; i<in.size(); i++) //normales - pour ne prendre qu'une normale par face on prendrai comme increment i =i+3
    {
        if(in[i]<nor.size())
        {   /*
            m_norm.push_back(nor[in[i]].x);
            m_norm.push_back(nor[in[i]].y);
            m_norm.push_back(nor[in[i]].z);
            */
            m_normales.push_back(nor[in[i]]);

        }
    }

    ///Boite englobante
    // Leger offset pour eviter que la boite englobante ne soit considéré comme une surface
    m_min-=0.01;
    m_max+=0.01;
    // Création des points
    CoordVector v1(m_min.x ,m_min.y ,m_max.z);
    CoordVector v2(m_min.x ,m_max.y ,m_max.z);
    CoordVector v3(m_min.x ,m_min.y ,m_min.z);
    CoordVector v4(m_min.x ,m_max.y ,m_min.z);
    CoordVector v5(m_max.x ,m_min.y ,m_max.z);
    CoordVector v6(m_max.x ,m_max.y ,m_max.z);
    CoordVector v7(m_max.x ,m_min.y ,m_min.z);
    CoordVector v8(m_max.x ,m_max.y ,m_min.z);

    // Création des normales
    CoordVector n1(0, 1, 0);
    CoordVector n2(0, -1, 0);
    CoordVector n3(0, 0, -1);
    CoordVector n4(-1, 0, 0);
    CoordVector n5(0, 0, 1);
    CoordVector n6(1, 0, 0);

    // Ajout des faces
    m_vertex.push_back(v5);m_vertex.push_back(v7);m_vertex.push_back(v3);
    m_vertex.push_back(v6);m_vertex.push_back(v2);m_vertex.push_back(v4);
    m_vertex.push_back(v6);m_vertex.push_back(v4);m_vertex.push_back(v8);
    m_vertex.push_back(v1);m_vertex.push_back(v5);m_vertex.push_back(v3);

    m_vertex.push_back(v6);m_vertex.push_back(v5);m_vertex.push_back(v2);
    m_vertex.push_back(v5);m_vertex.push_back(v1);m_vertex.push_back(v2);
    m_vertex.push_back(v8);m_vertex.push_back(v7);m_vertex.push_back(v6);
    m_vertex.push_back(v7);m_vertex.push_back(v5);m_vertex.push_back(v6);

    m_vertex.push_back(v4);m_vertex.push_back(v3);m_vertex.push_back(v7);
    m_vertex.push_back(v4);m_vertex.push_back(v7);m_vertex.push_back(v8);
    m_vertex.push_back(v2);m_vertex.push_back(v1);m_vertex.push_back(v3);
    m_vertex.push_back(v2);m_vertex.push_back(v3);m_vertex.push_back(v4);

    // Ajout des normales
    for (i = 0; i< 3 ; i++) m_normales.push_back(n1);
    for (i = 0; i< 6 ; i++) m_normales.push_back(n2);
    for (i = 0; i< 3 ; i++) m_normales.push_back(n1);
    for (i = 0; i< 6 ; i++) m_normales.push_back(n3);
    for (i = 0; i< 6 ; i++) m_normales.push_back(n4);
    for (i = 0; i< 6 ; i++) m_normales.push_back(n5);
    for (i = 0; i< 6 ; i++) m_normales.push_back(n6);

    // Ajout des materiaux 100 absorbant (12 faces, le nom du materiau est 1: 100%absorbant et les 8 coefficients valent 1)
    for (i = 0; i< 12*9; i++)
    {
        m_indMat.push_back(1);
    }

    // Création des vecteurs AB et AC pour chaque face ABC
    for(i = 0; i < m_vertex.size() ; i+=3)
    {
        m_vecteurFace.push_back(vecteur(m_vertex[i], m_vertex[i+1]));
        m_vecteurFace.push_back(vecteur(m_vertex[i], m_vertex[i+2]));
    }

   // m_nbData = m_vert.size();

    // nettoyage
    ver.clear();
    nor.clear();

    iv.clear();
    in.clear();
    imat.clear();

}



QString doubleSlash(QString s) // On remplace les // correspondant à l'absence de texture par des /1/ pour avoir un numero de texture
{
    QStringList groupeIndice = s.split("//");

    s = "";
    for(int i=0; i<groupeIndice.size()-1; i++)
    {
         s = s + groupeIndice[i] + "/1/";
    }
    s = s + groupeIndice[groupeIndice.size()-1];

    return s;
}

QString supprimeSlash(QString s) // On remplace les / par des espaces
{
    QStringList groupeIndice = s.split("/");

    s = "";
    for(int i=0; i<groupeIndice.size()-1; i++)
    {
         s = s + groupeIndice[i] + " ";
    }
    s = s + groupeIndice[groupeIndice.size()-1];

    return s;
}
