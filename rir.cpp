#include "rir.h"
#include "math.h"
#include <QMessageBox>

// les méthodes
std::vector<bool> toucheListener(Ray &rayon, Listener &listener)
{
    std::vector<bool> resultat;

    std::vector<CoordVector> posOld, posNew;
    posOld = rayon.getPos();
    posNew = rayon.getRay();

    float alpha, normeAL;

    CoordVector L(listener.getCentre()); // Point au centre du Listener
    float r(listener.getRayon()); // Rayon du Listener

    CoordVector AB, AL;

    for(int i = 0; i<posOld.size() ; i++)
    {
        AB = vecteur(posOld[i], posNew[i]);
        AL = vecteur(posOld[i], L);

       alpha = angle(AB,AL);
       normeAL = norme(AL);

       // test si le point A est dans le listener
       if (normeAL > r)
           {
           // test sur la direction
           if (cos(alpha) >= 0) // on peut mettre alpha tout court car acos (dans la fonction angle) renvoi la partie positive
           {
               // test sur la distance
               if (norme(AB) >= normeAL)
               {
                   // test sur l'angle
                   if (normeAL == 0)                  resultat.push_back(true);
                   else if (alpha <= asin(r/normeAL)) resultat.push_back(true);

                   else resultat.push_back(false);
               }   else resultat.push_back(false);
           }       else resultat.push_back(false);
        }          else resultat.push_back(true);
    }
    return resultat;
}

// La classe

SourceImage::SourceImage()
{
    m_xMax = 0;
}

SourceImage::~SourceImage()
{
}

std::vector<CoordVector> &SourceImage::getSourcesImages()
{
    return m_sourcesImages;
}

std::vector<float> &SourceImage::getNrgSI()
{
    return m_nrgSI;
}

std::vector<float> &SourceImage::getX()
{
    return m_x;
}
std::vector<float> &SourceImage::getY()
{
    return m_y;
}

void SourceImage::addSourcesImages(Ray &rayon, Listener &listener, float longueurMax, bool rayAuto, const std::vector<float>& absAir)
{
    std::vector<bool> touche = toucheListener(rayon,listener);
    CoordVector C; // la source image
    //std::vector<float> absAir = absorptionAir(20);

    std::vector<float> longueurRayonTot = rayon.getDist();
    std::vector<float> longueurRayonFin = rayon.getLong();
    std::vector<CoordVector> point = rayon.getPos();
    std::vector<CoordVector> vec = rayon.getDir();
    std::vector<float> nrg = rayon.getNRGbackup();
    CoordVector A, vect;
    float longueurRay, temps;

    int i, k;


    for (i = 0 ; i< touche.size() ; i++) // rayon par rayon
    {
        if ((rayAuto && longueurRayonTot[i]<longueurMax && touche[i]) || (!rayAuto && touche[i]) ) // si le rayon touche le listener
        {
            A = point[i];
            vect = vec[i];
            longueurRay = longueurRayonTot[i] - longueurRayonFin[i];

            C = vect*(-longueurRay) + A;

            // BONNE METHODE (on garde toutes les sources images) :
            // On ajoute les coordonnées au vecteur sources images
            m_sourcesImages.push_back(C);

            // Pour chaque nouvelle source image on enregistre les energies des 8 bandes
            for (k = 0 ; k<8 ; k ++)
            {
                //m_nrgSI.push_back(nrg[8*i+k]);
                //m_nrgSI.push_back(nrg[8*i+k] * exp(-absAir[k]*longueurRay));
                m_nrgSI.push_back(nrg[8*i+k] * pow(10,-absAir[k]*longueurRay/10));
            }

            temps = 1000 * norme(vecteur(C,listener.getCentre())) / VITESSE_SON; // en ms

            // On créé le vecteur des valeurs en temps
            m_sourcesImages_Tps.push_back(temps);

            // On garde le temps max
            if (temps > m_xMax)
            {
                m_xMax = temps;
            }

            // MAUVAISE METHODE (à conserver pour le traitement acoustique par la suite) :
            /*
            bool srcTrouvee = false;

            for (int j = 0 ; j < m_sourcesImages.size() ; j = j+3) // test parmi les Sources images deja enregistrées
            {
                if (proche(C.x , m_sourcesImages[j]) && proche(C.y , m_sourcesImages[j+1]) && proche(C.z , m_sourcesImages[j+2]))
                {
                    // SI la source image existe deja
                    srcTrouvee = true;

                    m_nbSI[j/3]++; // on augmente le compteur du correspondant à la j/3-ème source image

                    // on cumule les énergies pour chaque bande
                    for (int k = 0 ; k<8 ; k ++)
                    {
                       m_nrgSI[j/3*8 + k] = m_nrgSI[j/3*8 + k] + nrg[8*i+k];
                    }
                }
            }
            if (!srcTrouvee) // S'il s'agit d'une nouvelle source image
            {
                // On ajoute les coordonnée au vecteur sources images
                m_sourcesImages.push_back(C.x);
                m_sourcesImages.push_back(C.y);
                m_sourcesImages.push_back(C.z);

                // On crée une nouvelle valeur au vecteur compteur et on dit qu'il y a 1 source image
                m_nbSI.push_back(1);

                // Pour chaque nouvelle source image on enregistre les energies des 8 bandes
                for (int k = 0 ; k<8 ; k ++)
                {
                    m_nrgSI.push_back(nrg[8*i+k]);
                }
            }
            */
        }
    }

}

