#ifndef OBJREADER_H
#define OBJREADER_H

#include "physic.h"

// Les methodes
QString doubleSlash(QString s); // fonction qui remplace // par /1/
QString supprimeSlash(QString s); // fonction qui supprime les slashs d'un string

// Les classes

// Classe Source : vecteur XYZ des coordonnees du point d'emission
class Source
{
    public:
        Source();// Constructeur
        ~Source();// Destructeur
        void chargerSource(const CoordVector &cs);
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
        void chargerListener(const CoordVector &cs, float r); // affecte une valeur aux attribus
        QString afficher(); // pour afficher les coordonnées du centre et le rayon dans une fenetre

        //accesseurs
        CoordVector getCentre();
        float getRayon();

    private:
        CoordVector m_centreListener;
        float m_rayon;
};


class MeshObj
{
    public:
        MeshObj(QString s);
        ~MeshObj();

        void charger_obj(QString file_obj);

        Source getSource() const; //accesseur aux parametres de source
        Listener getListener()const; //accesseur aux parametres du listener
        std::vector<float>& getVertex() ; //accesseur au pointeur de vertex
        std::vector<float>& getNormals() ;
        std::vector<float>& getIndMat() ;
        int getNb_data() const;

    private:

        std::vector<float> m_vert;
        std::vector<float> m_norm;
        std::vector<float> m_indMat;

        int m_nbData;
        Source m_source;
        Listener m_listener;
        QVector<QString> m_materiaux;

};

#endif // OBJREADER_H

