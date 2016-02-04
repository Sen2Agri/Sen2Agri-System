#pragma once

#include <map>
#include <memory>

#include <QObject>
#include <QDBusContext>

#include "persistencemanager.hpp"
#include "orchestratorworker.hpp"
#include "processorhandler.hpp"

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
    PersistenceManagerDBProvider persistenceManager;
    OrgEsaSen2agriProcessorsExecutorInterface executorClient;
    OrchestratorWorker worker;

    void RescanEvents();
};
