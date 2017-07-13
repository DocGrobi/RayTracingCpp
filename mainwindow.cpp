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
  m_listener(m_meshObj.getListener()), m_source(m_meshObj.getSource())
{
   // AFFICHAGE FENETRE
   ui->setupUi(this);
   ui->label_source->setText(m_source.afficher());
   ui->label_listener->setText(m_listener.afficher());

   // CHARGEMENT PARAMETRES
   ui->checkBox__rebFixe->setChecked(true);
   on_radioButton_Fibonacci_toggled(true);
   on_checkBox_rayAuto_toggled(false);
   m_nbRebond = ui->spinBox_nbRebond->value();
   m_seuilAttenuation = pow(10,(-(ui->spinBox_attenuation->value()/10)));
   m_temperature = ui->spinBox_temperature->value();
   m_freq = ui->spinBox_freqEchan->value();
   m_seuilArret = ui->spinBox_seuilArret->value();
   m_nbRayon = ui->spinBox_nbRay->value();
   m_nbFaceFeuille = ui->spinBox_nbFaceFeuille->value();
   m_fichierExport = QCoreApplication::applicationDirPath() + "/meshForRayTracingEXPORT.obj";

   // On limite le nombre de faces par feuille au nombre de face total
   ui->spinBox_nbFaceFeuille->setMaximum(m_meshObj.getVert().size()/3);

   ui->checkBox_methodeRapide->setChecked(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}


////// LES BOUTONS
void MainWindow::on_bouton_normales_clicked()
{
    // EXPORT
    ObjWriter monObjWriter(m_fichierExport, 1);

    monObjWriter.display_normales(m_meshObj.getVert());
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

    if(m_rayAuto) {
        on_checkBox_rayAuto_toggled(true);
    }
}

void MainWindow::on_bouton_rayons_clicked()
{
    // RAYONS
    Ray monRay(m_nbRayon, m_source, m_fibonacci);
    //int nbRayons = monRay.getRay().size()/3; // m_ray est composé de 3 coordonnées par rayon

    // EXPORT
    ObjWriter monObjWriter(m_fichierExport, m_nbRayon);

    // OCTREE
    if (m_methodeRapide)
    {
        m_octree.chargerRayonRacine(m_nbRayon);
    }

    // Ouvrir fenetre de progress bar
    QProgressDialog progress(this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setLabelText("working...");
        progress.setRange(0,0);
        progress.setMinimumDuration(0);
        progress.show();

    // lancer le timer
    QElapsedTimer timer2;
    timer2.start();
    m_timer.start();

    if (m_nbRebondFixe)
    {
        progress.setRange(0,m_nbRebond);

        //Méthode d'affichage incrémentale
        for (int i =0; i<m_nbRebond ; i++)
        {
            // progress bar
            progress.setValue(i);
            if (progress.wasCanceled()) i=m_nbRebond;
            if (m_methodeRapide)
            {
                m_octree.chargerRayon(monRay.getRay(), monRay.getvDir());
                if(!monRay.rebondSansMemoire(m_meshObj, -1, m_octree)) // calcul des points d'intersection entre rayons et faces
                        i=m_nbRebond; // arrete la boucle
            }
            else
            {
                if(!monRay.rebondSansMemoire(m_meshObj, -1)) // calcul des points d'intersection entre rayons et faces
                        i=m_nbRebond; // arrete la boucle
            }
            monObjWriter.rec_Vert(m_source,monRay, m_nbRayon, i, -1); // ecriture des vertex

        }
        monObjWriter.rec_Line(m_nbRayon,m_nbRebond); // ecriture des edges entre les vertex
        progress.setValue(m_nbRebond);
    }
    else
    {
        progress.setRange(0,m_nbRayon);
        progress.setValue(1);

        int i(0);
        if (m_methodeRapide)
        {
            m_octree.chargerRayon(monRay.getRay(), monRay.getvDir());
            // TANT QUE TOUS LES RAYONS NE SONT PAS MORT
            while(monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation, m_octree))
            {
                // progress bar
                progress.setValue(monRay.getRayMorts());
                if (progress.wasCanceled()) break; // arrete la boucle

                monObjWriter.rec_Vert(m_source,monRay, m_nbRayon, i, m_seuilAttenuation); // ecriture des vertex
                i++;
                m_octree.chargerRayon(monRay.getRay(), monRay.getvDir());
            }
        }
        else
        {
            // TANT QUE TOUS LES RAYONS NE SONT PAS MORT
            while(monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation))
            {
                // progress bar
                progress.setValue(monRay.getRayMorts());
                if (progress.wasCanceled()) break; // arrete la boucle

                monObjWriter.rec_Vert(m_source,monRay, m_nbRayon, i, m_seuilAttenuation); // ecriture des vertex
                i++;
            }
        }

        monObjWriter.rec_Vert(m_source,monRay, m_nbRayon, i, m_seuilAttenuation); // ecriture du dernier vertex
        monObjWriter.rec_Line(m_nbRayon,0); // ecriture des edges entre les vertex

        progress.setValue(m_nbRayon);
    }

    double temps = timer2.elapsed();
    temps = temps /1000;
    ui->lcd_timer->display(temps);

    progress.cancel();
}


