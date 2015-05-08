#include <memory>

#include <QThread>

#include "dbprovider.hpp"

DBProvider::DBProvider()
{
}

ConfigurationParameterList DBProvider::GetConfigurationParameters(const QString &)
{
    QSqlDatabaseRAII db = getDatabase();

    return handleTransactionRetry(QStringLiteral("GetConfigurationParameters"), [&]() {
        auto query = db.prepareQuery("select 'param1' as key, 'value1' as value");
        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
        }

        ConfigurationParameterList result;

        auto dataRecord = query.record();
        auto keyCol = dataRecord.indexOf(QStringLiteral("key"));
        auto valueCol = dataRecord.indexOf(QStringLiteral("value"));

        while (query.next()) {
            result.append({ query.value(keyCol).toString(), query.value(valueCol).toString() });
        }

        return result;
    });
}

QSqlDatabaseRAII DBProvider::getDatabase()
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

void DBProvider::warnRecoverableError(const sql_error &e,
                                      const QString &operation,
                                      int retryDelay,
                                      int retryNumber,
                                      int maxRetries)
{
    Q_UNUSED(QStringLiteral("A recoverable error of type %1 has been detected for "
                            "operation %2. Retrying in %3 ms (%4/%5).")
                 .arg(getErrorName(e))
                 .arg(operation)
                 .arg(retryDelay)
                 .arg(retryNumber)
                 .arg(maxRetries));
}
