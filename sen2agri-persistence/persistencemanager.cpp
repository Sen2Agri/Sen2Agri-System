#include "persistencemanager.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <set>

#include <math.h>

#include "optional_util.hpp"
#include "json_conversions.hpp"

static QString getConfigurationUpsertJson(const ConfigurationUpdateActionList &actions);
static QString getJobConfigurationUpsertJson(const JobConfigurationUpdateActionList &actions);
static QString getArchivedProductsJson(const ArchivedProductList &products);
static QString getParentTasksJson(const TaskIdList &tasks);
static QString getNewStepsJson(const NewStepList &steps);
static QString getScheduledTasksStatusesJson(const std::vector<ScheduledTaskStatus> &statuses);
static QString getScheduledTaskJson(const ScheduledTask &task);

static QString getExecutionStatusListJson(const ExecutionStatusList &statusList);
static QString getTilesJson(const TileIdList &tilesList);
static QString getIntListJsonJson(const QList<int> &intList);

static void bindStepExecutionStatistics(QSqlQuery &query, const ExecutionStatistics &statistics);

static ConfigurationParameterValueList mapConfigurationParameters(QSqlQuery &query);
static KeyedMessageList mapUpdateConfigurationResult(QSqlQuery &query);

PersistenceManagerDBProvider::PersistenceManagerDBProvider(const Settings &settings)
    : provider(settings)
{
}

SqlDatabaseRAII PersistenceManagerDBProvider::getDatabase() const
{
    return provider.getDatabase(QStringLiteral("PersistenceManager"));
}

void PersistenceManagerDBProvider::TestConnection()
{
    getDatabase();
}

ConfigurationSet PersistenceManagerDBProvider::GetConfigurationSet()
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select * from sp_get_categories()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        auto dataRecord = query.record();
        auto idCol = dataRecord.indexOf(QStringLiteral("id"));
        auto nameCol = dataRecord.indexOf(QStringLiteral("name"));
        auto allowPerSiteCusomizationCol =
            dataRecord.indexOf(QStringLiteral("allow_per_site_customization"));

        ConfigurationSet result;
        while (query.next()) {
            result.categories.append({ query.value(idCol).toInt(), query.value(nameCol).toString(),
                                       query.value(allowPerSiteCusomizationCol).toBool() });
        }

        query = db.prepareQuery(QStringLiteral("select * from sp_get_sites()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        dataRecord = query.record();
        idCol = dataRecord.indexOf(QStringLiteral("id"));
        nameCol = dataRecord.indexOf(QStringLiteral("name"));
        auto shortNameCol = dataRecord.indexOf(QStringLiteral("short_name"));

        while (query.next()) {
            result.sites.append({ query.value(idCol).toInt(), query.value(nameCol).toString(),
                                  query.value(shortNameCol).toString() });
        }

        query = db.prepareQuery(QStringLiteral("select * from sp_get_config_metadata()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        dataRecord = query.record();
        auto keyCol = dataRecord.indexOf(QStringLiteral("key"));
        auto friendlyNameCol = dataRecord.indexOf(QStringLiteral("friendly_name"));
        auto dataTypeCol = dataRecord.indexOf(QStringLiteral("type"));
        auto isAdvancedCol = dataRecord.indexOf(QStringLiteral("is_advanced"));
        auto categoryCol = dataRecord.indexOf(QStringLiteral("config_category_id"));

        while (query.next()) {
            result.parameterInfo.append(
                { query.value(keyCol).toString(),          query.value(categoryCol).toInt(),
                  query.value(friendlyNameCol).toString(), query.value(dataTypeCol).toString(),
                  query.value(isAdvancedCol).toBool() });
        }

        query = db.prepareQuery(QStringLiteral("select * from sp_get_parameters(:prefix)"));
        query.bindValue(QStringLiteral(":prefix"), QString());

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        result.parameterValues = mapConfigurationParameters(query);

        std::set<QString> globalValues;
        for (const auto &value : result.parameterValues) {
            if (!value.siteId) {
                globalValues.emplace(value.key);
            }
        }

        auto endGlobalValues = std::end(globalValues);
        for (const auto &parameter : result.parameterInfo) {
            if (globalValues.find(parameter.key) == endGlobalValues) {
                throw std::runtime_error(QStringLiteral("Missing global value for parameter %1")
                                             .arg(parameter.key)
                                             .toStdString());
            }
        }

        return result;
    });
}

ConfigurationParameterValueList
PersistenceManagerDBProvider::GetConfigurationParameters(const QString &prefix)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select * from sp_get_parameters(:prefix)"));
        query.bindValue(QStringLiteral(":prefix"), prefix);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        return mapConfigurationParameters(query);
    });
}

JobConfigurationParameterValueList
PersistenceManagerDBProvider::GetJobConfigurationParameters(int jobId, const QString &prefix)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_get_job_parameters(:job_id, :prefix)"));
        query.bindValue(QStringLiteral(":job_id"), jobId);
        query.bindValue(QStringLiteral(":prefix"), prefix);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        auto dataRecord = query.record();
        auto keyCol = dataRecord.indexOf(QStringLiteral("key"));
        auto valueCol = dataRecord.indexOf(QStringLiteral("value"));

        JobConfigurationParameterValueList result;
        while (query.next()) {
            result.append({ query.value(keyCol).toString(), query.value(valueCol).toString() });
        }

        return result;
    });
}

KeyedMessageList PersistenceManagerDBProvider::UpdateConfigurationParameters(
    const ConfigurationUpdateActionList &actions, bool isAdmin)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_upsert_parameters(:parameters, :isAdmin)"));

        query.bindValue(QStringLiteral(":parameters"), getConfigurationUpsertJson(actions));
        query.bindValue(QStringLiteral(":isAdmin"), isAdmin);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        return mapUpdateConfigurationResult(query);
    });
}

KeyedMessageList PersistenceManagerDBProvider::UpdateJobConfigurationParameters(
    int jobId, const JobConfigurationUpdateActionList &parameters)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_upsert_job_parameters(:job_id, :parameters)"));

        query.bindValue(QStringLiteral(":job_id"), jobId);
        query.bindValue(QStringLiteral(":parameters"), getJobConfigurationUpsertJson(parameters));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        return mapUpdateConfigurationResult(query);
    });
}

ProductToArchiveList PersistenceManagerDBProvider::GetProductsToArchive()
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select * from sp_get_products_to_archive()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        auto dataRecord = query.record();
        auto productIdCol = dataRecord.indexOf(QStringLiteral("product_id"));
        auto currentPathCol = dataRecord.indexOf(QStringLiteral("current_path"));
        auto archivePathCol = dataRecord.indexOf(QStringLiteral("archive_path"));

        ProductToArchiveList result;
        while (query.next()) {
            result.append({ query.value(productIdCol).toInt(),
                            query.value(currentPathCol).toString(),
                            query.value(archivePathCol).toString() });
        }

        return result;
    });
}

