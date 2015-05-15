#include <QApplication>
#include <QDebug>

#include "maindialog.hpp"
#include "persistencemanager_interface.h"

using namespace std;

ConfigModel loadModel()
{
    return {};
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ConfigurationParameterInfo::registerMetaTypes();
    ConfigurationParameterValue::registerMetaTypes();
    ConfigurationCategory::registerMetaTypes();
    ConfigurationSet::registerMetaTypes();
    KeyedMessage::registerMetaTypes();

    auto interface = new OrgEsaSen2agriPersistenceManagerInterface(
        OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(), QStringLiteral("/"),
        QDBusConnection::systemBus());
    auto foo = interface->GetConfigurationSet();
    foo.waitForFinished();
    ConfigurationParameterInfoList params;
    if (foo.isValid()) {
        params = foo.value();

    } else if (foo.isError()) {
        qDebug() << foo.error().message();
    }

    ConfigurationSet configuration;
    configuration.categories.append({ 1, "General" });
    configuration.categories.append({ 2, "Not used" });
    configuration.categories.append({ 3, "L2A" });

    configuration.parameters.append({ "test.foo", 1, "Foo", "string", "val 1", false });
    configuration.parameters.append({ "test.bar", 1, "Boo", "string", "val 2", false });
    configuration.parameters.append({ "test.baz", 3, "Baz", "string", "val 2", false });

    ConfigModel model(configuration);

    MainDialog w(model);
    w.show();

    return a.exec();
}
