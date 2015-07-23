#pragma once

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusMessage>

#include "persistencemanagerdbprovider.hpp"
#include "asyncdbustask.hpp"
#include "model.hpp"
#include "settings.hpp"

class PersistenceManager : public QObject, protected QDBusContext
{
    Q_OBJECT

    PersistenceManagerDBProvider dbProvider;

    template <typename F>
    void RunAsync(F &&f)
    {
        setDelayedReply(true);

        if (auto task = makeAsyncDBusTask(message(), connection(), std::forward<F>(f))) {
            QThreadPool::globalInstance()->start(task);
        } else {
            sendErrorReply(QDBusError::NoMemory);
        }
    }

    bool IsCallerAdmin();

public:
    explicit PersistenceManager(const Settings &settings, QObject *parent = 0);

signals:

public slots:
    ConfigurationSet GetConfigurationSet();

    ConfigurationParameterValueList GetConfigurationParameters(QString prefix);
    JobConfigurationParameterValueList GetJobConfigurationParameters(int jobId, QString prefix);

    KeyedMessageList UpdateConfigurationParameters(ConfigurationUpdateActionList parameters);
    KeyedMessageList UpdateJobConfigurationParameters(int jobId,
                                                      JobConfigurationUpdateActionList parameters);

    ProductToArchiveList GetProductsToArchive();
    void MarkProductsArchived(ArchivedProductList products);

    int SubmitJob(NewJob job);
    int SubmitTask(NewTask task);
    void SubmitSteps(NewStepList steps);

    void MarkStepPendingStart(int taskId, QString name);
    void MarkStepStarted(int taskId, QString name);
    bool MarkStepFinished(int taskId, QString name, ExecutionStatistics statistics);

    void MarkJobPaused(int jobId);
    void MarkJobResumed(int jobId);
    void MarkJobCancelled(int jobId);
    void MarkJobFinished(int jobId);
    void MarkJobFailed(int jobId);
    void MarkJobNeedsInput(int jobId);

    TaskIdList GetJobTasksByStatus(int jobId, ExecutionStatusList statusList);
    JobStepToRunList GetTaskStepsForStart(int taskId);
    JobStepToRunList GetJobStepsForResume(int jobId);

    void InsertTaskAddedEvent(TaskRunnableEvent event);
    void InsertTaskFinishedEvent(TaskFinishedEvent event);
    void InsertProductAvailableEvent(ProductAvailableEvent event);
    void InsertJobCancelledEvent(JobCancelledEvent event);
    void InsertJobPausedEvent(JobPausedEvent event);
    void InsertJobResumedEvent(JobResumedEvent event);
    void InsertJobSubmittedEvent(JobSubmittedEvent event);

    UnprocessedEventList GetNewEvents();
    void MarkEventProcessingStarted(int eventId);
    void MarkEventProcessingComplete(int eventId);

    void InsertNodeStatistics(NodeStatistics statistics);

    int InsertProduct(NewProduct product);

    QString GetDashboardSystemOverviewData();
    QString GetDashboardProcessorStatistics();
    QString GetDashboardProductAvailability(const QDateTime since);
};
