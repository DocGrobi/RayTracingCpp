#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include "math.h"
#include "rir.h"
#include <QProgressDialog>
#include "plotwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_meshObj(QCoreApplication::applicationDirPath() + "/meshForRayTracing.obj"),
  m_listener(m_meshObj.getListener()), m_source(m_meshObj.getSource())//, m_octree(Octree(m_meshObj, ui->spinBox_nbFaceFeuille->value()))// , m_monRay(1,1,m_source)
{

   // IMPORT
   /*
    * m_listener = m_meshObj.getListener();
   m_source = m_meshObj.getSource();
   int nbRayons = 30; // Si on n'utilise pas les vertex de la source comme rayons
   Ray monRay(1,nbRayons,m_source);
   m_monRay = monRay;
   */

   // AFFICHAGE FENETRE
   ui->setupUi(this);
   ui->label_source->setText(m_source.afficher());
   ui->label_listener->setText(m_listener.afficher());

   // CHARGEMENT PARAMETRES
   on_checkBox__rebFixe_toggled(false);
   on_radioButton_Fibonacci_toggled(true);
   on_checkBox_rayAuto_toggled(false);
   on_checkBox_methodeRapide_toggled(false);
   m_nbRebond = ui->spinBox_nbRebond->value();
   m_seuilAttenuation = pow(10,(-(ui->spinBox_attenuation->value()/10)));
   m_temperature = ui->spinBox_temperature->value();
   m_freq = ui->spinBox_freqEchan->value();
   m_seuilArret = ui->spinBox_seuilArret->value();
   m_nbRayon = ui->spinBox_nbRay->value();
   m_nbFaceFeuille = ui->spinBox_nbFaceFeuille->value();


  //Octree monOctree(m_meshObj);


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

    if(m_rayAuto)
    {
        m_longueurRayMax = sqrt(m_nbRayon/m_seuilArret)*m_listener.getRayon();
        QMessageBox msgBox;
        msgBox.setText("Temps maximum pour mesure statistique : " + QString::number(m_longueurRayMax/VITESSE_SON)+ "s");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
    }

}

void MainWindow::on_bouton_rayons_clicked()
{

    // RAYONS
    Ray monRay(m_nbRayon, m_source, m_fibonacci);
    int nbRayons = monRay.getRay().size()/3; // m_ray est composé de 3 coordonnées par rayon

    // EXPORT
    QString fichierObj_2 = QCoreApplication::applicationDirPath() + "/meshForRayTracingEXPORT.obj";
    ObjWriter monObjWriter(fichierObj_2, nbRayons);


    // Ouvrir fenetre de progress bar
    QProgressDialog progress(this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setLabelText("working...");
        progress.setRange(0,0);
        progress.setMinimumDuration(0);
        progress.show();

    // lancer le timer
    m_timer.start();

    if (m_nbRebondFixe)
    {
        progress.setRange(0,m_nbRebond);

        //Méthode d'affichage incrémentale
        for (int i =0; i<m_nbRebond ; i++)
        {
            // progress bar
            progress.setValue(i);
            if (progress.wasCanceled())
                        break;

            monRay.rebondSansMemoire(m_meshObj, -1); // calcul des points d'intersection entre rayons et faces
            monObjWriter.rec_Vert(m_source,monRay, nbRayons, i, -1); // ecriture des vertex

        }
        monObjWriter.rec_Line(nbRayons,m_nbRebond); // ecriture des edges entre les vertex
        progress.setValue(m_nbRebond);
    }
    else
    {
        progress.setRange(0,nbRayons);
        progress.setValue(1);

        int i(0);
        // TANT QUE TOUS LES RAYONS NE SONT PAS MORT
        while(monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation))
        {
            // progress bar
            progress.setValue(monRay.getRayMorts());
            if (progress.wasCanceled())
                        break; // arrete la boucle


            monObjWriter.rec_Vert(m_source,monRay, nbRayons, i, m_seuilAttenuation); // ecriture des vertex
            i++;
        }
        monObjWriter.rec_Vert(m_source,monRay, nbRayons, i, m_seuilAttenuation); // ecriture du dernier vertex
        monObjWriter.rec_Line(nbRayons,0); // ecriture des edges entre les vertex

        progress.setValue(nbRayons);
    }

    double temps = m_timer.elapsed();
    temps = temps /1000;
    ui->lcd_timer->display(temps);

    progress.cancel();

    // Ancienne méthode :

    //monRay.rebond(m_meshObj,nbRebond);
    //nbRayons = monRay.getRay().size()/3/(nbRebond+2); // on divise par 3 coordonnées et par le nombre de rebond + 2 car pour 1 rebond on crée le point 0, le point d'intersection et le nouveau vecteur directeur
    //monObjWriter.display_ray(m_source,monRay.getRay(), nbRayons,nbRebond);


}


