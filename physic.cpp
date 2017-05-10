#include "physic.h"

//Les classes

Material::Material()
{
    // lire le fichier odéon

    // replissage à la main pour test
    m_nomMat.push_back("Mat1");
    m_nomMat.push_back("Mat2");
    m_nomMat.push_back("Mat3");

    m_indMat.push_back(0.1);
    m_indMat.push_back(0.2);
    m_indMat.push_back(0.3);

}

Material::~Material()
{
}

// Accesseurs
float Material::getIndMat(int num)
{
    return m_indMat[num];
}

std::vector<QString> Material::getNomMat()
{
    return m_nomMat;
}
