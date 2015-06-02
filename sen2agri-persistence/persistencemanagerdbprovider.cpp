#include "persistencemanagerdbprovider.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

static QString getConfigurationUpsertJson(const ConfigurationUpdateActionList &actions);
static ConfigurationParameterValueList mapConfigurationParameters(QSqlQuery &query);
static KeyedMessageList mapUpdateConfigurationResult(QSqlQuery &query);

template <typename T>
std::experimental::optional<T> to_optional(const QVariant &v)
{
    if (v.isNull()) {
        return std::experimental::nullopt;
    }
    return v.value<T>();
}

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

    return provider.handleTransactionRetry(QStringLiteral("GetConfigurationSet"), [&]() {
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

        return result;
    });
}

ConfigurationParameterValueList
PersistenceManagerDBProvider::GetConfigurationParameters(const QString &prefix)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(QStringLiteral("GetConfigurationParameters"), [&]() {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_get_parameters(:prefix)"));
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

    return provider.handleTransactionRetry(QStringLiteral("GetJobConfigurationParameters"), [&]() {
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
    const ConfigurationUpdateActionList &actions)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(QStringLiteral("UpdateConfigurationParameters"), [&]() {
        auto query =
            db.prepareQuery(QStringLiteral("select * from sp_upsert_parameters(:parameters)"));

        query.bindValue(QStringLiteral(":parameters"), getConfigurationUpsertJson(actions));

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

    return provider.handleTransactionRetry(
        QStringLiteral("UpdateJobConfigurationParameters"), [&]() {
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

    return provider.handleTransactionRetry(QStringLiteral("GetProductsToArchive"), [&]() {
        auto query = db.prepareQuery(QStringLiteral("select * from sp_get_products_to_archive()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
        }

        auto dataRecord = query.record();
        auto productIdCol = dataRecord.indexOf(QStringLiteral("productId"));
        auto currentPathCol = dataRecord.indexOf(QStringLiteral("currentPath"));
        auto archivePathCol = dataRecord.indexOf(QStringLiteral("archivePath"));

        ProductToArchiveList result;
        while (query.next()) {
            result.append({ query.value(productIdCol).toInt(),
                            query.value(currentPathCol).toString(),
                            query.value(archivePathCol).toString() });
        }

        return result;
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
        array.append(node);
    }
    return QString::fromUtf8(QJsonDocument(array).toJson());
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
