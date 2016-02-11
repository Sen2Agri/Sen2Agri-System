#include <QDBusPendingCallWatcher>

#include <optional.hpp>

#include "orchestrator.hpp"
#include "settings.hpp"
#include "configuration.hpp"

#include "make_unique.hpp"
#include "processor/cropmaskhandler.hpp"
#include "processor/croptypehandler.hpp"
#include "processor/compositehandler.hpp"
#include "processor/lairetrievalhandler.hpp"
#include "processor/phenondvihandler.hpp"
#include "processor/dummyprocessorhandler.hpp"

std::map<int, std::unique_ptr<ProcessorHandler>> & GetHandlersMap(PersistenceManagerDBProvider &persistenceManager) {
    ProcessorDescriptionList processorsDescriptions = persistenceManager.GetProcessorDescriptions();
    static std::map<int, std::unique_ptr<ProcessorHandler>> handlersMap;
    for(ProcessorDescription procDescr: processorsDescriptions) {
        if(procDescr.shortName == "l2a") {
            // TODO:
            //handlers.emplace(procDescr.processorId, std::make_unique<MACCSHandler>());
        } else if(procDescr.shortName == "l3a") {
            handlersMap.emplace(procDescr.processorId, std::make_unique<CompositeHandler>());
        } else if(procDescr.shortName == "l3b_lai") {
            handlersMap.emplace(procDescr.processorId, std::make_unique<LaiRetrievalHandler>());
        } else if(procDescr.shortName == "l3b_pheno") {
            handlersMap.emplace(procDescr.processorId, std::make_unique<PhenoNdviHandler>());
        } else if(procDescr.shortName == "l4a") {
            handlersMap.emplace(procDescr.processorId, std::make_unique<CropMaskHandler>());
        } else if(procDescr.shortName == "l4b") {
            handlersMap.emplace(procDescr.processorId, std::make_unique<CropTypeHandler>());
        } else if(procDescr.shortName == "dummy") {
            handlersMap.emplace(procDescr.processorId, std::make_unique<DummyProcessorHandler>());
        } else {
            throw std::runtime_error(
                QStringLiteral("Invalid processor configuration found in database: %1, exiting.")
                    .arg(procDescr.shortName).toStdString());
        }
    }

    return handlersMap;
}


Orchestrator::Orchestrator(QObject *parent)
    : QObject(parent),
      persistenceManager(
          Settings::readSettings(getConfigurationFile(*QCoreApplication::instance()))),
      executorClient(OrgEsaSen2agriProcessorsExecutorInterface::staticInterfaceName(),
                     QStringLiteral("/org/esa/sen2agri/processorsExecutor"),
                     QDBusConnection::systemBus()),
      worker(GetHandlersMap(persistenceManager), persistenceManager, executorClient)
{
    worker.RescanEvents();
}

void Orchestrator::NotifyEventsAvailable() { RescanEvents(); }

void Orchestrator::RescanEvents() { worker.RescanEvents(); }
