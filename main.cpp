#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QTranslator translator;
    if (translator.load("translations/customer_database_fr"))
        a.installTranslator(&translator);
    QTranslator baseTranslator;
    if (baseTranslator.load("translations/qt_fr"))
        a.installTranslator(&baseTranslator);


    w.showMaximized();
    return a.exec();
}
