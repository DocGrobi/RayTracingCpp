#include "objreader.h"
#include "QFile"
#include "QIODevice"
#include "QTextStream"
#include "QDebug"


CoordVector::CoordVector(float px,float py,float pz):x(px),y(py),z(pz)
{
}
CoordVector::~CoordVector()
{
}
CoordVector CoordVector::operator=(const CoordVector &fv)
{
    x=fv.x;
    y=fv.y;
    z=fv.z;
    //a=fv.a;

    return *this;
}

Source::Source()
{
    //m_centreSource = new CoordVector(0,0,0);
    m_centreSource = (0,0,0);

}
Source::~Source()
{
    //delete m_centreSource;
}

void Source::chargerSource(CoordVector cs)
{
    //m_centreSource = new CoordVector(cs);
    m_centreSource = cs;
    //qDebug() << "Centre source: " << m_centreSource.x << m_centreSource.y << m_centreSource.z;
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

Material::Material(float r,float g,float b,QString n):name(n)
{
    coul.x=r;
    coul.y=g;
    coul.z=b;
}
Material::Material(Material *mat)
{
    coul=mat->coul;
    name=mat->name;
}

MeshObj::MeshObj(QString s)
{
    charger_obj(s);
}
MeshObj::~MeshObj()
{
    free(m_vertice);
    free(m_normals);

   /* for(unsigned int i=0;i<materiaux.size();i++)
        delete materiaux[i];
    materiaux.clear();
    */
}

Source MeshObj::getSource() const //accesseur aux parametres de source
{
    return m_source;
}

Listener MeshObj::getListener() const //accesseur aux parametres du listener
{
    return m_listener;
}

void MeshObj::charger_obj(QString file_obj)
{
    QVector<CoordVector> ver, nor, tex, col; // vecteurs de coordonnees
    QVector<unsigned int> iv, in, imat; // indice des points à assembler
    QVector<QString> mat; // Vecteur de materiaux
    int indiceMat = 0, indiceMat_Curr =0; // Indice du materiaux
    bool lecture_source = false, lecture_listener = false;
    CoordVector coordFloat (0,0,0);
    int nb_ver = 0, nb_verSource = 0, nb_verListener = 0, nb_norSource = 0, nb_norListener = 0;
    float x_max = 0, rayon = 0;

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
                else if(ligne[0]=='o') // nouvel objet
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

                        ver.push_back(CoordVector(x,y,z)); // C'est un QVector rempli avec les CoordVector de coordonnees des vertex

                    }

                    /*
                    else if(ligne[1]=='t') //Texture (pas utile ...)
                    {
                        QStringList coord = ligne.split(" ");
                        float x,y;
                        x = coord[1].toFloat();
                        y = coord[2].toFloat();

                        tex.push_back(CoordVector(x,y)); // C'est un QVector rempli avec les CoordVector de coordonnees de textures

                    }
                    */

                    else if(ligne[1]=='n') //Normales
                    {
                        QStringList coord = ligne.split(" ");
                        float x,y,z;
                        x = coord[1].toFloat();
                        y = coord[2].toFloat();
                        z = coord[3].toFloat();

                        nor.push_back(CoordVector(x,y,z)); // C'est un QVector rempli avec les CoordVector de coordonnees de normales
                    }
                }

                // Les matériaux
                if(ligne[0]=='u')
                {
                    QStringList materiau = ligne.split(" ");
                    int i = mat.indexOf(materiau[1]);

                    if(i == -1) // si c'est la première fois qu'on rencontre ce materiau
                    {
                       indiceMat++; // on incremente l'indice du materiau
                       indiceMat_Curr = indiceMat; // on affecte ce numero à l'indice courant
                       mat.push_back(materiau[1]); // on sauvegarde son nom dans le vecteur mat
                    }
                    else
                    {
                        indiceMat_Curr = i; // on affecte ce numero à l'indice courant
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
                        //t=indice[i*3-1].toInt();
                        n=indice[i*3].toInt() - (nb_norSource + nb_norListener) -1;


                        iv.push_back(v); // indice des vertex
                        //it.push_back(t);
                        in.push_back(n); // indice des normales
                        imat.push_back(indiceMat_Curr); // indice du matériau
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
    }

    QVector<float> tv(0), tn(0);
    QVector<int> tmat(0); // on construit ensuite les tableaux de faces x1 y1 z1 x2 y2 z2 ...

    for(int i=0; i<iv.size(); i++) // vertex
    {
        if(iv[i]<ver.size())
        {
            tv.push_back(ver[iv[i]].x);
            tv.push_back(ver[iv[i]].y);
            tv.push_back(ver[iv[i]].z);

            tmat.push_back(imat[i]); // dupliqué trois fois pour que l'indice du materiaux soit associé à chaque coordonée de vertice
            tmat.push_back(imat[i]);
            tmat.push_back(imat[i]);
        }
    }
    for(int i=0; i<in.size(); i++) //normales - pour ne prendre qu'une normale par face on prendrai comme increment i =i+3
    {
        if(in[i]<nor.size())
        {
            tn.push_back(nor[in[i]].x);
            tn.push_back(nor[in[i]].y);
            tn.push_back(nor[in[i]].z);
        }
    }

    // test
    for(int i =0; i<tv.size()-2; i=i+3)
    {
        qDebug() << "Vertex : " << tv[i] << " "<< tv[i+1]<< " "<< tv[i+2] << " " << "Materiaux : " << tmat[i] <<" -> "<< mat[tmat[i]];

    }
    //qDebug() << tmat;
    // attributs : pointeurs vers les tableaux
    m_vertice = vector2float(tv);
    m_normals = vector2float(tn);


}


float* vector2float(QVector<float>& tableau)
{
    float* t=NULL;
    t=(float*)malloc(tableau.size()*sizeof(float));
    if(t==NULL||tableau.empty())
    {
        float *t1=(float*)malloc(sizeof(float)*3);
        for(int i=0;i<3;i++)
            t1[i]=0.;
        return t1;
    }

    for(unsigned int i=0;i<tableau.size();i++)
        t[i]=tableau[i];
    return t;
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
