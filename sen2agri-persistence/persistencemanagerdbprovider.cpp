#include "persistencemanagerdbprovider.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <set>

#include <optional_util.hpp>

static QString toJsonString(const QJsonDocument &document);
// static QString toJsonString(const QJsonObject &document);
static QString toJsonString(const QJsonArray &document);

static QString getConfigurationUpsertJson(const ConfigurationUpdateActionList &actions);
static QString getArchivedProductsJson(const ArchivedProductList &products);
static QString getNewStepsJson(const NewStepList &steps);

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

ConfigurationSet PersistenceManagerDBProvider::GetConfigurationSet()
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select * from sp_get_categories()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
        }

        auto dataRecord = query.record();
        auto idCol = dataRecord.indexOf(QStringLiteral("id"));
        auto nameCol = dataRecord.indexOf(QStringLiteral("name"));

        ConfigurationSet result;
        while (query.next()) {
            result.categories.append(
                { query.value(idCol).toInt(), query.value(nameCol).toString() });
        }

        query = db.prepareQuery(QStringLiteral("select * from sp_get_sites()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
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
            throw_query_error(query);
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
            throw_query_error(query);
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
            throw_query_error(query);
        }

        return mapConfigurationParameters(query);
    });
}

ConfigurationParameterValueList
PersistenceManagerDBProvider::GetJobConfigurationParameters(int jobId, const QString &prefix)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(QStringLiteral("GetJobConfigurationParameters"), [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_get_job_parameters(:job_id, :prefix)"));
        query.bindValue(QStringLiteral(":job_id"), jobId);
        query.bindValue(QStringLiteral(":prefix"), prefix);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
        }

        return mapConfigurationParameters(query);
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
            throw_query_error(query);
        }

        return mapUpdateConfigurationResult(query);
    });
}

KeyedMessageList PersistenceManagerDBProvider::UpdateJobConfigurationParameters(
    int jobId, const ConfigurationUpdateActionList &parameters)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_upsert_job_parameters(:job_id, :parameters)"));

        query.bindValue(QStringLiteral(":job_id"), jobId);
        query.bindValue(QStringLiteral(":parameters"), getConfigurationUpsertJson(parameters));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
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
            throw_query_error(query);
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
            throw_query_error(query);
        }
    });
}

int PersistenceManagerDBProvider::SubmitJob(const NewJob &job)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query =
            db.prepareQuery(QStringLiteral("select * from sp_submit_job(:name, :description, "
                                           ":processorId, :siteId, :startTypeId, :parameters)"));
        query.bindValue(QStringLiteral(":name"), job.name);
        query.bindValue(QStringLiteral(":description"), job.description);
        query.bindValue(QStringLiteral(":processorId"), job.processorId);
        query.bindValue(QStringLiteral(":siteId"), job.siteId);
        query.bindValue(QStringLiteral(":startTypeId"), static_cast<int>(job.startType));
        query.bindValue(QStringLiteral(":parameters"), QString::fromUtf8(job.parameters.toJson()));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
        }

        auto dataRecord = query.record();
        auto jobIdCol = dataRecord.indexOf(QStringLiteral("job_id"));

        if (query.next()) {
            return query.value(jobIdCol).toInt();
        } else {
            throw std::runtime_error("Expecting a return value from sp_submit_job, but none found");
        }
    });
}

int PersistenceManagerDBProvider::SubmitTask(const NewTask &task)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_submit_task(:jobId, :moduleId, :parameters)"));
        query.bindValue(QStringLiteral(":jobId"), task.jobId);
        query.bindValue(QStringLiteral(":moduleId"), task.moduleId);
        query.bindValue(QStringLiteral(":parameters"), task.parameters);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
        }

        auto dataRecord = query.record();
        auto taskIdCol = dataRecord.indexOf(QStringLiteral("task_id"));

        if (query.next()) {
            return query.value(taskIdCol).toInt();
        } else {
            throw std::runtime_error(
                "Expecting a return value from sp_submit_task, but none found");
        }
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
            throw_query_error(query);
        }
    });
}

void PersistenceManagerDBProvider::MarkStepStarted(int taskId, const QString &name)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select sp_mark_step_started(:taskId, :name)"));
        query.bindValue(QStringLiteral(":taskId"), taskId);
        query.bindValue(QStringLiteral(":name"), name);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
        }
    });
}

