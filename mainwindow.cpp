/*/////////////////////////////// INFORMATIONS ///////////////////////////////////////
Software name : Just4RIR
Creation date : November 30th 2018
Last modification : November 30th 2018
Author : Robin Gueguen
License : GPL 3.0 2018
Property : Institut des Sciences du Calcul et des Données - Sorbonne Université
*/////////////////////////////////////////////////////////////////////////////////////

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include "math.h"
#include "rir.h"
#include <QProgressDialog>
#include "audio.h"
#include "physic.h"
#include "fftext.h"
#include <QVector>

void MainWindow::courbeAtmo()
{
    std::vector<float> m = absair(m_temperature, m_humidite);
    QVector<double> x,y;
    for (int i=0; i<8; i++)
    {
        x.push_back(62.5*pow(2,i));
        y.push_back((double)m[i]);
    }

    if(ui->absPlot->removeGraph(0)) ui->absPlot->addGraph();
    ui->absPlot->graph(0)->setData(x,y);
    ui->absPlot->replot();
}

void MainWindow::saveParameters()
{
    QSettings setting("RobinG", "just4RIR");
    setting.beginGroup("MainWindow");

    setting.setValue("temperature", m_temperature);
    setting.setValue("humidite", m_humidite);
    setting.setValue("nbRebond", m_nbRebond);
    setting.setValue("freq", m_freq);
    setting.setValue("nbRayon", m_nbRayon);
    setting.setValue("seuilArret", m_seuilArret);
    //setting.setValue("nbFaceFeuille", m_nbFaceFeuille);
    setting.setValue("seuilAttenuation", -10*log10(m_seuilAttenuation));
    setting.setValue("gain", m_gain);
    setting.setValue("numListener", m_numListener+1);
    setting.setValue("numSource", m_numSource+1);
    setting.setValue("merge", ui->doubleSpinBox_regrouperSI->value());

    setting.setValue("fibonacci", m_fibonacci);
    setting.setValue("rayAuto", m_rayAuto);
    //setting.setValue("methodeRapide", m_methodeRapide);
    setting.setValue("nbRebondFixe", m_nbRebondFixe);
    setting.setValue("fichierAudio",m_fichierAudio);

    setting.setValue("tab", ui->tabWidget->currentIndex());
    setting.setValue("toolbox", ui->toolBox->currentIndex());

    setting.endGroup();
}

void MainWindow::loadParameters()
{
    //Courbe du coefficient m
    ui->absPlot->addGraph();
    ui->absPlot->xAxis->setRange(62.5, 8000);
    ui->absPlot->yAxis->setRange(0, 0.06);
    ui->absPlot->xAxis->setLabel("Frequency (Hz)");
    ui->absPlot->yAxis->setLabel("Air absorption (m-1)");


    // Chargement des derniers pamaretres sauvegardés dans l'iHM
    QSettings setting("RobinG", "just4RIR");
    setting.beginGroup("MainWindow");

    ui->spinBox_temperature->setValue(setting.value("temperature", ui->spinBox_temperature->value()).toInt());
    ui->spinBox_humidite->setValue(setting.value("humidite", ui->spinBox_humidite->value()).toInt());
    ui->spinBox_nbRebond->setValue(setting.value("nbRebond", ui->spinBox_nbRebond->value()).toInt());
    ui->spinBox_freqEchan->setValue(setting.value("freq", ui->spinBox_freqEchan->value()).toInt());
    ui->spinBox_nbRay->setValue(setting.value("nbRayon", ui->spinBox_nbRay->value()).toInt());
    ui->spinBox_seuilArret->setValue(setting.value("seuilArret", ui->spinBox_seuilArret->value()).toInt());
    ui->spinBox_attenuation->setValue(setting.value("seuilAttenuation", ui->spinBox_attenuation->value()).toInt());
    ui->spinBox_gain->setValue(setting.value("gain", ui->spinBox_gain->value()).toInt());
    ui->spinBox_numListener->setValue(setting.value("numListener", ui->spinBox_numListener->value()).toInt());
    ui->spinBox_numSource->setValue(setting.value("numSource", ui->spinBox_numSource->value()).toInt());
    ui->doubleSpinBox_regrouperSI->setValue(setting.value("merge", ui->doubleSpinBox_regrouperSI->value()).toDouble());

    ui->radioButton_Fibonacci->setChecked(setting.value("fibonacci", ui->radioButton_Fibonacci->isChecked()).toBool());
    ui->checkBox_rayAuto->setChecked(setting.value("rayAuto", ui->checkBox_rayAuto->isChecked()).toBool());
    ui->radioButton_rebFixe->setChecked(setting.value("nbRebondFixe", ui->radioButton_rebFixe->isChecked()).toBool());
    ui->textEdit_AudioFile->setText(setting.value("fichierAudio", "//").toString());

    ui->tabWidget->setCurrentIndex(setting.value("tab",ui->tabWidget->currentIndex()).toInt());
    ui->toolBox->setCurrentIndex(setting.value("toolbox",ui->toolBox->currentIndex()).toInt());

    setting.endGroup();// les parametres par defaut sont ceux implementés dans le designer


    // Attribus
   m_nbFaceFeuille = 5; // nombre de faces maximum dans les feuilles de l'octree
   m_temperature = ui->spinBox_temperature->value();
   m_humidite = ui->spinBox_humidite->value();
   m_nbRebond = ui->spinBox_nbRebond->value();
   m_freq = ui->spinBox_freqEchan->value();
   m_nbRayon = ui->spinBox_nbRay->value();
   m_seuilArret = ui->spinBox_seuilArret->value();
   if(ui->radioButton_rebFixe->isChecked()) on_radioButton_rebFixe_clicked();
   else on_radioButton_seuildB_clicked();
   if(ui->radioButton_Fibonacci->isChecked()) on_radioButton_Fibonacci_clicked();
   else on_radioButton_vertexSource_clicked();

   m_seuilAttenuation = pow(10,(-(float)(ui->spinBox_attenuation->value()/10)));
   m_gain = ui->spinBox_gain->value();
   m_numListener = ui->spinBox_numListener->value()-1;
   m_numSource = ui->spinBox_numSource->value()-1;

   m_fichierExport = QCoreApplication::applicationDirPath() + "/meshForRayTracingEXPORT.obj";
   on_checkBox_rayAuto_toggled(ui->checkBox_rayAuto->isChecked());


   // OCTREE
   if (m_meshObj.getVert().size() > 200) {
       m_methodeRapide = true;
       m_octree = Octree(m_meshObj,m_nbFaceFeuille);
   }
   else m_methodeRapide = false;

}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_meshObj(QCoreApplication::applicationDirPath() + "/meshForRayTracing.obj"),
    m_source(m_meshObj.getSource()), m_listener(m_meshObj.getListener())
{
   // AFFICHAGE FENETRE
   ui->setupUi(this);

   //AFFICHAGE DES SOURCES
   for(int i = 0 ; i< m_source.getNbSource(); i++)
   {
       text = new QLabel;
       text2 = new QLabel;
       text->setText("Source" + QString::number(i));
       ui->gridLayout_4->addWidget(text,i,0, Qt::AlignLeft);
       text2->setText(CoordVector2QString2(m_source.getCentre(i)));
       ui->gridLayout_4->addWidget(text2,i,1, Qt::AlignLeft);
   }
   ui->spinBox_numSource->setMaximum(m_source.getNbSource());

   // AFFICHAGE DES LISTENERS
   for(int i = 0 ; i< m_listener.size() ; i++)
   {
       text = new QLabel;
       text2 = new QLabel;
       text->setText("Listener" + QString::number(i));
       ui->gridLayout_8->addWidget(text,i,0, Qt::AlignLeft);
       text2->setText(m_listener[i].afficher());
       ui->gridLayout_8->addWidget(text2,i,1, Qt::AlignLeft);
   }
   ui->spinBox_numListener->setMaximum(m_listener.size());
   on_spinBox_numListener_editingFinished();


   // CHARGEMENT PARAMETRES
   loadParameters();


   // PLAYER
   player = new QMediaPlayer(this);
   connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged);
   connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::on_durationChanged);
   // Recupération du chemin de fichier audio
   QFile fichierAudio(QCoreApplication::applicationDirPath() + "/config.txt");
   if (fichierAudio.exists())
   {
       fichierAudio.open(QIODevice::ReadOnly | QIODevice::Text);
       QTextStream flux(&fichierAudio);
       m_fichierAudio = flux.readLine();
       ui->textEdit_AudioFile->setText(m_fichierAudio);
   }
   else (m_fichierAudio = "//");


   // play streaming
   WavFile wav;
   if(wav.open(m_fichierAudio))
   {
       arr = new QByteArray;

       QAudioFormat format = wav.fileFormat();

       audio = new QAudioOutput(format, this);
       connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));

       buffer = new QBuffer;
       //connect(buffer, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged);
       //connect(buffer, &QMediaPlayer::durationChanged, this, &MainWindow::on_durationChanged);
   }
}


