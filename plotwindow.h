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

    void makePlot(std::vector<float> &x, std::vector<float> &y);

private slots:
  void mousePress();
  void mouseWheel();
  void selectionChanged();
  void hideSelectedGraph();
  void hideAllGraphs();
  void displaySelectedGraph();
  void displayAllGraphs();
  void contextMenuRequest(QPoint pos);

private:
    Ui::plotWindow *ui;
};

#endif // PLOTWINDOW_H
