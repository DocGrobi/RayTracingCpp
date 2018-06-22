#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include "math.h"
#include "rir.h"
#include <QProgressDialog>
#include "audio.h"
#include "physic.h"
#include "fftext.h"
#include "XFir.hpp"
#include "XHrir.hpp"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_meshObj(QCoreApplication::applicationDirPath() + "/meshForRayTracing.obj"),
    m_source(m_meshObj.getSource()), m_listener(m_meshObj.getListener())
{
   // AFFICHAGE FENETRE
   ui->setupUi(this);
   ui->label_source->setText(m_source.afficher());
   ui->label_listener->setText(m_listener.afficher());

   // CHARGEMENT PARAMETRES
   on_checkBox__rebFixe_toggled(false);
   //on_checkBox__rebFixe_toggled(true);
   on_radioButton_Fibonacci_toggled(true);
   m_nbRebond = ui->spinBox_nbRebond->value();
   m_seuilAttenuation = pow(10,(-(ui->spinBox_attenuation->value()/10)));
   m_gain = ui->spinBox_gain->value();
   m_temperature = ui->spinBox_temperature->value();
   m_humidite = ui->spinBox_humidite->value();
   m_freq = ui->spinBox_freqEchan->value();
   m_seuilArret = ui->spinBox_seuilArret->value();
   m_nbRayon = ui->spinBox_nbRay->value();
   m_nbFaceFeuille = ui->spinBox_nbFaceFeuille->value();
   m_fichierExport = QCoreApplication::applicationDirPath() + "/meshForRayTracingEXPORT.obj";
   m_longueurRayMax = 9999999;
   on_checkBox_rayAuto_toggled(true);

   // OCTREE
   int nbvert = m_meshObj.getVert().size();
   qDebug() << nbvert;
   // On limite le nombre de faces par feuille au nombre de face total
   ui->spinBox_nbFaceFeuille->setMaximum(nbvert/3);
   // active l'octree selon le nombre d'éléments
   if (nbvert > 200) {
       //on_checkBox_methodeRapide_toggled(true);
       ui->checkBox_methodeRapide->setChecked(true);
   }
   else on_checkBox_methodeRapide_toggled(false);

   // PLAYER
   player = new QMediaPlayer(this);
   //player = new QMediaPlayer(this, QMediaPlayer::StreamPlayback);
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
   else (m_fichierAudio = "/home");

   //genererMTL();

   // play streaming
   WavFile wav;

   if(wav.open(m_fichierAudio))
   {
       arr = new QByteArray;

       QAudioFormat format = wav.fileFormat();

       audio = new QAudioOutput(format, this);
       connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));

       buffer = new QBuffer;
   }

}


MainWindow::~MainWindow()
{
    buffer->close();
    delete buffer;
    delete audio;
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

/*
void MainWindow::on_bouton_normales_clicked()
{
    suppFichier(); // Suppression des fichiers d'export existant
    // EXPORT
    ObjWriter monObjWriter(m_fichierExport, 1);

    monObjWriter.display_normales(m_meshObj.getVert());
}
*/

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
    suppFichier(); // Suppression des fichiers d'export existant

    //std::vector<float> absAir = absair(m_temperature, m_humidite);

    // OCTREE
    if (m_methodeRapide) m_octree.chargerRayonRacine(m_nbRayon);

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

    for (int nSrc = 0; nSrc < m_source.getNbSource() ; nSrc++)
    {
        // EXPORT
        ObjWriter monObjWriter(m_fichierExport, m_nbRayon);
        // RAYONS
        Ray monRay(m_nbRayon, m_source, nSrc, m_fibonacci);
        if(!m_fibonacci) m_nbRayon = monRay.getNbRay(); // Au cas où on prend la source blender

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
                    m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant());
                    if(!monRay.rebondSansMemoire(m_meshObj, -1, m_octree)) // calcul des points d'intersection entre rayons et faces
                            i=m_nbRebond; // arrete la boucle
                }
                else
                {
                    if(!monRay.rebondSansMemoire(m_meshObj, -1)) // calcul des points d'intersection entre rayons et faces
                            i=m_nbRebond; // arrete la boucle
                }
                monObjWriter.rec_Vert(m_source,nSrc,monRay, m_nbRayon, i, -1); // ecriture des vertex

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
                m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant());
                // TANT QUE TOUS LES RAYONS NE SONT PAS MORT
                while(monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation, m_octree))
                {
                    // progress bar
                    progress.setValue(monRay.getRayMorts());
                    if (progress.wasCanceled()) break; // arrete la boucle

                    monObjWriter.rec_Vert(m_source,nSrc,monRay, m_nbRayon, i, m_seuilAttenuation); // ecriture des vertex
                    i++;
                    m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant());
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

                    monObjWriter.rec_Vert(m_source,nSrc,monRay, m_nbRayon, i, m_seuilAttenuation); // ecriture des vertex
                    i++;
                }
            }

            monObjWriter.rec_Vert(m_source,nSrc,monRay, m_nbRayon, i, m_seuilAttenuation); // ecriture du dernier vertex
            monObjWriter.rec_Line(m_nbRayon,0); // ecriture des edges entre les vertex

            progress.setValue(m_nbRayon);
        }
    }

    double temps = timer2.elapsed();
    temps = temps /1000;
    ui->lcd_timer->display(temps);

    progress.cancel();
}

