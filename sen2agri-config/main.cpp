#include <QApplication>
#include <QDebug>

#include "maindialog.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ConfigurationParameterInfo::registerMetaTypes();
    ConfigurationParameterValue::registerMetaTypes();
    ConfigurationCategory::registerMetaTypes();
    Site::registerMetaTypes();
    ConfigurationSet::registerMetaTypes();

    ConfigurationUpdateAction::registerMetaTypes();

    KeyedMessage::registerMetaTypes();

    MainDialog w;
    w.show();

    return a.exec();
}