void PersistenceManagerDBProvider::MarkProductsArchived(const ArchivedProductList &products)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select sp_mark_products_archived(:products)"));
        query.bindValue(QStringLiteral(":products"), getArchivedProductsJson(products));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
    });
}

int PersistenceManagerDBProvider::SubmitJob(const NewJob &job)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_submit_job(:name, :description, "
                           ":processorId, :siteId, :startTypeId, :parameters, :configuration)"));
        query.bindValue(QStringLiteral(":name"), job.name);
        query.bindValue(QStringLiteral(":description"), job.description);
        query.bindValue(QStringLiteral(":processorId"), job.processorId);
        query.bindValue(QStringLiteral(":siteId"), job.siteId);
        query.bindValue(QStringLiteral(":startTypeId"), static_cast<int>(job.startType));
        query.bindValue(QStringLiteral(":parameters"), job.parametersJson);
        query.bindValue(QStringLiteral(":configuration"),
                        getJobConfigurationUpsertJson(job.configuration));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        if (!query.next()) {
            throw std::runtime_error("Expecting a return value from sp_submit_job, but none found");
        }

        return query.value(0).toInt();
    });
}

int PersistenceManagerDBProvider::SubmitTask(const NewTask &task)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral(
            "select * from sp_submit_task(:jobId, :module, :parameters, :parentTasks)"));
        query.bindValue(QStringLiteral(":jobId"), task.jobId);
        query.bindValue(QStringLiteral(":module"), task.module);
        query.bindValue(QStringLiteral(":parameters"), task.parametersJson);
        query.bindValue(QStringLiteral(":parentTasks"), getParentTasksJson(task.parentTasks));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        if (!query.next()) {
            throw std::runtime_error(
                "Expecting a return value from sp_submit_task, but none found");
        }

        return query.value(0).toInt();
    });
}

void PersistenceManagerDBProvider::SubmitSteps(const NewStepList &steps)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select sp_submit_steps(:steps)"));
        query.bindValue(QStringLiteral(":steps"), getNewStepsJson(steps));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
    });
}

void PersistenceManagerDBProvider::MarkStepPendingStart(int taskId, const QString &name)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query =
            db.prepareQuery(QStringLiteral("select sp_mark_step_pending_start(:taskId, :name)"));
        query.bindValue(QStringLiteral(":taskId"), taskId);
        query.bindValue(QStringLiteral(":name"), name);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
    });
}

void PersistenceManagerDBProvider::MarkStepStarted(int taskId, const QString &name)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        db.transaction();

        auto query = db.createQuery();

        query.setForwardOnly(true);
        if (!query.exec(QStringLiteral("set transaction isolation level repeatable read"))) {
            throw_query_error(db, query);
        }

        query = db.prepareQuery(QStringLiteral("select sp_mark_step_started(:taskId, :name)"));
        query.bindValue(QStringLiteral(":taskId"), taskId);
        query.bindValue(QStringLiteral(":name"), name);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
        query.finish();

        db.commit();
    });
}

bool PersistenceManagerDBProvider::MarkStepFinished(int taskId,
                                                    const QString &name,
                                                    const ExecutionStatistics &statistics)
{
    static_assert(sizeof(qlonglong) == sizeof(int64_t), "qlonglong must be 64-bit");

    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        db.transaction();

        auto query = db.createQuery();

        query.setForwardOnly(true);
        if (!query.exec(QStringLiteral("set transaction isolation level repeatable read"))) {
            throw_query_error(db, query);
        }

        query = db.prepareQuery(QStringLiteral(
            "select sp_mark_step_finished(:taskId, :name, :node, :exitCode, :userCpuMs, "
            ":systemCpuMs, :durationMs, :maxRssKb, :maxVmSizeKb, :diskReadBytes, "
            ":diskWriteBytes, :stdOutText, :stdErrText)"));
        query.bindValue(QStringLiteral(":taskId"), taskId);
        query.bindValue(QStringLiteral(":name"), name);
        bindStepExecutionStatistics(query, statistics);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        if (!query.next()) {
            throw std::runtime_error(
                "Expecting a return value from sp_mark_step_finished, but none found");
        }

        auto isTaskFinished = query.value(0).toBool();

        query.finish();

        db.commit();

        return isTaskFinished;
    });
}

void PersistenceManagerDBProvider::MarkStepFailed(int taskId,
                                                  const QString &name,
                                                  const ExecutionStatistics &statistics)
{
    static_assert(sizeof(qlonglong) == sizeof(int64_t), "qlonglong must be 64-bit");

    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        db.transaction();

        auto query = db.createQuery();

        query.setForwardOnly(true);
        if (!query.exec(QStringLiteral("set transaction isolation level repeatable read"))) {
            throw_query_error(db, query);
        }

        query = db.prepareQuery(
            QStringLiteral("select sp_mark_step_failed(:taskId, :name, :node, :exitCode, "
                           ":userCpuMs, :systemCpuMs, :durationMs, :maxRssKb, :maxVmSizeKb, "
                           ":diskReadBytes, :diskWriteBytes, :stdOutText, :stdErrText)"));
        query.bindValue(QStringLiteral(":taskId"), taskId);
        query.bindValue(QStringLiteral(":name"), name);
        bindStepExecutionStatistics(query, statistics);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
        query.finish();

        db.commit();
    });
}

void PersistenceManagerDBProvider::MarkJobCancelled(int jobId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        db.transaction();

        auto query = db.createQuery();

        query.setForwardOnly(true);
        if (!query.exec(QStringLiteral("set transaction isolation level repeatable read"))) {
            throw_query_error(db, query);
        }

        query = db.prepareQuery(QStringLiteral("select sp_mark_job_cancelled(:jobId)"));
        query.bindValue(QStringLiteral(":jobId"), jobId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
        query.finish();
        db.commit();
    });
}

void PersistenceManagerDBProvider::MarkJobPaused(int jobId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        db.transaction();

        auto query = db.createQuery();

        query.setForwardOnly(true);
        if (!query.exec(QStringLiteral("set transaction isolation level repeatable read"))) {
            throw_query_error(db, query);
        }

        query = db.prepareQuery(QStringLiteral("select sp_mark_job_paused(:jobId)"));
        query.bindValue(QStringLiteral(":jobId"), jobId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
        query.finish();
        db.commit();
    });
}

void PersistenceManagerDBProvider::MarkJobResumed(int jobId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        db.transaction();

        auto query = db.createQuery();

        query.setForwardOnly(true);
        if (!query.exec(QStringLiteral("set transaction isolation level repeatable read"))) {
            throw_query_error(db, query);
        }

        query = db.prepareQuery(QStringLiteral("select sp_mark_job_resumed(:jobId)"));
        query.bindValue(QStringLiteral(":jobId"), jobId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
        query.finish();
        db.commit();
    });
}