MainWindow::~MainWindow()
{
    saveParameters();
    delete ui;
}

void MainWindow::suppFichier()
{
    // Suppression des fichiers d'export existant
    QDir repertoire(QCoreApplication::applicationDirPath()); // chemin de type QDir
    QFileInfoList files = repertoire.entryInfoList(); //Liste des fichiers dans le répertoire contenant l'executable
         foreach (QFileInfo file, files)
         {
             if (file.baseName().contains("EXPORT")) {
                 QFile fichier(file.absoluteFilePath());
                 if(!fichier.remove())
                     QMessageBox::critical(NULL,"Erreur","Impossible de supprimer le fichier !");
             }
         }
}



////// LES BOUTONS

void MainWindow::on_bouton_source_clicked()
{
    // IMPORT
    QString fichierObj = QCoreApplication::applicationDirPath() + "/srcForRayTracing.obj";
    MeshObj monMeshObj(fichierObj);
    m_source = monMeshObj.getSource();

    // Suppression du texte
    supprLayoutContent(ui->gridLayout_4);

    for(int i = 0 ; i< m_source.getNbSource(); i++)
    {
        text = new QLabel;
        text2 = new QLabel;
        text->setText("Source" + QString::number(i+1));
        ui->gridLayout_4->addWidget(text,i,0, Qt::AlignLeft);
        text2->setText(CoordVector2QString2(m_source.getCentre(i)));
        ui->gridLayout_4->addWidget(text2,i,1, Qt::AlignLeft);
    }
}

void MainWindow::on_bouton_listener_clicked()
{
    // IMPORT
   QString fichierObj = QCoreApplication::applicationDirPath() + "/listenerForRayTracing.obj";
     MeshObj monMeshObj(fichierObj);
    m_listener = monMeshObj.getListener();

    // Suppression du texte
    supprLayoutContent(ui->gridLayout_8);

    for(int i = 0 ; i< m_listener.size() ; i++)
    {
        text = new QLabel;
        text2 = new QLabel;
        text->setText("Listener" + QString::number(i));
        ui->gridLayout_8->addWidget(text,i,0, Qt::AlignLeft);
        text2->setText(m_listener[i].afficher());
        ui->gridLayout_8->addWidget(text2,i,1, Qt::AlignLeft);
    }

    if(m_rayAuto) on_checkBox_rayAuto_toggled(true);
    ui->spinBox_numListener->setMaximum(m_listener.size());
}





