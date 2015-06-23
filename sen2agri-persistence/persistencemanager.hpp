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
    ConfigurationParameterValueList GetJobConfigurationParameters(int jobId, QString prefix);

    KeyedMessageList UpdateConfigurationParameters(ConfigurationUpdateActionList parameters);
    KeyedMessageList UpdateJobConfigurationParameters(int jobId,
                                                      ConfigurationUpdateActionList parameters);

    ProductToArchiveList GetProductsToArchive();
    void MarkProductsArchived(ArchivedProductList products);

    int SubmitJob(NewJob job);
    int SubmitTask(NewTask task);
    void SubmitSteps(NewStepList steps);

    void MarkStepStarted(int taskId, QString name);
    void MarkStepFinished(int taskId, QString name, ExecutionStatistics statistics);
    void MarkJobFinished(int jobId);

    void InsertTaskFinishedEvent(TaskFinishedEvent event);
    void InsertProductAvailableEvent(ProductAvailableEvent event);
    void InsertJobCancelledEvent(JobCancelledEvent event);
    void InsertJobPausedEvent(JobPausedEvent event);
    void InsertJobResumedEvent(JobResumedEvent event);

    SerializedEventList GetNewEvents();
};
