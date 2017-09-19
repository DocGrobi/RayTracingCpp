#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "objwriter.h"
#include "raytracing.h"
#include "octree.h"
#include <QElapsedTimer>
#include <QMediaPlayer>
#include <QAudioProbe>
#include <QAudioRecorder>
#include "plotwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void suppFichier();// Suppression des fichiers d'export existant

private slots:
    void on_bouton_normales_clicked();

    void on_bouton_rayons_clicked();

    void on_bouton_source_clicked();

    void on_bouton_listener_clicked();

    void on_spinBox_nbRebond_valueChanged(int arg1);

    void on_checkBox__rebFixe_toggled(bool checked);

    void on_spinBox_attenuation_valueChanged(int arg1);

    void on_bouton_sourcesImages_clicked();

    void on_radioButton_vertexSource_toggled(bool checked);

    void on_radioButton_Fibonacci_toggled(bool checked);

    void on_spinBox_nbRay_valueChanged(int arg1);

    void on_bouton_RIR_clicked();

    void on_checkBox_rayAuto_toggled(bool checked);

    void on_bouton_octree_clicked();

    void on_checkBox_methodeRapide_toggled(bool checked);

    void on_bouton_audioFile_clicked();

    void on_bouton_ecouter_clicked();

    void on_AudioSlider_valueChanged(int value);

    void on_positionChanged(qint64 position);

    void on_durationChanged(qint64 position);

    void on_spinBox_humidite_editingFinished();

    void on_spinBox_temperature_editingFinished();

    void on_spinBox_nbFaceFeuille_editingFinished();

    void on_spinBox_freqEchan_editingFinished();

    void on_spinBox_seuilArret_editingFinished();

   // void slotGetMusicData(QAudioBuffer musicBuffer);

    void on_bouton_convolution_clicked();

    void on_bouton_projection_clicked();

private:
    Ui::MainWindow *ui;
    MeshObj m_meshObj;
    Source m_source;
    Listener m_listener;
    SourceImage m_sourceImage;
    Octree m_octree;
    QElapsedTimer m_timer; 
    QString m_fichierExport, m_fichierAudio;
    float m_seuilAttenuation, m_longueurRayMax;
    int m_temperature, m_humidite, m_nbRebond, m_freq, m_nbRayon, m_seuilArret, m_nbFaceFeuille;
    bool m_fibonacci, m_rayAuto, m_methodeRapide, m_nbRebondFixe;
    QMediaPlayer *player;

    QAudioRecorder *audioRecorder;

    plotWindow *plot, *audioPlot, *audioPlot2, *firPlot, *filtrePlot;

    //QAudioProbe *audioProbe;
    //QByteArray *musicDataBuffer;
};

#endif // MAINWINDOW_H
