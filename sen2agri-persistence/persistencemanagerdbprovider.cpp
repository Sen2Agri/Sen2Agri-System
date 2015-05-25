#include "persistencemanagerdbprovider.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

static QString getConfigurationUpsertJson(const ConfigurationParameterValueList &parameters);
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

        query = db.prepareQuery(QStringLiteral("select * from sp_get_parameter_set()"));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
        }

        dataRecord = query.record();
        auto keyCol = dataRecord.indexOf(QStringLiteral("key"));
        auto friendlyNameCol = dataRecord.indexOf(QStringLiteral("friendly_name"));
        auto valueCol = dataRecord.indexOf(QStringLiteral("value"));
        auto dataTypeCol = dataRecord.indexOf(QStringLiteral("type"));
        auto isAdvancedCol = dataRecord.indexOf(QStringLiteral("is_advanced"));
        auto categoryCol = dataRecord.indexOf(QStringLiteral("config_category_id"));

        while (query.next()) {
            result.parameters.append({ query.value(keyCol).toString(),
                                       query.value(categoryCol).toInt(),
                                       query.value(friendlyNameCol).toString(),
                                       query.value(dataTypeCol).toString(),
                                       query.value(valueCol).toString(),
                                       query.value(isAdvancedCol).toBool() });
        }

        return result;
    });
}

ConfigurationParameterValueList
PersistenceManagerDBProvider::GetConfigurationParameters(const QString &prefix)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(QStringLiteral("GetConfigurationParameters"), [&]() {
        auto query = db.prepareQuery(
            QStringLiteral("select * from sp_get_configuration_parameters(:prefix)"));
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
            QStringLiteral("select * from sp_get_subscription_parameters(:job_id, :prefix)"));
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
    const ConfigurationParameterValueList &parameters)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(QStringLiteral("UpdateConfigurationParameters"), [&]() {
        auto query =
            db.prepareQuery(QStringLiteral("select * from sp_upsert_parameters(:parameters)"));

        query.bindValue(QStringLiteral(":parameters"), getConfigurationUpsertJson(parameters));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
        }

        return mapUpdateConfigurationResult(query);
    });
}

KeyedMessageList PersistenceManagerDBProvider::UpdateJobConfigurationParameters(
    int jobId, const ConfigurationParameterValueList &parameters)
{
    auto db = getDatabase();

    return provider.handleTransactionRetry(
        QStringLiteral("UpdateJobConfigurationParameters"), [&]() {
            auto query = db.prepareQuery(QStringLiteral(
                "select * from sp_upsert_subscription_parameters(:job_id, :parameters)"));

            query.bindValue(QStringLiteral(":job_id"), jobId);
            query.bindValue(QStringLiteral(":parameters"), getConfigurationUpsertJson(parameters));

            query.setForwardOnly(true);
            if (!query.exec()) {
                throw_query_error(query);
            }

            return mapUpdateConfigurationResult(query);
        });
}

static QString getConfigurationUpsertJson(const ConfigurationParameterValueList &parameters)
{
    QJsonArray array;
    for (const auto &p : parameters) {
        QJsonObject node;
        node[QStringLiteral("key")] = p.key;
        node[QStringLiteral("value")] = p.value;
        array.append(node);
    }
    return QString::fromUtf8(QJsonDocument(array).toJson());
}

static ConfigurationParameterValueList mapConfigurationParameters(QSqlQuery &query)
{
    auto dataRecord = query.record();
    auto keyCol = dataRecord.indexOf(QStringLiteral("key"));
    auto valueCol = dataRecord.indexOf(QStringLiteral("value"));

    ConfigurationParameterValueList result;
    while (query.next()) {
        result.append({ query.value(keyCol).toString(), query.value(valueCol).toString() });
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
        result.append(
            { query.value(keyCol).toString(), query.value(errorMessageCol).toString() });
    }

    return result;
}
