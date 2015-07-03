#include <QDBusPendingCallWatcher>

#include <optional.hpp>

#include "orchestrator.hpp"

Orchestrator::Orchestrator(QObject *parent)
    : QObject(parent),
      persistenceManagerClient(OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(),
                               QStringLiteral("/org/esa/sen2agri/persistenceManager"),
                               QDBusConnection::systemBus()),
      executorClient(OrgEsaSen2agriProcessorsExecutorInterface::staticInterfaceName(),
                     QStringLiteral("/"),
                     QDBusConnection::systemBus()),
      worker(persistenceManagerClient, executorClient)
{
    worker.RescanEvents();
}

void Orchestrator::NotifyEventsAvailable()
{
    RescanEvents();
}

void Orchestrator::RescanEvents()
{
    worker.RescanEvents();
}
