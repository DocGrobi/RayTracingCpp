#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "objwriter.h"
#include "raytracing.h"
#include "octree.h"
#include <QElapsedTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_bouton_normales_clicked();

    void on_bouton_rayons_clicked();

    void on_bouton_source_clicked();

    void on_bouton_listener_clicked();

    void on_spinBox_nbRebond_valueChanged(int arg1);

    void on_checkBox__rebFixe_toggled(bool checked);

    void on_spinBox_attenuation_valueChanged(int arg1);

    void on_spinBox_temperature_valueChanged(int arg1);

    void on_bouton_sourcesImages_clicked();

    void on_radioButton_vertexSource_toggled(bool checked);

    void on_radioButton_Fibonacci_toggled(bool checked);

    void on_spinBox_nbRay_valueChanged(int arg1);

    void on_bouton_RIR_clicked();

    void on_spinBox_freqEchan_valueChanged(int arg1);

    void on_checkBox_rayAuto_toggled(bool checked);

    void on_spinBox_seuilArret_valueChanged(int arg1);

    void on_bouton_octree_clicked();

    void on_spinBox_nbFaceFeuille_valueChanged(int arg1);

    void on_checkBox_methodeRapide_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    MeshObj m_meshObj;
    Source m_source;
    Listener m_listener;
    SourceImage m_sourceImage;
    Octree m_octree;
    QElapsedTimer m_timer;
    QString m_fichierExport;
    float m_seuilAttenuation, m_longueurRayMax;
    int m_temperature, m_nbRebond, m_freq, m_nbRayon, m_seuilArret, m_nbFaceFeuille;
    bool m_fibonacci, m_rayAuto, m_methodeRapide, m_nbRebondFixe;
};

#endif // MAINWINDOW_H
