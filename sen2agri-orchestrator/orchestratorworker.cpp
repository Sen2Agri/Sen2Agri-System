#include <map>

#include <QDebug>
#include <QDBusPendingCallWatcher>

#include "logger.hpp"
#include "orchestratorworker.hpp"
#include "dbus_future_utils.hpp"

class ProcessorHandler
{
public:
    void HandleJobSubmitted(EventProcessingContext &ctx, const JobSubmittedEvent &event);
    void HandleTaskFinished(EventProcessingContext &ctx, const TaskFinishedEvent &event);

private:
    virtual ~ProcessorHandler();

    virtual void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event) = 0;
    virtual void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                        const TaskFinishedEvent &event) = 0;
};

void ProcessorHandler::HandleJobSubmitted(EventProcessingContext &ctx,
                                          const JobSubmittedEvent &event)
{
    HandleJobSubmittedImpl(ctx, event);
}

void ProcessorHandler::HandleTaskFinished(EventProcessingContext &ctx,
                                          const TaskFinishedEvent &event)
{
    HandleTaskFinishedImpl(ctx, event);
}

static std::map<QString, QString>
GetModulePathMap(const ConfigurationParameterValueList &parameters)
{
    std::map<QString, QString> modulePaths;
    for (const auto &p : parameters) {
        if (p.key.endsWith(QLatin1String(".path"))) {
            auto firstPath = p.key.indexOf('.', p.key.indexOf('.') + 1) + 1;
            auto lastDot = p.key.length() - 5;
            const auto &path = p.key.mid(firstPath, lastDot - firstPath);

            modulePaths.emplace(path, p.value);
        }
    }

    return modulePaths;
}

OrchestratorWorker::OrchestratorWorker(
    OrgEsaSen2agriPersistenceManagerInterface &persistenceManagerClient,
    OrgEsaSen2agriProcessorsExecutorInterface &executorClient)
    : persistenceManagerClient(persistenceManagerClient), executorClient(executorClient)
{
    moveToThread(&workerThread);
    workerThread.start();
}

void OrchestratorWorker::RescanEvents()
{
    if (QThread::currentThread() != &workerThread) {
        QMetaObject::invokeMethod(this, "RescanEvents", Qt::QueuedConnection);
        return;
    }

    try {
        bool rescanRequested;
        do {
            EventProcessingContext ctx(persistenceManagerClient);

            const auto &events = ctx.GetNewEvents();
            for (const auto &event : events) {
                DispatchEvent(ctx, event);
            }

            rescanRequested = ctx.IsRescanRequested();
        } while (rescanRequested);
    } catch (const std::exception &e) {
        Logger::error(QStringLiteral("Unable to retrieve new events: %1").arg(e.what()));
    }
}

void OrchestratorWorker::DispatchEvent(EventProcessingContext &ctx,
                                       const UnprocessedEvent &event) noexcept
{
    try {
        ctx.MarkEventProcessingStarted(event.eventId);
    } catch (const std::exception &e) {
        Logger::error(QStringLiteral("Unable to mark event id %1 as started: ").arg(e.what()));
        return;
    }

    try {
        EventProcessingContext innerCtx(persistenceManagerClient);

        switch (event.type) {
            case EventType::TaskFinished:
                ProcessEvent(innerCtx, TaskFinishedEvent::fromJson(event.data));
                break;
            case EventType::ProductAvailable:
                ProcessEvent(innerCtx, ProductAvailableEvent::fromJson(event.data));
                break;
            case EventType::JobCancelled:
                ProcessEvent(innerCtx, JobCancelledEvent::fromJson(event.data));
                break;
            case EventType::JobPaused:
                ProcessEvent(innerCtx, JobPausedEvent::fromJson(event.data));
                break;
            case EventType::JobResumed:
                ProcessEvent(innerCtx, JobResumedEvent::fromJson(event.data));
                break;
            case EventType::JobSubmitted:
                ProcessEvent(innerCtx, JobSubmittedEvent::fromJson(event.data));
                break;
            case EventType::StepFailed:
                ProcessEvent(innerCtx, StepFailedEvent::fromJson(event.data));
                break;
            default:
                throw std::runtime_error(
                    QStringLiteral("Unknown event type %1 for event id %2 with data %3")
                        .arg(static_cast<int>(event.type), event.eventId)
                        .arg(QString::fromUtf8(event.data.toJson()))
                        .toStdString());
        }

        if (innerCtx.IsRescanRequested()) {
            ctx.ScheduleRescan();
        }
    } catch (const std::exception &e) {
        Logger::error(QStringLiteral("Unable to process event id %1: ").arg(e.what()));
    }

    try {
        ctx.MarkEventProcessingComplete(event.eventId);
    } catch (const std::exception &e) {
        Logger::error(QStringLiteral("Unable to mark event id %1 as complete: ").arg(e.what()));
    }
}

