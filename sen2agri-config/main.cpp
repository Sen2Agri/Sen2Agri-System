#include <QApplication>
#include <QDebug>

#include "maindialog.hpp"
#include "keyedmessage.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ConfigurationParameterInfo::registerMetaTypes();
    ConfigurationParameterValue::registerMetaTypes();
    ConfigurationCategory::registerMetaTypes();
    ConfigurationSet::registerMetaTypes();
    KeyedMessage::registerMetaTypes();

    MainDialog w;
    w.show();

    return a.exec();
}
