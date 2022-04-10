#include "mainwindow.h"
#include <QDebug>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug() << "Here";
    qApp->setApplicationName("Breather");
    MainWindow w;
    w.show();
    return a.exec();
}
