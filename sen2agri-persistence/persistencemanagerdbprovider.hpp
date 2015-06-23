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

    void InsertEvent(const SerializedEvent &event);

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
    int SubmitTask(const NewTask &task);
    void SubmitSteps(const NewStepList &steps);

    void MarkStepStarted(int taskId, const QString &name);
    void MarkStepFinished(int taskId, const QString &name, const ExecutionStatistics &statistics);
    void MarkJobFinished(int jobId);

    void InsertTaskFinishedEvent(const TaskFinishedEvent &event);
    void InsertProductAvailableEvent(const ProductAvailableEvent &event);
    void InsertJobCancelledEvent(const JobCancelledEvent &event);
    void InsertJobPausedEvent(const JobPausedEvent &event);
    void InsertJobResumedEvent(const JobResumedEvent &event);

    UnprocessedEventList GetNewEvents();

    void InsertNodeStatistics(const NodeStatistics &statistics);
};
