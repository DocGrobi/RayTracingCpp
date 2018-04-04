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
   //on_checkBox__rebFixe_toggled(false);
   on_checkBox__rebFixe_toggled(true);

   on_radioButton_Fibonacci_toggled(true);
   on_checkBox_rayAuto_toggled(false);
   m_nbRebond = ui->spinBox_nbRebond->value();
   m_seuilAttenuation = pow(10,(-(ui->spinBox_attenuation->value()/10)));
   m_temperature = ui->spinBox_temperature->value();
   m_humidite = ui->spinBox_humidite->value();
   m_freq = ui->spinBox_freqEchan->value();
   m_seuilArret = ui->spinBox_seuilArret->value();
   m_nbRayon = ui->spinBox_nbRay->value();
   m_nbFaceFeuille = ui->spinBox_nbFaceFeuille->value();
   m_fichierExport = QCoreApplication::applicationDirPath() + "/meshForRayTracingEXPORT.obj";
   m_longueurRayMax = 9999999;

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
void MainWindow::on_bouton_normales_clicked()
{
    suppFichier(); // Suppression des fichiers d'export existant
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
                maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, absAir);
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
                while(monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation, m_octree))
                {
                    // progress bar
                    progress.setValue(monRay.getRayMorts());
                    if (progress.wasCanceled())
                                break; // arrete la boucle

                    maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, absAir);
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
                    if (progress.wasCanceled())
                                break; // arrete la boucle

                    maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, absAir);
                }
            }
            maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, absAir); // On le refait une fois à la sortie de boucle pour les dernier rayon
            monObjWriter.display_sourceImages(maSourceImage.getSourcesImages());

            progress.setValue(m_nbRayon);
        }
    }

    double temps = timer2.elapsed();
    temps = temps /1000;
    ui->lcd_timer->display(temps);

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

        // création des impacts
        Ray monRay(m_listener.getCentre(), SI2);
        if (m_methodeRapide)
        {
            m_octree.chargerRayonRacine(monRay.getNbRay());
            m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant());
            monRay.rebondSansMemoire(m_meshObj,-1,m_octree);
        }
        else monRay.rebondSansMemoire(m_meshObj,-1);

        //EXPORT
        suppFichier(); // Suppression des fichiers d'export existant
        ObjWriter monObjWriter(m_fichierExport, monRay.getNbRay());
        if (ui->checkBox_projeter->isChecked())
            monObjWriter.display_coloredTriangle(monRay.getRay(),nrg2, m_listener.getCentre());
        else
            monObjWriter.display_coloredTriangle(SI2, nrg2, m_listener.getCentre());
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
    if (m_sourceImage.calculerRIR(m_freq))
    {
        plot->XY(m_sourceImage.getX(), m_sourceImage.getY(), m_seuilAttenuation);
        plot->makePlot();
        plot->show();
        //plot.exec();
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
       m_longueurRayMax =  sqrt(m_listener.getRayon()/2*sqrt(m_nbRayon/m_seuilArret));
       QMessageBox msgBox;
       msgBox.setText("Temps maximum pour mesure statistique : " + QString::number(m_longueurRayMax/VITESSE_SON)+ "s");
       msgBox.setIcon(QMessageBox::Information);
       msgBox.exec();
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
        if (m_sourceImage.calculerRIR(wav.fileFormat().sampleRate()))
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
    CoordVector L(10,10,10); //longueur, largeur, hauteur

    // Position src et listener
    CoordVector src = m_source.getCentre();
    CoordVector mic = m_listener.getCentre();

    qDebug() << "source :";
    src.debug();

    // Positions relatives et énergie des sources images
    std::vector<CoordVector> src_im;
    std::vector<float> src_im_tps;
    std::vector< std::vector<float> > nrg;
    nrg.resize(8);
    float d;
    float i,j, k, l;
    float ordre = m_nbRebond;
    float ordrebuf;
    //float Rmax = (ordre-1)*coordMin(L);
    CoordVector si;
    float nrgMax;
    for (i = -ordre ; i <=ordre ; i++)
    {      
        //si.x = src.x*pow(-1,i) + L.x*(i + 0.5 - 0.5*pow(-1,i));
        si.x = i*L.x + src.x*pow(-1,i);
        for (j = -(ordre-abs(i)) ; j<= ordre-abs(i); j++)
        {
            if (ordre-abs(i)-abs(j)< 0) ordrebuf = 0;
            else ordrebuf = ordre-abs(i)-abs(j);
            //si.y = src.y*pow(-1,j) + L.y*(j + 0.5 - 0.5*pow(-1,j));
            si.y = j*L.y + src.y*pow(-1,j);
            for (k = -ordrebuf ; k<= ordrebuf; k++)
            {
                //si.z = src.z*pow(-1,k) + L.z*(k + 0.5 - 0.5*pow(-1,k));
                si.z = k*L.z + src.z*pow(-1,k);

                d = norme(vecteur(si,mic)); // distance
                //if (d<Rmax)
                {
                    src_im.push_back(si); // ajout position
                    src_im_tps.push_back(1000*d/VITESSE_SON); // distance temporelle ms

                    if (d > 1e-6) // pour eviter les divisions par 0 (cas source et mic confondus)
                    {
                        for (l = 0; l< 8 ; l++){
                            nrg[l].push_back(1/pow(d,2));        
                        }
                        if(nrgMax<1/pow(d,2)) nrgMax = 1/pow(d,2);
                    }
                    else {
                        QMessageBox::warning(NULL,"Attention","Source et micro confondus");
                    }
                }
            }
        }
    }


    float tpsMin, tpsMax;

    tpsMax = *std::max_element(src_im_tps.begin(),src_im_tps.end());

    std::vector<float> ind0, si_tps_save;
    si_tps_save = src_im_tps;
    while (tpsMin<tpsMax)
    {
        tpsMin = *std::min_element(src_im_tps.begin(),src_im_tps.end());
        for(i=0; i<src_im_tps.size(); i++)
        {
            if(src_im_tps[i]==tpsMin) {
                ind0.push_back(i);
                src_im_tps[i]=tpsMax;
                break;
            }
        }
    }


    std::vector<float> src_im_exp_tps = m_sourceImage.getSI_Tps();
    float tpsMin1, tpsMax1;

    tpsMax1 = *std::max_element(src_im_tps.begin(),src_im_tps.end());

    std::vector<float> ind1;
    while (tpsMin1<tpsMax1)
    {
        tpsMin1 = *std::min_element(src_im_exp_tps.begin(),src_im_exp_tps.end());
        for(i=0; i<src_im_exp_tps.size(); i++)
        {
            if(src_im_exp_tps[i]==tpsMin1) {
                ind1.push_back(i);
                src_im_exp_tps[i]=tpsMax1;
                break;
            }
        }
    }

    src_im_exp_tps = m_sourceImage.getSI_Tps();
    std::vector<float> x;
    std::vector<std::vector<float> > y;
    y.resize(2);
    std::vector<float> nrgExp = m_sourceImage.getNrgSI();
    std::vector<CoordVector> src_im_exp = m_sourceImage.getSourcesImages();

    // normalisation
    for (l = 0; l< 8 ; l++)
    {
        for (auto&a : nrg[l]) a/=nrgMax;
    }
    float nrgExpMax = *std::max_element(nrgExp.begin(), nrgExp.end());
    for (auto&a : nrgExp) a/=nrgExpMax;

    std::vector<float> erreurRelative;
    std::vector<float> erreurRelative2;

    for(i=0; i<src_im_exp_tps.size(); i++)
    {
        x.push_back(i);
        //y[0].push_back(nrg[0][ind0[i]]);
        //y[1].push_back(nrgExp[8*ind1[i]]);
        erreurRelative.push_back(std::abs(nrg[0][ind0[i]]-nrgExp[8*ind1[i]])/nrg[0][ind0[i]]);
        erreurRelative2.push_back(std::abs(si_tps_save[ind0[i]]-src_im_exp_tps[ind1[i]])/si_tps_save[ind0[i]]);
        qDebug() << src_im_exp_tps[ind1[i]];
        src_im[ind0[i]].debug();
        arrondir(src_im_exp[ind1[i]]);
        src_im_exp[ind1[i]].debug();


    }

    y[0]=erreurRelative;
    y[1]=erreurRelative2;

    float moyenne = std::accumulate(erreurRelative.begin(), erreurRelative.end(), 0.0);
    moyenne/=erreurRelative.size();
    qDebug() << "Moyenne : " << moyenne;

    float moyenne2 = std::accumulate(erreurRelative2.begin(), erreurRelative2.end(), 0.0);
    moyenne2/=erreurRelative2.size();
    qDebug() << "Moyenne2 : " << moyenne2;

/*

    for(auto& a: src_im_exp) arrondir(a);
    std::vector<int> indices;
    std::vector<CoordVector> SItriees = ranger(src_im_exp, indices);



    qDebug() << "nrg theo size :" << nrg[0].size();
    qDebug() << "nrg Exp size :" << nrgExp.size()/8;
    qDebug() << "indices size :" << indices.size();




    qDebug() << "SI THEO :";
    for (i=src_im.size()/2; i<src_im.size(); i++)
    {
        //src_im[i].debug();
        //if(nrg[0][i]>0.0001)
        x.push_back(i);
        y[0].push_back(nrg[0][i]);
        qDebug() << nrg[0][i];
    }

    qDebug() << "SI EXP :";
    for (i=SItriees.size()/2; i<SItriees.size(); i++)
    {

        //SItriees[i].debug();
    }

    // trie des enérgies
    for (i=0; i<indices.size(); i++)
    {
        if(8*indices[i]>nrgExp.size()/2 //&& nrgExp[8*indices[i]]>0.0001
                )
                y[1].push_back(nrgExp[8*indices[i]]);
    }

*/


    // A FAIRE : dissipation par les parois et par l'air


    // Création de la RIR temporelle
/*
     std::vector<std::vector<float> > y_rir = m_sourceImage.getY();

    //float tps_max = *std::max_element(src_im_tps.begin(), src_im_tps.end());

    float freq = (float)ui->spinBox_freqEchan->value()/1000; // car on a des temps en ms (convertion en float)
    //int nb_ech = ceil(tps_max*freq);
    int nb_ech = y_rir[0].size();

    qDebug() << freq;
    qDebug() << nb_ech;
    qDebug() << nrg[0][0];
    qDebug() << src_im_tps[0];



    float max(0), maxbuf(0);
    if (nb_ech > 0)
    {
        std::vector<float> x;
        std::vector<std::vector<float> > y;
        std::vector<std::vector<float> > yBis;
        x.resize(nb_ech, 0);
        y.resize(8);
        yBis.resize(2);
        yBis[0].resize(nb_ech, 0);
        yBis[1].resize(nb_ech, 0);


       // Abscisses
        for (i = 0 ; i <nb_ech ; i++)
       {
           x[i] = i/freq; // valeurs en ms     
       }

        for (j = 0 ; j < 8 ; j++) // pour chaque bande
        {
            y[j].resize(nb_ech);
            for (i=0 ; i< src_im_tps.size(); i++) // pour chaque source image
            {
                if (floor(src_im_tps[i]*freq) < nb_ech)
                    y[j][floor(src_im_tps[i]*freq)] += nrg[j][i];
            }
            maxbuf = *std::max_element(y[j].begin(), y[j].end());
            if(max<maxbuf) max = maxbuf; // recuperation du max
        }
        qDebug() << "max : " << max;

        for (j = 0 ; j < 8 ; j++) // pour chaque bande
        {
            for (auto &a : y[j]) {a/=max;} // normalisation
        }

        // Affichage de la différence



        if (y_rir.size()>0)
        {
            qDebug() << "y : " << y[0].size();
            qDebug() << "y rir : " << y_rir[0].size();

            //std::transform(y[0].begin(), y[0].begin()+y_rir[0].size(), y_rir[0].begin(), y[0].begin(), std::minus<float>());

            for (i = 0 ; i <nb_ech ; i++)
           {
              if (y[0][i] == 0) y[0][i] = m_seuilAttenuation;//abs(y_rir[0][i]);
              else y[0][i] = abs((y_rir[0][i]-y[0][i])/y[0][i]);
           }

        }
        else QMessageBox::warning(NULL,"Attention","Veuillez d'abord exporter les SI ET calculer la RIR pour afficher la différence");

        yBis[0] = y_rir[0];
        yBis[1] =y[1];


    }

*/
    plotWindow *plot = new plotWindow;
    //plot->XY(x,y, m_seuilAttenuation);
    plot->XY(x,y[1]);
    //plot->XY(x,erreurRelative);
    plot->makePlot();
    plot->show();

    //affichage
    suppFichier();
    ObjWriter monObjWriter(m_fichierExport, 1);
    monObjWriter.display_sourceImages(src_im);


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

void MainWindow::test3() //fonction rangement
{
    CoordVector a(1,3,5), b(-1, 8, 1), c(-1, 8, 0), d(-1, 1, 0), e(1,3,1);
    std::vector<CoordVector> test;
    test.push_back(a);
    test.push_back(b);
    test.push_back(c);
    test.push_back(d);
    test.push_back(e);
    std::vector<int> indices;
    std::vector<CoordVector> resultat = ranger(test, indices);
    for(auto& a: resultat) a.debug();
    debugStdVect(indices);

}

void MainWindow::on_bouton_test_clicked()
{
    tests();

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


