#include <QCoreApplication>
#include "ApplicationClosingListener.h"

#include "persistencemanager.h"
#include "persistencemanager_adaptor.h"

#define SERVICE_NAME "org.esa.sen2agri.processorsexecutor"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    registerMetaTypes();

    QDBusConnection connection = QDBusConnection::sessionBus();
    PersistenceManager persistenceManager;
    // Initialize the configuration manager
    QString strCfgPath("../../config/ProcessorsExecutorCfg.ini");
    if(argc >= 2)
    {
        strCfgPath = argv[1];
    }
    persistenceManager.Initialize(strCfgPath);

    new PersistenceManagerAdaptor(&persistenceManager);

    if (!connection.registerObject("/org/esa/sen2agri/persistencemanager", &persistenceManager)) {
        throw std::runtime_error(
            QStringLiteral("Error registering the object with D-Bus: %1, exiting.")
                .arg(connection.lastError().message())
                .toStdString());
    }

    if (!connection.registerService("org.esa.sen2agri.persistenceManager")) {
        throw std::runtime_error(
            QStringLiteral("Error registering the service with D-Bus: %1, exiting.")
                .arg(connection.lastError().message())
                .toStdString());
    }


    // Task parented to the application so that it
    // will be deleted by the application.
    ApplicationClosingListener *closingListener = new ApplicationClosingListener(&a);
    closingListener->SetPersistenceManager(&persistenceManager);

    // This will cause the application to exit when
    // the closingListener signals finished.
    QObject::connect(closingListener, SIGNAL(finished()), &a, SLOT(quit()));

    QTimer::singleShot(30000, closingListener, SLOT(SendExecuteProcessor()));

    QTimer::singleShot(32000, closingListener, SLOT(SendCancelProcessor()));

    // This will run the closingListener from the application event loop.
    QTimer::singleShot(35000, closingListener, SLOT(run()));

    return a.exec();
}
