#include "physic.h"
#include "QFile"
#include "QIODevice"
#include "QTextStream"
#include "QCoreApplication"
#include "math.h"
#include <QMessageBox>



// Les Méthodes
std::vector<float> absorptionAir(int temperature)
{
    // source : https://fr.wikipedia.org/wiki/Imp%C3%A9dance_acoustique
    //http://www.formules-physique.com/categorie/429
    /*
    // tableau des temperatures
    std::vector<float> temp;
    for (int i = -10 ; i<=30 ; i=i+5)
    {
        temp.push_back(i);
    }
    // tableau des masses volumiques
    std::vector<float> mv;
    mv.push_back(1.341);
    mv.push_back(1.316);
    mv.push_back(1.293);
    mv.push_back(1.269);
    mv.push_back(1.247);
    mv.push_back(1.225);
    mv.push_back(1.204);
    mv.push_back(1.184);
    mv.push_back(1.164);

    // tableau des impédences acoustiques
    std::vector<float> ia;
    ia.push_back(436.5);
    ia.push_back(432.4);
    ia.push_back(428.3);
    ia.push_back(424.5);
    ia.push_back(420.7);
    ia.push_back(417);
    ia.push_back(413.5);
    ia.push_back(410);
    ia.push_back(406.6);

    for (int i= 0 ; i <temp.size(); i++)
    {
        if(temp[i] == temperature)
        {
            return (mv[i]*ia[i]);
        }
    }
    */

    // A 20°C

    int humidite = 30; // %
    std::vector<float> freq;
    std::vector<float> resultat;
    freq.resize(8,0);
    resultat.resize(8,0);
    freq[0] = 62.5;
    resultat[0] = 5.5 * 50/humidite * pow(freq[0]/1000,1.7)/10000;
    // tableau des fréquences
    for (int i = 0 ; i<7 ; i++)
    {
        freq[i+1] = freq[i]*2;
        resultat[i+1] = 5.5 * 50/humidite * pow(freq[i+1]/1000,1.7)/10000;
    }

    return resultat;
}

//Les classes

Material::Material()
{
    // LECTURE FICHIER ODEON
    QFile fichier(QCoreApplication::applicationDirPath() + "/Material.Li8"); // fichier de materiaux odeon

    if(fichier.open(QIODevice::ReadOnly | QIODevice::Text)) // Si on peut ouvrir le fichier
    {
        QTextStream flux(&fichier);
        while(!flux.atEnd())
        {
            QString ligne = flux.readLine(); // Lecture ligne par ligne du fichier
            QStringList donnee = ligne.split("\t");

            if (donnee[0] == "") // Si la ligne commence par "" et qu'elle comporte plusieurs données
            {
                if (donnee.size() > 1)
                {
                    //remplissage des coeff d'absorption
                    for (int i = 1; i<donnee.size() ; i++)
                    {
                        m_indMat.push_back(donnee[i].toFloat());
                    }
                }
            }
            else // Sinon on est sur la ligne du nom du materiau
            {
                // replissage du numero de materiaux
                m_nomMat.push_back(donnee[0]);
            }
        }
    }
    else
    {
        QMessageBox::critical(NULL,"Erreur","Le fichier Material.Li8 n'est pas placé dans le même dossier que l'executable de l'application. \nLa valeur d'absorption des materiaux sera par défault de 0.5");
        m_nomMat.push_back("materiauNonReference");
        for (int i = 0 ; i<64 ; i++) // la ligne du fichier Odéon avec des coefficient à 50% est la 7-eme, on inscrit donc les (7+1)*8 permiers terme du tableau
        {
            m_indMat.push_back(0.5);
        }
    }

}

Material::~Material()
{
}

// Accesseurs
float Material::getIndMat(int num)
{
    return m_indMat[num];
}

std::vector<QString> &Material::getNomMat()
{
    return m_nomMat;
}
