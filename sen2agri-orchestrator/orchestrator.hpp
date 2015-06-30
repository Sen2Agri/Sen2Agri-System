#pragma once

#include <QObject>
#include <QDBusContext>

#include "persistencemanager_interface.h"
#include "orchestratorworker.hpp"

class Orchestrator : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    explicit Orchestrator(QObject *parent = 0);

signals:

public slots:
    void NotifyEventsAvailable();

private:
    OrgEsaSen2agriPersistenceManagerInterface persistenceManagerClient;
    OrgEsaSen2agriProcessorsExecutorInterface executorClient;
    OrchestratorWorker worker;

    void RescanEvents();
};