void MainWindow::on_bouton_faisceau_clicked()
{
    suppFichier(); // Suppression des fichiers d'export existant

    // Ouvrir fenetre de progress bar
    QProgressDialog progress(this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setLabelText("working...");
        progress.setRange(0,0);
        progress.setMinimumDuration(0);
        progress.show();

    // OCTREE
    if (m_methodeRapide) m_octree.chargerRayonRacine(m_nbRayon);


    for (int nSrc = 0; nSrc < m_source.getNbSource() ; nSrc++)
    {
        // EXPORT
        ObjWriter monObjWriter(m_fichierExport, m_nbRayon);
        monObjWriter.display_Beam_init();
        // RAYONS
        Ray monRay(m_nbRayon, m_source, nSrc, m_fibonacci);
        if(!m_fibonacci) m_nbRayon = monRay.getNbRay(); // Au cas où on prend la source blender

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
                    m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant());
                    if(!monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation, m_octree)) // calcul des points d'intersection entre rayons et faces
                            i=m_nbRebond; // arrete la boucle
                }
                else
                {
                    if(!monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation)) // calcul des points d'intersection entre rayons et faces
                            i=m_nbRebond; // arrete la boucle
                }
                monObjWriter.display_Beam_vert(monRay, m_listener); // ecriture des vertex

            }
            monObjWriter.display_Beam_line(); // ecriture des edges entre les vertex
            progress.setValue(m_nbRebond);
        }
    }
}


void MainWindow::on_bouton_sourcesImages_clicked()
{
    suppFichier(); // Suppression des fichiers d'export existant

    std::vector<float> absAir = absair(m_temperature, m_humidite);

    // OCTREE
    if (m_methodeRapide) m_octree.chargerRayonRacine(m_nbRayon);

    //SOURCES IMAGES
    SourceImage maSourceImage;



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

    for (int nSrc = 0; nSrc < m_source.getNbSource() ; nSrc++)
    {
        // EXPORT
        ObjWriter monObjWriter(m_fichierExport, m_nbRayon);
        // RAYONS
        Ray monRay(m_nbRayon, m_source, nSrc, m_fibonacci);
        if(!m_fibonacci) m_nbRayon = monRay.getNbRay(); // Au cas où on prend la source blender

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
                    m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant());
                    qDebug() << "temps octree : " << m_timer.restart() << "ms";
                    if (!monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation, m_octree)) i=m_nbRebond;
                    qDebug() << "temps rayons : " << m_timer.restart() << "ms";
                }
                else
                {
                    m_timer.restart();
                    if (!monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation)) i=m_nbRebond;
                    qDebug() << "temps rayons : " << m_timer.restart() << "ms";
                }
                maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, absAir,m_seuilAttenuation );
            }
            monObjWriter.display_sourceImages(maSourceImage.getSourcesImages());
            progress.setValue(m_nbRebond);

        }
        else
        {
            progress.setRange(0,m_nbRayon);
            progress.setValue(1);

            if (m_methodeRapide)
            {
                // TANT QUE TOUS LES RAYONS NE SONT PAS MORT
                m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant());
                monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation, m_octree);
                //while(monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation, m_octree))
               // {


                while(maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, absAir, m_seuilAttenuation))
                {
                    // progress bar
                    progress.setValue(monRay.getRayMorts());
                    if (progress.wasCanceled())
                                break; // arrete la boucle
                    //maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, absAir);
                    m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant());
                    monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation, m_octree);// en plus
                }
            }
            else
            {
                // TANT QUE TOUS LES RAYONS NE SONT PAS MORT
                //while(
                      monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation);
                              //)
                {                    
                    //maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, absAir);
                    while(maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, absAir, m_seuilAttenuation))
                    {
                        // progress bar
                        progress.setValue(monRay.getRayMorts());
                        if (progress.wasCanceled())
                                    break; // arrete la boucle
                        monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation);
                    }
                    qDebug() << "calcul source image";
                }
            }
           // maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, absAir); // On le refait une fois à la sortie de boucle pour les dernier rayon
            monObjWriter.display_sourceImages(maSourceImage.getSourcesImages());

            progress.setValue(m_nbRayon);
        }
    }

    double temps = timer2.elapsed();
    temps = temps /1000;
    ui->lcd_timer->display(temps);

    qDebug() << "temps :" << temps;
    progress.cancel();

    m_sourceImage = maSourceImage;
}

