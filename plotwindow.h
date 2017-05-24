#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QDialog>

namespace Ui {
class plotWindow;
}

class plotWindow : public QDialog
{
    Q_OBJECT

public:
    explicit plotWindow(QWidget *parent = 0);
    ~plotWindow();

private slots:
    void makePlot(std::vector<float> x, std::vector<float> y);
private:
    Ui::plotWindow *ui;
};

#endif // PLOTWINDOW_H