void MainWindow::on_bouton_rayons_clicked() // Affichage de rayons
{
    suppFichier(); // Suppression des fichiers d'export existant

    // Ouvrir fenetre de progress bar
    QProgressDialog progress(this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setLabelText("working...");
        progress.setMinimumDuration(0);
        progress.show();

    // lancer le timer
    QElapsedTimer timer2;
    timer2.start();
    m_timer.start();


    if (m_sourceImage.empty()) // tracé des rayons depuis la/les source(s) initiale(s)
    {
        int ns, time;

        for (int nSrc = 0; nSrc < m_source.getNbSource() ; nSrc++)
        {
            time = m_source.getNbSource();
            if (m_numSource >= 0) // Cas d'un calcul monosource
            {
                ns = m_numSource;
                time=1;
            }
            else ns = nSrc;

            progress.setLabelText("Source " + QString::number(ns+1) + " - working...");
            progress.setRange(0,m_nbRebond*time);

            // RAYONS
            Ray monRay(m_nbRayon, m_source, ns, m_fibonacci);
            if(!m_fibonacci) m_nbRayon = monRay.getNbRay(); // Au cas où on prend la source blender


            // EXPORT
            ObjWriter monObjWriter(m_fichierExport, m_nbRayon);
            if (m_methodeRapide) m_octree.chargerRayonRacine(m_nbRayon);


            if (m_nbRebondFixe)
            {          
                //Méthode d'affichage incrémentale
                for (int i =0; i<m_nbRebond ; i++)
                {
                    progress.setValue(i+m_nbRebond*nSrc);
                    if (progress.wasCanceled()) {
                        i=m_nbRebond;
                        nSrc = m_source.getNbSource();
                    }
                    if (m_methodeRapide)
                    {
                        m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant());
                        if(!monRay.rebondSansMemoire(m_meshObj, m_octree)) // calcul des points d'intersection entre rayons et faces
                                i=m_nbRebond; // arrete la boucle si tous les rayons sont morts
                    }
                    else
                    {
                        if(!monRay.rebondSansMemoire(m_meshObj))
                                i=m_nbRebond;
                    }
                    monObjWriter.rec_Vert(m_source,ns,monRay, i); // ecriture des vertex

                }
                monObjWriter.rec_Line(m_nbRayon); // ecriture des edges entre les vertex
                progress.setValue(m_nbRebond*time);
            }
           else QMessageBox::warning(NULL,"Attention","Veuillez passer en mode rebond fixe");
            if (m_numSource >= 0) break; // Cas d'un calcul monosource
        }
    }
    else // tracé des rayons depuis les sources images
    {
        std::vector<CoordVector> si = m_sourceImage[m_numListener].getSourcesImages();
        std::vector<float> longueurRayon;
        std::vector<float> SItps=m_sourceImage[m_numListener].getRaySITps();
        std::vector<CoordVector> point;
        std::vector<CoordVector> dir;
        float tps;
        // EXPORT
        if (ui->radioButton_coordSI->isChecked())
        {
            CoordVector siUnique(ui->doubleSpinBox_x->value(),
                                 ui->doubleSpinBox_z->value(),
                                 -ui->doubleSpinBox_y->value());

            for(int i=0; i<si.size(); i++)
            {
                if(proche(siUnique, si[i], 0.01))
                {
                    point.push_back(m_sourceImage[m_numListener].getRaySI()[i]);
                    dir.push_back(m_sourceImage[m_numListener].getRaySIvec()[i]);
                    longueurRayon.push_back(m_sourceImage[m_numListener].getRaySIlong()[i]);
                    tps = SItps[i];
                    break;
                }
            }
            if(point.empty())
            {
                QMessageBox::warning(NULL,"Attention","La source image n'a pas été trouvée");
                return;
            }
            else QMessageBox::information(NULL,"Position temporelle de la source image",QString::number(tps)+"ms");
        }
        else
        {
            float sitps = ui->doubleSpinBox_siTps->value();
            float sitpserror = ui->doubleSpinBox_siTps_erreur->value();
            std::vector<int> indices;
            for(int i=0; i<si.size(); i++)
            {
                if(sitps < SItps[i] && SItps[i] < sitpserror)
                {
                    point.push_back(m_sourceImage[m_numListener].getRaySI()[i]);
                    dir.push_back(m_sourceImage[m_numListener].getRaySIvec()[i]);
                    longueurRayon.push_back(m_sourceImage[m_numListener].getRaySIlong()[i]);
                    tps = m_sourceImage[m_numListener].getRaySITps()[i];
                    indices.push_back(i);
                }
            }
            if(point.empty())
            {
                std::vector<float> ecart;
                for(int i=0; i<si.size(); i++)
                {
                    ecart.push_back(fabs(sitps-SItps[i]));
                }
                int ind = std::distance(ecart.begin(), std::min_element(ecart.begin(), ecart.end()));
                point.push_back(m_sourceImage[m_numListener].getRaySI()[ind]);
                dir.push_back(m_sourceImage[m_numListener].getRaySIvec()[ind]);
                longueurRayon.push_back(m_sourceImage[m_numListener].getRaySIlong()[ind]);
                tps = m_sourceImage[m_numListener].getRaySITps()[ind];

                QString txt = "Temps : " + QString::number(tps) + "\n";
                txt+=CoordVector2QString2(si[ind])+"\n";
                QMessageBox::information(NULL,"Source la plus proche trouvée",txt);
            }
            else
            {
                QString txt;
                for (int i=0; i<indices.size(); i++)
                {
                    txt+=CoordVector2QString2(si[indices[i]])+"\n";
                }
                QMessageBox::information(NULL,"Coordonnées de la source image",txt);
            }
        }


        ObjWriter monObjWriter(m_fichierExport, point.size());
        Ray monRay(point,dir);

        monObjWriter.rec_Vert_init(point);

        std::vector<float> dist, lg;

        if (m_methodeRapide)
        {
            m_octree.chargerRayonRacine(monRay.getNbRay());
            m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant2());

            while(monRay.rebondSansMemoire(m_meshObj, m_octree)) // TANT QUE TOUS LES RAYONS NE SONT PAS MORTS
            {
                progress.setValue(monRay.getRayMorts());
                if (progress.wasCanceled()) break; // arrete la boucle

                monObjWriter.rec_Vert(monRay);

                dist = monRay.getDist();
                lg = monRay.getLong();
                for(int i = 0;i<longueurRayon.size(); i++)
                {
                    if (dist[i]+lg[i]>longueurRayon[i])
                    {
                        monRay.killRay(i);
                        monObjWriter.rec_Vert(monRay, i, m_source.getCentre(0));
                    }
                }                
                m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant2());
            }
        }
        else
        {
            while(monRay.rebondSansMemoire(m_meshObj))
            {
                progress.setValue(monRay.getRayMorts());
                if (progress.wasCanceled()) break; // arrete la boucle


                monObjWriter.rec_Vert(monRay); // on inscrit les autres rayons

                dist = monRay.getDist();
                lg = monRay.getLong();
                for(int i = 0;i<longueurRayon.size(); i++) // on incrit tous les rayons morts
                {
                    if (dist[i]+lg[i]>longueurRayon[i])
                    {
                        monRay.killRay(i);
                        monObjWriter.rec_Vert(monRay, i, m_source.getCentre(0));
                    }
                }
            }
        }
    }

    double temps = timer2.elapsed();
    temps = temps /1000;
    ui->lcd_timer->display(temps);

    progress.cancel();
}



void MainWindow::on_bouton_faisceau_clicked() // Affiche uniquement les rayons captés par le listener
{
    suppFichier(); // Suppression des fichiers d'export existant

    // Ouvrir fenetre de progress bar
    QProgressDialog progress(this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setLabelText("working...");
        //progress.setRange(0,0);
        progress.setMinimumDuration(0);
        progress.show();

    // OCTREE
    if (m_methodeRapide) m_octree.chargerRayonRacine(m_nbRayon);

    int ns,time(m_source.getNbSource());

    for (int nSrc = 0; nSrc < m_source.getNbSource() ; nSrc++)
    {
        if (m_numSource >= 0) // Cas d'un calcul monosource
        {
            nSrc = m_source.getNbSource(); // la boucle n'a lieu qu'une fois
            ns = m_numSource;
            time=1;
        }
        else ns = nSrc;

        progress.setLabelText("Source " + QString::number(ns+1) + " - working...");
        progress.setRange(0,m_nbRebond*time);

        // EXPORT
        ObjWriter monObjWriter(m_fichierExport, m_nbRayon);
        monObjWriter.display_Beam_init();
        // RAYONS
        Ray monRay(m_nbRayon, m_source, ns, m_fibonacci);
        if(!m_fibonacci) m_nbRayon = monRay.getNbRay(); // Au cas où on prend la source blender

        if (m_nbRebondFixe)
        {
            //Méthode d'affichage incrémentale
            for (int i =0; i<m_nbRebond ; i++)
            {
                progress.setValue(i+m_nbRebond*nSrc);
                if (progress.wasCanceled()) {
                    i=m_nbRebond;
                    nSrc = m_source.getNbSource();
                }

                if (m_methodeRapide)
                {
                    m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant());
                    if(!monRay.rebondSansMemoire(m_meshObj, m_octree)) // calcul des points d'intersection entre rayons et faces
                            i=m_nbRebond; // arrete la boucle
                }
                else
                {
                    if(!monRay.rebondSansMemoire(m_meshObj))
                            i=m_nbRebond;
                }
                monObjWriter.display_Beam_vert(monRay, m_listener[0]); // ecriture des vertex

            }
            monObjWriter.display_Beam_line(); // ecriture des edges entre les vertex

        }
        else  QMessageBox::warning(NULL,"Beware !","Please select un fixed number of iteration");
        progress.setValue(m_nbRebond*time);
    }
}




