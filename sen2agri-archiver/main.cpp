#include <QCoreApplication>
#include "archivermanager.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    ConfigurationParameterInfo::registerMetaTypes();
    ConfigurationParameterValue::registerMetaTypes();
    ConfigurationCategory::registerMetaTypes();
    Site::registerMetaTypes();
    ConfigurationSet::registerMetaTypes();

    ConfigurationUpdateAction::registerMetaTypes();

    KeyedMessage::registerMetaTypes();
    Product::registerMetaTypes();
    ArchiverManager arch;
    arch.start(app);
    return app.exec();
}