void MainWindow::on_bouton_octree_clicked()
{
    if (m_methodeRapide)
    {
        suppFichier(); // Suppression des fichiers d'export existant

        // EXPORT
        ObjWriter monObjWriter(m_fichierExport, 0);

        monObjWriter.display_octree(m_octree.getVectBoite());
    }
    else QMessageBox::warning(NULL,"Attention","Veuillez activer la méthode rapide");


}

void MainWindow::on_bouton_projection_clicked()
{
    if (m_sourceImage.getSourcesImages().empty()) QMessageBox::warning(NULL,"Attention","Veuillez calculer les sources images");
    else
    {
        std::vector<CoordVector> SI = m_sourceImage.getSourcesImages();
        std::vector<CoordVector> SI2;
        std::vector<float> nrg = m_sourceImage.getNrgSI();
        std::vector<float> nrg2;
        int seuil = ui->spinBox_regrouperSI->value();

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
                Ray monRay(m_listener.getCentre(), SI2);
                if (m_methodeRapide)
                {
                    m_octree.chargerRayonRacine(monRay.getNbRay());
                    m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant());
                    monRay.rebondSansMemoire(m_meshObj,-1,m_octree);
                }
                else monRay.rebondSansMemoire(m_meshObj,-1);

                monObjWriter.display_coloredTriangle(monRay.getRay(),nrg2, m_listener.getCentre(), m_source.getCentre());
            }
            else
                monObjWriter.display_coloredTriangle(SI2, nrg2, m_listener.getCentre(), m_source.getCentre());
        }
    }
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
    // ouvre une nouvelle fenetre
    plotWindow *plot = new plotWindow;
    plotWindow *plotcumsum = new plotWindow;
    std::vector<float> absR = absair(m_temperature, m_humidite);
    if (m_sourceImage.calculerRIR(m_freq, absR, m_gain, ui->checkBox_decaycurve->isChecked()))
    {
        plot->XY(m_sourceImage.getX(), m_sourceImage.getY(), m_seuilAttenuation);
        plot->makePlot();
        plot->setYLabel("SPL (dB)");
        //plot->setWindowTitle("Source position : " + CoordVector2QString(m_source.getCentre())+
        //                     "\nListener position : " + CoordVector2QString(m_listener.getCentre()));
        plot->setTitle("Source position : " + CoordVector2QString2(m_source.getCentre())+
                                            "\nListener position : " + CoordVector2QString2(m_listener.getCentre()));
        QPoint position = QApplication::desktop()->screenGeometry().topLeft();// + QApplication::desktop()->screenGeometry().center()/2;
        plot->move(position);
        plot->show();
        //plot.exec();
        /*
        plotcumsum->XY(m_sourceImage.getX(), m_sourceImage.getCurve(), m_seuilAttenuation);
        plotcumsum->makePlot();
        plotcumsum->setYLabel("Curve decay (dB)");
        plotcumsum->setWindowTitle("Source position : " + CoordVector2QString2(m_source.getCentre())+
                                   "\nListener position : " + CoordVector2QString2(m_listener.getCentre()));
        position = QApplication::desktop()->screenGeometry().topRight();
        plotcumsum->move(position);
        plotcumsum->show();*/

    }
    else QMessageBox::warning(NULL,"Attention","La durée de la RIR est de 0s");
}