void PersistenceManagerDBProvider::MarkJobFinished(int jobId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select sp_mark_job_finished(:jobId)"));
        query.bindValue(QStringLiteral(":jobId"), jobId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
    });
}

void PersistenceManagerDBProvider::MarkEmptyJobFailed(int jobId, const QString &err) {
    // add also a generic error task and step that contain the error string
    NewTask task{ jobId, "generic-error-task", "{}", {} };
    int taskId = SubmitTask(task);
    // Imediately set the job failed as it sets also the task canceled in order to avoid sending it to executor
    MarkJobFailed(jobId);
    // Add also a step in order to have the error
    NewStepList liststeps = {{taskId, "generic-error-step", "{\"arguments\":[\"/usr/bin/false\"]}"}};
    SubmitSteps(liststeps);
    ExecutionStatistics newStats;
    newStats.node = "local_node";
    newStats.stdErrText = err;
    newStats.stdOutText = err;
    newStats.exitCode = 1;
    MarkStepFinished(taskId, "generic-error-step", newStats);
}

void PersistenceManagerDBProvider::MarkJobFailed(int jobId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select sp_mark_job_failed(:jobId)"));
        query.bindValue(QStringLiteral(":jobId"), jobId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
    });
}

void PersistenceManagerDBProvider::MarkJobNeedsInput(int jobId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select sp_mark_job_needs_input(:jobId)"));
        query.bindValue(QStringLiteral(":jobId"), jobId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
    });
}

UnprocessedEventList PersistenceManagerDBProvider::GetNewEvents()
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select * from sp_get_new_events()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        auto dataRecord = query.record();
        auto eventIdCol = dataRecord.indexOf(QStringLiteral("id"));
        auto eventTypeCol = dataRecord.indexOf(QStringLiteral("type_id"));
        auto eventDataCol = dataRecord.indexOf(QStringLiteral("data"));
        auto submittedCol = dataRecord.indexOf(QStringLiteral("submitted_timestamp"));
        auto processingStartedCol =
            dataRecord.indexOf(QStringLiteral("processing_started_timestamp"));

        UnprocessedEventList result;
        while (query.next()) {
            result.append({ query.value(eventIdCol).toInt(),
                            static_cast<EventType>(query.value(eventTypeCol).toInt()),
                            query.value(eventDataCol).toString(),
                            query.value(submittedCol).toDateTime(),
                            to_optional<QDateTime>(query.value(processingStartedCol)) });
        }

        return result;
    });
}

void PersistenceManagerDBProvider::MarkEventProcessingStarted(int eventId)
{
    auto db = getDatabase();

    provider.handleTransactionRetry(__func__, [&] {
        auto query =
            db.prepareQuery(QStringLiteral("select sp_mark_event_processing_started(:eventId)"));
        query.bindValue(QStringLiteral(":eventId"), eventId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
    });
}

void PersistenceManagerDBProvider::MarkEventProcessingComplete(int eventId)
{
    auto db = getDatabase();

    provider.handleTransactionRetry(__func__, [&] {
        auto query =
            db.prepareQuery(QStringLiteral("select sp_mark_event_processing_completed(:eventId)"));
        query.bindValue(QStringLiteral(":eventId"), eventId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
    });
}

TaskIdList PersistenceManagerDBProvider::GetJobTasksByStatus(int jobId,
                                                             const ExecutionStatusList &statusList)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_get_job_tasks_by_status(:jobId, :statusList)"));
        query.bindValue(QStringLiteral(":jobId"), jobId);
        query.bindValue(QStringLiteral(":statusList"), getExecutionStatusListJson(statusList));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        auto dataRecord = query.record();
        auto taskIdCol = dataRecord.indexOf(QStringLiteral("task_id"));

        TaskIdList result;
        while (query.next()) {
            result.append({ query.value(taskIdCol).toInt() });
        }

        return result;
    });
}

JobStepToRunList PersistenceManagerDBProvider::GetTaskStepsForStart(int taskId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query =
            db.prepareQuery(QStringLiteral("select * from sp_get_task_steps_for_start(:taskId)"));
        query.bindValue(QStringLiteral(":taskId"), taskId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        auto dataRecord = query.record();
        auto taskIdCol = dataRecord.indexOf(QStringLiteral("task_id"));
        auto moduleCol = dataRecord.indexOf(QStringLiteral("module_short_name"));
        auto stepNameCol = dataRecord.indexOf(QStringLiteral("step_name"));
        auto parametersCol = dataRecord.indexOf(QStringLiteral("parameters"));

        JobStepToRunList result;
        while (query.next()) {
            result.append(
                { query.value(taskIdCol).toInt(),      query.value(moduleCol).toString(),
                  query.value(stepNameCol).toString(), query.value(parametersCol).toString() });
        }

        return result;
    });
}

JobStepToRunList PersistenceManagerDBProvider::GetJobStepsForResume(int jobId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query =
            db.prepareQuery(QStringLiteral("select * from sp_get_job_steps_for_resume(:jobId)"));
        query.bindValue(QStringLiteral(":jobId"), jobId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        auto dataRecord = query.record();
        auto taskIdCol = dataRecord.indexOf(QStringLiteral("task_id"));
        auto moduleCol = dataRecord.indexOf(QStringLiteral("module_short_name"));
        auto stepNameCol = dataRecord.indexOf(QStringLiteral("step_name"));
        auto parametersCol = dataRecord.indexOf(QStringLiteral("parameters"));

        JobStepToRunList result;
        while (query.next()) {
            result.append(
                { query.value(taskIdCol).toInt(),      query.value(moduleCol).toString(),
                  query.value(stepNameCol).toString(), query.value(parametersCol).toString() });
        }

        return result;
    });
}

StepConsoleOutputList PersistenceManagerDBProvider::GetTaskConsoleOutputs(int taskId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query =
            db.prepareQuery(QStringLiteral("select * from sp_get_task_console_outputs(:taskId)"));
        query.bindValue(QStringLiteral(":taskId"), taskId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        auto dataRecord = query.record();
        auto taskIdCol = dataRecord.indexOf(QStringLiteral("task_id"));
        auto stepNameCol = dataRecord.indexOf(QStringLiteral("step_name"));
        auto stdOutTextCol = dataRecord.indexOf(QStringLiteral("stdout_text"));
        auto stdErrTextCol = dataRecord.indexOf(QStringLiteral("stderr_text"));

        StepConsoleOutputList result;
        while (query.next()) {
            result.append(
                { query.value(taskIdCol).toInt(),        query.value(stepNameCol).toString(),
                  query.value(stdOutTextCol).toString(), query.value(stdErrTextCol).toString() });
        }

        return result;
    });
}

void PersistenceManagerDBProvider::InsertEvent(const SerializedEvent &event)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query =
            db.prepareQuery(QStringLiteral("select sp_insert_event(:eventType, :eventData)"));
        query.bindValue(QStringLiteral(":eventType"), static_cast<int>(event.type));
        query.bindValue(QStringLiteral(":eventData"), event.data);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
    });
}