void MainWindow::on_bouton_sourcesImages_clicked()
{
    // RAYONS
    Ray monRay(m_nbRayon, m_source, m_fibonacci);

    // OCTREE
    if (m_methodeRapide) {
        m_octree.chargerRayonRacine(m_nbRayon);
    }

    //SOURCES IMAGES
    SourceImage maSourceImage;

    // EXPORT
    ObjWriter monObjWriter(m_fichierExport, m_nbRayon);


    // Ouvrir fenetre de progress bar
    QProgressDialog progress(this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setLabelText("working...");
        progress.setRange(0,0);
        progress.setMinimumDuration(0);
        progress.show();

    // lancer le timer
    QElapsedTimer timer2;
    timer2.start();
    m_timer.start();


    if (m_nbRebondFixe)
    {
        progress.setRange(0,m_nbRebond);

        //Méthode d'affichage incrémentale
        for (int i =0; i<m_nbRebond ; i++)
        {
            // progress bar
            progress.setValue(i);
            if (progress.wasCanceled()) i=m_nbRebond;                        

            if (m_methodeRapide)
            {
                m_timer.restart();
                m_octree.chargerRayon(monRay.getRay(), monRay.getvDir());
                qDebug() << "temps octree : " << m_timer.restart() << "ms";
                if (!monRay.rebondSansMemoire(m_meshObj, -1, m_octree)) i=m_nbRebond;
                qDebug() << "temps rayons : " << m_timer.restart() << "ms";
            }
            else
            {
                m_timer.restart();
                if (!monRay.rebondSansMemoire(m_meshObj, -1)) i=m_nbRebond;
                qDebug() << "temps rayons : " << m_timer.restart() << "ms";
            }
            maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, m_rayAuto);
        }
        monObjWriter.display_sourceImages(maSourceImage, -1);
        progress.setValue(m_nbRebond);

    }
    else
    {
        progress.setRange(0,m_nbRayon);
        progress.setValue(1);

        if (m_methodeRapide)
        {
            // TANT QUE TOUS LES RAYONS NE SONT PAS MORT
            m_octree.chargerRayon(monRay.getRay(), monRay.getvDir());
            while(monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation, m_octree))
            {
                // progress bar
                progress.setValue(monRay.getRayMorts());
                if (progress.wasCanceled())
                            break; // arrete la boucle

                maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, m_rayAuto);
                m_octree.chargerRayon(monRay.getRay(), monRay.getvDir());
            }
        }
        else
        {
            // TANT QUE TOUS LES RAYONS NE SONT PAS MORT
            while(monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation))
            {
                // progress bar
                progress.setValue(monRay.getRayMorts());
                if (progress.wasCanceled())
                            break; // arrete la boucle

                maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, m_rayAuto);
            }
        }
        maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, m_rayAuto); // On le refait une fois à la sortie de boucle pour les dernier rayon
        monObjWriter.display_sourceImages(maSourceImage, m_seuilAttenuation);

        progress.setValue(m_nbRayon);
    }

    double temps = timer2.elapsed();
    temps = temps /1000;
    ui->lcd_timer->display(temps);

    progress.cancel();

    m_sourceImage = maSourceImage;
}

