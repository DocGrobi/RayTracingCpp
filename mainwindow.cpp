#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "objreader.h"

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

}

MainWindow::~MainWindow()
{
    delete ui;
}
