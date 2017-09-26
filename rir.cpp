#include "rir.h"
#include "math.h"
#include <QMessageBox>
#include "QDebug"

// les méthodes
std::vector<bool> toucheListener(Ray &rayon, Listener &listener)
{
    std::vector<bool> resultat;

    std::vector<CoordVector> posOld, posNew;
    posOld = rayon.getPos();
    posNew = rayon.getRay();

    std::vector<bool> vivant = rayon.getRayVivant();

    float alpha, normeAL;

    CoordVector L(listener.getCentre()); // Point au centre du Listener
    float r(listener.getRayon()); // Rayon du Listener

    CoordVector AB, AL;

    for(int i = 0; i<posOld.size() ; i++)
    {
        if (vivant[i]) // on ne prend pas les rayons morts
        {
            AB = vecteur(posOld[i], posNew[i]);
            AL = vecteur(posOld[i], L);

           alpha = angle(AB,AL);
           normeAL = norme(AL);

           // test si le point A n'est pas dans le listener
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
        }              else resultat.push_back(false);
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
std::vector<std::vector<float> > &SourceImage::getY()
{
    return m_y;
}

std::vector< std::vector<float> >&SourceImage::getFIR(){
    return m_FIR;
}

std::vector< std::vector<float> >&SourceImage::getFirPart(){
    return m_firPart;
}

void SourceImage::addSourcesImages(Ray &rayon, Listener &listener, float longueurMax, bool rayAuto, const std::vector<float>& absAir)
{
    std::vector<bool> touche = toucheListener(rayon,listener);
    CoordVector C; // la source image

    std::vector<float> longueurRayonTot = rayon.getDist();
    std::vector<float> longueurRayonFin = rayon.getLong();
    std::vector<CoordVector> point = rayon.getPos();
    std::vector<CoordVector> vec = rayon.getDir();
    std::vector<float> nrg = rayon.getNRGbackup();
    CoordVector A, vect;
    float temps;

    int i, k;


    for (i = 0 ; i< touche.size() ; i++) // rayon par rayon
    {
        if ((rayAuto && longueurRayonTot[i]<longueurMax && touche[i]) || (!rayAuto && touche[i]) ) // si le rayon touche le listener
        {
            A = point[i];
            vect = vec[i];

            C = A - vect*(longueurRayonTot[i] - longueurRayonFin[i]);

            // On ajoute les coordonnées au vecteur sources images
            m_sourcesImages.push_back(C);

            // Pour chaque nouvelle source image on enregistre les energies des 8 bandes
            for (k = 0 ; k<8 ; k ++)
            {
                //m_nrgSI.push_back(nrg[8*i+k]);
                m_nrgSI.push_back(nrg[8*i+k] * pow(10,-absAir[k]*longueurRayonTot[i]/10));
            }

            temps = 1000 * norme(vecteur(C,listener.getCentre())) / VITESSE_SON; // en ms

            // On créé le vecteur des valeurs en temps
            m_sourcesImages_Tps.push_back(temps);

            // On garde le temps max
            if (temps > m_xMax) m_xMax = temps;
        }
    }

}



bool SourceImage::calculerRIR(int f_ech)
{
    float freq = (float)f_ech/1000; // car on a des temps en ms (convertion en float)

    int nb_ech = ceil(m_xMax*freq);

    float max(0), maxbuf(0);
    if (nb_ech > 0)
    {
        m_x.clear();
        m_y.clear();
        m_x.resize(nb_ech, 0);
        m_y.resize(8);
        m_FIR.clear();

       // Abscisses
        for (float i = 0 ; i <nb_ech ; i++)
       {
           m_x[i] = i/freq; // valeurs en ms
       }

        // Ordonnées
        m_FIR.resize(7);

        for (int k = 0 ; k < 8 ; k++) // pour chaque bande
        {
            m_y[k].resize(nb_ech);
            for (int i=0 ; i< m_sourcesImages_Tps.size(); i++) // pour chaque source image
            {
                m_y[k][floor(m_sourcesImages_Tps[i]*freq)] += m_nrgSI[i*8 + k];
            }
            maxbuf = *std::max_element(m_y[k].begin(), m_y[k].end());
            if(max<maxbuf) max = maxbuf; // recuperation du max
        }

        for (int k = 0 ; k < 8 ; k++) // pour chaque bande
        {
            for(float& a : m_y[k])
            {
                a/=max;// normalisation
                if (k>0) m_FIR[k-1].push_back(sqrt(a)); // passage en puissance
            }
        }
        return true;
    }
    else return false;
}


void SourceImage::partitionnage(int taille)
{

    int j, k;
    int i(0), l(0);
    int n = taille/2;
    int firsize = m_FIR[0].size();

    int nPart = ceil((float)firsize/(float)n)*m_FIR.size();

    m_firPart.resize(nPart);

    for (k = 0 ; k < nPart ; k++) // pour chaque fir de taille n
    {
        m_firPart[k].resize(2*n, 0); // On met des zero partout

        for (j = n ; j <2*n ; j++) // on remplit la deuxième partie de chaque m_firPart
        {
            if (i == firsize) // si on arrive à la fin de la fir
            {
                i=0; // on remet à 0 le comtpeur
                l++; // on passe à la fir suivante
                j = 2*n; // on change de m_firPart (met fin à la boucle)
            }
            else
            {
                m_firPart[k][j] = m_FIR[l][i]; // on range les valeurs
                i++; // puis on décale
            }
        }
    }
}

void partitionnage(std::vector< std::vector<float> > &fir, std::vector< std::vector<float> > &firPart, int taille)
{

    int j, k;
    int i(0), l(0);
    int n = taille/2;
    int firsize = fir[0].size();

    int nPart = ceil((float)firsize/(float)n)*fir.size();

    firPart.resize(nPart);

    for (k = 0 ; k < nPart ; k++) // pour chaque fir de taille n
    {
        firPart[k].resize(2*n, 0); // On met des zero partout

        for (j = n ; j <2*n ; j++) // on remplit la deuxième partie de chaque firPart
        {
            if (i == firsize) // si on arrive à la fin de la fir
            {
                i=0; // on remet à 0 le comtpeur
                l++; // on passe à la fir suivante
                j = 2*n; // on change de firPart (met fin à la boucle)
            }
            else
            {
                firPart[k][j] = fir[l][i]; // on range les valeurs
                i++; // puis on décale
            }
        }
    }
}

void unite(std::vector< CoordVector> &si_in, std::vector< CoordVector> &si_out, std::vector<float> &nrg_in, std::vector<float> &nrg_out, float distance)
{
    bool srcCommune = false;
    int i, j, k;

    for (i = 0; i< si_in.size() ; i++)
    {
        for (k = 0; k< si_out.size() ; k++)
        {
            if (proche(si_in[i],si_out[k], distance))
            {
                srcCommune = true;
                for (j=0 ; j<8 ; j++)
                {
                    nrg_out[k] += nrg_in[i];
                }
            }
        }
        if (!srcCommune)
        {
            si_out.push_back(si_in[i]);
            for (j=0 ; j<8 ; j++)
            {
                nrg_out.push_back(nrg_in[i+j]);
            }
        }
        else srcCommune = false;
    }
}
