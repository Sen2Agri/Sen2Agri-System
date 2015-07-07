#pragma once

#include <map>
#include <memory>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QObject>
#include <QThread>

#include <optional.hpp>

#include "eventprocessingcontext.hpp"
#include "processorhandler.hpp"
#include "persistencemanager_interface.h"
#include "processorsexecutor_interface.h"

class OrchestratorWorker : public QObject
{
    Q_OBJECT

public:
    OrchestratorWorker(std::map<int, std::unique_ptr<ProcessorHandler>> &handlerMap,
                       OrgEsaSen2agriPersistenceManagerInterface &persistenceManagerClient,
                       OrgEsaSen2agriProcessorsExecutorInterface &executorClient);

signals:

public slots:
    void RescanEvents();

private:
    QThread workerThread;
    OrgEsaSen2agriPersistenceManagerInterface &persistenceManagerClient;
    OrgEsaSen2agriProcessorsExecutorInterface &executorClient;
    std::map<int, std::unique_ptr<ProcessorHandler>> &handlerMap;

    OrchestratorWorker(const OrchestratorWorker &) = delete;
    OrchestratorWorker &operator=(const OrchestratorWorker &) = delete;

    void DispatchEvent(EventProcessingContext &ctx, const UnprocessedEvent &event) noexcept;

    void ProcessEvent(EventProcessingContext &ctx, const TaskAddedEvent &event);
    void ProcessEvent(EventProcessingContext &ctx, const TaskFinishedEvent &event);
    void ProcessEvent(EventProcessingContext &ctx, const ProductAvailableEvent &event);
    void ProcessEvent(EventProcessingContext &ctx, const JobCancelledEvent &event);
    void ProcessEvent(EventProcessingContext &ctx, const JobPausedEvent &event);
    void ProcessEvent(EventProcessingContext &ctx, const JobResumedEvent &event);
    void ProcessEvent(EventProcessingContext &ctx, const JobSubmittedEvent &event);
    void ProcessEvent(EventProcessingContext &ctx, const StepFailedEvent &event);

    ProcessorHandler &GetHandler(int processorId);
};
