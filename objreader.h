#ifndef OBJREADER_H
#define OBJREADER_H

#endif // OBJREADER_H

#include <QString>
#include <QVector>

// Les methodes
QString doubleSlash(QString s);
QString supprimeSlash(QString s);

// Les classes
class CoordVector
{
    /*
    Classe CoordVector : simple vecteur de coordonnées XYZ
    */
    public:
        CoordVector(float px=0,float py=0,float pz=0);
        /* CoordVector(float px=0,float py=0,float pz=0,float pa=0);
           Constructeur, prend en paramètres des flottants correspondant respectivement à x, y, z et a.
        */
        ~CoordVector();
        /* ~CoordVector();
           Destructeur, totalement inutile.
        */
        CoordVector operator=(const CoordVector &fv);
        /* CoordVector operator=(const CoordVector &fv);
           Affecte au vecteur courant le contenu du vecteur passé en argument.
           Retourne le vecteur courant ainsi modifié.
        */
        float x,y,z;
};


// Classe Source : vecteur XYZ des coordonnees du point d'emission
class Source
{

    public:
        Source();// Constructeur
        ~Source();// Destructeur
        void chargerSource(CoordVector cs);
        QString afficher() const;

    private:
        CoordVector m_centreSource;
};

// Classe Listener : vecteur XYZ des coordonnees du point de reception et le rayon de mesure
class Listener
{
      public:
        Listener();// Constructeur
        ~Listener();// Destructeur
        void chargerListener(CoordVector cs, float r);
        QString afficher();
    private:
        CoordVector m_centreListener;
        float m_rayon;

};



// pas utile pour l'instant
class Material
{
    /*
    Classe Material : définition d'un matériau, composé d'une couleur et d'un nom spécifique.
    */
    public:
        Material(float r,float g,float b,QString n);
        /* Material(float r,float g,float b,std::string n);
           Constructeur, les trois premiers arguments représentent la couleur RGB du matériau et n est son nom.
        */
        Material(Material *mat);
        /* Material(Material *mat);
           Constructeur alternatif, affecte au matériau courant le contenu du matériau passé en argument.
        */
        ~Material();
        /* ~Material();
           Destructeur, totalement inutile.
        */

        CoordVector coul;
        QString name;
};

class MeshObj
{
    /*
    Classe MeshObj : définition d'un modèle statique.
    */
    public:
        MeshObj(QString s);
        /* MeshObj(std::string,MeshObj *first=NULL);
           Constructeur, prend en arguments le nom du modèle à charger et le pointeur de la première frame si le modèle appartient à une animation (sinon laissez-le à NULL).
        */
        ~MeshObj();
        /* ~MeshObj();
           Destructeur, libère toute la mémoire qui lui a été allouée.
        */
        void charger_obj(QString file_obj);
        /* void charger_obj(std::string,MeshObj *first=NULL);
           Charge un fichier OBJ et son MTL, prend en arguments le nom du modèle à charger et le pointeur de la première frame si le modèle appartient à une animation (sinon laissez-le à NULL). Cette fonction est appelée par le constructeur.
           Aucune valeur de retour.
        */
        //void charger_mtl(QString file_mtl);
        /* void charger_mtl(std::string);
           Charge un fichier MTL, prend en argument le nom du fichier à charger. Cette fonction est appelée par charger_obj.
           Aucune valeur de retour.
        */
        Source getSource() const; //accesseur aux parametres de source
        Listener getListener()const; //accesseur aux parametres du listener

    private:
        //int n_data;
        float *vertice,*normals,*textures,*colours;
        QVector<Material*> materiaux;
        Source m_source;
        Listener m_listener;
        //int m_rayonListener;
};


