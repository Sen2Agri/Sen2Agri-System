#include "simulator.h"

#include "persistencemanager.h"
#include "persistencemanager_adaptor.h"

Simulator::Simulator()
{
    registerMetaTypes();

    QDBusConnection connection = QDBusConnection::sessionBus();

    m_pPersistenceMng = new PersistenceManager();
    new PersistenceManagerAdaptor(m_pPersistenceMng);

    if (!connection.registerObject("/org/esa/sen2agri/persistenceManager", m_pPersistenceMng)) {
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

}

Simulator::~Simulator()
{

}

