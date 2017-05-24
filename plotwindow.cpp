#include "plotwindow.h"
#include "ui_plotwindow.h"

plotWindow::plotWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::plotWindow)
{
    ui->setupUi(this);
    std::vector<float> x, y;
    x.resize(101,1);
    y.resize(101,1);
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0 - 1; // x goes from -1 to 1
      y[i] = x[i]*x[i]; // let's plot a quadratic function
    }
    plotWindow::makePlot(x, y);
}

plotWindow::~plotWindow()
{
    delete ui;
}


void plotWindow::makePlot(std::vector<float> x, std::vector<float> y)
{
    std::vector<double> vX(x.begin(),x.end());
    std::vector<double> vY(y.begin(),y.end());
    QVector<double> vectX = QVector<double>::fromStdVector(vX);
    QVector<double> vectY = QVector<double>::fromStdVector(vY);

    // ECHELLES A CHANGER !!!


    // create graph and assign data to it:
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setData(vectX, vectY);
    // give the axes some labels:
    ui->customPlot->xAxis->setLabel("x");
    ui->customPlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->customPlot->xAxis->setRange(-1, 1);
    ui->customPlot->yAxis->setRange(0, 1);
    ui->customPlot->replot();
}
