#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("htmlTester");
    a.setApplicationVersion("0.1");
    a.setOrganizationName("HoMSoft");
    a.setOrganizationDomain("www.mairon.net");

    MainWindow w;
    w.show();
    return a.exec();
}
