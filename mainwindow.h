#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "objwriter.h"
#include "raytracing.h"
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

    void on_spinBox_nbRay_valueChanged(int arg1);

    void on_checkBox__rayFixe_toggled(bool checked);

    void on_spinBox_attenuation_valueChanged(int arg1);

    void on_spinBox_temperature_valueChanged(int arg1);

    void on_bouton_sourcesImages_clicked();

private:
    Ui::MainWindow *ui;
    MeshObj m_meshObj;
    Source m_source;
    Listener m_listener;
    Ray m_monRay;
    int m_nbRebond;
    float m_seuilAttenuation;
    int m_temperature;
    bool m_nbRebondFixe;
    QElapsedTimer m_timer;

};

#endif // MAINWINDOW_H
