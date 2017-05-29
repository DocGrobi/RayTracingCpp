#include "plotwindow.h"
#include "ui_plotwindow.h"

plotWindow::plotWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::plotWindow)
{
    ui->setupUi(this);
    /*ui->customPlot->setInteraction(QCP::iRangeDrag, true);
    ui->customPlot->setInteraction(QCP::iRangeZoom, true);
    ui->customPlot->setInteraction(QCP::iSelectPlottables, true);
    ui->customPlot->setInteraction(QCP::iSelectLegend, true);
    ui->customPlot->setInteraction(QCP::iMultiSelect, true);
    */
    ui->customPlot->legend->setVisible(true);
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                      QCP::iSelectLegend | QCP::iSelectPlottables | QCP::iMultiSelect);
    ui->customPlot->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

      // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
      connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
      connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

      // connect slot that ties some axis selections together (especially opposite axes):
      connect(ui->customPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

      // cache un graph si on double click sur sa legende
      connect(ui->customPlot, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*)));

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

    QVector<double> y0,y1,y2,y3,y4,y5,y6,y7;
    // Normalisation sur les y
    for (int i = 0 ; i < vectY.size() ; i=i+8)
    {
        for (int k = 0 ; k < 8 ; k++) // pour chaque bande
        {
            vectY[i+k] = vectY[i+k]/yMax;
        }
        y0.push_back(vectY[i]);
        y1.push_back(vectY[i+1]);
        y2.push_back(vectY[i+2]);
        y3.push_back(vectY[i+3]);
        y4.push_back(vectY[i+4]);
        y5.push_back(vectY[i+5]);
        y6.push_back(vectY[i+6]);
        y7.push_back(vectY[i+7]);
    }


    //Création des 8 graphes de bande
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setData(vectX, y0);
    ui->customPlot->graph(0)->setPen(QPen(QColor(Qt::red)));
    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setData(vectX, y1);
    ui->customPlot->graph(1)->setPen(QPen(QColor(Qt::yellow)));
    ui->customPlot->addGraph();
    ui->customPlot->graph(2)->setData(vectX, y2);
    ui->customPlot->graph(2)->setPen(QPen(QColor(Qt::green)));
    ui->customPlot->addGraph();
    ui->customPlot->graph(3)->setData(vectX, y3);
    ui->customPlot->graph(3)->setPen(QPen(QColor(Qt::cyan)));
    ui->customPlot->addGraph();
    ui->customPlot->graph(4)->setData(vectX, y4);
    ui->customPlot->graph(4)->setPen(QPen(QColor(Qt::blue)));
    ui->customPlot->addGraph();
    ui->customPlot->graph(5)->setData(vectX, y5);
    ui->customPlot->graph(5)->setPen(QPen(QColor(Qt::magenta)));
    ui->customPlot->addGraph();
    ui->customPlot->graph(6)->setData(vectX, y6);
    ui->customPlot->graph(6)->setPen(QPen(QColor(Qt::darkRed)));
    ui->customPlot->addGraph();
    ui->customPlot->graph(7)->setData(vectX, y7);
    ui->customPlot->graph(7)->setPen(QPen(QColor(Qt::black)));

    // Donner des noms aux graphes
    for (int k = 0 ; k < 8 ; k++) // pour chaque bande
    {
        QString nom = QString::number(62.5*pow(2,k)) + "Hz";
        ui->customPlot->graph(k)->setName(nom);
        //ui->customPlot->graph(k)->setSelectable(true);

    }

    // give the axes some labels:
    ui->customPlot->xAxis->setLabel("Temps (ms)");
    ui->customPlot->yAxis->setLabel("Energie nomalisée");
    // set axes ranges, so we see all data:

    //xMax = 40; //pour les tests
    ui->customPlot->xAxis->setRange(0, xMax);
    ui->customPlot->yAxis->setRange(0, 1);
    ui->customPlot->replot();
}

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

void plotWindow::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
  // Cacher un graph sur on double clic sur sa legende
  Q_UNUSED(legend)
  if (item) // only react if item was clicked (user could have clicked on border padding of legend where there is no item, then item is 0)
  {
    QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
    bool ok;
    QString newName = QInputDialog::getText(this, "QCustomPlot example", "New graph name:", QLineEdit::Normal, plItem->plottable()->name(), &ok);
    if (ok)
    {
      plItem->plottable()->setName(newName);
      ui->customPlot->replot();
    }
  }
}

void plotWindow::contextMenuRequest(QPoint pos)
{
  QMenu *menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);

  if (ui->customPlot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
  {
      // A REMPLIR !

  } else  // general context menu on graphs requested
  {
    menu->addAction("Add random graph", this, SLOT(addRandomGraph()));
    if (ui->customPlot->selectedGraphs().size() > 0)
      menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
    if (ui->customPlot->graphCount() > 0)
      menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
  }

  menu->popup(ui->customPlot->mapToGlobal(pos));
}

void plotWindow::removeSelectedGraph()
{
  if (ui->customPlot->selectedGraphs().size() > 0)
  {
    ui->customPlot->removeGraph(ui->customPlot->selectedGraphs().first());
    ui->customPlot->replot();
  }
}

void plotWindow::removeAllGraphs()
{
  ui->customPlot->clearGraphs();
  ui->customPlot->replot();
}