void MainWindow::on_bouton_sourcesImages_clicked() // Calcul des positions et énergies des sources images pour chaque source et chaque listener
{
    suppFichier(); // Suppression des fichiers d'export existant

    std::vector<float> absAir = absair(m_temperature, m_humidite);

    // OCTREE
    if (m_methodeRapide) m_octree.chargerRayonRacine(m_nbRayon);

    //SOURCES IMAGES
    SourceImage SI;
    std::vector<SourceImage> maSourceImage;
    maSourceImage.resize(m_listener.size(), SI);


    // Ouvrir fenetre de progress bar
    QProgressDialog progress(this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setLabelText("working...");
        //progress.setRange(0,0);
        progress.setMinimumDuration(0);
        progress.show();

    // lancer le timer
    QElapsedTimer timer2;
    timer2.start();
    m_timer.start();
    int i, j, ns, time(m_source.getNbSource());

    for (int nSrc = 0; nSrc < m_source.getNbSource() ; nSrc++)
    {

        if (m_numSource >= 0) // Cas d'un calcul monosource
        {
            nSrc = m_source.getNbSource(); // la boucle n'a lieu qu'une fois
            ns = m_numSource;
            time=1;
        }
        else ns = nSrc;

        progress.setLabelText("Source " + QString::number(ns+1) + " - working...");


        // RAYONS
        Ray monRay(m_nbRayon, m_source, ns, m_fibonacci);
        if(!m_fibonacci) m_nbRayon = monRay.getNbRay(); // Au cas où on prend la source blender

        if (m_nbRebondFixe)
        {
            progress.setRange(0,m_nbRebond*time);

            //Méthode d'affichage incrémentale
            for (i =0; i<m_nbRebond ; i++)
            {
                progress.setValue(i+m_nbRebond*nSrc);
                if (progress.wasCanceled()) {
                    i=m_nbRebond;
                    nSrc=m_source.getNbSource();
                }

                if (m_methodeRapide)
                {
                    m_timer.restart();
                    m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant());
                    if (!monRay.rebondSansMemoire(m_meshObj, m_octree)) i=m_nbRebond;
                }
                else
                {
                    m_timer.restart();
                    if (!monRay.rebondSansMemoire(m_meshObj)) i=m_nbRebond;
                }
                for(j = 0; j < m_listener.size() ; j++)
                {
                   maSourceImage[j].addSourcesImages(monRay , m_listener[j], 1e6, absAir,-1 );
                }
            }
            progress.setValue(m_nbRebond*time);
        }
        else
        {
            progress.setRange(0,m_nbRayon*time);
            progress.setValue(1);

            bool seuilNonAtteind = true;

            while (seuilNonAtteind)
            {
                progress.setValue(monRay.getRayMorts()+m_nbRayon*nSrc);
                if (progress.wasCanceled())
                {
                    nSrc = m_source.getNbSource();
                    break; // arrete la boucle
                }
                if (m_methodeRapide)
                {
                    m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant());
                    monRay.rebondSansMemoire(m_meshObj, m_octree);// en plus
                }
                else
                {
                    monRay.rebondSansMemoire(m_meshObj);
                }

                seuilNonAtteind = false;
                for( j = 0; j < m_listener.size() ; j++)
                {
                   if (maSourceImage[j].addSourcesImages(monRay , m_listener[j], m_longueurRayMax[j], absAir,m_seuilAttenuation ))
                       seuilNonAtteind = true;
                }
            }
            progress.setValue(m_nbRayon*time);
        }
    }

    double temps = timer2.elapsed();
    temps = temps /1000;
    ui->lcd_timer->display(temps);

    progress.cancel();

    m_sourceImage = maSourceImage;

}


void MainWindow::on_bouton_projection_clicked()
{
    if (m_sourceImage.empty()) QMessageBox::warning(NULL,"Attention","Veuillez calculer les sources images");
    else
    {
        std::vector<CoordVector> SI = m_sourceImage[m_numListener].getSourcesImages();
        std::vector<CoordVector> SI2;
        std::vector<float> nrg = m_sourceImage[m_numListener].getNrgSI();
        std::vector<float> nrg2;
        int seuil = ui->doubleSpinBox_regrouperSI->value();

        unite(SI,SI2,nrg, nrg2,seuil); // unicité des sources images

        qDebug() << "SI : " << SI.size() << " ; SI2 : " << SI2.size() << " ; nrg : " << nrg2.size();

        if (SI2.size() < 2) QMessageBox::warning(NULL,"Attention","Pas assez de sources images");
        else
        {
            //EXPORT
            suppFichier(); // Suppression des fichiers d'export existant
            ObjWriter monObjWriter(m_fichierExport, SI2.size());
            if (ui->checkBox_projeter->isChecked())
            {
                // création des impacts
                Ray monRay(m_listener[m_numListener].getCentre(), SI2);
                if (m_methodeRapide)
                {
                    m_octree.chargerRayonRacine(monRay.getNbRay());
                    m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant());
                    monRay.rebondSansMemoire(m_meshObj,m_octree);
                }
                else monRay.rebondSansMemoire(m_meshObj);

                monObjWriter.display_coloredTriangle(monRay.getRay(),nrg2, m_listener[m_numListener].getCentre(), m_source.getCentre(), m_seuilAttenuation);
            }
            else
                monObjWriter.display_coloredTriangle(SI2, nrg2, m_listener[m_numListener].getCentre(), m_source.getCentre(), m_seuilAttenuation);
        }
        for(auto &a : SI2) a.debug();
    }
}

void MainWindow::on_bouton_RIR_clicked()
{
    // ouvre une nouvelle fenetre
    plotWindow *plot = new plotWindow;
    std::vector<float> absR = absair(m_temperature, m_humidite);
    if (m_sourceImage[m_numListener].calculerRIR(m_freq, absR, m_gain, ui->checkBox_decaycurve->isChecked(), m_seuilAttenuation))
    {
        // recupération du min
        std::vector<double> y = m_sourceImage[m_numListener].getY()[0];
        double max = *std::max_element(y.begin(), y.end());

        plot->XY(m_sourceImage[m_numListener].getX(), m_sourceImage[m_numListener].getY(), max*m_seuilAttenuation);
        plot->makePlot();
        plot->setYLabel("SPL (dB)");
        plot->setTitle("Source position : " + CoordVector2QString2(m_source.getCentre())+
                                            "\nListener position : " + CoordVector2QString2(m_listener[m_numListener].getCentre()));
        QPoint position = QApplication::desktop()->screenGeometry().topLeft();
        plot->move(position);
        plot->show();

    }
    else QMessageBox::warning(NULL,"Attention","La durée de la RIR est de 0s");
}


// SLOTS DE PARAMETRAGE

void MainWindow::on_spinBox_nbRebond_valueChanged(int arg1) {
    m_nbRebond = arg1;
}
/*
void MainWindow::on_radioButton_rebFixe_toggled(bool checked)
{
    m_nbRebondFixe = checked;
    if (checked) {
        ui->spinBox_nbRebond->setEnabled(true);
        ui->spinBox_attenuation->setEnabled(false);
        ui->spinBox_seuilArret->setEnabled(false);
        //on_checkBox_rayAuto_toggled(false);
        ui->checkBox_rayAuto->setChecked(false);
    }
    else {
        ui->spinBox_nbRebond->setEnabled(false);
        ui->spinBox_attenuation->setEnabled(true);
        ui->spinBox_seuilArret->setEnabled(true);
        //on_checkBox_rayAuto_toggled(true);
        ui->checkBox_rayAuto->setChecked(true);
    }
}
*/

void MainWindow::on_spinBox_attenuation_valueChanged(int arg1) {
    m_seuilAttenuation = pow(10,(-(float)arg1/10));
}

/*
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
*/

