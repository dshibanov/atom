#include "mainwindow.h"
#include <QApplication>

#ifdef WIN32
long apuErrorStatus = 0;
#endif


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();    
    
    return a.exec();
}