void PersistenceManagerDBProvider::InsertNodeStatistics(const NodeStatistics &statistics)
{
    static_assert(sizeof(qlonglong) == sizeof(int64_t), "qlonglong must be 64-bit");

    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select sp_insert_node_statistics(:node, :cpuUser, :cpuSystem, "
                           ":memTotalKb, :memUsedKb, :swapTotalKb, :swapUsedKb, :loadAvg1, "
                           ":loadAvg5, :loadAvg15, :diskTotalBytes, :diskUsedBytes, :timestamp)"));
        query.bindValue(QStringLiteral(":node"), statistics.node);
        query.bindValue(QStringLiteral(":cpuUser"),
                        static_cast<int16_t>(lrint(1000.0 * statistics.cpuUser)));
        query.bindValue(QStringLiteral(":cpuSystem"),
                        static_cast<int16_t>(lrint(1000.0 * statistics.cpuSystem)));
        query.bindValue(QStringLiteral(":memTotalKb"), qlonglong{ statistics.memTotalKb });
        query.bindValue(QStringLiteral(":memUsedKb"), qlonglong{ statistics.memUsedKb });
        query.bindValue(QStringLiteral(":swapTotalKb"), qlonglong{ statistics.swapTotalKb });
        query.bindValue(QStringLiteral(":swapUsedKb"), qlonglong{ statistics.swapUsedKb });
        query.bindValue(QStringLiteral(":loadAvg1"),
                        static_cast<int32_t>(lrint(100.0 * statistics.loadAvg1)));
        query.bindValue(QStringLiteral(":loadAvg5"),
                        static_cast<int32_t>(lrint(100.0 * statistics.loadAvg5)));
        query.bindValue(QStringLiteral(":loadAvg15"),
                        static_cast<int32_t>(lrint(100.0 * statistics.loadAvg15)));
        query.bindValue(QStringLiteral(":diskTotalBytes"), qlonglong{ statistics.diskTotalBytes });
        query.bindValue(QStringLiteral(":diskUsedBytes"), qlonglong{ statistics.diskUsedBytes });
        query.bindValue(QStringLiteral(":timestamp"), QDateTime::currentDateTimeUtc());

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
    });
}

int PersistenceManagerDBProvider::InsertProduct(const NewProduct &product)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_insert_product(:productType, :processorId, "
                           ":satelliteId, :siteId, :jobId, :fullPath, :createdTimestamp, :name, "
                           ":quicklookImage, :footprint, :orbitId, :tiles)"));
        query.bindValue(QStringLiteral(":productType"), static_cast<int>(product.productType));
        query.bindValue(QStringLiteral(":processorId"), product.processorId);
        query.bindValue(QStringLiteral(":satelliteId"), (product.satelliteId<=0) ? QVariant() : product.satelliteId);
        query.bindValue(QStringLiteral(":siteId"), product.siteId);
        query.bindValue(QStringLiteral(":jobId"), product.jobId);
        query.bindValue(QStringLiteral(":fullPath"), product.fullPath);
        query.bindValue(QStringLiteral(":createdTimestamp"), product.createdTimestamp);
        query.bindValue(QStringLiteral(":name"), product.name);
        query.bindValue(QStringLiteral(":quicklookImage"), product.quicklookImage);
        query.bindValue(QStringLiteral(":footprint"), product.footprint);
        if (product.orbitId) {
            query.bindValue(QStringLiteral(":orbitId"), *product.orbitId);
        } else {
            query.bindValue(QStringLiteral(":orbitId"), QVariant());
        }
        query.bindValue(QStringLiteral(":tiles"), getTilesJson(product.tiles));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        if (!query.next()) {
            throw std::runtime_error(
                "Expecting a return value from sp_insert_product, but none found");
        }

        return query.value(0).toInt();
    });
}

ProductList PersistenceManagerDBProvider::GetProducts(int siteId,
                                                      int productTypeId,
                                                      const QDateTime &startDate,
                                                      const QDateTime &endDate)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query =
            db.prepareQuery(QStringLiteral("select * from sp_get_products("
                                           ":siteId, :productTypeId, :startDate, :endDate)"));
        query.bindValue(QStringLiteral(":siteId"), siteId);
        query.bindValue(QStringLiteral(":productTypeId"), productTypeId);
        query.bindValue(QStringLiteral(":startDate"), startDate);
        query.bindValue(QStringLiteral(":endDate"), endDate);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        auto dataRecord = query.record();
        auto productIdCol = dataRecord.indexOf(QStringLiteral("ProductId"));
        auto processorIdCol = dataRecord.indexOf(QStringLiteral("ProcessorId"));
        auto productTypeIdCol = dataRecord.indexOf(QStringLiteral("ProductTypeId"));
        auto siteIdCol = dataRecord.indexOf(QStringLiteral("SiteId"));
        auto fullPathCol = dataRecord.indexOf(QStringLiteral("full_path"));
        auto creationDateCol = dataRecord.indexOf(QStringLiteral("created_timestamp"));
        auto insertionDateCol = dataRecord.indexOf(QStringLiteral("inserted_timestamp"));

        ProductList result;
        while (query.next()) {
            result.append({ query.value(productIdCol).toInt(), query.value(processorIdCol).toInt(),
                            static_cast<ProductType>(query.value(productTypeIdCol).toInt()),
                            query.value(siteIdCol).toInt(), query.value(fullPathCol).toString(),
                            QDateTime().fromString(query.value(creationDateCol).toString(),
                                                   Qt::ISODate),
                            QDateTime().fromString(query.value(insertionDateCol).toString(),
                                                   Qt::ISODate)});
        }

        return result;
    });
}

ProductList PersistenceManagerDBProvider::GetProductsByInsertedTime(int siteId,
                                                      int productTypeId,
                                                      const QDateTime &startDate,
                                                      const QDateTime &endDate)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query =
            db.prepareQuery(QStringLiteral("select * from sp_get_products_by_inserted_time("
                                           ":siteId, :productTypeId, :startDate, :endDate)"));
        query.bindValue(QStringLiteral(":siteId"), siteId);
        query.bindValue(QStringLiteral(":productTypeId"), productTypeId);
        query.bindValue(QStringLiteral(":startDate"), startDate);
        query.bindValue(QStringLiteral(":endDate"), endDate);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        auto dataRecord = query.record();
        auto productIdCol = dataRecord.indexOf(QStringLiteral("ProductId"));
        auto processorIdCol = dataRecord.indexOf(QStringLiteral("ProcessorId"));
        auto productTypeIdCol = dataRecord.indexOf(QStringLiteral("ProductTypeId"));
        auto siteIdCol = dataRecord.indexOf(QStringLiteral("SiteId"));
        auto fullPathCol = dataRecord.indexOf(QStringLiteral("full_path"));
        auto creationDateCol = dataRecord.indexOf(QStringLiteral("created_timestamp"));
        auto insertionDateCol = dataRecord.indexOf(QStringLiteral("inserted_timestamp"));

        ProductList result;
        while (query.next()) {
            result.append({ query.value(productIdCol).toInt(), query.value(processorIdCol).toInt(),
                            static_cast<ProductType>(query.value(productTypeIdCol).toInt()),
                            query.value(siteIdCol).toInt(), query.value(fullPathCol).toString(),
                            QDateTime().fromString(query.value(creationDateCol).toString(),
                                                   Qt::ISODate),
                            QDateTime().fromString(query.value(insertionDateCol).toString(),
                                                   Qt::ISODate) });
        }

        return result;
    });
}

