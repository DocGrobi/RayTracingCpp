#include "rir.h"
#include "math.h"
#include <QMessageBox>
#include "QDebug"

// les méthodes
std::vector<float> toucheListener(Ray &rayon, Listener &listener) // par equ parametrique
{
    std::vector<float> resultat;

    std::vector<CoordVector> posOld, posNew, u;
    posOld = rayon.getPos();
    posNew = rayon.getRay();
    u = rayon.getDir();

    std::vector<bool> vivant = rayon.getRayVivant();

    float alpha, normeAL;

    CoordVector L(listener.getCentre()); // Point au centre du Listener
    float r(listener.getRayon()); // Rayon du Listener

    CoordVector AL;


    float delta, lambda1, lambda2, a, b, c;

    for(int i = 0; i<posOld.size() ; i++)
    {
        if (vivant[i]) // on ne prend pas les rayons morts
        {
            //AB = vecteur(posOld[i], posNew[i]);
            AL = vecteur(L, posOld[i]);

           //alpha = angle(AB,AL);
           normeAL = norme(AL);

            a=pow(norme(u[i]), 2);
            b= 2*produitScalaire(u[i], AL);
            c= pow(normeAL,2) - pow(r,2);
            delta = pow(b,2)-4*a*c;
            if(delta >=0)
            {
                lambda1 = (-b-sqrt(delta))/(2*a);
                lambda2 = (-b+sqrt(delta))/(2*a);
                if(lambda1 >= 0 || lambda2 >= 0) resultat.push_back(normeAL);
                else resultat.push_back(-1);
            }
           else resultat.push_back(-1);
        }
        else resultat.push_back(-1);
    }

    return resultat;
}

