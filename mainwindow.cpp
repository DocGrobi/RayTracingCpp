#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include "math.h"
#include "rir.h"
#include <QProgressDialog>
#include "audio.h"
#include "physic.h"
#include "fftext.h"



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
   ui->checkBox__rebFixe->setChecked(false);
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

   // On limite le nombre de faces par feuille au nombre de face total
   ui->spinBox_nbFaceFeuille->setMaximum(m_meshObj.getVert().size()/3);

   ui->checkBox_methodeRapide->setChecked(true);

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
   else (m_fichierAudio = "/home");

/*
   audioProbe = new QAudioProbe(this);
   qDebug() << audioProbe->setSource(player);

   // Probing succeeded, audioProbe->isValid() should be true.
   connect(audioProbe, SIGNAL(audioBufferProbed(QAudioBuffer)),
           this, SLOT(slotGetMusicData(QAudioBuffer)));
*/



}

MainWindow::~MainWindow()
{
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

    std::vector<float> absAir = absair(m_temperature, m_humidite);

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
                    if(!monRay.rebondSansMemoire(m_meshObj, -1, m_octree, absAir)) // calcul des points d'intersection entre rayons et faces
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
                while(monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation, m_octree, absAir))
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
                    if (!monRay.rebondSansMemoire(m_meshObj, -1, m_octree, absAir)) i=m_nbRebond;
                    qDebug() << "temps rayons : " << m_timer.restart() << "ms";
                }
                else
                {
                    m_timer.restart();
                    if (!monRay.rebondSansMemoire(m_meshObj, -1)) i=m_nbRebond;
                    qDebug() << "temps rayons : " << m_timer.restart() << "ms";
                }
                maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, m_rayAuto, absAir);
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
                m_octree.chargerRayon(monRay.getRay(), monRay.getvDir(), monRay.getRayVivant());
                while(monRay.rebondSansMemoire(m_meshObj, m_seuilAttenuation, m_octree, absAir))
                {
                    // progress bar
                    progress.setValue(monRay.getRayMorts());
                    if (progress.wasCanceled())
                                break; // arrete la boucle

                    maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, m_rayAuto, absAir);
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

                    maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, m_rayAuto, absAir);
                }
            }
            maSourceImage.addSourcesImages(monRay , m_listener, m_longueurRayMax, m_rayAuto, absAir); // On le refait une fois à la sortie de boucle pour les dernier rayon
            monObjWriter.display_sourceImages(maSourceImage, m_seuilAttenuation);

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
        plot->XY(m_sourceImage.getX(), m_sourceImage.getFIR(), m_seuilAttenuation);
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
       m_longueurRayMax = sqrt(m_nbRayon/m_seuilArret)*m_listener.getRayon();
       QMessageBox msgBox;
       msgBox.setText("Temps maximum pour mesure statistique : " + QString::number(m_longueurRayMax/VITESSE_SON)+ "s");
       msgBox.setIcon(QMessageBox::Information);
       msgBox.exec();
   }
}


