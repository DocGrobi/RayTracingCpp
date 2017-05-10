#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"


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
    ObjWriter monObjWriter(fichierObj_2);

    monObjWriter.display_normales(m_meshObj.getVertex(), m_meshObj.getNormals(), m_meshObj.getNb_data());
}

void MainWindow::on_bouton_rayons_clicked()
{
    // RAYONS
    int nbRayons = 30; // Si on n'utilise pas les vertex de la source comme rayons
    Ray monRay(1,nbRayons,m_source);


    //CALCUL DES REBONDS
    int nbRebond = m_nbRebond;
    monRay.rebond(m_meshObj,nbRebond);

    nbRayons = monRay.getRay().size()/3/(nbRebond+2); // on divise par 3 coordonnées et par le nombre de rebond + 2 car pour 1 rebond on crée le point 0, le point d'intersection et le nouveau vecteur directeur

    // EXPORT
    QString fichierObj_2 = QCoreApplication::applicationDirPath() + "/meshForRayTracingEXPORT.obj";
    ObjWriter monObjWriter(fichierObj_2);

    //affichage incrémentale
    for (int i =0; i<nbRebond ; i++)
    {
        monRay.rebondSansMemoire(m_meshObj);
        monObjWriter.rec_Vert(m_source,monRay.getRay(), nbRayons,nbRebond, i);
    }
    monObjWriter.rec_Line(nbRayons,nbRebond);

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
