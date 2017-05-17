#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "objwriter.h"
#include "raytracing.h"

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

private:
    Ui::MainWindow *ui;
    MeshObj m_meshObj;
    Source m_source;
    Listener m_listener;
    int m_nbRebond;
    int m_seuilAttenuation;
    int m_temperature;
    bool m_nbRebondFixe;
};

#endif // MAINWINDOW_H
