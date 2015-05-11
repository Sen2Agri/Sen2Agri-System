#include <memory>

#include <QJsonDocument>
#include <QThread>

#include "dbprovider.hpp"

DBProvider::DBProvider()
{
}

ConfigurationParameterList
PersistenceManagerDBProvider::GetConfigurationParameters(const QString &prefix)
{
    auto db = provider.getDatabase();

    return provider.handleTransactionRetry(QStringLiteral("GetConfigurationParameters"), [&]() {
        auto query = db.prepareQuery("select * from sp_get_configuration_parameters(:prefix)");
        query.bindValue(":prefix", prefix);

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
        }

        auto dataRecord = query.record();
        auto keyCol = dataRecord.indexOf(QStringLiteral("key"));
        auto valueCol = dataRecord.indexOf(QStringLiteral("value"));

        ConfigurationParameterList result;
        while (query.next()) {
            result.append({ query.value(keyCol).toString(), query.value(valueCol).toString() });
        }

        return result;
    });
}

KeyedMessageList PersistenceManagerDBProvider::UpdateConfigurationParameters(
    const ConfigurationParameterList &parameters)
{
    auto db = provider.getDatabase();

    return provider.handleTransactionRetry(QStringLiteral("UpdateConfigurationParameters"), [&]() {
        auto query =
            db.prepareQuery("select * from sp_update_configuration_parameters(:parameters)");
        query.bindValue(QStringLiteral(":parameters"), toJson(parameters));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
        }

        auto dataRecord = query.record();
        auto keyCol = dataRecord.indexOf(QStringLiteral("key"));
        auto errorCol = dataRecord.indexOf(QStringLiteral("error"));

        KeyedMessageList result;
        while (query.next()) {
            result.append({ query.value(keyCol).toString(), query.value(errorCol).toString() });
        }

        return result;
    });
}

QSqlDatabaseRAII DBProvider::getDatabase() const
{
    return QSqlDatabaseRAII();
}

bool DBProvider::shouldRetryTransaction(const sql_error &e)
{
    return e.error_code() == "40001" || e.error_code() == "40P01";
}

QString DBProvider::getErrorName(const sql_error &e)
{
    if (e.error_code() == "40001") {
        return QStringLiteral("serialization failure");
    } else if (e.error_code() == "40P01") {
        return QStringLiteral("deadlock");
    } else {
        return QStringLiteral("unknown");
    }
}

void DBProvider::warnRecoverableErrorAbort(const sql_error &e, const QString &operation)
{
    Q_UNUSED(QStringLiteral("A recoverable error of type %1 has been detected for "
                            "operation %2, but the retry limit has been reached, aborting.")
                 .arg(getErrorName(e))
                 .arg(operation));
}

void DBProvider::warnRecoverableError(
    const sql_error &e, const QString &operation, int retryDelay, int retryNumber, int maxRetries)
{
    Q_UNUSED(QStringLiteral("A recoverable error of type %1 has been detected for "
                            "operation %2. Retrying in %3 ms (%4/%5).")
                 .arg(getErrorName(e))
                 .arg(operation)
                 .arg(retryDelay)
                 .arg(retryNumber)
                 .arg(maxRetries));
}