void SourceImage::filtrerSourceImages()
{
    // FAUX NE MARCHE PAS POUR LA SPHERE
    /*
    // On ne garde que les sources images en plus de 10 exemplaires
    for (int i = 0 ; i< m_nbSI.size(); i++)
    {
        if (m_nbSI[i] > 10)
        {
            for (int j =0 ; j <3 ; j++)
            {
                m_sourcesImages_Filtrees.push_back(m_sourcesImages[3*i+j]);
            }

            for (int j =0 ; j <8 ; j++)
            {
                m_nrgSI_Filtrees.push_back(m_nrgSI[8*i+j]);
            }
        }
    }
    */
}


void SourceImage::calculerRIR(int f_ech)
{

    float freq = (float)f_ech/1000; // car on a des temps en ms (convertion en float)

    int nb_ech = ceil(m_xMax*freq);

    if(nb_ech == 0) // Securité dans le cas de la source et listener confondu et sans rebond
    {
        nb_ech = 1;
    }

    if (nb_ech <= 0) QMessageBox::critical(NULL,"Erreur","Aucune source image");
    else {

        m_x.clear();
        m_y.clear();
        m_x.resize(nb_ech, 0);
        m_y.resize(nb_ech*8, 0);


       // Abscisses
        for (float i = 0 ; i <nb_ech ; i++)
       {
           m_x[i] = i/freq; // valeurs en ms
       }
    /*
        // Ordonnées
        for (int i=0 ; i< m_sourcesImages_Tps.size(); i++) // pour chaque source image
        {
            for (int k = 0 ; k < 8 ; k++) // pour chaque bande
            {
                m_y[round(8*m_sourcesImages_Tps[i]*freq + k) ] += m_nrgSI[i*8 + k];

            }

        }
    */
        // Ordonnées
        for (int k = 0 ; k < 8 ; k++) // pour chaque bande
        {
            for (int i=0 ; i< m_sourcesImages_Tps.size(); i++) // pour chaque source image
            {
               m_y[floor(m_sourcesImages_Tps[i]*freq) + k*nb_ech] += m_nrgSI[i*8 + k];
            }

        }

        /*
        // Minoration par le seuil
        for (int j = 0; j < m_y.size(); j++)
        {
            if (m_y[j] < seuil) m_y[j] = seuil;
        }
        */
    }

}
