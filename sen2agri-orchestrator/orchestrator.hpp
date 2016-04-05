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
    explicit Orchestrator(QObject *parent = 0);

signals:

public slots:
    void NotifyEventsAvailable();
    JobDefinition GetJobDefinition(const ProcessingRequest &request);
    void SubmitJob(const JobDefinition &job);

private:
    PersistenceManagerDBProvider persistenceManager;
    OrgEsaSen2agriProcessorsExecutorInterface executorClient;
    OrchestratorWorker worker;

    void RescanEvents();

    QString GetProcessorShortName(int processorId);
    QString GetSiteName(int siteId);
};

class Timer : public QTimer {
  Q_OBJECT
public:
  explicit Timer(Orchestrator *pOrchestrator, QObject *parent = 0) : QTimer(parent) {
        m_pOrchestrator = pOrchestrator;
        connect(this, SIGNAL(timeout()), this, SLOT(doRescanEvents()));
  }
private slots:
    void doRescanEvents() {
        m_pOrchestrator->NotifyEventsAvailable();
    }

private:
    Orchestrator *m_pOrchestrator;
};