Product PersistenceManagerDBProvider::GetProduct(int productId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(
        __func__, [&] {
            auto query = db.prepareQuery(QStringLiteral("select * from sp_get_product_by_id("
                                                        ":productId)"));
            query.bindValue(QStringLiteral(":productId"), productId);
            query.setForwardOnly(true);
            if (!query.exec()) {
                throw_query_error(db, query);
            }

            auto dataRecord = query.record();
            auto productIdCol = dataRecord.indexOf(QStringLiteral("product_id"));
            auto processorIdCol = dataRecord.indexOf(QStringLiteral("processor_id"));
            auto productTypeIdCol = dataRecord.indexOf(QStringLiteral("product_type_id"));
            auto siteIdCol = dataRecord.indexOf(QStringLiteral("site_id"));
            auto fullPathCol = dataRecord.indexOf(QStringLiteral("full_path"));
            auto creationDateCol = dataRecord.indexOf(QStringLiteral("created_timestamp"));
            auto insertionDateCol = dataRecord.indexOf(QStringLiteral("inserted_timestamp"));

            Product result;
            while (query.next()) {
                result = { query.value(productIdCol).toInt(), query.value(processorIdCol).toInt(),
                           static_cast<ProductType>(query.value(productTypeIdCol).toInt()),
                           query.value(siteIdCol).toInt(), query.value(fullPathCol).toString(),
                           QDateTime().fromString(query.value(creationDateCol).toString(),
                                                  Qt::ISODate),
                           QDateTime().fromString(query.value(insertionDateCol).toString(),
                                                  Qt::ISODate) };
            }

            return result;
        });
}

Product PersistenceManagerDBProvider::GetProduct(int siteId, const QString &productName)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(
        __func__, [&] {
            auto query = db.prepareQuery(QStringLiteral("select * from sp_get_product_by_name("
                                                        ":siteId, :productName)"));
            query.bindValue(QStringLiteral(":siteId"), siteId);
            query.bindValue(QStringLiteral(":productName"), productName);
            query.setForwardOnly(true);
            if (!query.exec()) {
                throw_query_error(db, query);
            }

            auto dataRecord = query.record();
            auto productIdCol = dataRecord.indexOf(QStringLiteral("product_id"));
            auto processorIdCol = dataRecord.indexOf(QStringLiteral("processor_id"));
            auto productTypeIdCol = dataRecord.indexOf(QStringLiteral("product_type_id"));
            auto siteIdCol = dataRecord.indexOf(QStringLiteral("site_id"));
            auto fullPathCol = dataRecord.indexOf(QStringLiteral("full_path"));
            auto creationDateCol = dataRecord.indexOf(QStringLiteral("created_timestamp"));
            auto insertionDateCol = dataRecord.indexOf(QStringLiteral("inserted_timestamp"));

            Product result;
            while (query.next()) {
                result = { query.value(productIdCol).toInt(), query.value(processorIdCol).toInt(),
                           static_cast<ProductType>(query.value(productTypeIdCol).toInt()),
                           query.value(siteIdCol).toInt(), query.value(fullPathCol).toString(),
                           QDateTime().fromString(query.value(creationDateCol).toString(),
                                                  Qt::ISODate),
                           QDateTime().fromString(query.value(insertionDateCol).toString(),
                                                  Qt::ISODate) };
            }

            return result;
        });
}

ProductList PersistenceManagerDBProvider::GetProductsForTile(int siteId, const QString &tileId, ProductType productType,
                                                             int satelliteId, int targetSatelliteId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(
        __func__, [&] {
            auto query = db.prepareQuery(QStringLiteral("select * from sp_get_products_for_tile("
                                                        ":siteId, :tileId, :productType, :satelliteId, :targeSatelliteId)"));
            query.bindValue(QStringLiteral(":siteId"), siteId);
            query.bindValue(QStringLiteral(":tileId"), tileId);
            query.bindValue(QStringLiteral(":productType"), static_cast<int>(productType));
            query.bindValue(QStringLiteral(":satelliteId"), satelliteId);
            query.bindValue(QStringLiteral(":targeSatelliteId"), targetSatelliteId);
            query.setForwardOnly(true);
            if (!query.exec()) {
                throw_query_error(db, query);
            }

            auto dataRecord = query.record();
            auto fullPathCol = dataRecord.indexOf(QStringLiteral("full_path"));
            auto productDateCol = dataRecord.indexOf(QStringLiteral("product_date"));

            ProductList result;
            while (query.next()) {
                Product product;
                product.fullPath = query.value(fullPathCol).toString();
                product.created = query.value(productDateCol).toDateTime();

                result.append(std::move(product));
            }

            return result;
        });
}

L1CProductList PersistenceManagerDBProvider::GetL1CProducts(int siteId,
                                                      const SatellitesList &satelliteIds,
                                                      const StatusesList &statusIds,
                                                      const QDateTime &startDate,
                                                      const QDateTime &endDate)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query =
            db.prepareQuery(QStringLiteral("select * from sp_get_l1c_products("
                                           ":siteId, :satIds, :statusIds, :startDate, :endDate)"));
        query.bindValue(QStringLiteral(":siteId"), siteId);
        query.bindValue(QStringLiteral(":satIds"), getIntListJsonJson(satelliteIds));
        query.bindValue(QStringLiteral(":statusIds"), getIntListJsonJson(statusIds));
        query.bindValue(QStringLiteral(":startDate"), startDate);
        query.bindValue(QStringLiteral(":endDate"), endDate);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        auto dataRecord = query.record();
        auto productIdCol = dataRecord.indexOf(QStringLiteral("ProductId"));
        auto satelliteIdCol = dataRecord.indexOf(QStringLiteral("SatelliteId"));
        auto statusIdCol = dataRecord.indexOf(QStringLiteral("StatusId"));
        auto siteIdCol = dataRecord.indexOf(QStringLiteral("SiteId"));
        auto fullPathCol = dataRecord.indexOf(QStringLiteral("full_path"));
        auto creationDateCol = dataRecord.indexOf(QStringLiteral("product_date"));
        auto insertionDateCol = dataRecord.indexOf(QStringLiteral("created_timestamp"));

        L1CProductList result;
        while (query.next()) {
            result.append({ query.value(productIdCol).toInt(), query.value(satelliteIdCol).toInt(),
                            query.value(statusIdCol).toInt(),
                            query.value(siteIdCol).toInt(), query.value(fullPathCol).toString(),
                            QDateTime().fromString(query.value(creationDateCol).toString(),
                                                   Qt::ISODate),
                            QDateTime().fromString(query.value(insertionDateCol).toString(),
                                                   Qt::ISODate)});
        }

        return result;
    });
}

