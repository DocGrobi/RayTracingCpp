#ifndef OBJREADER_H
#define OBJREADER_H

#include "fonction.h"
#include <QString>


// Les methodes
QString doubleSlash(QString s); // fonction qui remplace // par /1/
QString supprimeSlash(QString s); // fonction qui supprime les slashs d'un string
//float* vector2float(std::vector<float>& tableau); // fonction retournant un pointeur vers un tableau de float
//int* vector2int(std::vector<int>& tableau);

// Les classes

// Classe Source : vecteur XYZ des coordonnees du point d'emission
class Source
{
    public:
        Source();// Constructeur
        ~Source();// Destructeur
        void chargerSource(CoordVector cs);
        void chargerVertSource(float coord);
        CoordVector centre();
        std::vector<float> vert();
        QString afficher() const;

    private:
        CoordVector m_centreSource;
        std::vector<float> m_vertSource;
};

// Classe Listener : vecteur XYZ des coordonnees du point de reception et le rayon de mesure
class Listener
{
      public:
        Listener();// Constructeur
        ~Listener();// Destructeur
        void chargerListener(CoordVector cs, float r); // affecte une valeur aux attribus
        QString afficher(); // pour afficher les coordonnées du centre et le rayon dans une fenetre

    private:
        CoordVector m_centreListener;
        float m_rayon;
};

/*
// pas utile pour l'instant
class Material
{

    //Classe Material : définition d'un matériau, composé d'une couleur et d'un nom spécifique.

    public:
        Material(float r,float g,float b,QString n);
        //Constructeur, les trois premiers arguments représentent la couleur RGB du matériau et n est son nom.

        Material(Material *mat);
        // Constructeur alternatif, affecte au matériau courant le contenu du matériau passé en argument.

        ~Material();
         //Destructeur, totalement inutile.

        CoordVector coul;
        QString name;
};
*/


class MeshObj
{
    /*
    Classe MeshObj : définition d'un modèle statique.
    */
    public:
        MeshObj(QString s);
        /* Anciennement : MeshObj(std::string,MeshObj *first=NULL);
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
        std::vector<float> getVertex() const; //accesseur au pointeur de vertex
        std::vector<float> getNormals() const;
        std::vector<int> getIndMat() const;
        int getNb_data() const;

    private:

        std::vector<float> m_vert;
        std::vector<float> m_norm;
        std::vector<int> m_indMat;


        int m_nbData;
        //float *m_vertice,*m_normals;
        //int *m_indicesMateriaux; // Pointeurs vers les tableaux de stockages des vertex, des normales et des materiaux.
        Source m_source;
        Listener m_listener;
        QVector<QString> m_materiaux;


};




#endif // OBJREADER_H