void MainWindow::on_checkBox_rayAuto_toggled(bool checked) {
   m_rayAuto = checked;
   if(m_rayAuto)
   {
       /*m_longueurRayMax =   sqrt(m_nbRayon/m_seuilArret-1)
                            * 2*m_seuilArret/m_nbRayon
                            * m_listener.getRayon();
       */
       //m_longueurRayMax =  sqrt(m_listener.getRayon()/2*sqrt(m_nbRayon/m_seuilArret));
       m_longueurRayMax =  m_nbRayon*m_listener.getRayon()/(2*m_seuilArret*sqrt(m_nbRayon/m_seuilArret-1));
       ui->lcdTempsmax->display(m_longueurRayMax/VITESSE_SON);
       /*QMessageBox msgBox;
       msgBox.setText("Temps maximum pour mesure statistique : " + QString::number(m_longueurRayMax/VITESSE_SON)+ "s");
       msgBox.setIcon(QMessageBox::Information);
       msgBox.exec();*/
   }
   else m_longueurRayMax = 9999999;
}


void MainWindow::on_checkBox_methodeRapide_toggled(bool checked)
{

    if(checked) {
        // lancer le timer
        QElapsedTimer timer2;
        timer2.start();

        m_octree = Octree(m_meshObj,m_nbFaceFeuille);

        double temps = timer2.elapsed();
        //temps = temps /1000;
        ui->lcd_timer->display(temps);
        qDebug() << "temps création octree : " << temps << "ms";
    }
    m_methodeRapide = checked;


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
    else (m_fichierAudio = "/home");

}

