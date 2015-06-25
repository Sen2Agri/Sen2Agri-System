#pragma once

#include "sqldatabaseraii.hpp"
#include "dbprovider.hpp"
#include "model.hpp"
#include "settings.hpp"
#include "serializedevent.hpp"

class PersistenceManagerDBProvider
{
    DBProvider provider;

    SqlDatabaseRAII getDatabase() const;

    PersistenceManagerDBProvider(const PersistenceManagerDBProvider &) = delete;
    PersistenceManagerDBProvider &operator=(const PersistenceManagerDBProvider &) = delete;

public:
    PersistenceManagerDBProvider(const Settings &settings);

    ConfigurationSet GetConfigurationSet();

    ConfigurationParameterValueList GetConfigurationParameters(const QString &prefix);
    ConfigurationParameterValueList GetJobConfigurationParameters(int jobId, const QString &prefix);

    KeyedMessageList UpdateConfigurationParameters(const ConfigurationUpdateActionList &actions,
                                                   bool isAdmin);
    KeyedMessageList UpdateJobConfigurationParameters(int jobId,
                                                      const ConfigurationUpdateActionList &actions);

    ProductToArchiveList GetProductsToArchive();
    void MarkProductsArchived(const ArchivedProductList &products);

    int SubmitJob(const NewJob &job);
    int SubmitTask(const NewTask &task, const NewStepList &steps);

    void MarkStepPendingStart(int taskId, const QString &name);
    void MarkStepStarted(int taskId, const QString &name);
    void MarkStepFinished(int taskId, const QString &name, const ExecutionStatistics &statistics);
    void MarkStepFailed(int taskId, const QString &name, const ExecutionStatistics &statistics);

    void MarkJobPaused(int jobId);
    void MarkJobCancelled(int jobId);
    void MarkJobFinished(int jobId);
    void MarkJobFailed(int jobId);
    void MarkJobNeedsInput(int jobId);

    void InsertEvent(const SerializedEvent &event);

    UnprocessedEventList GetNewEvents();

    void InsertNodeStatistics(const NodeStatistics &statistics);
};
