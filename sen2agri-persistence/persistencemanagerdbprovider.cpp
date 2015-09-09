#include "persistencemanagerdbprovider.hpp"

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
static QString getExecutionStatusListJson(const ExecutionStatusList &statusList);

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

void PersistenceManagerDBProvider::TestConnection() { getDatabase(); }

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
            result.categories.append({ query.value(idCol).toInt(),
                                       query.value(nameCol).toString(),
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

        while (query.next()) {
            result.sites.append({ query.value(idCol).toInt(), query.value(nameCol).toString() });
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
            result.parameterInfo.append({ query.value(keyCol).toString(),
                                          query.value(categoryCol).toInt(),
                                          query.value(friendlyNameCol).toString(),
                                          query.value(dataTypeCol).toString(),
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
        auto query = db.prepareQuery(QStringLiteral("select sp_mark_job_cancelled(:jobId)"));
        query.bindValue(QStringLiteral(":jobId"), jobId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
    });
}

void PersistenceManagerDBProvider::MarkJobPaused(int jobId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select sp_mark_job_paused(:jobId)"));
        query.bindValue(QStringLiteral(":jobId"), jobId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
    });
}

void PersistenceManagerDBProvider::MarkJobResumed(int jobId)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select sp_mark_job_resumed(:jobId)"));
        query.bindValue(QStringLiteral(":jobId"), jobId);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(db, query);
        }
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
            result.append({ query.value(taskIdCol).toInt(),
                            query.value(moduleCol).toString(),
                            query.value(stepNameCol).toString(),
                            query.value(parametersCol).toString() });
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
            result.append({ query.value(taskIdCol).toInt(),
                            query.value(moduleCol).toString(),
                            query.value(stepNameCol).toString(),
                            query.value(parametersCol).toString() });
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
            result.append({ query.value(taskIdCol).toInt(),
                            query.value(stepNameCol).toString(),
                            query.value(stdOutTextCol).toString(),
                            query.value(stdErrTextCol).toString() });
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
                        static_cast<int16_t>(lrint(100.0 * statistics.loadAvg1)));
        query.bindValue(QStringLiteral(":loadAvg5"),
                        static_cast<int16_t>(lrint(100.0 * statistics.loadAvg5)));
        query.bindValue(QStringLiteral(":loadAvg15"),
                        static_cast<int16_t>(lrint(100.0 * statistics.loadAvg15)));
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
        auto query =
            db.prepareQuery(QStringLiteral("select * from sp_insert_product(:productType, "
                                           ":processorId, :taskId, :fullPath, :createdTimestamp)"));
        query.bindValue(QStringLiteral(":productType"), static_cast<int>(product.productType));
        query.bindValue(QStringLiteral(":processorId"), product.processorId);
        query.bindValue(QStringLiteral(":taskId"), product.taskId);
        query.bindValue(QStringLiteral(":fullPath"), product.fullPath);
        query.bindValue(QStringLiteral(":createdTimestamp"), product.createdTimestamp);

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

QString PersistenceManagerDBProvider::GetDashboardCurrentJobData()
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query =
            db.prepareQuery(QStringLiteral("select * from sp_get_dashboard_current_job_data()"));

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

static QString getExecutionStatusListJson(const ExecutionStatusList &statusList)
{
    QJsonArray array;
    for (const auto &s : statusList) {
        array.append(static_cast<int>(s));
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
        result.append({ query.value(keyCol).toString(),
                        to_optional<int>(query.value(siteIdCol)),
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