void MainWindow::on_bouton_ecouter_clicked()
{
    /*
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
*/
    if (audio->state() != QAudio::IdleState) {
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

void MainWindow::on_spinBox_humidite_editingFinished() {
    m_humidite = ui->spinBox_humidite->value();
}

void MainWindow::on_spinBox_temperature_editingFinished() {
    m_temperature = ui->spinBox_temperature->value();
}

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

void MainWindow::on_spinBox_freqEchan_editingFinished() {
     m_freq = ui->spinBox_freqEchan->value();
}

void MainWindow::on_spinBox_seuilArret_editingFinished()
{
    m_seuilArret = ui->spinBox_seuilArret->value();
    if(m_rayAuto) on_checkBox_rayAuto_toggled(true);
}


void MainWindow::on_bouton_convolution_clicked()
{

    WavFile wav;
    int nfft(1024);
    long nlog;
    int k, j, i;

    m_fichierAudio = ui->textEdit_AudioFile->toPlainText();

    if(wav.open(m_fichierAudio))
    {
        std::vector<float> absR = absair(m_temperature, m_humidite);
        if (m_sourceImage.calculerRIR(wav.fileFormat().sampleRate(), absR, m_gain, ui->checkBox_decaycurve->isChecked()))
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
                fir.push_back(convolution_temporelle(m_sourceImage.getFIR()[k],filtres[k]));
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

/*
            // TEST avec un dirac
            std::vector<float> dirac;
            dirac.resize(2500, 0);
            dirac[0] = 1;
            dirac[583] = 1;
            fir.resize(filtres.size(),dirac);
            partitionnage(fir, firPart, nfft);

            std::vector<float> x2;
            for (k= 0 ; k < 1024 ; k++) { x2.push_back(k);};

            plotWindow *firPlot = new plotWindow;
            firPlot->setWindowTitle("FIRs");
            firPlot->XY(x2, firPart, 1e-6);
            //firPlot->XY(x2, firPart[0]);
            firPlot->makePlot();
            //firPlot->hideLegend();
            firPlot->show();

            plotWindow *filtrePlot = new plotWindow;
            filtrePlot->setWindowTitle("Filtres");
            //zeroPadding(filtres, nfft);
            filtrePlot->XY(x2, filtres);
            filtrePlot->makePlot();
            filtrePlot->show();
 */

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

     // HRIR
            /*
            std::vector<float> v1;
            v1.resize(128);
            std::vector<std::vector<float> > v2;
            v2.resize(12303,v1);
            std::vector<std::vector<std::vector<float> > > array3D;
            array3D.resize(2, v2);

            QString fichier = QCoreApplication::applicationDirPath() + "/MyHead1_.xhead";
            std::string file = fichier.toStdString();
            openXFile(file,2, 12308, 128, array3D);
            //XFir xfir(1,2,nfft,wav.fileFormat().sampleRate());
            XHrir xhrir(1, wav.fileFormat().sampleRate(), file);
            // chercher les HRIR
            int indice = xhrir.sph2ind(0,0); // valeur à modifier selon orientation du listener
            std::vector<float> azel(2);
            xhrir.ind2sph(indice,azel.data());
            xhrir.getFilter(1,2);
            */




            //for (auto &a : wavPart) {rffts(a.data(), nlog, 1);} // on fait d'abord la fft

            // initialisation lecture temps reel
            std::vector<qint16> newData2;
            newData2.resize(nfft/2);

            qint64 pos;
            audio->setBufferSize(nfft/2);
            audio->start(buffer);

            //QElapsedTimer timer;


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
            m_fichierAudio = QCoreApplication::applicationDirPath() + "/resultat.wav";
            ui->bouton_ecouter->setText("Resultat");

            wav.close();
            progress.setValue(1.1*prog);


        }
        else QMessageBox::warning(NULL,"Attention","La durée de la RIR est de 0s");
    }    
    else QMessageBox::warning(NULL,"Attention","Pas de fichier audio lisible");

}


void MainWindow::tests()
{

    /// RIR de cube analytique

    // Dimension du cube
    CoordVector L(2,4,3); //longueur, largeur, hauteur

    std::vector<float> absR = absair(m_temperature, m_humidite);//absorptionAir(20);
    //std::vector<float> absR = absorptionAir(20);
    float f[] = {0,0,0,0,0,0};//x, -x, y, -y, z, -z
    //float f[] = {60,50,40,30,20,10};//x, -x, y, -y, z, -z en % d'attenaution
    std::vector<float> att(f, f+sizeof(f)/sizeof(float));

    // Position src et listener
    CoordVector src = m_source.getCentre();
    CoordVector mic = m_listener.getCentre();
    //float r = m_listener.getRayon();

    debugStdVect(att);
    qDebug() << "source :";
    src.debug();

    // Positions relatives et énergie des sources images
    std::vector<CoordVector> src_im;
    std::vector<float> src_im_tps, nrgbis;
    //std::vector< std::vector<float> > nrg;
    //nrg.resize(8);
    float d, attenuation, i0, i1, i2, i3, i4, i5;
    float i,j, k, l;
    float ordre = m_nbRebond;
    float ordrebuf;
    //float Rmax = (ordre-1)*coordMin(L);
    CoordVector si;

    /*
    for(i=0; i< att.size(); i++)
    {
        att[i]=1-(att[i]/100);
    }
    */
    //correspondance
    att[0]=1-(f[1]/100);
    att[1]=1-(f[0]/100);
    att[2]=1-(f[5]/100);
    att[3]=1-(f[4]/100);
    att[4]=1-(f[2]/100);
    att[5]=1-(f[3]/100);
    debugStdVect(att);

/*
    src.x += L.x/2;
    src.y += L.y/2;
    src.z += L.z/2;

    CoordVector mic2;
    mic2.x = mic.x + L.x/2;
    mic2.y = mic.y + L.y/2;
    mic2.z = mic.z + L.z/2;
*/
    bool sitrouvee = false;
    for (i = -ordre ; i <=ordre ; i++)
    {      
        //si.x = src.x*pow(-1,i) + L.x*(i + 0.5 - 0.5*pow(-1,i)) - mic2.x;
        si.x = i*L.x + src.x*pow(-1,i);
        i0 = std::abs(0.5*i - 0.25 + 0.25*pow(-1, i));
        i1 = std::abs(0.5*i + 0.25 - 0.25*pow(-1, i));
        for (j = -(ordre-abs(i)) ; j<= ordre-abs(i); j++)
        {
            if (ordre-abs(i)-abs(j)< 0) ordrebuf = 0;
            else ordrebuf = ordre-abs(i)-abs(j);
            //si.y = src.y*pow(-1,j) + L.y*(j + 0.5 - 0.5*pow(-1,j)) - mic2.y;
            si.y = j*L.y + src.y*pow(-1,j);
            i2 = std::abs(0.5*j - 0.25 + 0.25*pow(-1, j));
            i3 = std::abs(0.5*j + 0.25 - 0.25*pow(-1, j));
            for (k = -ordrebuf ; k<= ordrebuf; k++)
            {
                //si.z = src.z*pow(-1,k) + L.z*(k + 0.5 - 0.5*pow(-1,k)) - mic2.z;
                si.z = k*L.z + src.z*pow(-1,k);
                i4 = std::abs(0.5*k - 0.25 + 0.25*pow(-1, k));
                i5 = std::abs(0.5*k + 0.25 - 0.25*pow(-1, k));


                for (l=0 ; l< src_im.size() ; l++)
                {
                    if (proche(si,src_im[l])) {sitrouvee = true; qDebug() <<"source trouvée !";}
                }
                if(!sitrouvee)
                    {
                        //attenuation
                       /* if(pow(-1, i) < 0) attenuation = pow(att[0], i);
                        else attenuation = pow(att[1], i);
                        if(pow(-1, j) < 0) attenuation *= pow(att[2], j);
                        else attenuation *= pow(att[3], j);
                        if(pow(-1, k) < 0) attenuation *= pow(att[4], k);
                        else attenuation *= pow(att[5], k);
*/

                        attenuation =    pow(att[0], i0)
                                        *pow(att[1], i1)
                                        *pow(att[2], i2)
                                        *pow(att[3], i3)
                                        *pow(att[4], i4)
                                        *pow(att[5], i5);

                        if(attenuation > 0 && attenuation <=1)
                        {
                            //qDebug() << attenuation;
                            src_im.push_back(si); // ajout position
                            src_im_tps.push_back(1000*d/VITESSE_SON); // distance temporelle ms

                            d = norme(vecteur(si,mic)); // distance

                            if (d > 1e-6) // pour eviter les divisions par 0 (cas source et mic confondus)
                            {
                                for (l = 0; l< 8 ; l++){
                                    //nrg[l].push_back(1/pow(d,2));
                                    //nrgbis.push_back(1/pow(d,2));
                                    nrgbis.push_back(attenuation/pow(d,2)*exp(-absR[l]*d));
                                    //nrgbis.push_back(attenuation/pow(d,2)*pow(10,-absR[l]*d/10));
                                }
                                //if(nrgMax<1/pow(d,2)) nrgMax = 1/pow(d,2);
                            }
                            else {
                                QMessageBox::warning(NULL,"Attention","Source et micro confondus");
                            }
                        }


                    }
                else sitrouvee=false;
            }
        }
    }




    std::vector<float> x;
    std::vector<std::vector<float> > y;
    y.resize(8);
    std::vector<float> nrgExp = m_sourceImage.getNrgSI();
    std::vector<CoordVector> src_im_exp = m_sourceImage.getSourcesImages();

    if(src_im_exp.size() < 1)
    {
        QMessageBox::warning(NULL,"Attention","Pas de source image expérimentale");
        return;
    }

    float max0 = *std::max_element(nrgExp.begin(), nrgExp.end());
    float max1 = *std::max_element(nrgbis.begin(), nrgbis.end());

    if(src_im.size()<1 || max1 < 0.000001)
    {
        QMessageBox::warning(NULL,"Attention","Pas de sources images théoriques");
        return;
    }

    for(auto&a : nrgbis) a/=(max1/max0);
    max1 = *std::max_element(nrgbis.begin(), nrgbis.end());

    for (j=0; j< src_im_exp.size(); j++)
    {
        //y[1].push_back(nrg[0][i]);
        for (i=0; i< src_im.size(); i++)
        {
            if (proche(src_im_exp[j],src_im[i]) && nrgExp[8*j] > 100)
            {
                //y[0].push_back(norme(vecteur(src_im_exp[j],src_im[i])));

                for (l=0 ; l < 8 ; l++)
                {
                    y[l].push_back((nrgbis[8*i+l]-nrgExp[8*j+l])/nrgbis[8*i+l]);
                    //y[l].push_back((nrgbis[8*i+l]-nrgExp[8*j+l])/max1);
                }

                //y[2].push_back((nrgbis[8*i]-nrgExp[8*j])/max1);
                //y[0].push_back(nrgExp[8*j]);
                //y[1].push_back(nrg[0][i]);
                //nrgExp.erase(nrgExp.begin()+8*j);
                //src_im[i].debug();
                //qDebug() << "exp : " << nrgExp[8*j] << " -> theo : " << nrg[0][i];
                //j = src_im_exp.size();
                //src_im_exp.erase(src_im_exp.begin()+j);
                //break;
                if(proche(src_im_exp[j],src))
                {qDebug() << "source" << "i = " << i << " ; j = " << j;}
            }
        }
    }


    for (i=0; i< y[0].size() ; i++)
    {
        x.push_back(i);
    }
    if(i<1)
    {
        QMessageBox::warning(NULL,"Attention","Pas de sources images théoriques");
        return;
    }


    plotWindow *plot = new plotWindow;
    //plot->XY(x,y, m_seuilAttenuation);
    plot->XY(x,y);
    //plot->XY(x,erreurRelative);
    plot->makePlot();
    plot->show();


    //affichage
    suppFichier();
    ObjWriter monObjWriter(m_fichierExport, 1);
    monObjWriter.display_sourceImages(src_im);
    //monObjWriter.display_coloredTriangle(src_im, nrgbis, mic, src);


}

void MainWindow::test2() //fonction 1/d^2
{
    std::vector<std::vector<float> > y_rir = m_sourceImage.getY();

   float freq = (float)ui->spinBox_freqEchan->value()/1000; // car on a des temps en ms (convertion en float)
   int nb_ech = y_rir[0].size();

   if (nb_ech > 0)
   {
       std::vector<float> x;
       std::vector<std::vector<float> > y;
       x.resize(nb_ech, 0);
       y.resize(2);

       y[0].push_back(1);
       y[1].push_back(1);

      // Abscisses
       for (float i = 1 ; i <nb_ech ; i++)
      {
          x[i] = i/freq; // valeurs en ms
          y[0].push_back(1/pow(x[i],2));
          y[1].push_back(y_rir[0][i]/2);
      }

       plotWindow *plot = new plotWindow;
       plot->XY(x,y,m_seuilAttenuation);
       plot->makePlot();
       plot->show();
   }

}

void MainWindow::test3()
{



}

void MainWindow::on_bouton_test_clicked()
{
    //tests();


    // play streaming
/*    WavFile wav;

    if(wav.open(m_fichierAudio))
    {
        arr->clear();
        arr->append(wav.readAll());
        qint64 pos;
        pos = buffer->pos();
        qDebug() << pos;
        //pos = ceil(audio->elapsedUSecs()*44100*1e-6);
        buffer->seek(round(buffer->size()));
        //buffer->seek(ceil(audio->elapsedUSecs()*44100*1e-6)+audio->bufferSize()+5000);
        qDebug() << ceil(audio->elapsedUSecs()*44100*1e-6);
        buffer->write(arr->data(), arr->length());
        buffer->seek(pos);
    }
*/

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

    std::vector<std::vector<float> > rir = m_sourceImage.getY();
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
     std::vector<std::vector<float> > diff_rir = m_sourceImage.getY();
     if(!diff_rir.empty())
     {
         QString chemin = QCoreApplication::applicationDirPath() + "/rirSauvegardee.txt";
         QFile fichier(chemin);

         float i(0), j;
         float min;
         QStringList valString;
         QString ligne;
         std::vector<float> val;
         std::vector<std::vector<float> > resultat;
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
         std::vector<float> SIx = m_sourceImage.getX();
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
    std::vector<std::vector<float> > rir = m_sourceImage.getY();
    std::vector<std::vector<float> > curve = m_sourceImage.getCurve();
    std::vector<CoordVector> si = m_sourceImage.getSourcesImages();
    std::vector<float> nrgSi = m_sourceImage.getNrgSI();
    CoordVector vect1 = vecteur(m_listener.getCentre(),m_source.getCentre());
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

    if(!rir.empty())
    {
        //std::vector<float> rirX = m_sourceImage.getX();
        float i, j;
        std::vector<float> Tr60, Tr30, Tr30_buf, C80, D50, Ts, EDT, SPL, LF, G;
        float C80_0, C80_1, D50_0, Ts_buf, LF_buf, LF_tot, Tr60_buf, EDT_buf;
        // determination du son direct
        int sonDirect; // numéro de l'échantilllon
        for (j=0; j<rir[0].size(); j++)
        {
            if (rir[0][j]>0)
            {
                sonDirect = j;
                break;
            }
        }
        qDebug() << "son direct" << sonDirect;
        //QString text;
        for (i=0; i<8; i++) // pour chaque bande
        {
            for (j=sonDirect; j<rir[0].size(); j++) // Pour chaque échantillon à partir du son direct
            {
                if(curve[i][j] >= maxY[i]*1e-6) Tr60_buf = (j-sonDirect)/m_freq*1000;//rirX[j]; // on conserve le dernier terme

                if (curve[i][j] <= maxY[i]*pow(10,-0.5) && curve[i][j] >= maxY[i]*pow(10,-3.5)) Tr30_buf.push_back((j-sonDirect)/m_freq*1000);//Tr30_buf.push_back(rirX[j]);

                if(curve[i][j] >= maxY[i]*0.1) EDT_buf = (j-sonDirect)/m_freq*1000;//rirX[j];

                //if (j >= sonDirect)
                {
                    if(j <= 80*m_freq/1000 + sonDirect) C80_0 += rir[i][j];
                    else C80_1 += rir[i][j];

                    if(j <= 50*m_freq/1000 + sonDirect) D50_0 += rir[i][j];
                    //D50_1 += rir[i][j];

                    //Ts_buf += rirX[j]*rir[i][j];
                    if (j>=sonDirect)
                    Ts_buf += ((j-sonDirect)/m_freq*1000)*rir[i][j];
                }
            }

            //qDebug() << 62.5*pow(2,i);

            //RT 60
            Tr60.push_back(Tr60_buf);

            // temps pour passer de -5dB à -35dB
            Tr30.push_back(2*(*std::max_element(Tr30_buf.begin(), Tr30_buf.end())-*std::min_element(Tr30_buf.begin(), Tr30_buf.end())));
            Tr30_buf.clear();

            // temps où l'énergie descend sous 10dB
            EDT.push_back(EDT_buf*6);


            // ratio avant et après 80ms
            C80.push_back(10*log10(C80_0/C80_1));
            //qDebug() << "C80_0 = " << C80_0 << "C80_1 = " << C80_1;
            C80_0=0;
            C80_1=0;

            // ratio avant 50ms sur total
            D50.push_back(100*D50_0/maxY[i]);
            //qDebug() << "D50_0 = " << D50_0 << "maxY[i] = " << maxY[i];
            D50_0=0;

            // ratio temps central
            Ts.push_back(Ts_buf/maxY[i]);
            Ts_buf = 0;

            // spl
            SPL.push_back(10*log10(maxY[i])-m_gain);

            // Gain
            G.push_back(10*log10(maxY[i]/maxY2[i]));
            //D50_1=0;

            // LF somme des energie multiplié par l'angle
            for (j=0; j<si.size(); j++)
            {
                vect2 = vecteur(m_listener.getCentre(), si[j]);
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

        // Affiher valeur
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

        // Exporter tableau
        QString newName(QCoreApplication::applicationDirPath() + "/data4latex.txt");
        QFile fichier(QCoreApplication::applicationDirPath() + "/data4latex.txt");
         i=0;
        while(fichier.exists()) // incrementation de version de fichier s'il existe deja
        {
            //qDebug() << fichier.fileName() << "existe !";
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
            //qDebug() << "newname" << newName ;
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

        CoordVector s(m_source.getCentre()), r(m_listener.getCentre());
        arrondir(s);
        arrondir(r);


       text = "\\end{tabular} \n \\caption{Facteurs perceptifs pour une source en [";
       text+=QString::number(s.x) + " ; " + QString::number(-s.z) + " ; " + QString::number(s.y);
       text+="] et un auditeur en [";
       text+=QString::number(r.x) + " ; " + QString::number(-r.z) + " ; " + QString::number(r.y);
       text+="] et ";
       text+= QString::number(m_nbRayon);
       text+= " de rayons.} \n \\label{tab_fact} \n \\end{tableth}";

         fichier.write(text.toLatin1());

    }
    else QMessageBox::warning(NULL,"Attention","La durée de la RIR est de 0s");
}

void MainWindow::on_spinBox_gain_editingFinished()
{
    m_gain = ui->spinBox_gain->value();
}