void MainWindow::on_bouton_octree_clicked()
{
    // EXPORT
    ObjWriter monObjWriter(m_fichierExport, 0);

    on_checkBox_methodeRapide_toggled(true); // recalcule l'octree
    ui->checkBox_methodeRapide->setChecked(true);

    monObjWriter.display_octree(m_octree.getVectBoite());
}

void MainWindow::on_spinBox_nbRebond_valueChanged(int arg1) {
    m_nbRebond = arg1;
}

void MainWindow::on_checkBox__rebFixe_toggled(bool checked)
{
    m_nbRebondFixe = checked;
    if (checked) {
        ui->spinBox_attenuation->setEnabled(false);
        ui->spinBox_nbRebond->setEnabled(true);
    }
    else {
        ui->spinBox_attenuation->setEnabled(true);
        ui->spinBox_nbRebond->setEnabled(false);
    }
}

void MainWindow::on_spinBox_attenuation_valueChanged(int arg1) {
    m_seuilAttenuation = pow(10,(-arg1/10));
}

void MainWindow::on_spinBox_temperature_valueChanged(int arg1) {
    m_temperature = arg1;
}

void MainWindow::on_radioButton_vertexSource_toggled(bool checked)
{
    if(checked) {
        if (ui->radioButton_Fibonacci->isChecked())
        {
            on_radioButton_Fibonacci_toggled(false);
        }
    }
    else {
        if (!ui->radioButton_Fibonacci->isChecked())
        {
            on_radioButton_Fibonacci_toggled(true);
        }
    }
}

void MainWindow::on_radioButton_Fibonacci_toggled(bool checked)
{
    if(checked) {
        m_fibonacci = true;
        if (ui->radioButton_vertexSource->isChecked())
        {
            on_radioButton_vertexSource_toggled(false);
        }
    }
    else {
        m_fibonacci = false;
        if (!ui->radioButton_vertexSource->isChecked())
        {
            on_radioButton_vertexSource_toggled(true);
        }
    }
}

void MainWindow::on_spinBox_nbRay_valueChanged(int arg1) {
    m_nbRayon = arg1;
    if(m_rayAuto) {
        on_checkBox_rayAuto_toggled(true);
    }
}

void MainWindow::on_bouton_RIR_clicked()
{
     m_sourceImage.calculerRIR(m_freq);

    // ouvre une nouvelle fenetre
    plotWindow plot;
    if (m_sourceImage.getX().size() == 1) QMessageBox::warning(NULL,"Attention","Trajet direct uniquement");
    else
    {
        plot.XY(m_sourceImage.getX(), m_sourceImage.getY(), m_seuilAttenuation);
        plot.makePlot();
        plot.setModal(true);
        plot.exec();
    }
}

void MainWindow::on_spinBox_freqEchan_valueChanged(int arg1) {
    m_freq = arg1;
}

void MainWindow::on_checkBox_rayAuto_toggled(bool checked) {
   m_rayAuto = checked;
   if(m_rayAuto)
   {
       m_longueurRayMax = sqrt(m_nbRayon/m_seuilArret)*m_listener.getRayon();
       QMessageBox msgBox;
       msgBox.setText("Temps maximum pour mesure statistique : " + QString::number(m_longueurRayMax/VITESSE_SON)+ "s");
       msgBox.setIcon(QMessageBox::Information);
       msgBox.exec();
   }
}

void MainWindow::on_spinBox_seuilArret_valueChanged(int arg1) {
    m_seuilArret = arg1;
    if(m_rayAuto) {
        on_checkBox_rayAuto_toggled(true);
    }
}

void MainWindow::on_spinBox_nbFaceFeuille_valueChanged(int arg1)
{
    m_nbFaceFeuille = arg1;
    if (arg1 > 0) on_checkBox_methodeRapide_toggled(true);
    ui->checkBox_methodeRapide->setChecked(true);
}

void MainWindow::on_checkBox_methodeRapide_toggled(bool checked)
{
    if(checked) m_octree = Octree(m_meshObj,m_nbFaceFeuille);
    m_methodeRapide = checked;
}
