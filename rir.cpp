#include "rir.h"
#include "math.h"

// les méthodes
std::vector<bool> toucheListener(Ray rayon, Listener listener)
{
    std::vector<bool> resultat;
    bool hypA, hypB, hypC;

    std::vector<float> posOld, rayNew;
    posOld = rayon.getPos();
    //dirOld = rayon.getDir();
    rayNew = rayon.getRay();

    //int n(rayon.getNbRay());

    CoordVector L(listener.getCentre()); // Point au centre du Listener
    float r(listener.getRayon()); // Rayon du Listener

    for(int i = 0; i<rayon.getNbRay() ; i=i+3)
    {

       CoordVector A(posOld[i],posOld[i+1],posOld[i+2]); // Point de départ du rayon
       CoordVector B(rayNew[i],rayNew[i+1],rayNew[i+2]); // Point d'arrivée du rayon
       //CoordVector vectDir(rayNew[n+i]-rayNew[i],rayNew[n+i+1]-rayNew[i+1],rayNew[n+i+2]-rayNew[i+2]);

       float alpha = angle(vecteur(A,B),vecteur(A,L));


       // test sur l'angle
       if (norme(vecteur(A,L)) == 0)
       {
            hypA = true;
       }
       else if (alpha <= asin(r/norme(vecteur(A,L))))
       {
           hypA = true;
       }
       else
       {
           hypA = false;
       }

       // test sur la direction
       if (cos(alpha) >= 0)
       {
           hypB = true;
       }
       else
       {
           hypB = false;
       }

       // test sur la distance
       if ( norme(vecteur(A,B)) >= norme(vecteur(A,L)) * (tan(alpha) + 1) - r)
       {
           hypC = true;
       }
       else
       {
           hypC = false;
       }

       if (hypA && hypB && hypC)
       {
           resultat.push_back(true);
       }
       else
       {
           resultat.push_back(false);
       }
    }

    return resultat;

}

CoordVector sourceImage(Ray rayon)
{

}

// La classe

SourceImage::SourceImage()
{
    m_xMax = 0;
}

SourceImage::~SourceImage()
{

}

std::vector<float> SourceImage::getSourcesImages()
{
    return m_sourcesImages;
}

std::vector<float> SourceImage::getNrgSI()
{
    return m_nrgSI;
}

std::vector<float> SourceImage::getX()
{
    return m_x;
}
std::vector<float> SourceImage::getY()
{
    return m_y;
}

void SourceImage::addSourcesImages(Ray rayon, Listener listener)
{
    std::vector<bool> touche = toucheListener(rayon,listener);
    CoordVector C; // la source image

    std::vector<float> longueurRayonTot = rayon.getDist();
    std::vector<float> longueurRayonFin = rayon.getLong();
    std::vector<float> point = rayon.getPos();
    std::vector<float> vec = rayon.getDir();
    std::vector<float> nrg = rayon.getNRG();


    for (int i = 0 ; i< touche.size() ; i++) // rayon par rayon
    {
        if (touche[i]) // si le rayon touche le listener
        {
            CoordVector A (point[3*i], point[3*i + 1], point[3*i + 2]);
            CoordVector vect(vec[3*i], vec[3*i + 1], vec[3*i + 2]);
            //float norm = norme(vect);
            float longueurRay = longueurRayonTot[i] - longueurRayonFin[i];
            /*
            C.x = -longueurRay * vect.x / norm + A.x;
            C.y = -longueurRay * vect.y / norm + A.y;
            C.z = -longueurRay * vect.z / norm + A.z;
            */
            C.x = -longueurRay * vect.x + A.x;
            C.y = -longueurRay * vect.y + A.y;
            C.z = -longueurRay * vect.z + A.z;

            // BONNE METHODE (on garde toutes les sources images) :
            // On ajoute les coordonnée au vecteur sources images
            m_sourcesImages.push_back(C.x);
            m_sourcesImages.push_back(C.y);
            m_sourcesImages.push_back(C.z);

            // Pour chaque nouvelle source image on enregistre les energies des 8 bandes
            for (int k = 0 ; k<8 ; k ++)
            {
                m_nrgSI.push_back(nrg[8*i+k]);
            }

            float temps = 1000 * norme(vecteur(C,listener.getCentre())) / VITESSE_SON; // en ms

            // On créé le vecteur des valeurs en temps
            m_sourcesImages_Tps.push_back(temps);

            // On garde le temps max
            if (temps > m_xMax)
            {
                m_xMax = temps;
            }

            // MAUVAISE METHODE :
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
}


void SourceImage::calculerRIR(int f_ech)
{

    float freq = (float)f_ech/1000; // car on a des temps en ms (convertion en float)

    int nb_ech = ceil(m_xMax*freq);
    m_x.clear();
    m_y.clear();
    m_x.resize(nb_ech, 0);
    m_y.resize(nb_ech*8, 0);

   // Abscisses
    for (float i = 0 ; i <nb_ech ; i++)
   {
       m_x[i] = i/freq; // valeurs en ms
   }

    // Ordonnées
    for (int i=0 ; i< m_sourcesImages_Tps.size(); i++) // pour chaque source image
    {      
        for (int k = 0 ; k < 8 ; k++) // pour chaque bande
        {
            m_y[round(8*m_sourcesImages_Tps[i]*freq + k) ] = m_y[round(8*m_sourcesImages_Tps[i]*freq + k)] + m_nrgSI[i*8 + k];

        }

    }


}
