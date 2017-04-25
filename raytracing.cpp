#include "raytracing.h"
#include <math.h>
#include <vector>
#include "QDebug"
#include "QVector"


// Méthodes
CoordVector sph2cart(float ro, float theta, float phi)
{
    float x,y,z;
    x = ro*cos(phi)*cos(theta);
    y = ro*cos(phi)*sin(theta);
    z = ro*sin(phi);
    CoordVector coord(x,y,z);

    return coord;
}

// Les classes
Ray::Ray(float phy, int N, CoordVector S)
{
    m_Nray = N;
    //m_col(0);
    //m_dist(0)
    m_dMax = 0;
    m_src = S;
    float ro(1), theta(0), phi(0);

    m_ray.clear();
    // OPTION 1 : Repartition uniforme des rayons (2D pour l'instant)
    for (int i = 0; i<N ; i++)
    {
        theta = theta + 2*M_PI/N;
        CoordVector coord(sph2cart(ro,theta,phi));
        m_ray.push_back(coord.x + S.x);
        m_ray.push_back(coord.y + S.y);
        m_ray.push_back(coord.z + S.z);
    }


    //pour le debug
    //QVector<float> vector = QVector<float>::fromStdVector(m_dirTot);
    //qDebug() << vector;

    //m_ray = vector2float(m_dirTot); // poiteur vers le tableau de vecteurs rayon


    // OPTION 2 : Discretisation des rayons sur une grille uniforme de Fibonacci
    /*
    float OR = (1+sqrt(5))/2;

    std::vector<float> theta(0);
    for (int i; i<N; i++)
    {

        //float mod = (i*2*M_PI/OR) % (2*M_PI); // a inverser et comprendre
        float mod = (i*2*M_PI/OR) - (2*M_PI)*(floor((i*2*M_PI/OR)/(2*M_PI)));
        theta.push_back(mod);
    }

    std::vector<float> phi(0);
    for (int i; i<N; i++)
    {
        float angle = asin(-1 + 2/(N-1)*i);
        phi.push_back(angle);
    }
    */


}

Ray::~Ray()
{
    //free(m_ray);
}

std::vector<float> Ray::getRay() const // Accesseur au vecteur de rayons
{
    return m_ray;
}

