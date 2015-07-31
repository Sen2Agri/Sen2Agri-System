#pragma once

#include <map>
#include <memory>

#include <QObject>
#include <QDBusContext>

#include "orchestratorworker.hpp"
#include "processorhandler.hpp"
#include "persistencemanager_interface.h"

class Orchestrator : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    explicit Orchestrator(std::map<int, std::unique_ptr<ProcessorHandler>> &handlerMap,
                          QObject *parent = 0);

signals:

public slots:
    void NotifyEventsAvailable();

private:
    OrgEsaSen2agriPersistenceManagerInterface persistenceManagerClient;
    OrgEsaSen2agriProcessorsExecutorInterface executorClient;
    OrchestratorWorker worker;

    void RescanEvents();
};