TileList PersistenceManagerDBProvider::GetSiteTiles(int siteId, int satelliteId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(
        __func__, [&] {
            auto query = db.prepareQuery(QStringLiteral("select * from sp_get_site_tiles("
                                                        ":siteId, :satelliteId)"));
            query.bindValue(QStringLiteral(":siteId"), siteId);
            query.bindValue(QStringLiteral(":satelliteId"), satelliteId);
            query.setForwardOnly(true);
            if (!query.exec()) {
                throw_query_error(db, query);
            }

            auto dataRecord = query.record();
            auto tileIdCol = dataRecord.indexOf(QStringLiteral("tile_id"));

            TileList result;
            while (query.next()) {
                Tile tile;
                tile.satellite = static_cast<Satellite>(satelliteId);
                tile.tileId = query.value(tileIdCol).toString();

                result.append(std::move(tile));
            }

            return result;
        });
}

TileList PersistenceManagerDBProvider::GetIntersectingTiles(Satellite satellite, const QString &tileId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(
        __func__, [&] {
            auto query = db.prepareQuery(QStringLiteral("select * from sp_get_intersecting_tiles("
                                                        ":satelliteId, :tileId)"));
            query.bindValue(QStringLiteral(":satelliteId"), static_cast<int>(satellite));
            query.bindValue(QStringLiteral(":tileId"), tileId);
            query.setForwardOnly(true);
            if (!query.exec()) {
                throw_query_error(db, query);
            }

            auto dataRecord = query.record();
            auto satelliteIdCol = dataRecord.indexOf(QStringLiteral("satellite_id"));
            auto tileIdCol = dataRecord.indexOf(QStringLiteral("tile_id"));

            TileList result;
            while (query.next()) {
                Tile tile;
                tile.satellite = static_cast<Satellite>(query.value(satelliteIdCol).toInt());
                tile.tileId = query.value(tileIdCol).toString();

                result.append(std::move(tile));
            }

            return result;
        });
}

QString PersistenceManagerDBProvider::GetDashboardCurrentJobData(int page)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_get_dashboard_current_job_data(:page)"));
        query.bindValue(QStringLiteral(":page"), page);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        if (!query.next()) {
            throw std::runtime_error(
                "Expecting a return value from sp_get_dashboard_current_job_data, but none found");
        }

        return query.value(0).toString();
    });
}

QString PersistenceManagerDBProvider::GetDashboardServerResourceData()
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_get_dashboard_server_resource_data()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        if (!query.next()) {
            throw std::runtime_error("Expecting a return value from "
                                     "sp_get_dashboard_server_resource_data, but none found");
        }

        return query.value(0).toString();
    });
}

QString PersistenceManagerDBProvider::GetDashboardProcessorStatistics()
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_get_dashboard_processor_statistics()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        if (!query.next()) {
            throw std::runtime_error("Expecting a return value from "
                                     "sp_get_dashboard_processor_statistics, but none found");
        }

        return query.value(0).toString();
    });
}

QString PersistenceManagerDBProvider::GetDashboardProductAvailability(const QDateTime &since)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_get_dashboard_product_availability(:since)"));
        query.bindValue(QStringLiteral(":since"), since);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        if (!query.next()) {
            throw std::runtime_error("Expecting a return value from "
                                     "sp_get_dashboard_product_availability, but none found");
        }

        return query.value(0).toString();
    });
}

QString PersistenceManagerDBProvider::GetDashboardJobTimeline(int jobId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query =
            db.prepareQuery(QStringLiteral("select * from sp_get_dashboard_job_timeline(:jobId)"));
        query.bindValue(QStringLiteral(":jobId"), jobId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        if (!query.next()) {
            throw std::runtime_error("Expecting a return value from "
                                     "sp_get_dashboard_job_timeline, but none found");
        }

        return query.value(0).toString();
    });
}

QString PersistenceManagerDBProvider::GetDashboardProducts(const DashboardSearch &search)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_get_dashboard_products(:siteId, :processorId)"));
        if (search.siteId)
            query.bindValue(QStringLiteral(":siteId"), *search.siteId);
        else
            query.bindValue(QStringLiteral(":siteId"), QVariant());

        if (search.processorId)
            query.bindValue(QStringLiteral(":processorId"), *search.processorId);
        else
            query.bindValue(QStringLiteral(":processorId"), QVariant());

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        if (!query.next()) {
            throw std::runtime_error(
                "Expecting a return value from sp_get_dashboard_products, but none found");
        }

        return query.value(0).toString();
    });
}

QString PersistenceManagerDBProvider::GetDashboardSites()
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select * from sp_get_dashboard_sites()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        if (!query.next()) {
            throw std::runtime_error(
                "Expecting a return value from sp_get_dashboard_sites, but none found");
        }

        return query.value(0).toString();
    });
}

QString PersistenceManagerDBProvider::GetDashboardSentinelTiles(int siteId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_get_dashboard_sentinel_tiles(:siteId)"));
        query.bindValue(QStringLiteral(":siteId"), siteId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        if (!query.next()) {
            throw std::runtime_error(
                "Expecting a return value from sp_get_dashboard_sentinel_tiles, but none found");
        }

        return query.value(0).toString();
    });
}

QString PersistenceManagerDBProvider::GetDashboardLandsatTiles(int siteId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_get_dashboard_landsat_tiles(:siteId)"));
        query.bindValue(QStringLiteral(":siteId"), siteId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        if (!query.next()) {
            throw std::runtime_error(
                "Expecting a return value from sp_get_dashboard_landsat_tiles, but none found");
        }

        return query.value(0).toString();
    });
}

QString PersistenceManagerDBProvider::GetDashboardProcessors()
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select * from sp_get_dashboard_processors()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        if (!query.next()) {
            throw std::runtime_error(
                "Expecting a return value from sp_get_dashboard_processors, but none found");
        }

        return query.value(0).toString();
    });
}

