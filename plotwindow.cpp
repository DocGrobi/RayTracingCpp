#include "plotwindow.h"
#include "ui_plotwindow.h"

plotWindow::plotWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::plotWindow)
{
    ui->setupUi(this);

    ui->customPlot->legend->setVisible(true);
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                      QCP::iSelectLegend | QCP::iSelectPlottables | QCP::iMultiSelect);
    ui->customPlot->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

      // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
      connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
      connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

      // connect slot that ties some axis selections together (especially opposite axes):
      connect(ui->customPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

      // setup policy and connect slot for context menu popup:
      ui->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
      connect(ui->customPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
}

plotWindow::~plotWindow()
{
    delete ui;
}


void plotWindow::makePlot(std::vector<float> x, std::vector<float> y)
{
    // Conversion en double
    std::vector<double> vX(x.begin(),x.end());
    std::vector<double> vY(y.begin(),y.end());
    // Conversion en QVector
    QVector<double> vectX = QVector<double>::fromStdVector(vX);
    QVector<double> vectY = QVector<double>::fromStdVector(vY);

    // Recupération des maximums
    double xMax = vectX[vectX.size() - 1];
    double yMax(0) ;

    for (int i = 0 ; i < vectY.size() ; i++)
    {
        if (vectY[i] > yMax)
        {
            yMax = vectY[i];
        }
    }

    // Normalisation sur les y et repartition sur les 8 courbes
    QVector<QVector<double> > courbe;
    courbe.resize(8);

    for (int i = 0 ; i < vectY.size() ; i=i+8)
    {
        for (int k = 0 ; k < 8 ; k++) // pour chaque bande
        {
            courbe[k].push_back(vectY[i+k]/yMax);
        }
    }

    // Création graphique des courbes
    for (int k = 0 ; k < 8 ; k++) // pour chaque bande
    {
        ui->customPlot->addGraph();
        ui->customPlot->graph(k)->setData(vectX, courbe[k]);
        QString nom = QString::number(62.5*pow(2,k)) + "Hz";
        ui->customPlot->graph(k)->setName(nom);
        ui->customPlot->graph(k)->setPen(QPen(QColor::fromHsv(360/8*k,255,255)));
    }

    // Noms des axes
    ui->customPlot->xAxis->setLabel("Temps (ms)");
    ui->customPlot->yAxis->setLabel("Energie nomalisée");

    // Regalges des plages des axes
    //xMax = 40; //pour les tests
    ui->customPlot->xAxis->setRange(0, xMax);
    ui->customPlot->yAxis->setRange(0, 1);
    ui->customPlot->replot();
}



// SLOTS :

void plotWindow::mousePress()
{
  // if an axis is selected, only allow the direction of that axis to be dragged
  // if no axis is selected, both directions may be dragged

  if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->xAxis->orientation());
  else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->yAxis->orientation());
  else
    ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void plotWindow::mouseWheel()
{
  // if an axis is selected, only allow the direction of that axis to be zoomed
  // if no axis is selected, both directions may be zoomed

  if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->xAxis->orientation());
  else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->yAxis->orientation());
  else
    ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void plotWindow::selectionChanged()
{
  /*
   normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
   the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
   and the axis base line together. However, the axis label shall be selectable individually.

   The selection state of the left and right axes shall be synchronized as well as the state of the
   bottom and top axes.

   Further, we want to synchronize the selection of the graphs with the selection state of the respective
   legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
   or on its legend item.
  */

  // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      ui->customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    ui->customPlot->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->customPlot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }
  // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    ui->customPlot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->customPlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }

  // synchronize selection of graphs with selection of corresponding legend items:
  for (int i=0; i<ui->customPlot->graphCount(); ++i)
  {
    QCPGraph *graph = ui->customPlot->graph(i);
    QCPPlottableLegendItem *item = ui->customPlot->legend->itemWithPlottable(graph);
    if (item->selected() || graph->selected())
    {
      item->setSelected(true);
      graph->setSelected(true);
    }
  }
}


void plotWindow::contextMenuRequest(QPoint pos)
{
  QMenu *menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);

    if (ui->customPlot->selectedGraphs().size() > 0)
    {
        menu->addAction("Hide selected graph", this, SLOT(hideSelectedGraph()));
        menu->addAction("Display selected graph", this, SLOT(displaySelectedGraph()));
    }
    if (ui->customPlot->graphCount() > 0)
    {
        menu->addAction("Hide all graphs", this, SLOT(hideAllGraphs()));
        menu->addAction("Display all graphs", this, SLOT(displayAllGraphs()));
    }

  menu->popup(ui->customPlot->mapToGlobal(pos));
}


void plotWindow::hideSelectedGraph()
{
  if (ui->customPlot->selectedGraphs().size() > 0)
  {
      foreach (QCPGraph* gr, ui->customPlot->selectedGraphs())
      {
          gr->setVisible(false);
      }
      ui->customPlot->replot();
  }
}

void plotWindow::displaySelectedGraph()
{
  if (ui->customPlot->selectedGraphs().size() > 0)
  {
      foreach (QCPGraph* gr, ui->customPlot->selectedGraphs())
      {
          gr->setVisible(true);
      }
      ui->customPlot->replot();
  }
}

void plotWindow::hideAllGraphs()
{
    for (int i = 0 ; i <ui->customPlot->graphCount() ; i++)
    {
        ui->customPlot->graph(i)->setVisible(false);
    }
    ui->customPlot->replot();
}

void plotWindow::displayAllGraphs()
{
    for (int i = 0 ; i <ui->customPlot->graphCount() ; i++)
    {
        ui->customPlot->graph(i)->setVisible(true);
    }
    ui->customPlot->replot();
}

