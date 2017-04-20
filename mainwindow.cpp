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
    ui->setupUi(this);

   QString fichierObj = QCoreApplication::applicationDirPath() + "/meshForRayTracing.obj";

   MeshObj monMeshObj(fichierObj);
   //monMeshObj.charger_obj(fichierObj);

  // MeshObj monMeshObj("meshForRayTracing.obj");
   //monMeshObj.charger_obj("meshForRayTracing.obj");

   Listener monlistener = monMeshObj.getListener();
   Source maSource = monMeshObj.getSource();

   ui->label->setText(maSource.afficher() + "\n" + monlistener.afficher() );

   // creation fichier .obj
   QString fichierObj_2 = QCoreApplication::applicationDirPath() + "/meshForRayTracingEXPORT.obj";
   ObjWriter monObjWriter(fichierObj_2);
   //monObjWriter.display_normales(monMeshObj);
   float *ptrVertex(0), *ptrNormals(0);
   ptrVertex = new float;
   ptrNormals = new float;
   ptrVertex = monMeshObj.getVertex();
   ptrNormals = monMeshObj.getNormals();
   int nb_data(monMeshObj.getNb_data());
    //qDebug() << "dans le main : " << ptr;

   CoordVector s(0,0,0);
   Ray monRay(1,10,s);



    monObjWriter.display_normales(ptrVertex, ptrNormals, nb_data);
}

MainWindow::~MainWindow()
{
    delete ui;

}
