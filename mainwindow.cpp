#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include "math.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_meshObj(QCoreApplication::applicationDirPath() + "/meshForRayTracing.obj")
{

   // IMPORT
   m_listener = m_meshObj.getListener();
   m_source = m_meshObj.getSource();

   // AFFICHAGE FENETRE
   ui->setupUi(this);
   ui->label_source->setText(m_source.afficher());
   ui->label_listener->setText(m_listener.afficher());

   // CHARGEMENT PARAMETRES
   m_nbRebond = ui->spinBox_nbRay->value();
   m_seuilAttenuation = ui->spinBox_attenuation->value();
   m_temperature = ui->spinBox_temperature->value();
   on_checkBox__rayFixe_toggled(false);

}


MainWindow::~MainWindow()
{
    delete ui;
}



////// LES BOUTONS

void MainWindow::on_bouton_normales_clicked()
{

    // EXPORT
    QString fichierObj_2 = QCoreApplication::applicationDirPath() + "/meshForRayTracingEXPORT.obj";
    ObjWriter monObjWriter(fichierObj_2, 1);

    monObjWriter.display_normales(m_meshObj.getVertex(), m_meshObj.getNormals(), m_meshObj.getNb_data());
}

void MainWindow::on_bouton_rayons_clicked()
{
    // REBONDS
    int nbRebond = m_nbRebond;
    float seuil = pow(10,(-m_seuilAttenuation/10));

    // RAYONS
    int nbRayons = 30; // Si on n'utilise pas les vertex de la source comme rayons
    Ray monRay(1,nbRayons,m_source);
    nbRayons = monRay.getRay().size()/6; // m_ray est composé de 2 points par rayons chacun avec 3 coordonnées

    // EXPORT
    QString fichierObj_2 = QCoreApplication::applicationDirPath() + "/meshForRayTracingEXPORT.obj";
    ObjWriter monObjWriter(fichierObj_2, nbRayons);

    if (m_nbRebondFixe)
    {
        //Méthode d'affichage incrémentale
        for (int i =0; i<nbRebond ; i++)
        {
            monRay.rebondSansMemoire(m_meshObj, -1); // calcul des points d'intersection entre rayons et faces
            monObjWriter.rec_Vert(m_source,monRay, nbRayons, i, -1); // ecriture des vertex
        }
        monObjWriter.rec_Line(nbRayons,nbRebond); // ecriture des edges entre les vertex
    }
    else
    {
        int i(0);
        // TANT QUE TOUS LES RAYONS NE SONT PAS MORT
        while(monRay.rebondSansMemoire(m_meshObj, seuil))
        {
            monObjWriter.rec_Vert(m_source,monRay, nbRayons, i, seuil); // ecriture des vertex
            i++;
        }
        monObjWriter.rec_Vert(m_source,monRay, nbRayons, i, seuil); // ecriture du dernier vertex
        monObjWriter.rec_Line(nbRayons,0); // ecriture des edges entre les vertex
    }

    // Ancienne méthode :

    //monRay.rebond(m_meshObj,nbRebond);
    //nbRayons = monRay.getRay().size()/3/(nbRebond+2); // on divise par 3 coordonnées et par le nombre de rebond + 2 car pour 1 rebond on crée le point 0, le point d'intersection et le nouveau vecteur directeur
    //monObjWriter.display_ray(m_source,monRay.getRay(), nbRayons,nbRebond);


}

void MainWindow::on_bouton_source_clicked()
{
    // IMPORT
    QString fichierObj = QCoreApplication::applicationDirPath() + "/srcForRayTracing.obj";
    MeshObj monMeshObj(fichierObj);
    m_source = monMeshObj.getSource();


    ui->label_source->setText(m_source.afficher());

}

void MainWindow::on_bouton_listener_clicked()
{
    // IMPORT
    QString fichierObj = QCoreApplication::applicationDirPath() + "/listenerForRayTracing.obj";
    MeshObj monMeshObj(fichierObj);
    m_listener = monMeshObj.getListener();

    ui->label_listener->setText(m_listener.afficher());
}

void MainWindow::on_spinBox_nbRay_valueChanged(int arg1)
{
    m_nbRebond = arg1;
}

void MainWindow::on_checkBox__rayFixe_toggled(bool checked)
{
    m_nbRebondFixe = checked;
    if (checked)
    {
        ui->spinBox_attenuation->setEnabled(false);
        ui->spinBox_nbRay->setEnabled(true);
    }
    else
    {
        ui->spinBox_attenuation->setEnabled(true);
        ui->spinBox_nbRay->setEnabled(false);
    }
}

void MainWindow::on_spinBox_attenuation_valueChanged(int arg1)
{
    m_seuilAttenuation = arg1;
}

void MainWindow::on_spinBox_temperature_valueChanged(int arg1)
{
    m_temperature = arg1;
}
