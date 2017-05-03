#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "objreader.h"
#include "QDebug"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_meshObj(QCoreApplication::applicationDirPath() + "/meshForRayTracing.obj")
{

   // IMPORT
   //QString fichierObj = QCoreApplication::applicationDirPath() + "/meshForRayTracing.obj";
   //MeshObj monMeshObj(fichierObj);
   //m_meshObj(fichierObj);
   //MeshObj monMeshObj = m_meshObj;
   m_listener = m_meshObj.getListener();
   m_source = m_meshObj.getSource();

   // Recuperation des pointeurs de donnée
   /*
   float *ptrVertex(0), *ptrNormals(0); //ptrVertex = new float; ptrNormals = new float;
   ptrVertex = monMeshObj.getVertex();
   ptrNormals = monMeshObj.getNormals();
   */

   //int nb_data(monMeshObj.getNb_data());



   // AFFICHAGE FENETRE
   ui->setupUi(this);
   ui->label_source->setText(m_source.afficher());
   ui->label_listener->setText(m_listener.afficher());


}


MainWindow::~MainWindow()
{
    delete ui;
}


// LES BOUTONS --> il faudra par la suite eviter de refaire le calcul en utilisant des attributs
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
    int nbRayons = 30;
    int nbRebond = 3;
    Ray monRay(1,nbRayons,m_source);



    //CALCUL DES REBONDS
    monRay.rebond(m_meshObj,nbRebond);

    nbRayons = monRay.getRay().size()/3/(nbRebond+2); // on divise par 3 coordonnées et par le nombre de rebond + 2 car pour 1 rebond on crée le point 0, le point d'intersection et le nouveau vecteur directeur
    // EXPORT
    QString fichierObj_2 = QCoreApplication::applicationDirPath() + "/meshForRayTracingEXPORT.obj";
    ObjWriter monObjWriter(fichierObj_2);

    monObjWriter.display_ray(m_source,monRay.getRay(), nbRayons,nbRebond);
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
