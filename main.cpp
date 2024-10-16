#include "mainwindow.h"
#include "battlefield.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BattleField field;
    field.show();
    return a.exec();
}
