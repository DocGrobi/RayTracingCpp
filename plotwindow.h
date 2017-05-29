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

//private slots:
    void makePlot(std::vector<float> x, std::vector<float> y);

private slots:
  void mousePress();
  void mouseWheel();
  void selectionChanged();
  void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
  void removeSelectedGraph();
  void removeAllGraphs();
  void contextMenuRequest(QPoint pos);

private:
    Ui::plotWindow *ui;
};

#endif // PLOTWINDOW_H
