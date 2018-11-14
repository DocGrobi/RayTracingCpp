#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;    
    w.setWindowIcon(QIcon(QCoreApplication::applicationDirPath()+ "/Logo_GM.icns")); //Parametrage du Logo
    w.show();

    return a.exec();
}