void OrchestratorWorker::ProcessEvent(EventProcessingContext &ctx, const TaskFinishedEvent &event)
{
    Q_UNUSED(event);

    // 1. using the task module, find the relevant processor/task handler and run it
    // 2. the handler can choose to submit a new task and its steps
    // 3. if a task was submitted, send its steps to the executor
    // 4. otherwise submit a product available event to self

    // 5. pend an event queue rescan
    ctx.ScheduleRescan();
}

void OrchestratorWorker::ProcessEvent(EventProcessingContext &ctx,
                                      const ProductAvailableEvent &event)
{
    Q_UNUSED(event);

    // 1. determine what jobs need to be submitted (???)
    // 2. submit the new jobs to the database
    // 3. submit job sumbitted events to self

    // 4. pend an event queue rescan
    ctx.ScheduleRescan();
}

void OrchestratorWorker::ProcessEvent(EventProcessingContext &ctx, const JobCancelledEvent &event)
{
    const auto &tasks = ctx.GetJobTasksByStatus(
        event.jobId,
        { ExecutionStatus::Submitted, ExecutionStatus::Running, ExecutionStatus::Paused });

    WaitForResponseAndThrow(executorClient.CancelTasks(tasks));
    ctx.MarkJobCancelled(event.jobId);
}

void OrchestratorWorker::ProcessEvent(EventProcessingContext &ctx, const JobPausedEvent &event)
{
    const auto &tasks = ctx.GetJobTasksByStatus(
        event.jobId, { ExecutionStatus::Submitted, ExecutionStatus::Running });

    WaitForResponseAndThrow(executorClient.CancelTasks(tasks));
    ctx.MarkJobPaused(event.jobId);
}

void OrchestratorWorker::ProcessEvent(EventProcessingContext &ctx, const JobResumedEvent &event)
{
    const auto &stepsPromise = persistenceManagerClient.GetJobStepsForResume(event.jobId);

    const auto &modulePaths = GetModulePathMap(
        ctx.GetJobConfigurationParameters(event.jobId, QStringLiteral("executor.processor.")));

    const auto &steps = WaitForResponseAndThrow(stepsPromise);

    NewExecutorStepList stepsToSubmit;
    stepsToSubmit.reserve(steps.size());
    auto modulePathsEnd = std::end(modulePaths);
    for (const auto &s : steps) {
        auto it = modulePaths.find(s.module);
        if (it == modulePathsEnd) {
            throw std::runtime_error(QStringLiteral("Cannot find executable path for module %1")
                                         .arg(s.module)
                                         .toStdString());
        }

        if (!s.parameters.isObject()) {
            throw std::runtime_error(
                QStringLiteral("Unexpected step parameter JSON schema: root node should be an "
                               "object. The parameters object  was: '%1'")
                    .arg(QString::fromUtf8(s.parameters.toJson()))
                    .toStdString());
        }

        const auto &argNode = s.parameters.object()[QStringLiteral("arguments")];
        if (!argNode.isArray()) {
            throw std::runtime_error(
                QStringLiteral(
                    "Unexpected step parameter JSON schema: node 'arguments' should be an "
                    "array. The parameters object  was: '%1'")
                    .arg(QString::fromUtf8(s.parameters.toJson()))
                    .toStdString());
        }
        const auto &argArray = argNode.toArray();

        StepArgumentList arguments;
        arguments.reserve(argArray.count());
        for (const auto &arg : argArray) {
            if (!arg.isString()) {
                throw std::runtime_error(
                    QStringLiteral("Unexpected step parameter JSON schema: arguments should be "
                                   "strings. The parameters object was: '%1'")
                        .arg(QString::fromUtf8(s.parameters.toJson()))
                        .toStdString());
            }

            arguments.append(arg.toString());
        }

        stepsToSubmit.append({ s.taskId, it->second, s.stepName, arguments });
    }

    ctx.MarkJobResumed(event.jobId);
    WaitForResponseAndThrow(executorClient.SubmitSteps(stepsToSubmit));
}

void OrchestratorWorker::ProcessEvent(EventProcessingContext &, const JobSubmittedEvent &event)
{
    Q_UNUSED(event);

    // 1. get job information from db
    // 2. find the relevant processor and run it
    // 3. submit the returned task and step list to the database
    // 4. submit the steps to the executor
}

void OrchestratorWorker::ProcessEvent(EventProcessingContext &ctx, const StepFailedEvent &event)
{
    const auto &tasks = ctx.GetJobTasksByStatus(event.jobId, { ExecutionStatus::Running });

    WaitForResponseAndThrow(executorClient.CancelTasks(tasks));
    ctx.MarkJobFailed(event.jobId);
}