void PersistenceManagerDBProvider::MarkStepFinished(int taskId,
                                                    const QString &name,
                                                    const ExecutionStatistics &statistics)
{
    static_assert(sizeof(qlonglong) == sizeof(int64_t), "qlonglong must be 64-bit");

    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        db.transaction();

        auto query =
            db.prepareQuery(QStringLiteral("set transaction isolation level repeatable read"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
        }

        query = db.prepareQuery(QStringLiteral(
            "select sp_mark_step_finished(:taskId, :name, :node, :userCpuMs, :systemCpuMs, "
            ":durationMs, :maxRssKb, :maxVmSizeKb, :diskReadBytes, :diskWriteBytes)"));
        query.bindValue(QStringLiteral(":taskId"), taskId);
        query.bindValue(QStringLiteral(":name"), name);
        query.bindValue(QStringLiteral(":node"), statistics.node);
        query.bindValue(QStringLiteral(":userCpuMs"), qlonglong{ statistics.userCpuMs });
        query.bindValue(QStringLiteral(":systemCpuMs"), qlonglong{ statistics.systemCpuMs });
        query.bindValue(QStringLiteral(":durationMs"), qlonglong{ statistics.durationMs });
        query.bindValue(QStringLiteral(":maxRssKb"), statistics.maxRssKb);
        query.bindValue(QStringLiteral(":maxVmSizeKb"), statistics.maxVmSizeKb);
        query.bindValue(QStringLiteral(":diskReadBytes"), qlonglong{ statistics.diskReadBytes });
        query.bindValue(QStringLiteral(":diskWriteBytes"), qlonglong{ statistics.diskWriteBytes });

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
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
            throw_query_error(query);
        }
    });
}

void PersistenceManagerDBProvider::InsertTaskFinishedEvent(const TaskFinishedEvent &event)
{
    InsertEvent(event);
}

void PersistenceManagerDBProvider::InsertProductAvailableEvent(const ProductAvailableEvent &event)
{
    InsertEvent(event);
}

void PersistenceManagerDBProvider::InsertJobCancelledEvent(const JobCancelledEvent &event)
{
    InsertEvent(event);
}

void PersistenceManagerDBProvider::InsertJobPausedEvent(const JobPausedEvent &event)
{
    InsertEvent(event);
}

void PersistenceManagerDBProvider::InsertJobResumedEvent(const JobResumedEvent &event)
{
    InsertEvent(event);
}

UnprocessedEventList PersistenceManagerDBProvider::GetNewEvents()
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(QStringLiteral("select * from sp_get_new_events()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
        }

        auto dataRecord = query.record();
        auto eventTypeCol = dataRecord.indexOf(QStringLiteral("type_id"));
        auto eventDataCol = dataRecord.indexOf(QStringLiteral("data"));
        auto submittedCol = dataRecord.indexOf(QStringLiteral("submitted_timestamp"));
        auto processingStartedCol =
            dataRecord.indexOf(QStringLiteral("processing_started_timestamp"));

        UnprocessedEventList result;
        while (query.next()) {
            result.append({ static_cast<EventType>(query.value(eventTypeCol).toInt()),
                            QJsonDocument::fromJson(query.value(eventDataCol).toString().toUtf8()),
                            query.value(submittedCol).toDateTime(),
                            to_optional<QDateTime>(query.value(processingStartedCol)) });
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
        query.bindValue(QStringLiteral(":eventData"), toJsonString(event.data));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
        }
    });
}

void PersistenceManagerDBProvider::InsertNodeStatistics(const NodeStatistics &statistics)
{
    static_assert(sizeof(qlonglong) == sizeof(int64_t), "qlonglong must be 64-bit");

    auto db = getDatabase();

    return provider.handleTransactionRetry(__func__, [&] {
        auto query = db.prepareQuery(
            QStringLiteral("select sp_insert_node_statistics(:node, :freeRamKb, :freeDiskBytes)"));
        query.bindValue(QStringLiteral(":node"), statistics.node);
        query.bindValue(QStringLiteral(":freeRamKb"), statistics.freeRamKb);
        query.bindValue(QStringLiteral(":freeDiskBytes"), qlonglong{ statistics.freeDiskBytes });

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
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

    return toJsonString(array);
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

    return toJsonString(array);
}

static QString getNewStepsJson(const NewStepList &steps)
{
    QJsonArray array;
    for (const auto &s : steps) {
        QJsonObject node;
        node[QStringLiteral("task_id")] = s.taskId;
        node[QStringLiteral("name")] = s.name;
        node[QStringLiteral("parameters")] = s.parameters.object();
        array.append(std::move(node));
    }

    return toJsonString(array);
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

static QString toJsonString(const QJsonDocument &document)
{
    return QString::fromUtf8(document.toJson());
}

/*
static QString toJsonString(const QJsonObject &object)
{
    return toJsonString(QJsonDocument(object));
}
*/

static QString toJsonString(const QJsonArray &array)
{
    return toJsonString(QJsonDocument(array));
}
