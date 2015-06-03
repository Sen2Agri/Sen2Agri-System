#include <QApplication>
#include <QDebug>

#include "maindialog.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    registerMetaTypes();

    MainDialog w;
    w.show();

    return a.exec();
}