std::vector<float> toucheListener2(Ray &rayon, Listener &listener)
{
    std::vector<float> resultat, ray_long;
    ray_long = rayon.getLong();

    std::vector<CoordVector> posOld, dir;
    posOld = rayon.getPos();
    dir = rayon.getDir();

    std::vector<bool> vivant = rayon.getRayVivant();

    float alpha, normeAL;

    CoordVector L(listener.getCentre()); // Point au centre du Listener
    float r(listener.getRayon()); // Rayon du Listener

    CoordVector AL;

    int ray_mort(0);
    for(int i = 0; i<posOld.size() ; i++)
    {
        if (vivant[i]) // on ne prend pas les rayons morts
        {
            //AB = vecteur(posOld[i], posNew[i]);
           AL = vecteur(posOld[i], L);
           normeAL = norme(AL);

           // test si le point A n'est pas dans le listener
           if (normeAL > r)
               {
               alpha = angle(dir[i],AL);
               // test sur la direction
               //if (produitScalaire(dir[i],AL) >= 0) // on peut mettre alpha tout court car acos (dans la fonction angle) renvoi la partie positive
               if (cos(alpha) >= 0) // on peut mettre alpha tout court car acos (dans la fonction angle) renvoi la partie positive
               {
                   // test sur la distance
                   if (ray_long[i] >= normeAL)
                   {
                       // test sur l'angle
                       //if (normeAL == 0)                  resultat.push_back(normeAL);
                       //else
                       if (alpha <= asin(r/normeAL)) resultat.push_back(normeAL);

                       else resultat.push_back(-1);
                   }   else resultat.push_back(-1);
               }       else resultat.push_back(-1);
            }          else resultat.push_back(normeAL);
        }              else {resultat.push_back(-1); ray_mort++;}
    }

    qDebug() << "rayons morts :" << ray_mort;
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

std::vector<float> &SourceImage::getSI_Tps()
{
    return m_sourcesImages_Tps;
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

std::vector<std::vector<float> > &SourceImage::getCurve()
{
    return m_curve;
}

std::vector< std::vector<float> >&SourceImage::getFIR(){
    return m_FIR;
}

std::vector< std::vector<float> >&SourceImage::getFirPart(){
    return m_firPart;
}

bool SourceImage::addSourcesImages(Ray &rayon, Listener &listener, float longueurMax, const std::vector<float>& absAir, float seuil)
{
    //std::vector<bool> touche = toucheListener(rayon,listener);
    std::vector<float> touche = toucheListener2(rayon,listener);
    CoordVector C; // la source image

    std::vector<float> longueurRayonTot = rayon.getDist(); // Distance parcourue avant le dernier rebond
    //std::vector<float> longueurRayonFin = rayon.getLong();
    std::vector<CoordVector> point = rayon.getPos();
    std::vector<CoordVector> vec = rayon.getDir();
    std::vector<float> nrg = rayon.getNRGbackup();
    CoordVector A, vect;
    float temps;

    int i, k, j;
    int tailleSI = m_sourcesImages.size();
    bool SI_trouvee = false;

    //bool SI_supSeuil = false;

    //qDebug() << "seuil : " << seuil;
    if(!m_nrgSI.empty() && seuil >0) seuil*= *std::max_element(m_nrgSI.begin(), m_nrgSI.end()); // Normalisation du seuil
    //qDebug() << "seuil : " << seuil;


    for (i = 0 ; i< touche.size() ; i++) // rayon par rayon
    {
        if ((longueurRayonTot[i]+touche[i])>longueurMax) {rayon.killRay(i);}
        else
        {
            //if ((rayAuto && longueurRayonTot[i]<longueurMax && touche[i]) || (!rayAuto && touche[i]) ) // si le rayon touche le listener
            if (touche[i]>=0) // si le rayon touche le listener
            {
                A = point[i];
                vect = vec[i];

               C = A - vect*longueurRayonTot[i];

               longueurRayonTot[i]+=touche[i]; // ajout de la distance au listener à la longueur totale

               /// Nouvelle méthode
               for (j= m_sourcesImages.size()-1 ; j>=0; j--) // balayage des source images deja existante depuis la fin
               {
                   if (proche(C, m_sourcesImages[j])) // si la nouvelle SI est proche d'une ancienne
                   {
                       for (k = 0 ; k<8 ; k ++) // On ajoute les énérgies
                       {
                           //m_nrgSI[8*j+k]+=nrg[8*i+k];
                           m_nrgSI[8*j+k]+=(nrg[8*i+k] * exp(-absAir[k]*longueurRayonTot[i]));
                           //if (m_nrgSI[8*j+k]>seuil) SI_supSeuil = true; // NEW !
                       }

                       SI_trouvee = true; // on a trouvé une SI en doublons
                       break; // et on sort de la boucle
                   }
               }
               if (SI_trouvee) SI_trouvee = false; // Si on a trouvé une SI on n'a rien à faire
               else /// Ancienne méthode
               {
                    // On ajoute les coordonnées au vecteur sources images
                    m_sourcesImages.push_back(C);

                    // Pour chaque nouvelle source image on enregistre les energies des 8 bandes
                    for (k = 0 ; k<8 ; k ++)
                    {
                        //m_nrgSI.push_back(nrg[8*i+k]);
                        m_nrgSI.push_back(nrg[8*i+k] * exp(-absAir[k]*longueurRayonTot[i]));
                        //if (*m_nrgSI.end()>seuil) SI_supSeuil = true; // NEW !
                    }

                    temps = 1000 * longueurRayonTot[i] / VITESSE_SON; // en ms //  A FAIRE : utiliser longueurRayonTot

                    // On créé le vecteur des valeurs en temps
                    m_sourcesImages_Tps.push_back(temps);

                    // On garde le temps max
                    //if (temps > m_xMax) m_xMax = temps;
                }
            }
        }
    }
    qDebug() << m_sourcesImages.size();
    // suppression des si inférieurs au seuil
    if (longueurMax<1e6) // dans le cas ou on n'est pas en mode rebond fixe.
    {
        bool b;
        for(i= m_sourcesImages.size()-1; i>=0 ; i--)
        {
            b = true;
            for(k=0 ; k<8 ; k++) if(m_nrgSI[8*i+k] > seuil) b =false; // s'il y a une energie au dessus du seuil
            if(b)
            {
                m_sourcesImages.erase(m_sourcesImages.begin()+i);
                m_sourcesImages_Tps.erase(m_sourcesImages_Tps.begin()+i);
                for(k=7 ; k>=0 ; k--) m_nrgSI.erase(m_nrgSI.begin()+8*i+k);
            }
        }
    }
    qDebug() << "si size" << m_sourcesImages.size();
    qDebug() << "si temps" << m_sourcesImages_Tps.size();

    // On garde le temps max
    m_xMax = *std::max_element(m_sourcesImages_Tps.begin(), m_sourcesImages_Tps.end());

    if (m_sourcesImages.size()>tailleSI) return true;
    else return false;

   // return SI_supSeuil;
}



bool SourceImage::calculerRIR(int f_ech, std::vector<float> &absR, float gain, bool curve)
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

        // mise à l'échelle du son direct
        float sondirect = *std::min_element(m_sourcesImages_Tps.begin(), m_sourcesImages_Tps.end())*VITESSE_SON/1000; // en m

        //std::vector<float> attair;

        m_curve.clear();
        m_curve.resize(m_y.size());

        /*
        ///// REGRESSION LINEAIRE
        float xmoy, coeffA, coeffB;
        std::vector<float> ymoy;
        xmoy = std::accumulate(m_x.begin(), m_x.end(), 0)/m_x.size();
        */
        //float buf;

        for (int k = 0 ; k < 8 ; k++) // pour chaque bande
        {
            for(float& a : m_y[k])
            {
                a/=max;// normalisation

                if (sondirect>0) // -31dB à 10m
                {
                    a*= pow(10,(gain-31)/10)*100*exp(-absR[k]*sondirect)/pow(sondirect,2); // on retire l'offset du son direct (ok si le premier son n'a pas touché de paroi)
                }

                if (k>0) m_FIR[k-1].push_back(sqrt(a)); // passage en puissance
            }

            // creation des decay curve

            m_curve[k] = m_y[k];
            for (int i = m_curve[k].size()-2; i >= 0 ; i--)
            {
                m_curve[k][i] += m_curve[k][i+1];
            }
            if (curve) m_y.push_back(m_curve[k]);
            /*
            ///// REGRESSION LINEAIRE
            ymoy.push_back(std::accumulate(m_y[i].begin(), m_y[i].end(), 0)/m_y[i].size();


            for (int i = 0; i< m_x.size(); i++)
            {
                coeffA += (m_x[i]-xmoy)*(m_y[k][i]-ymoy[k]);
                coeffB += pow((m_x[i]-xmoy),2);
            }
            coeffA /=;
            */


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
                    nrg_out[8*k+j] += nrg_in[8*i+j];
                }
            }
        }
        if (!srcCommune)
        {
            si_out.push_back(si_in[i]);
            for (j=0 ; j<8 ; j++)
            {
                nrg_out.push_back(nrg_in[8*i+j]);
            }
        }
        else srcCommune = false;
    }
}