std::vector<ScheduledTask> PersistenceManagerDBProvider::GetScheduledTasks()
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select * from sp_get_scheduled_tasks()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        auto dataRecord = query.record();
        auto taskIdCol = dataRecord.indexOf(QStringLiteral("id"));
        auto nameCol = dataRecord.indexOf(QStringLiteral("name"));
        auto processorIdCol = dataRecord.indexOf(QStringLiteral("processor_id"));
        auto siteIdCol = dataRecord.indexOf(QStringLiteral("site_id"));
        auto seasonIdCol = dataRecord.indexOf(QStringLiteral("season_id"));
        auto processorParamsCol = dataRecord.indexOf(QStringLiteral("processor_params"));

        auto repeatTypeCol = dataRecord.indexOf(QStringLiteral("repeat_type"));
        auto repeatAfterDaysCol = dataRecord.indexOf(QStringLiteral("repeat_after_days"));
        auto repeatOnMonthDayCol = dataRecord.indexOf(QStringLiteral("repeat_on_month_day"));

        auto retrySecondsCol = dataRecord.indexOf(QStringLiteral("retry_seconds"));
        auto priorityCol = dataRecord.indexOf(QStringLiteral("priority"));
        auto firstRunTimeCol = dataRecord.indexOf(QStringLiteral("first_run_time"));

        auto statusIdCol = dataRecord.indexOf(QStringLiteral("status_id"));
        auto nextScheduleCol = dataRecord.indexOf(QStringLiteral("next_schedule"));
        auto lastScheduledRunCol = dataRecord.indexOf(QStringLiteral("last_scheduled_run"));

        auto lastRunTimestampCol = dataRecord.indexOf(QStringLiteral("last_run_timestamp"));
        auto lastRetryTimestampCol = dataRecord.indexOf(QStringLiteral("last_retry_timestamp"));
        auto estimatedNextRunTimeCol =
            dataRecord.indexOf(QStringLiteral("estimated_next_run_time"));

        std::vector<ScheduledTask> taskList;
        while (query.next()) {
            ScheduledTaskStatus ss;
            ss.id = query.value(statusIdCol).toInt();
            ss.taskId = query.value(taskIdCol).toInt();
            ss.nextScheduledRunTime = QDateTime::fromString(query.value(nextScheduleCol).toString(),
                                               Qt::ISODate);
            ss.lastSuccesfullScheduledRun = QDateTime::fromString(query.value(lastScheduledRunCol).toString(),
                                                     Qt::ISODate);
            ss.lastSuccesfullTimestamp = QDateTime::fromString(query.value(lastRunTimestampCol).toString(),
                                                  Qt::ISODate);
            ss.lastRetryTime = QDateTime::fromString(query.value(lastRetryTimestampCol).toString(), Qt::ISODate);
            ss.estimatedRunTime = QDateTime::fromString(query.value(estimatedNextRunTimeCol).toString(),
                                           Qt::ISODate);

            taskList.emplace_back(
                query.value(taskIdCol).toInt(),
                query.value(nameCol).toString(),
                query.value(processorIdCol).toInt(),
                query.value(siteIdCol).toInt(),
                query.value(seasonIdCol).toInt(),
                query.value(processorParamsCol).toString(),
                query.value(repeatTypeCol).toInt(),
                query.value(repeatAfterDaysCol).toInt(),
                query.value(repeatOnMonthDayCol).toInt(),
                QDateTime::fromString(query.value(firstRunTimeCol).toString(), Qt::ISODate),
                query.value(retrySecondsCol).toInt(),
                query.value(priorityCol).toInt(),
                ss);
        }

        return taskList;
    });
}

void
PersistenceManagerDBProvider::UpdateScheduledTasksStatus(std::vector<ScheduledTaskStatus> &statuses)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query =
            db.prepareQuery(QStringLiteral("select sp_submit_scheduled_tasks_statuses(:statuses)"));
        QString retJson = getScheduledTasksStatusesJson(statuses);
        query.bindValue(QStringLiteral(":statuses"), retJson);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
    });
}

void PersistenceManagerDBProvider::InsertScheduledTask(ScheduledTask &task)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select sp_insert_scheduled_task(:task)"));
        query.bindValue(QStringLiteral(":task"), getScheduledTaskJson(task));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
    });
}

static QString getConfigurationUpsertJson(const ConfigurationUpdateActionList &actions)
{
    QJsonArray array;
    for (const auto &p : actions) {
        QJsonObject node;
        node[QStringLiteral("key")] = p.key;
        node[QStringLiteral("site_id")] = p.siteId ? QJsonValue(p.siteId.value()) : QJsonValue();
        node[QStringLiteral("value")] = p.value ? QJsonValue(p.value.value()) : QJsonValue();
        array.append(std::move(node));
    }

    return jsonToString(array);
}

static QString getJobConfigurationUpsertJson(const JobConfigurationUpdateActionList &actions)
{
    QJsonArray array;
    for (const auto &p : actions) {
        QJsonObject node;
        node[QStringLiteral("key")] = p.key;
        node[QStringLiteral("value")] = p.value;
        array.append(std::move(node));
    }

    return jsonToString(array);
}

static QString getArchivedProductsJson(const ArchivedProductList &products)
{
    QJsonArray array;
    for (const auto &p : products) {
        QJsonObject node;
        node[QStringLiteral("product_id")] = p.productId;
        node[QStringLiteral("archive_path")] = p.archivePath;
        array.append(std::move(node));
    }

    return jsonToString(array);
}

static QString getParentTasksJson(const TaskIdList &tasks)
{
    QJsonArray array;
    for (const auto &task : tasks) {
        array.append(task);
    }

    return jsonToString(array);
}

static QString getNewStepsJson(const NewStepList &steps)
{
    QJsonArray array;
    for (const auto &s : steps) {
        QJsonObject node;
        node[QStringLiteral("task_id")] = s.taskId;
        node[QStringLiteral("name")] = s.name;
        node[QStringLiteral("parameters")] =
            QJsonDocument::fromJson(s.parametersJson.toUtf8()).object();
        array.append(std::move(node));
    }

    return jsonToString(array);
}

static QString getScheduledTasksStatusesJson(const std::vector<ScheduledTaskStatus> &statuses)
{
    QJsonArray array;
    for (const auto &s : statuses) {
        QJsonObject node;
        node[QStringLiteral("id")] = s.id;
        node[QStringLiteral("next_schedule")] = s.nextScheduledRunTime.toString(Qt::ISODate);
        node[QStringLiteral("last_scheduled_run")] =
            s.lastSuccesfullScheduledRun.toString(Qt::ISODate);
        node[QStringLiteral("last_run_timestamp")] =
            s.lastSuccesfullTimestamp.toString(Qt::ISODate);
        node[QStringLiteral("last_retry_timestamp")] = s.lastRetryTime.toString(Qt::ISODate);
        node[QStringLiteral("estimated_next_run_time")] = s.estimatedRunTime.toString(Qt::ISODate);

        array.append(std::move(node));
    }

    return jsonToString(array);
}

