#include <QDBusPendingCallWatcher>

#include <optional.hpp>

#include "orchestrator.hpp"
#include "settings.hpp"
#include "configuration.hpp"

Orchestrator::Orchestrator(std::map<int, std::unique_ptr<ProcessorHandler>> &handlerMap,
                           QObject *parent)
    : QObject(parent),
      persistenceManager(Settings::readSettings(getConfigurationFile(*QCoreApplication::instance()))),
      executorClient(OrgEsaSen2agriProcessorsExecutorInterface::staticInterfaceName(),
                     QStringLiteral("/org/esa/sen2agri/processorsExecutor"),
                     QDBusConnection::systemBus()),
      worker(handlerMap, persistenceManager, executorClient)
{
    worker.RescanEvents();
}

void Orchestrator::NotifyEventsAvailable() { RescanEvents(); }

void Orchestrator::RescanEvents() { worker.RescanEvents(); }
