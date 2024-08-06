#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    for (int I = 0; I < argc; I++)
    {
        w.Params.push_back(string(argv[I]));
    }
    w.show();

    return a.exec();
}
