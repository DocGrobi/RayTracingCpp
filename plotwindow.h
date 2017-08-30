#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QDialog>
#include"qcustomplot.h"

namespace Ui {
class plotWindow;
}

class plotWindow : public QDialog
{
    Q_OBJECT

public:
    explicit plotWindow(QWidget *parent = 0);
    ~plotWindow();

    void makePlot();
    void XY(std::vector<float> &x, std::vector<std::vector<float> > &y, float seuil);
    void XY(std::vector<float> &x, std::vector<float> &y); // courbe quelconque
    void XY(std::vector<float> &x, std::vector<qint16> &y);
    void setYLabel(QString nom);
    void hideLegend();
    void setX(int min, int max);

private slots:
  void mousePress();
  void mouseWheel();
  void selectionChanged();
  void hideSelectedGraph();
  void hideAllGraphs();
  void displaySelectedGraph();
  void displayAllGraphs();
  void contextMenuRequest(QPoint pos);
  void linScale();
  void logScale();

private:
    Ui::plotWindow *ui;
    bool m_echelleLog;
    QVector<double> vectX, vectY;
    double xMax, yMax, yMin;
    QVector<QVector<double> > courbe;
};

#endif // PLOTWINDOW_H