void MainWindow::on_checkBox_rayAuto_toggled(bool checked) {
   m_rayAuto = checked;
   m_longueurRayMax.resize(m_listener.size(), 1e6);
   if(m_rayAuto)
   {       
       for (int i = 0 ; i< m_longueurRayMax.size() ; i++)
           m_longueurRayMax[i]=m_nbRayon*m_listener[i].getRayon()/(2*m_seuilArret*sqrt(m_nbRayon/m_seuilArret-1));

       if(m_longueurRayMax.size()<2 && !m_longueurRayMax.empty()) // mode mono-listener
       {
           ui->lcdTempsmax->setEnabled(true);
           ui->lcdTempsmax->display(m_longueurRayMax[0]/VITESSE_SON);
       }
       else {
           ui->lcdTempsmax->setEnabled(false);
           QString text;
           for (int i =0 ; i< m_longueurRayMax.size() ; i++) text += "Listener"+QString::number(i+1) + "\n" + m_listener[i].afficher() + "\nDurée maximum : " + QString::number(m_longueurRayMax[i]/VITESSE_SON) + "\n\n";
           QMessageBox::information(NULL,"Mode multi listener",text);
       }
   }
}

/*
void MainWindow::on_checkBox_methodeRapide_toggled(bool checked)
{

    if(checked) {
        m_octree = Octree(m_meshObj,m_nbFaceFeuille);
    }
    m_methodeRapide = checked;
}
*/
/*
void MainWindow::on_spinBox_nbFaceFeuille_editingFinished()
{
    if (ui->spinBox_nbFaceFeuille->value() != m_nbFaceFeuille)
    {
        m_nbFaceFeuille = ui->spinBox_nbFaceFeuille->value();
        on_checkBox_methodeRapide_toggled(true);
        if (ui->checkBox_methodeRapide->checkState() == Qt::Unchecked)
            ui->checkBox_methodeRapide->setChecked(true);
    }
}
*/




void MainWindow::on_spinBox_freqEchan_editingFinished() {
     m_freq = ui->spinBox_freqEchan->value();
}

void MainWindow::on_spinBox_seuilArret_editingFinished()
{
    m_seuilArret = ui->spinBox_seuilArret->value();
    if(m_rayAuto) on_checkBox_rayAuto_toggled(true);
}

void MainWindow::on_bouton_audioFile_clicked()
{

    player->stop();// On arrete la lecture

    ui->bouton_ecouter->setText("Ecouter");

    m_fichierAudio = QFileDialog::getOpenFileName(this, tr("Open WAV File"),
                                                    m_fichierAudio,
                                                    tr("Audio (*.wav)"));
    if (m_fichierAudio !="")
    {
        ui->textEdit_AudioFile->setText(m_fichierAudio);
        QFile fichierConf(QCoreApplication::applicationDirPath() + "/config.txt");
        fichierConf.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier
        fichierConf.write(m_fichierAudio.toLatin1());
    }
    else (m_fichierAudio = "//");


}

void MainWindow::on_bouton_ecouter_clicked()
{

    if (audio->state() == QAudio::StoppedState || audio->state() == QAudio::IdleState)
    {
        if (player->state() == QMediaPlayer::PlayingState) // si lecture en cours
        {
            player->pause();
            ui->bouton_ecouter->setText("Lecture");

        }else
        if (player->state() == QMediaPlayer::PausedState) // si en pause
        {
            player->play();
            ui->bouton_ecouter->setText("Pause");
        }else
        if (player->state() == QMediaPlayer::StoppedState) // si à l'arret
        {
            player->setMedia(QUrl::fromLocalFile(m_fichierAudio));
            player->setVolume(50);
            player->play();
            ui->bouton_ecouter->setText("Pause");
        }
    }
    else
    {
        qDebug() << "audio state :" << audio->state();
        if (audio->state() == QAudio::ActiveState) audio->suspend();
        else audio->start(buffer);
    }
}


void MainWindow::on_AudioSlider_valueChanged(int value) {
    player->setPosition(value);

}

