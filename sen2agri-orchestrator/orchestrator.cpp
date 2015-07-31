#include <QDBusPendingCallWatcher>

#include <optional.hpp>

#include "orchestrator.hpp"

Orchestrator::Orchestrator(std::map<int, std::unique_ptr<ProcessorHandler>> &handlerMap,
                           QObject *parent)
    : QObject(parent),
      persistenceManagerClient(OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(),
                               QStringLiteral("/org/esa/sen2agri/persistenceManager"),
                               QDBusConnection::systemBus()),
      executorClient(OrgEsaSen2agriProcessorsExecutorInterface::staticInterfaceName(),
                     QStringLiteral("/org/esa/sen2agri/processorsExecutor"),
                     QDBusConnection::systemBus()),
      worker(handlerMap, persistenceManagerClient, executorClient)
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