static QString getScheduledTaskJson(const ScheduledTask &task)
{
    QJsonObject node;
    node[QStringLiteral("id")] = task.taskId;
    node[QStringLiteral("name")] = task.taskName;
    node[QStringLiteral("processor_id")] = task.processorId;
    node[QStringLiteral("site_id")] = task.siteId;
    node[QStringLiteral("processor_params")] = task.processorParameters;
    node[QStringLiteral("repeat_type")] = task.repeatType;
    node[QStringLiteral("repeat_after_days")] = task.repeatAfterDays;
    node[QStringLiteral("repeat_on_month_day")] = task.repeatOnMonthDay;
    node[QStringLiteral("first_run_time")] = task.firstScheduledRunTime.toString(Qt::ISODate);
    node[QStringLiteral("retry_seconds")] = task.retryPeriod;
    node[QStringLiteral("priority")] = task.taskPriority;

    return jsonToString(node);
}

static QString getExecutionStatusListJson(const ExecutionStatusList &statusList)
{
    QJsonArray array;
    for (const auto &s : statusList) {
        array.append(static_cast<int>(s));
    }

    return jsonToString(array);
}

static QString getTilesJson(const TileIdList &tilesList)
{
    QJsonArray array;
    for (const auto &t : tilesList) {
        array.append(t);
    }

    return jsonToString(array);
}

static QString getIntListJsonJson(const QList<int> &intList)
{
    QJsonArray array;
    for (const auto &t : intList) {
        array.append(t);
    }

    return jsonToString(array);
}

static void bindStepExecutionStatistics(QSqlQuery &query, const ExecutionStatistics &statistics)
{
    query.bindValue(QStringLiteral(":node"), statistics.node);
    query.bindValue(QStringLiteral(":exitCode"), statistics.exitCode);
    query.bindValue(QStringLiteral(":userCpuMs"), qlonglong{ statistics.userCpuMs });
    query.bindValue(QStringLiteral(":systemCpuMs"), qlonglong{ statistics.systemCpuMs });
    query.bindValue(QStringLiteral(":durationMs"), qlonglong{ statistics.durationMs });
    query.bindValue(QStringLiteral(":maxRssKb"), statistics.maxRssKb);
    query.bindValue(QStringLiteral(":maxVmSizeKb"), statistics.maxVmSizeKb);
    query.bindValue(QStringLiteral(":diskReadBytes"), qlonglong{ statistics.diskReadBytes });
    query.bindValue(QStringLiteral(":diskWriteBytes"), qlonglong{ statistics.diskWriteBytes });
    query.bindValue(QStringLiteral(":stdOutText"), statistics.stdOutText);
    query.bindValue(QStringLiteral(":stdErrText"), statistics.stdErrText);
}

static ConfigurationParameterValueList mapConfigurationParameters(QSqlQuery &query)
{
    auto dataRecord = query.record();
    auto keyCol = dataRecord.indexOf(QStringLiteral("key"));
    auto siteIdCol = dataRecord.indexOf(QStringLiteral("site_id"));
    auto valueCol = dataRecord.indexOf(QStringLiteral("value"));

    ConfigurationParameterValueList result;
    while (query.next()) {
        result.append({ query.value(keyCol).toString(), to_optional<int>(query.value(siteIdCol)),
                        query.value(valueCol).toString() });
    }

    return result;
}

static KeyedMessageList mapUpdateConfigurationResult(QSqlQuery &query)
{
    auto dataRecord = query.record();
    auto keyCol = dataRecord.indexOf(QStringLiteral("key"));
    auto errorMessageCol = dataRecord.indexOf(QStringLiteral("error_message"));

    KeyedMessageList result;
    while (query.next()) {
        result.append({ query.value(keyCol).toString(), query.value(errorMessageCol).toString() });
    }

    return result;
}

ProcessorDescriptionList PersistenceManagerDBProvider::GetProcessorDescriptions()
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        ProcessorDescriptionList result;
        auto query = db.prepareQuery(QStringLiteral("select * from sp_get_processors()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        auto dataRecord = query.record();
        auto idCol = dataRecord.indexOf(QStringLiteral("id"));
        auto shortNameCol = dataRecord.indexOf(QStringLiteral("short_name"));
        auto nameCol = dataRecord.indexOf(QStringLiteral("name"));

        while (query.next()) {
            result.append({ query.value(idCol).toInt(), query.value(shortNameCol).toString(),
                            query.value(nameCol).toString() });
        }

        return result;
    });
}

SiteList PersistenceManagerDBProvider::GetSiteDescriptions()
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        SiteList result;
        auto query = db.prepareQuery(QStringLiteral("select * from sp_get_sites()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        auto dataRecord = query.record();
        auto idCol = dataRecord.indexOf(QStringLiteral("id"));
        auto nameCol = dataRecord.indexOf(QStringLiteral("name"));
        auto shortNameCol = dataRecord.indexOf(QStringLiteral("short_name"));

        while (query.next()) {
            result.append({ query.value(idCol).toInt(), query.value(nameCol).toString(),
                            query.value(shortNameCol).toString() });
        }
        return result;
    });
}

SeasonList PersistenceManagerDBProvider::GetSiteSeasons(int siteId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        SeasonList result;

        auto query = db.prepareQuery(QStringLiteral("select * from sp_get_site_seasons(:siteId)"));
        query.bindValue(QStringLiteral(":siteId"), siteId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }

        auto dataRecord = query.record();
        auto idCol = dataRecord.indexOf(QStringLiteral("id"));
        auto siteIdCol = dataRecord.indexOf(QStringLiteral("site_id"));
        auto nameCol = dataRecord.indexOf(QStringLiteral("name"));
        auto startDateCol = dataRecord.indexOf(QStringLiteral("start_date"));
        auto endDateCol = dataRecord.indexOf(QStringLiteral("end_date"));
        auto midDateCol = dataRecord.indexOf(QStringLiteral("mid_date"));
        auto enabledCol = dataRecord.indexOf(QStringLiteral("enabled"));

        while (query.next()) {
            result.append({ query.value(idCol).toInt(),
                            query.value(siteIdCol).toInt(),
                            query.value(nameCol).toString(),
                            query.value(startDateCol).toDate(),
                            query.value(endDateCol).toDate(),
                            query.value(midDateCol).toDate(),
                            query.value(enabledCol).toBool() });
        }

        return result;
    });
}

QString PersistenceManagerDBProvider::GetProcessorShortName(int processorId)
{
    ProcessorDescriptionList listProcDescr = GetProcessorDescriptions();
    for (const ProcessorDescription &procDescr : listProcDescr) {
        if (procDescr.processorId == processorId) {
            return procDescr.shortName;
        }
    }
    return "";
}

QString PersistenceManagerDBProvider::GetSiteName(int siteId)
{
    SiteList listSiteDescr = GetSiteDescriptions();
    for (const Site &siteDescr : listSiteDescr) {
        if (siteDescr.siteId == siteId) {
            return siteDescr.name;
        }
    }
    return "";
}

QString PersistenceManagerDBProvider::GetSiteShortName(int siteId)
{
    SiteList listSiteDescr = GetSiteDescriptions();
    for (const Site &siteDescr : listSiteDescr) {
        if (siteDescr.siteId == siteId) {
            return siteDescr.shortName;
        }
    }
    return "";
}
