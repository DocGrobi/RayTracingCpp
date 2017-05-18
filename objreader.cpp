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

void Source::chargerSource(CoordVector cs)
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

void Listener::chargerListener(CoordVector cs, float r)
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



MeshObj::MeshObj(QString s) : m_nbData(1)// : m_vertice(NULL), m_normals(NULL), m_indicesMateriaux(NULL)
{
    charger_obj(s);
}

MeshObj::~MeshObj()
{

}


std::vector<float> MeshObj::getVertex() const //accesseur au pointeur de vertex
{
    return m_vert;
}

std::vector<float> MeshObj::getNormals() const //accesseur au pointeur de vertex
{
    return m_norm;
}

std::vector<float> MeshObj::getIndMat() const //accesseur au pointeur de vertex
{

    return m_indMat;
}

int MeshObj::getNb_data() const //accesseur au pointeur de vertex
{
    return m_nbData;
}


void MeshObj::charger_obj(QString file_obj)
{
    std::vector<CoordVector> ver, nor; // vecteurs de coordonnees
    std::vector<unsigned int> iv, in; // indice des points à assembler
    std::vector<float> imat;
    //float indiceMat = 0, indiceMat_Curr = 0; // Indice du materiaux
    bool lecture_source = false, lecture_listener = false;
    CoordVector coordFloat (0,0,0);
    int nb_ver = 0, nb_verSource = 0, nb_verListener = 0, nb_norSource = 0, nb_norListener = 0;
    float x_max = -10000000, rayon = 0;

    Material matOdeon; // chargement des materiaux Odéons
    std::vector<QString> matOdeon_vect = matOdeon.getNomMat();

    int rangCoeff = 7; // par default c'est le materiaux 50% absorbant
    QString nomMat;

    m_vert.clear();
    m_norm.clear();
    m_indMat.clear();

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

                    coordFloat.x = coordFloat.x + coord[1].toFloat();
                    coordFloat.y = coordFloat.y + coord[2].toFloat();
                    coordFloat.z = coordFloat.z + coord[3].toFloat();
                    nb_ver= nb_ver+1; // incrementation du nombre d'element dans la source

                    if (lecture_listener) // mesure de x max pour calcul du rayon
                    {
                        if(x_max < coord[1].toFloat())
                        {
                            x_max = coord[1].toFloat();
                        }
                    }
                }
                else if(ligne[1]=='n') // comptage des normales
                {
                    if(lecture_source)
                    {
                        nb_norSource++;
                    }
                    if(lecture_listener)
                    {
                        nb_norListener++;
                    }
                }                                
                else if(ligne[0]=='o') // fin des vertices et des normales
                {
                    // le centre de la source et la moyenne de ses coordonnées
                    coordFloat.x = coordFloat.x/nb_ver;
                    coordFloat.y = coordFloat.y/nb_ver;
                    coordFloat.z = coordFloat.z/nb_ver;

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

                    /*
                    int i = m_materiaux.indexOf(materiau[1]);

                    if(i == -1) // si c'est la première fois qu'on rencontre ce materiau
                    {

                       indiceMat_Curr = indiceMat; // on affecte ce numero à l'indice courant
                       indiceMat++; // on incremente l'indice du materiau
                       m_materiaux.push_back(materiau[1]); // on sauvegarde son nom dans le vecteur mat
                    }
                    else
                    {
                        indiceMat_Curr = i; // on affecte ce numero à l'indice courant
                    }*/

                    //indiceMat_Curr = 0.1; // valeur par default de l'attenuation du materiau
                    rangCoeff = 7; // valeur par default de l'absorption du materiau correspond à 50% sur toutes les bandes
                    for(int i=0 ; i < matOdeon_vect.size(); i++ )
                    {
                        if (matOdeon_vect[i] == indice[0]) // si le materiau existe dans le tableau odeon
                        {
                            //indiceMat_Curr = matOdeon.getIndMat(i);// on a 8 indices par materiau
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

                    for(int i= 1; i<=nbDonnees;i++)
                    {
                        unsigned int v,n;
                        v=indice[i*3-2].toInt() - (nb_verSource + nb_verListener) -1; // on enleve le nombre de vertex de source et listener s'il y en a car ils ne s'enregistrent pas dans les vecteurs de coordonnées et 1 pour que indice 1 corresponde à 0
                        n=indice[i*3].toInt() - (nb_norSource + nb_norListener) -1;

                        iv.push_back(v); // indice des vertex
                        in.push_back(n); // indice des normales
                        //imat.push_back(indiceMat_Curr); // indice du matériau
                    }
                    // pour chaque face on rempli le vecteur materiau avec son nom et ses 8 coefficients d'absorption
                    m_indMat.push_back(nomMat.toFloat());
                    for (int i = 0; i< 8; i++)
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
            coordFloat.x = coordFloat.x/nb_ver;
            coordFloat.y = coordFloat.y/nb_ver;
            coordFloat.z = coordFloat.z/nb_ver;

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

    // classement des coordonnées face par face
    for(int i=0; i<iv.size(); i++) // vertex
    {
        if(iv[i]<ver.size())
        {
            m_vert.push_back(ver[iv[i]].x);
            m_vert.push_back(ver[iv[i]].y);
            m_vert.push_back(ver[iv[i]].z);
        }
    }
    for(int i=0; i<in.size(); i++) //normales - pour ne prendre qu'une normale par face on prendrai comme increment i =i+3
    {
        if(in[i]<nor.size())
        {
            m_norm.push_back(nor[in[i]].x);
            m_norm.push_back(nor[in[i]].y);
            m_norm.push_back(nor[in[i]].z);

            /*
            m_indMat.push_back(imat[i]); // dupliqué trois fois pour que l'indice du materiaux soit associé à chaque coordonée de vertice
            m_indMat.push_back(imat[i]);
            m_indMat.push_back(imat[i]);
            */
        }
    }

    m_nbData = m_vert.size();

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
