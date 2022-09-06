#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QTranslator translator;
    if (!translator.load("customer_database_fr"))
        return 1;
    a.installTranslator(&translator);
    QTranslator baseTranslator;
    if (!baseTranslator.load("qt_fr"))
        return 2;
    a.installTranslator(&baseTranslator);


    w.showMaximized();
    return a.exec();
}