void MainWindow::on_checkBox_methodeRapide_toggled(bool checked)
{
    if(checked) m_octree = Octree(m_meshObj,m_nbFaceFeuille);
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

    if(wav.open(m_fichierAudio))
    {
        if (m_sourceImage.calculerRIR(wav.fileFormat().sampleRate()))
        {
            int wavLength = wav.bytesAvailable();
            qDebug() << "taille du wav :" << wavLength;
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


/*
            QVector<float> vectWav1;
            std::vector<float> vectWav;
            std::vector<float> x;
            int samplerate = wav.fileFormat().sampleRate()/1000;

            QByteArray donnees = wav.readAll();
            QDataStream in (&donnees,QIODevice::ReadOnly);
            //QDataStream in(wav.readAll());
            while (!in.atEnd())
            {
                float f;
                in >> f;
                vectWav1 << f;
                x.push_back((float)i/samplerate);
            }
            for(float &a : vectWav1) {vectWav.push_back(a);}
            */

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
            qDebug() << "nlog : " << nlog;

            // Partitionnement de la FIR
            //m_sourceImage.partitionnage(nfft);
            //std::vector< std::vector<float> > firPart = m_sourceImage.getFirPart();
            std::vector< std::vector<float> > firPart;
            //partitionnage(m_sourceImage.getFIR(), firPart, nfft);


            std::vector< std::vector<float> > filtres;
            bandFilters(filtres);

            //*
            // TEST avec un dirac
            std::vector<float> dirac;
            //dirac.resize(nfft, 0);
            //dirac[nfft/2+1] = 1;
            //dirac[nfft/2+1] = 1;
            dirac.resize(441000, 0);
            dirac[0] = 1;
            dirac[132300] = 1;
            dirac[264600] = 1;
            std::vector< std::vector<float> > fir;
            fir.resize(filtres.size(),dirac);
            partitionnage(fir, firPart, nfft);
            ///
            //*/

            std::vector<float> x2;
            for (k= 0 ; k < nfft ; k++) { x2.push_back(k);};




            plotWindow *firPlot = new plotWindow;
            firPlot->setWindowTitle("FIRs");
            firPlot->XY(x2, firPart, 1e-6);
            //firPlot->XY(x2, firPart[0]);
            firPlot->makePlot();
            //firPlot->hideLegend();
            firPlot->show();

/*

            plotWindow *filtrePlot = new plotWindow;
            filtrePlot->setWindowTitle("Filtres");
            //zeroPadding(filtres, nfft);
            filtrePlot->XY(x2, filtres);
            filtrePlot->makePlot();
            filtrePlot->show();
 */

            int nFiltre = m_sourceImage.getFIR().size(); // nombre de bande fréquentielle
            int nPart = firPart.size()/nFiltre; // nombre de partition par bande

            qDebug() << "n fir : " << firPart.size();
            qDebug() << "n part : " << nPart;
            qDebug() << "code erreur fft init : " << fftInit(nlog);

            // fft des FIR partitionnées
            for (auto &a : firPart) { rffts(a.data(), nlog, 1); // on passe fir en frequentielle (directement enregistré dans lui-même)
            }
            qDebug() << "Firs spectrales !";

            if (filtres.size() != nFiltre) QMessageBox::warning(NULL,"Attention", QString::number(nFiltre) + " bandes et " + QString::number(filtres.size()) + " filtres");
            else {
                for (k = 0; k< nFiltre ; k++) // pour chaque bande
                {
                    zeroPadding(filtres[k], nfft);
                    rffts(filtres[k].data(), nlog, 1); // on passe les filtres en frequentielle sur nfft points
                    for (j=0 ; j <nPart ; j++) // Pour chaque partie de FIR de nfft points
                    {
                        rspectprod(firPart[j+k*nPart].data(), filtres[k].data(), firPart[j+k*nPart].data(), nfft);
                    }
                }
            }


            // somme par bande
            for (j=0 ; j <nPart ; j++) // pour chaque partie d'une bande
            {
                for (i=0 ; i <nfft ; i++) // pour chaque element
                {
                     for (k = 1; k< nFiltre ; k++) // on somme les valeurs des 7 bandes dans la premiere
                    {
                        firPart[j][i] += firPart[j+k*nPart][i];
                    }
                }
            }
            qDebug() << "Somme effectuee !";

           /* ///TEST
            //for (k=1 ; k < firPart.size() ; k++) {std::transform(firPart[0].begin(), firPart[0].end(), firPart[k].begin(), firPart[0].begin(), std::plus<float>());}
            riffts(firPart[0].data(), nlog,1);
            //for (auto &a : firPart) { riffts(a.data(), nlog, 1);}
            qDebug() << "Fir et filtres convolues !";
            plotWindow *firPlot = new plotWindow;
            firPlot->setWindowTitle("FIRs");
            //firPlot->XY(x2, firPart);
            firPlot->XY(x2, firPart[0]);
            firPlot->makePlot();
            //firPlot->hideLegend();
            firPlot->show();
            rffts(firPart[0].data(), nlog,1);
            ///
            */

            // découpage du wav
            std::vector< std::vector<float> > wavPart;
            partitionner(vectWav, nfft, wavPart);

            qDebug() << "Wav partitionne !";

            /*
            plotWindow *filtrePlot = new plotWindow;
            filtrePlot->setWindowTitle("Wav");
            filtrePlot->XY(x2, wavPart);
            filtrePlot->makePlot();
            filtrePlot->show();
            */

            //passage du wav en fft
            std::vector<float> buf1;
            buf1.resize(nfft, 0);
            std::vector< std::vector<float> >  buf2;
            buf2.resize(wavPart.size()+nPart-1);
            for (auto &a : buf2) { a.resize(nfft, 0); }

            for (k = 0; k < wavPart.size(); k++)
            {
                rffts(wavPart[k].data(), nlog, 1); // fft
                for(j=0 ; j <nPart ; j++) // pour chaque partie du filtre
                {
                    // multiplication spectrale du wav et des filtres
                    rspectprod(wavPart[k].data(), firPart[j].data(), buf1.data(), nfft);
                    //std::transform(wavPart[k].begin(), wavPart[k].end(), firPart[j].begin(), buf1.begin(), std::multiplies<float>());

                    std::transform(buf2[j+k].begin(), buf2[j+k].end(), buf1.begin(), buf2[j+k].begin(), std::plus<float>()); // somme terme à terme http://www.cplusplus.com/reference/algorithm/transform/
                }
            }
            qDebug() << "Wavs spectrales convolues !";
            qDebug() << "nb wavPart :" << wavPart.size();

            // iFFT
            for (auto &a : buf2) { riffts(a.data(), nlog, 1);}
            qDebug() << "iFFT OK !";




            std::vector<float> newWav;
            recombiner(buf2, newWav);
            qDebug() << "taille newWav : " << newWav.size();


            for (i=vectWav.size() ; i < newWav.size() ; i++)
            {
                x.push_back((float)i/samplerate);
            }

            // fin
            fftFree();

            // Création du nouveau fichier audio
            //float max = *std::max_element(newWav.begin(), newWav.end());

            std::vector<int> newData;
            for (auto &a : newWav) { newData.push_back((int)a); }

            // test
            //std::transform(vectWav.begin(), vectWav.end(), newWav.begin()+129, vectWav.begin(), std::minus<float>()); // somme terme à terme http://www.cplusplus.com/reference/algorithm/transform/


            // Affichage du fichier de sortie
            plotWindow *audioPlot2 = new plotWindow;
            audioPlot2->setWindowTitle("Audio Output");
            //audioPlot2->XY(x,newData);
            audioPlot2->XY(x,newWav);
            audioPlot2->makePlot();
            audioPlot2->setYLabel("Amplitude");
            audioPlot2->hideLegend();
            audioPlot2->show();

            wav.writeNewWav(newData);
            m_fichierAudio = QCoreApplication::applicationDirPath() + "/resultat.wav";
            ui->bouton_ecouter->setText("Resultat");

            wav.close();

            /*
            // Création du nouveau fichier audio
            std::vector<qint16> newData;
            for (auto &a : newWav) { newData.push_back((qint16)a); }
            //QByteArray* newDonnees= new QByteArray(reinterpret_cast<const char*>(newData.data()), newData.size());
            //QByteArray* newDonnees= new QByteArray(reinterpret_cast<const char*>(newWav.data()), newWav.size());
            QByteArray newDonnees;
            QDataStream out (&newDonnees,QIODevice::WriteOnly);
            for(auto &a : newData){out << a;}



            QFile wavOut;
            //QAudioFormat audioformat = wav.m_fileFormat;

            wavOut.setFileName(QCoreApplication::applicationDirPath() + "/output.raw");
            qDebug() << "wav open : " << wavOut.open(QIODevice::WriteOnly);
            wavOut.write(newDonnees);
            */




/*
            QBuffer *buffer = new QBuffer(player);
            //buffer->setBuffer(newDonnees);
            buffer->setData(newDonnees);
            qDebug() << "buffer size : " << buffer->size();
            buffer->open(QIODevice::ReadOnly);
            player->setMedia(QMediaContent(),buffer);
            qDebug() << "duration : " << player->duration();
            player->play();
            qDebug() << "Player error = " << player->error();

            qDebug() << "Player media = " << player->mediaStatus();
*/

            /*
            audioRecorder = new QAudioRecorder(this);
            audioRecorder->setOutputLocation(QCoreApplication::applicationDirPath() + "/output");

            QAudioEncoderSettings settings;
            settings.setCodec("audio/pcm");
            settings.setSampleRate(44100);
            settings.setChannelCount(1);

            audioRecorder->setEncodingSettings(settings, QVideoEncoderSettings(), "audio/x-wav");

            audioRecorder->record();
*/
/*
            const char * data2=newDonnees.constData(); // retourne le pointeur d'accés aux données
            const qint16 * datai2=reinterpret_cast<const qint16 *>(data2);

           // newfile.writeData(data2, newDonnees.size());

            //const qint16 * datai2=reinterpret_cast<const qint16 *>(newDonnees);
            //int len2=newDonnees.size()/(sizeof(qint16));
            int len2= x.size();
            std::vector<float> vectWav2;

            for (i=0;i<len2;++i) {
                vectWav2.push_back((float)datai2[i]);
            }

            plotWindow audioPlot3;
            audioPlot3.XY(x,vectWav2);
            audioPlot3.makePlot();
            audioPlot3.exec();


*/
            /// POUR ENREGISTRER LES WAV VOIR L'EXEMPLE QT AUDIO-RECORDER

        }
        else QMessageBox::warning(NULL,"Attention","La durée de la RIR est de 0s");
    }    
    else QMessageBox::warning(NULL,"Attention","Pas de fichier audio lisible");

}
