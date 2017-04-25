#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "objreader.h"
#include "objwriter.h"
#include "QDebug"
#include "raytracing.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

   // IMPORT
   QString fichierObj = QCoreApplication::applicationDirPath() + "/meshForRayTracing.obj";
   MeshObj monMeshObj(fichierObj);
   Listener monListener = monMeshObj.getListener();
   Source maSource = monMeshObj.getSource();

   // Recuperation des pointeurs de donnée
   /*
   float *ptrVertex(0), *ptrNormals(0); //ptrVertex = new float; ptrNormals = new float;
   ptrVertex = monMeshObj.getVertex();
   ptrNormals = monMeshObj.getNormals();
   */

   //int nb_data(monMeshObj.getNb_data());



   // AFFICHAGE FENETRE
   ui->setupUi(this);
   ui->label_source->setText(maSource.afficher());
   ui->label_listener->setText(monListener.afficher());


}


MainWindow::~MainWindow()
{
    delete ui;
}


// LES BOUTONS --> il faudra par la suite eviter de refaire le calcul en utilisant des attributs
void MainWindow::on_bouton_normales_clicked()
{

    // IMPORT
    QString fichierObj = QCoreApplication::applicationDirPath() + "/meshForRayTracing.obj";

    MeshObj monMeshObj(fichierObj);

    // EXPORT
    QString fichierObj_2 = QCoreApplication::applicationDirPath() + "/meshForRayTracingEXPORT.obj";
    ObjWriter monObjWriter(fichierObj_2);

    monObjWriter.display_normales(monMeshObj.getVertex(), monMeshObj.getNormals(), monMeshObj.getNb_data());
}

void MainWindow::on_bouton_rayons_clicked()
{

    // IMPORT
    QString fichierObj = QCoreApplication::applicationDirPath() + "/meshForRayTracing.obj";

    MeshObj monMeshObj(fichierObj);
    Source maSource = monMeshObj.getSource();

    // RAYONS
    int nbRayons = 30;
    Ray monRay(1,nbRayons,maSource.centre());
    /*float *ptrRay(0);
    ptrRay = monRay.getRay();
*/
    // EXPORT
    QString fichierObj_2 = QCoreApplication::applicationDirPath() + "/meshForRayTracingEXPORT.obj";
    ObjWriter monObjWriter(fichierObj_2);

    monObjWriter.display_ray(maSource,monRay.getRay(), nbRayons);
}

void MainWindow::on_bouton_source_clicked()
{
    // IMPORT
    QString fichierObj = QCoreApplication::applicationDirPath() + "/srcForRayTracing.obj";
    MeshObj monMeshObj(fichierObj);
    Source maSource = monMeshObj.getSource();

    ui->label_source->setText(maSource.afficher());

}

void MainWindow::on_bouton_listener_clicked()
{
    // IMPORT
    QString fichierObj = QCoreApplication::applicationDirPath() + "/listenerForRayTracing.obj";
    MeshObj monMeshObj(fichierObj);
    Listener monListener = monMeshObj.getListener();

    ui->label_listener->setText(monListener.afficher());
}
