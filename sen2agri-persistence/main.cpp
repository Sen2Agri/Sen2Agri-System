#include <stdexcept>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>
#include <QDBusError>

#include "persistencemanager.hpp"
#include "persistencemanager_adaptor.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    PersistenceManager::registerMetaTypes();

    PersistenceManager persistenceManager;
    new PersistenceManagerAdaptor(&persistenceManager);

    try {
        QDBusConnection connection = QDBusConnection::systemBus();
        if (!connection.registerObject("/", &persistenceManager)) {
            throw std::runtime_error(connection.lastError().message().toStdString());
        }

        if (!connection.registerService("org.esa.sen2agri.persistenceManager")) {
            throw std::runtime_error(connection.lastError().message().toStdString());
        }
    } catch (const std::exception &e) {
        qDebug() << QStringLiteral("Error registering to the system bus: %1").arg(e.what());
    }

    return app.exec();
}