void MainWindow::on_positionChanged(qint64 position)
{
    ui->AudioSlider->setValue(position);
    if (position >= ui->AudioSlider->maximum()){
        ui->AudioSlider->setValue(0);
        if(player->currentMedia().canonicalUrl() == QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/resultat.wav"))
            ui->bouton_ecouter->setText("Resultat");
        else
            ui->bouton_ecouter->setText("Lecture");
    }
}

void MainWindow::on_durationChanged(qint64 position) {
    ui->AudioSlider->setMaximum(position);

}


void MainWindow::on_bouton_convolution_clicked()
{
// AJOUTER GAIN !

    WavFile wav;
    int nfft(1024);
    long nlog;
    int k, j, i;

    m_fichierAudio = ui->textEdit_AudioFile->toPlainText();

    if(wav.open(m_fichierAudio))
    {
        std::vector<float> absR = absair(m_temperature, m_humidite);
        if (m_sourceImage[m_numListener].calculerRIR(wav.fileFormat().sampleRate(), absR, -1, ui->checkBox_decaycurve->isChecked(), m_seuilAttenuation))
        {

            // Ouvrir fenetre de progress bar
            QProgressDialog progress(this);
                progress.setWindowModality(Qt::WindowModal);
                progress.setLabelText("working...");
                progress.setRange(0,13);
                progress.setMinimumDuration(0);
                progress.show();

            // Reinitialisation du buffer
            audio->stop();
            if (buffer->isOpen()) buffer->close();
            delete buffer;
            buffer = new QBuffer;
            buffer->open(QIODevice::ReadWrite);

            // Ecriture des données du wav
            QByteArray donnees = wav.readAll();

            const char * data=donnees.constData(); // retourne le pointeur d'accés aux données
            const qint16 * datai=reinterpret_cast<const qint16 *>(data);
            int len=donnees.size()/(sizeof(qint16));
            std::vector<float> vectWav;
            std::vector<float> x;
            float samplerate = wav.fileFormat().sampleRate()/1000;
            for (i=0;i<len;++i) {
                vectWav.push_back(((float)datai[i]));
                x.push_back((float)i/samplerate);
            }

            qDebug() << "taille du wav :" << vectWav.size();

            plotWindow *audioPlot = new plotWindow;
            audioPlot->setWindowTitle("Audio Input");
            audioPlot->XY(x,vectWav);
            audioPlot->makePlot();
            audioPlot->setYLabel("Amplitude");
            audioPlot->hideLegend();
            audioPlot->show();

            if (nfft < 257) nfft = 256; // Pour avoir une valeur plus grande que la taille des filtres
            else nfft = qNextPowerOfTwo(nfft-1);
            qDebug() << "nfft : " << nfft;
            nlog = round(log(nfft) / log(2));


            std::vector< std::vector<float> > filtres;
            bandFilters(filtres);

            std::vector< std::vector<float> > fir;
            // convolution avec les filtres
            for (k= 0; k<filtres.size() ; k++)
            {
                fir.push_back(convolution_temporelle(m_sourceImage[m_numListener].getFIR()[k],filtres[k]));
            }
            // Partitionnement de la FIR
            std::vector< std::vector<float> > firPart;
            partitionnage(fir, firPart, nfft);


            qDebug() << "code erreur fft init : " << fftInit(nlog);

            // fft des FIR partitionnées
            for (auto &a : firPart) { rffts(a.data(), nlog, 1); // on passe fir en frequentielle (directement enregistré dans lui-même)
            }
            qDebug() << "Firs spectrales !";

            // Somme fréquencielle
            int nFiltre = fir.size(); // nombre de bande fréquentielle
            int nPart = firPart.size()/nFiltre; // nombre de partition par bande
            qDebug() << "n fir : " << firPart.size();
            qDebug() << "n part : " << nPart;

            for (k=1; k<nFiltre ; k++)
            {
                for (i=0; i<nPart ; i++)
                {
                    *std::transform(firPart[i].begin(), firPart[i].end(), firPart[i+k*nPart].begin(), firPart[i].begin(), std::plus<float>()); // somme des 7 bandes dans la première
                }
            }

            // découpage du wav
            std::vector< std::vector<float> > wavPart;
            partitionner(vectWav, nfft, wavPart);

            qDebug() << "Wav partitionne !";
            qDebug() << "nb wavPart :" << wavPart.size();

            //passage du wav en fft
            std::vector<float> buf1;
            buf1.resize(nfft, 0);
            std::vector< std::vector<float> >  buf2;
            buf2.resize(wavPart.size()+nPart-1);
            for (auto &a : buf2) { a.resize(nfft, 0); }

            int prog = wavPart.size();
            progress.setRange(0,prog*1.1);


            //for (auto &a : wavPart) {rffts(a.data(), nlog, 1);} // on fait d'abord la fft

            // initialisation lecture temps reel
            std::vector<qint16> newData2;
            newData2.resize(nfft/2);

            qint64 pos;
            audio->setBufferSize(nfft/2);
            audio->start(buffer);

            for (k = 0; k < wavPart.size(); k++)
            {
                rffts(wavPart[k].data(), nlog, 1); // fft
                for(j=0 ; j <nPart ; j++) // pour chaque partie du filtre
                {
                    // multiplication spectrale du wav et des filtres
                    rspectprod(wavPart[k].data(), firPart[j].data(), buf1.data(), nfft);
                    std::transform(buf2[j+k].begin(), buf2[j+k].end(), buf1.begin(), buf2[j+k].begin(), std::plus<float>()); // somme terme à terme http://www.cplusplus.com/reference/algorithm/transform/
                }
                progress.setValue(k);

                riffts(buf2[k].data(), nlog, 1); //ifft

                for (i = 0; i< nfft/2 ; i++) newData2[i]=(qint16)buf2[k][i];
                //newData2.assign(buf2[k].begin(),buf2[k].begin()+nfft/2);
                arr->clear();
                arr->append(reinterpret_cast<const char *>(newData2.data()), newData2.size()*2); // on passe en QByteArray

                pos = buffer->pos(); //on garde la position courante (on pourra enlever cette etape)
                buffer->seek(buffer->size()); // on se place à la fin
                buffer->write(arr->data(), arr->length()); // on ecrit le nouveau buffer
                buffer->seek(pos); // on se remet à la bonne position pour lire
            }


            qDebug() << "Wavs spectrales convolues !";


            for (k=wavPart.size(); k < buf2.size() ; k++)
            {
                riffts(buf2[k].data(), nlog, 1);

                for (i = 0; i< nfft/2 ; i++) newData2[i]=(qint16)buf2[k][i];
                arr->clear();
                arr->append(reinterpret_cast<const char *>(newData2.data()), newData2.size()*2); // on passe en QByteArray

                pos = buffer->pos(); //on garde la position courante
                buffer->seek(buffer->size()); // on se place à la fin
                buffer->write(arr->data(), arr->length()); // on ecrit le nouveau buffer
                buffer->seek(pos); // on se remet à la bonne position pour lire
            }

/*
            // iFFT
            for (auto &a : buf2) { riffts(a.data(), nlog, 1);}
            qDebug() << "iFFT OK !";

*/
            std::vector<float> newWav;
            recombiner(buf2, newWav);
            qDebug() << "taille newWav : " << newWav.size();


            for (i=vectWav.size() ; i < newWav.size() ; i++) {
                x.push_back((float)i/samplerate);
            }

            x.clear();
            for (i=0;i<newWav.size();i++) {
                x.push_back(i);
            }
            // fin
            fftFree();


            // Création du nouveau fichier audio
            std::vector<int> newData;
            for (auto &a : newWav) { newData.push_back((int)a); }

            // Affichage du fichier de sortie
            plotWindow *audioPlot2 = new plotWindow;
            audioPlot2->setWindowTitle("Audio Output");
            audioPlot2->XY(x,newWav);
            audioPlot2->makePlot();
            audioPlot2->setYLabel("Amplitude");
            audioPlot2->hideLegend();
            audioPlot2->show();


            wav.writeNewWav(newData);
            //wav.close();
            //wav.open(m_fichierAudio);
            m_fichierAudio = QCoreApplication::applicationDirPath() + "/resultat.wav";
            ui->bouton_ecouter->setText("Resultat");

            wav.close();
            progress.setValue(1.1*prog);

        }
        else QMessageBox::warning(NULL,"Attention","La durée de la RIR est de 0s");
    }    
    else QMessageBox::warning(NULL,"Attention","Pas de fichier audio lisible");

}






void MainWindow::handleStateChanged(QAudio::State newState)
{
    switch (newState) {
        case QAudio::IdleState:
            // Finished playing (no more data)
            //audio->stop();
            //buffer->close();
            //buffer->seek(0);
            //delete audio;
            qDebug() << "idle state";
            break;

        case QAudio::StoppedState:
            // Stopped for other reasons
            if (audio->error() != QAudio::NoError) {
                qDebug() << audio->error(); // Error handling
            }
            qDebug() << "stop state";
            break;
        case QAudio::ActiveState:
            qDebug() << "playing state";
            break;

        default:
            // ... other cases as appropriate
            break;
    }
}



void MainWindow::on_bouton_saveRir_clicked()
{

    std::vector<std::vector<double> > rir = m_sourceImage[m_numListener].getY();
    if(!rir.empty())
    {
        QString chemin = QCoreApplication::applicationDirPath() + "/rirSauvegardee.txt";
        QFile fichier(chemin);

         // sauvegarder rir
        fichier.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier
        QString text;
        float freq = 62.5/2;

        for(auto &a : rir)
            {
            freq*=2;
            text = QString::number(freq) + "\n";
            fichier.write(text.toLatin1());
            text = "";
                for(auto &b : a)
                {
                    text += QString::number(b) + " ";
                    //fichier.write(text.toLatin1());
                }
                text += "\n";
                fichier.write(text.toLatin1());
            }
         fichier.close(); // ferme le fichier
    }
    else QMessageBox::warning(NULL,"Attention","La durée de la RIR est de 0s");
}


void MainWindow::on_bouton_diffRir_clicked()
{
     // différence de rir
     std::vector<std::vector<double> > diff_rir = m_sourceImage[m_numListener].getY();
     if(!diff_rir.empty())
     {
         QString chemin = QCoreApplication::applicationDirPath() + "/rirSauvegardee.txt";
         QFile fichier(chemin);

         float i(0), j;
         float min;
         QStringList valString;
         QString ligne;
         std::vector<float> val;
         std::vector<std::vector<double> > resultat;
         resultat.resize(8);
         if(fichier.open(QIODevice::ReadOnly | QIODevice::Text)) // Si on peut ouvrir le fichier
         {
             QTextStream flux(&fichier);
             while(!flux.atEnd())
             {
                 ligne = flux.readLine(); // Lecture ligne par ligne du fichier

                 valString = ligne.split(" ");
                 val.clear();
                 for(auto &a : valString) val.push_back(a.toFloat()); // récupération des valeurs d'une ligne en float
                 if (val[0] < 2 ) // on ne lit pas les lignes de titre
                 {
                     min = std::min(val.size(), diff_rir[i].size());
                     for (j = 0; j<min; j++)
                     {
                         resultat[i].push_back(std::abs(diff_rir[i][j] - val[j]));
                     }
                     i++;
                 }
             }
         }
         fichier.close();

         std::vector<float> x;
         std::vector<float> SIx = m_sourceImage[m_numListener].getX();
         for (j = 0; j<min; j++)
             x.push_back(SIx[j]);

        plotWindow *plot = new plotWindow;
         plot->XY(x, resultat, m_seuilAttenuation);
         plot->makePlot();
         plot->show();
     }
     else QMessageBox::warning(NULL,"Attention","La durée de la RIR est de 0s");


}

void MainWindow::on_bouton_data_clicked()
{
    std::vector<float> absR = absair(m_temperature, m_humidite);
    for(int l=0; l < m_listener.size() ; l++)
    {
        if(m_sourceImage[l].getY().empty()) m_sourceImage[l].calculerRIR(m_freq, absR, m_gain, ui->checkBox_decaycurve->isChecked(), m_seuilAttenuation);
        std::vector<std::vector<double> > rir = m_sourceImage[l].getY();
        std::vector<std::vector<double> > curve = m_sourceImage[l].getCurve();
        std::vector<CoordVector> si = m_sourceImage[l].getSourcesImages();
        std::vector<float> nrgSi = m_sourceImage[l].getNrgSI();

        CoordVector vect1 = vecteur(m_listener[l].getCentre(),m_source.getCentre());
        //std::vector<float> absR = absair(m_temperature, m_humidite);
        // vecteur normal dans le plan xy.
        float buf=  vect1.x;
        vect1.x = -vect1.z;
        vect1.z= buf;
        vect1=vect1/norme(vect1);

        float cosBeta, nor;
        CoordVector vect2;

        std::vector<float> maxY;
        for(auto &a : curve) maxY.push_back(*std::max_element(a.begin(), a.end()));

        std::vector<float> maxY2;
        for(auto &a : rir) maxY2.push_back(*std::max_element(a.begin(), a.end()));

            float i(0), j(0);
            std::vector<float> Tr60, Tr30, Tr30_buf, C80, D50, Ts, EDT, SPL, LF, G;
            float C80_0, C80_1, D50_0, Ts_buf, LF_buf, LF_tot, Tr60_buf, EDT_buf;
            // determination du son direct
            int sonDirect=0; // numéro de l'échantilllon
            for (j=0; j<rir[0].size(); j++)
            {
                if (rir[0][j]>0)
                {
                    sonDirect = j;
                    break;
                }
            }
            qDebug() << "son direct" << sonDirect;
            for (i=0; i<8; i++) // pour chaque bande
            {
                for (j=sonDirect; j<rir[0].size(); j++) // Pour chaque échantillon à partir du son direct
                {
                    if(curve[i][j] >= maxY[i]*1e-6) Tr60_buf = (j-sonDirect)/m_freq*1000; // on conserve le dernier terme

                    if (curve[i][j] <= maxY[i]*pow(10,-0.5) && curve[i][j] >= maxY[i]*pow(10,-3.5)) Tr30_buf.push_back((j-sonDirect)/m_freq*1000);

                    if(curve[i][j] >= maxY[i]*0.1) EDT_buf = (j-sonDirect)/m_freq*1000;

                    if(j <= 80*m_freq/1000 + sonDirect) C80_0 += rir[i][j];
                    else C80_1 += rir[i][j];

                    if(j <= 50*m_freq/1000 + sonDirect) D50_0 += rir[i][j];

                    if (j>=sonDirect)
                    Ts_buf += ((j-sonDirect)/m_freq*1000)*rir[i][j];

                }

                //RT 60
                Tr60.push_back(Tr60_buf);

                // temps pour passer de -5dB à -35dB
                Tr30.push_back(2*(*std::max_element(Tr30_buf.begin(), Tr30_buf.end())-*std::min_element(Tr30_buf.begin(), Tr30_buf.end())));
                Tr30_buf.clear();

                // temps où l'énergie descend sous 10dB
                EDT.push_back(EDT_buf*6);

                // ratio avant et après 80ms
                C80.push_back(10*log10(C80_0/C80_1));
                C80_0=0;
                C80_1=0;

                // ratio avant 50ms sur total
                D50.push_back(100*D50_0/maxY[i]);
                D50_0=0;

                // ratio temps central
                Ts.push_back(Ts_buf/maxY[i]);
                Ts_buf = 0;

                // spl
                SPL.push_back(10*log10(maxY[i])-m_gain);

                // Gain
                G.push_back(10*log10(maxY[i]/maxY2[i]));

                // LF somme des energie multiplié par l'angle
                for (j=0; j<si.size(); j++)
                {
                    vect2 = vecteur(m_listener[l].getCentre(), si[j]);
                    nor = norme(vect2);
                    if (1000*nor/VITESSE_SON < (80 + 1000*sonDirect/m_freq) && nor/VITESSE_SON > sonDirect/m_freq)
                    {
                        LF_tot += nrgSi[8*j+i];
                        if (1000*nor/VITESSE_SON > (5 + 1000*sonDirect/m_freq) && nor/VITESSE_SON > sonDirect/m_freq)
                        {
                            cosBeta = produitScalaire(vect1, vect2)/nor;
                            LF_buf += nrgSi[8*j+i]*pow(cosBeta,2);
                        }
                    }
                }
                LF.push_back(LF_buf/LF_tot);
                LF_buf=0;
                LF_tot=0;


            }
            if(m_listener.size()<2)
            {
                // Afficher valeur
                fenetre = new Data;
                for ( i=0; i<8; i++)
                {
                    fenetre->addValue(QString::number(EDT[i])   +"ms",  1, i+1);
                    fenetre->addValue(QString::number(Tr30[i])  +"ms",  2, i+1);
                    fenetre->addValue(QString::number(Tr60[i])  +"ms",  3, i+1);
                    fenetre->addValue(QString::number(SPL[i])   +"dB",  4, i+1);
                    fenetre->addValue(QString::number(G[i])     +"dB",  5, i+1);
                    fenetre->addValue(QString::number(C80[i])   +"dB",  6, i+1);
                    fenetre->addValue(QString::number(D50[i])   +"%",   7, i+1);
                    fenetre->addValue(QString::number(Ts[i])    +"ms",  8, i+1);
                    fenetre->addValue(QString::number(LF[i])    +"dB",  9, i+1);
                }
                fenetre->move((QApplication::desktop()->screenGeometry().bottomLeft()+QApplication::desktop()->screenGeometry().bottomRight())/2);
                fenetre->show();
            }

            // Exporter tableau
            QString newName(QCoreApplication::applicationDirPath() + "/data4latex.txt");
            QFile fichier(QCoreApplication::applicationDirPath() + "/data4latex.txt");
             i=0;
            while(fichier.exists()) // incrementation de version de fichier s'il existe deja
            {
                QStringList nom = fichier.fileName().split(".txt");
                if (nom[0].contains("data4latex_"))
                {
                    QStringList nom2 = nom[0].split("data4latex_");
                    newName = nom2[0] + "data4latex_" + QString::number(i) + ".txt" ;
                }
                else
                {
                    newName = nom[0] + "_" + QString::number(i) + ".txt" ;
                }

                i++;
                fichier.setFileName(newName);
            }

            fichier.open(QIODevice::WriteOnly | QIODevice::Text); // ouvre le fichier
            QString text;
            text = "\\begin{tableth} \n \\begin{tabular}{| *{9}{c|}} \n \\hline \n Facteur & 62,5Hz & 125Hz & 250Hz & 500Hz & 1kHz & 2kHz & 4kHz & 8kHz \\\\ \n \\hline \n \\hline \n";
            fichier.write(text.toLatin1());

            text = "\\gls{EDT} (ms)";
            for (i=0; i<8; i++) text+= "& " + QString::number(round(EDT[i]));
            text+= " \\\\ \n \\hline \n";
            fichier.write(text.toLatin1());
            text = "\\gls{T30} (ms)";
            for (i=0; i<8; i++) text+= "& " + QString::number(round(Tr30[i]));
            text+= " \\\\ \n \\hline \n";
            fichier.write(text.toLatin1());
            text = "\\gls{RT60} (ms)";
            for (i=0; i<8; i++) text+= "& " + QString::number(round(Tr60[i]));
            text+= " \\\\ \n \\hline \n";
            fichier.write(text.toLatin1());
            text = "\\gls{spl} (dB)";
            for (i=0; i<8; i++) text+= "& " + QString::number(round(SPL[i]));
            text+= " \\\\ \n \\hline \n";
            fichier.write(text.toLatin1());
            text = "\\gls{G} (dB)";
            for (i=0; i<8; i++) text+= "& " + QString::number(round(G[i]*10)/10);
            text+= " \\\\ \n \\hline \n";
            fichier.write(text.toLatin1());
            text = "\\gls{C80} (dB)";
            for (i=0; i<8; i++) text+= "& " + QString::number(round(C80[i]*100)/100);
            text+= " \\\\ \n \\hline \n";
            fichier.write(text.toLatin1());
            text = "\\gls{D50} (\\%)";
            for (i=0; i<8; i++) text+= "& " + QString::number(round(D50[i]*100)/100);
            text+= " \\\\ \n \\hline \n";
            fichier.write(text.toLatin1());
            text = "\\gls{Ts} (ms)";
            for (i=0; i<8; i++) text+= "& " + QString::number(round(Ts[i]));
            text+= " \\\\ \n \\hline \n";
            fichier.write(text.toLatin1());
            text = "\\gls{LF80} (dB)";
            for (i=0; i<8; i++) text+= "& " + QString::number(round(LF[i]*1000)/1000);
            text+= " \\\\ \n \\hline \n";
            fichier.write(text.toLatin1());

            CoordVector s(m_source.getCentre()), r(m_listener[l].getCentre());
            arrondir(s,2);
            arrondir(r,2);


           text = "\\end{tabular} \n \\caption{Facteurs perceptifs pour une source en [";
           text+=QString::number(s.x) + " ; " + QString::number(-s.z) + " ; " + QString::number(s.y);
           text+="] et un auditeur en [";
           text+=QString::number(r.x) + " ; " + QString::number(-r.z) + " ; " + QString::number(r.y);
           text+="] et ";
           text+= QString::number(m_nbRayon);
           text+= " rayons.} \n \\label{tab_fact} \n \\end{tableth}";

           fichier.write(text.toLatin1());
    }
}

void MainWindow::on_spinBox_gain_editingFinished()
{
    m_gain = ui->spinBox_gain->value();
}

void MainWindow::on_spinBox_numListener_editingFinished()
{
    m_numListener = ui->spinBox_numListener->value()-1;
}

void MainWindow::on_spinBox_numSource_editingFinished()
{
    m_numSource = ui->spinBox_numSource->value()-1;
}

void MainWindow::on_spinBox_nbRay_editingFinished()
{
    m_nbRayon = ui->spinBox_nbRay->value();
    if(m_rayAuto) {
        on_checkBox_rayAuto_toggled(true);
    }
}



void MainWindow::on_radioButton_coordSI_toggled(bool checked)
{
    if(checked) {
        if (ui->radioButton_tpsSI->isChecked())
        {
            on_radioButton_tpsSI_toggled(false);
        }
    }
    else {
        if (!ui->radioButton_tpsSI->isChecked())
        {
            on_radioButton_tpsSI_toggled(true);
        }
    }
}

void MainWindow::on_radioButton_tpsSI_toggled(bool checked)
{
    if(checked) {
        if (ui->radioButton_coordSI->isChecked())
        {
            on_radioButton_coordSI_toggled(false);
        }
    }
    else {
        if (!ui->radioButton_coordSI->isChecked())
        {
            on_radioButton_coordSI_toggled(true);
        }
    }
}

void MainWindow::on_doubleSpinBox_siTps_editingFinished()
{
    ui->doubleSpinBox_siTps_erreur->setValue(ui->doubleSpinBox_siTps->value()+m_tpsDiff);
}

void MainWindow::on_doubleSpinBox_siTps_erreur_editingFinished()
{
    m_tpsDiff = ui->doubleSpinBox_siTps_erreur->value() - ui->doubleSpinBox_siTps->value();
}


void supprLayoutContent(QGridLayout *layout)
{
    QLayoutItem *child;
    QWidget *wid;
    while(layout->count())
    {
        child = layout->takeAt(0);
        wid = child->widget();
        wid->deleteLater();
    }
}



void MainWindow::on_actionInfo_triggered()
{
    QString textInfo = "Software name : Just4RIR\n";
            textInfo+="Creation date : November 30th 2018\n";
            textInfo+="Last modification : November 30th 2018\n";
            textInfo+="Author : Robin Gueguen\n";
            textInfo+="License : GPL 3.0 2018\n";
            textInfo+="Property : Institut des Sciences du Calcul et des Données - Sorbonne Université";

    QMessageBox::information(NULL,"Information",textInfo);
}


void MainWindow::on_radioButton_rebFixe_clicked()
{
    m_nbRebondFixe=true;
    ui->spinBox_nbRebond->setEnabled(true);
    ui->radioButton_seuildB->setChecked(false);
    ui->spinBox_attenuation->setEnabled(false);
    ui->checkBox_rayAuto->setEnabled(false);
    ui->lcdTempsmax->setEnabled(false);
    ui->spinBox_seuilArret->setEnabled(false);
    ui->label_tpsmax->setEnabled(false);
}

void MainWindow::on_radioButton_seuildB_clicked()
{
    m_nbRebondFixe=false;
    ui->spinBox_nbRebond->setEnabled(false);
    ui->radioButton_rebFixe->setChecked(false);
    ui->spinBox_attenuation->setEnabled(true);
    ui->checkBox_rayAuto->setEnabled(true);
    ui->lcdTempsmax->setEnabled(true);
    ui->spinBox_seuilArret->setEnabled(true);
    ui->label_tpsmax->setEnabled(true);


}

void MainWindow::on_radioButton_Fibonacci_clicked()
{
    m_fibonacci=true;
    ui->spinBox_nbRay->setEnabled(true);
}

void MainWindow::on_radioButton_vertexSource_clicked()
{
    m_fibonacci=false;
    ui->spinBox_nbRay->setEnabled(false);
}

void MainWindow::on_spinBox_temperature_valueChanged(int arg1)
{
    m_temperature = arg1;
    courbeAtmo();
}

void MainWindow::on_spinBox_humidite_valueChanged(int arg1)
{
    m_humidite = arg1;
    courbeAtmo();
}
