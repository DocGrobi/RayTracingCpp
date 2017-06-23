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
    //std::vector<Boite> m_octree;
    //Ray m_monRay;
    int m_nbRebond;
    float m_seuilAttenuation;
    int m_temperature;
    bool m_nbRebondFixe;
    QElapsedTimer m_timer;
    bool m_fibonacci;
    int m_nbRayon;
    int m_freq;
    int m_seuilArret;
    bool m_rayAuto;
    float m_longueurRayMax;
    int m_nbFaceFeuille;
    bool m_methodeRapide;


};

#endif // MAINWINDOW_H