void MainWindow::on_bouton_sourcesImages_clicked()
{
    // lancer le timer
    //m_timer.start();
    // RAYONS
    Ray monRay(m_nbRayon, m_source, m_fibonacci);
    int nbRayons = monRay.getRay().size()/6; // m_ray est composé de 2 points par rayons chacun avec 3 coordonnées

    //SOURCES IMAGES
    SourceImage maSourceImage;

    // EXPORT
    QString fichierObj_2 = QCoreApplication::applicationDirPath() + "/meshForRayTracingEXPORT.obj";
    ObjWriter monObjWriter(fichierObj_2, nbRayons);

    // Ouvrir fenetre de progress bar
    QProgressDialog progress(this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setLabelText("working...");
        progress.setRange(0,0);
        progress.setMinimumDuration(0);
        progress.show();

    // lancer le timer
    m_timer.start();
    //qDebug() << "creation rayons" << m_timer.restart() << "ms";




    if (m_nbRebondFixe)
    {
        progress.setRange(0,m_nbRebond);

        //Méthode d'affichage incrémentale
        for (int i =0; i<m_nbRebond ; i++)
        {
            // progress bar
            progress.setValue(i);
            if (progress.wasCanceled())
                        break;

            monRay.rebondSansMemoire(m_meshObj, -1); // calcul des points d'intersection entre rayons et faces

            //qDebug() << i << "eme iteration (rayons) : " << m_timer.restart() << "ms";
            maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, m_rayAuto);
            //qDebug() << i << "eme iteration (src img) : " << m_timer.restart() << "ms";
        }
        //maSourceImage.filtrerSourceImages();
        monObjWriter.display_sourceImages(maSourceImage, -1);
        progress.setValue(m_nbRebond);
        //qDebug() << "creation src img : " << m_timer.restart() << "ms";

    }
    else
    {
        progress.setRange(0,nbRayons);
        progress.setValue(1);


        // TANT QUE TOUS LES RAYONS NE SONT PAS MORT
        while(monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation))
        {
            // progress bar
            progress.setValue(monRay.getRayMorts());
            if (progress.wasCanceled())
                        break; // arrete la boucle

            maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, m_rayAuto);
        }
        maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, m_rayAuto); // On le refait une fois à la sortie de boucle pour les dernier rayon
        //maSourceImage.filtrerSourceImages();
        monObjWriter.display_sourceImages(maSourceImage, m_seuilAttenuation);

        progress.setValue(nbRayons);

    }
    double temps = m_timer.elapsed();
    temps = temps /1000;
    ui->lcd_timer->display(temps);

    progress.cancel();

    m_sourceImage = maSourceImage;
}

void MainWindow::on_bouton_octree_clicked()
{

    // EXPORT
    QString fichierObj_2 = QCoreApplication::applicationDirPath() + "/meshForRayTracingEXPORT.obj";
    ObjWriter monObjWriter(fichierObj_2, 0);

    on_checkBox_methodeRapide_toggled(true); // recalcule l'octree
    ui->checkBox_methodeRapide->setChecked(true);

    monObjWriter.display_octree(m_octree);

}

void MainWindow::on_spinBox_nbRebond_valueChanged(int arg1)
{
    m_nbRebond = arg1;
}

void MainWindow::on_checkBox__rebFixe_toggled(bool checked)
{
    m_nbRebondFixe = checked;
    if (checked)
    {
        ui->spinBox_attenuation->setEnabled(false);
        ui->spinBox_nbRebond->setEnabled(true);
    }
    else
    {
        ui->spinBox_attenuation->setEnabled(true);
        ui->spinBox_nbRebond->setEnabled(false);
    }
}

void MainWindow::on_spinBox_attenuation_valueChanged(int arg1)
{
    m_seuilAttenuation = pow(10,(-arg1/10));

}

void MainWindow::on_spinBox_temperature_valueChanged(int arg1)
{
    m_temperature = arg1;
}

void MainWindow::on_radioButton_vertexSource_toggled(bool checked)
{
    if(checked)
    {
        if (ui->radioButton_Fibonacci->isChecked())
        {
            on_radioButton_Fibonacci_toggled(false);
        }
    }
    else
    {
        if (!ui->radioButton_Fibonacci->isChecked())
        {
            on_radioButton_Fibonacci_toggled(true);
        }
    }

}

void MainWindow::on_radioButton_Fibonacci_toggled(bool checked)
{
    if(checked)
    {
        m_fibonacci = true;
        if (ui->radioButton_vertexSource->isChecked())
        {
            on_radioButton_vertexSource_toggled(false);
        }
    }
    else
    {
        m_fibonacci = false;
        if (!ui->radioButton_vertexSource->isChecked())
        {
            on_radioButton_vertexSource_toggled(true);
        }
    }
}

void MainWindow::on_spinBox_nbRay_valueChanged(int arg1)
{
    m_nbRayon = arg1;
}

void MainWindow::on_bouton_RIR_clicked()
{

     m_sourceImage.calculerRIR(m_freq);

    // ouvre une nouvelle fenetre
    plotWindow plot;
    plot.makePlot(m_sourceImage.getX(), m_sourceImage.getY());
    plot.setModal(true);
    plot.exec();

}

void MainWindow::on_spinBox_freqEchan_valueChanged(int arg1)
{
    m_freq = arg1;
}


void MainWindow::on_checkBox_rayAuto_toggled(bool checked)
{

   //ui->spinBox_nbRay->setEnabled(!checked);
   m_rayAuto = checked;
}

void MainWindow::on_spinBox_seuilArret_valueChanged(int arg1)
{
    m_seuilArret = arg1;
}


void MainWindow::on_spinBox_nbFaceFeuille_valueChanged(int arg1)
{
    m_nbFaceFeuille = arg1;
}

void MainWindow::on_checkBox_methodeRapide_toggled(bool checked)
{
    if(checked)
    {
        m_octree = Octree(m_meshObj,m_nbFaceFeuille).getVectBoite();
    }
    m_methodeRapide = checked;
}
