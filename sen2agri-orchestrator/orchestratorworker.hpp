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
#include "processorsexecutor_interface.h"

class OrchestratorWorker : public QObject
{
    Q_OBJECT

public:
    OrchestratorWorker(std::map<int, std::unique_ptr<ProcessorHandler>> &handlerMap,
                       PersistenceManagerDBProvider &persistenceManager,
                       OrgEsaSen2agriProcessorsExecutorInterface &executorClient);

    ProcessorHandler &GetHandler(int processorId);
signals:

public slots:
    void RescanEvents();

private:
    QThread workerThread;
    PersistenceManagerDBProvider &persistenceManager;
    OrgEsaSen2agriProcessorsExecutorInterface &executorClient;
    std::map<int, std::unique_ptr<ProcessorHandler>> &handlerMap;

    OrchestratorWorker(const OrchestratorWorker &) = delete;
    OrchestratorWorker &operator=(const OrchestratorWorker &) = delete;

    void DispatchEvent(EventProcessingContext &ctx, const UnprocessedEvent &event) noexcept;

    void ProcessEvent(EventProcessingContext &ctx, const TaskRunnableEvent &event);
    void ProcessEvent(EventProcessingContext &ctx, const TaskFinishedEvent &event);
    void ProcessEvent(EventProcessingContext &ctx, const ProductAvailableEvent &event);
    void ProcessEvent(EventProcessingContext &ctx, const JobCancelledEvent &event);
    void ProcessEvent(EventProcessingContext &ctx, const JobPausedEvent &event);
    void ProcessEvent(EventProcessingContext &ctx, const JobResumedEvent &event);
    void ProcessEvent(EventProcessingContext &ctx, const JobSubmittedEvent &event);
    void ProcessEvent(EventProcessingContext &ctx, const StepFailedEvent &event);
};
