#include <map>

#include <QDebug>
#include <QDBusPendingCallWatcher>

#include "logger.hpp"
#include "orchestratorworker.hpp"
#include "dbus_future_utils.hpp"

static std::map<QString, QString>
getModulePathMap(const JobConfigurationParameterValueList &parameters);
static StepArgumentList getStepArguments(const JobStepToRun &step);
static NewExecutorStepList
getExecutorStepList(EventProcessingContext &ctx, int jobId, const JobStepToRunList &steps);

OrchestratorWorker::OrchestratorWorker(
    std::map<int, std::unique_ptr<ProcessorHandler>> &handlerMap,
    OrgEsaSen2agriPersistenceManagerInterface &persistenceManagerClient,
    OrgEsaSen2agriProcessorsExecutorInterface &executorClient)
    : persistenceManagerClient(persistenceManagerClient),
      executorClient(executorClient),
      handlerMap(handlerMap)
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
        while (true) {
            EventProcessingContext ctx(persistenceManagerClient);

            const auto &events = ctx.GetNewEvents();
            if (events.empty()) {
                break;
            }

            for (const auto &event : events) {
                DispatchEvent(ctx, event);
            }
        }
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
            case EventType::TaskAdded:
                ProcessEvent(innerCtx, TaskAddedEvent::fromJson(event.dataJson));
                break;
            case EventType::TaskFinished:
                ProcessEvent(innerCtx, TaskFinishedEvent::fromJson(event.dataJson));
                break;
            case EventType::ProductAvailable:
                ProcessEvent(innerCtx, ProductAvailableEvent::fromJson(event.dataJson));
                break;
            case EventType::JobCancelled:
                ProcessEvent(innerCtx, JobCancelledEvent::fromJson(event.dataJson));
                break;
            case EventType::JobPaused:
                ProcessEvent(innerCtx, JobPausedEvent::fromJson(event.dataJson));
                break;
            case EventType::JobResumed:
                ProcessEvent(innerCtx, JobResumedEvent::fromJson(event.dataJson));
                break;
            case EventType::JobSubmitted:
                ProcessEvent(innerCtx, JobSubmittedEvent::fromJson(event.dataJson));
                break;
            case EventType::StepFailed:
                ProcessEvent(innerCtx, StepFailedEvent::fromJson(event.dataJson));
                break;
            default:
                throw std::runtime_error(
                    QStringLiteral("Unknown event type %1 for event id %2 with data %3")
                        .arg(static_cast<int>(event.type), event.eventId)
                        .arg(event.dataJson)
                        .toStdString());
        }
    } catch (const std::exception &e) {
        Logger::error(
            QStringLiteral("Unable to process event id %1: %2").arg(event.eventId).arg(e.what()));
    }

    try {
        ctx.MarkEventProcessingComplete(event.eventId);
    } catch (const std::exception &e) {
        Logger::error(QStringLiteral("Unable to mark event id %1 as complete: %2")
                          .arg(event.eventId)
                          .arg(e.what()));
    }
}

void OrchestratorWorker::ProcessEvent(EventProcessingContext &ctx, const TaskAddedEvent &event)
{
    const auto &steps =
        WaitForResponseAndThrow(persistenceManagerClient.GetTaskStepsForStart(event.taskId));

    const auto &stepsToSubmit = getExecutorStepList(ctx, event.jobId, steps);

    WaitForResponseAndThrow(executorClient.SubmitSteps(stepsToSubmit));
}

void OrchestratorWorker::ProcessEvent(EventProcessingContext &ctx, const TaskFinishedEvent &event)
{
    GetHandler(event.processorId).HandleTaskFinished(ctx, event);
}

void OrchestratorWorker::ProcessEvent(EventProcessingContext &ctx,
                                      const ProductAvailableEvent &event)
{
    for (auto &&handler : handlerMap) {
        handler.second->HandleProductAvailable(ctx, event);
    }
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
    const auto &steps =
        WaitForResponseAndThrow(persistenceManagerClient.GetJobStepsForResume(event.jobId));

    const auto &stepsToSubmit = getExecutorStepList(ctx, event.jobId, steps);

    WaitForResponseAndThrow(executorClient.SubmitSteps(stepsToSubmit));
    ctx.MarkJobResumed(event.jobId);
}

void OrchestratorWorker::ProcessEvent(EventProcessingContext &ctx, const JobSubmittedEvent &event)
{
    GetHandler(event.processorId).HandleJobSubmitted(ctx, event);
}

void OrchestratorWorker::ProcessEvent(EventProcessingContext &ctx, const StepFailedEvent &event)
{
    const auto &tasks = ctx.GetJobTasksByStatus(event.jobId, { ExecutionStatus::Running });

    WaitForResponseAndThrow(executorClient.CancelTasks(tasks));
    ctx.MarkJobFailed(event.jobId);
}

ProcessorHandler &OrchestratorWorker::GetHandler(int processorId)
{
    auto it = handlerMap.find(processorId);
    if (it == std::end(handlerMap)) {
        throw std::runtime_error(QStringLiteral("No handler present for processor id %1")
                                     .arg(processorId)
                                     .toStdString());
    }

    return *it->second;
}

static std::map<QString, QString>
getModulePathMap(const JobConfigurationParameterValueList &parameters)
{
    std::map<QString, QString> modulePaths;
    for (const auto &p : parameters) {
        modulePaths.emplace(p.key.mid(p.key.lastIndexOf('.') + 1), p.value);
    }

    return modulePaths;
}

static StepArgumentList getStepArguments(const JobStepToRun &step)
{
    const auto &parametersDoc = QJsonDocument::fromJson(step.parametersJson.toUtf8());
    if (!parametersDoc.isObject()) {
        throw std::runtime_error(
            QStringLiteral("Unexpected step parameter JSON schema: root node should be an "
                           "object. The parameter JSON was: '%1'")
                .arg(step.parametersJson)
                .toStdString());
    }

    const auto &argNode = parametersDoc.object()[QStringLiteral("arguments")];
    if (!argNode.isArray()) {
        throw std::runtime_error(
            QStringLiteral("Unexpected step parameter JSON schema: node 'arguments' should be an "
                           "array. The parameter JSON was: '%1'")
                .arg(step.parametersJson)
                .toStdString());
    }
    const auto &argArray = argNode.toArray();

    StepArgumentList arguments;
    arguments.reserve(argArray.count());
    for (const auto &arg : argArray) {
        if (!arg.isString()) {
            throw std::runtime_error(
                QStringLiteral("Unexpected step parameter JSON schema: arguments should be "
                               "strings. The parameter JSON object was: '%1'")
                    .arg(step.parametersJson)
                    .toStdString());
        }

        arguments.append(arg.toString());
    }

    return arguments;
}

static NewExecutorStepList
getExecutorStepList(EventProcessingContext &ctx, int jobId, const JobStepToRunList &steps)
{
    const auto &modulePaths = getModulePathMap(
        ctx.GetJobConfigurationParameters(jobId, QStringLiteral("executor.module.path.")));

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

        const auto &arguments = getStepArguments(s);
        stepsToSubmit.append({ s.taskId, it->second, s.stepName, arguments });
    }

    return stepsToSubmit;
}
