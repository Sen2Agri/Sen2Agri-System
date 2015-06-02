#include "archivermanager.hpp"
#include <QTextStream>

ArchiverManager::ArchiverManager() :
    clientInterface(OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(),
                                                  QStringLiteral("/"),
                                                  QDBusConnection::systemBus())
{


}

ArchiverManager::~ArchiverManager()
{

}

void ArchiverManager::start(const QCoreApplication& app)
{
    connect(&app, SIGNAL(aboutToQuit()), this, SLOT(lastAction()));
    connect(&mExitTimer, SIGNAL(timeout()), this, SLOT(exit()));
    //TODO: take parameters
    //TODO create for each site a FILE_INFO obj insert it into files

    //TODO: split in groups based on sites
    //
    auto promise = clientInterface.GetConfigurationParameters("archiver.");
    connect(new QDBusPendingCallWatcher(promise, this), &QDBusPendingCallWatcher::finished,
            [this, promise]() {
                if (promise.isValid()) {
                    loadConfigParam(promise.value());
                    applyConfig();
                } else if (promise.isError()) {
                    qDebug() << promise.error().message();
                }
            });
    //mExitTimer.start(5000);

}

void ArchiverManager::loadConfigParam(const ConfigurationParameterValueList &configuration)
{
    for (const auto &p : configuration) {
        if(p.key == "archiver.archive_path")
            readConfigParams.rootPath = p.value;
        if(p.key == "archiver.max_age.l2a")
            readConfigParams.maxAgeL2a = p.value.toInt();
        if(p.key == "archiver.max_age.l3a")
            readConfigParams.maxAgeL3a = p.value.toInt();
        if(p.key == "archiver.max_age.l3b")
            readConfigParams.maxAgeL3b = p.value.toInt();
        if(p.key == "archiver.max_age.l4a")
            readConfigParams.maxAgeL4a = p.value.toInt();
        if(p.key == "archiver.max_age.l4b")
            readConfigParams.maxAgeL4b = p.value.toInt();
    }

}

void ArchiverManager::applyConfig()
{
    //readConfigParams
}

void ArchiverManager::lastAction()
{
    QTextStream(stdout) << "BYE\n";
}

void ArchiverManager::exit()
{
    QCoreApplication::exit(0);
}
