#ifndef DATA_H
#define DATA_H

#include <QDialog>
#include <QLabel>
#include <QGridLayout>

namespace Ui {
class Data;
}

class Data : public QDialog
{
    Q_OBJECT

public:
    explicit Data(QWidget *parent = 0);
    ~Data();
    void addValue(QString val, int row, int col);



private:
    Ui::Data *ui;
    QLabel *text;
    //QGridLayout *layout;
};

#endif // DATA_H
