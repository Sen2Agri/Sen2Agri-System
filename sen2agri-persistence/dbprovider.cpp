#include <memory>

#include <QThread>

#include "dbprovider.hpp"
#include "logger.hpp"

DBProvider::DBProvider(const Settings &settings) : settings(settings)
{
}

SqlDatabaseRAII DBProvider::getDatabase(const QString &name) const
{
    return SqlDatabaseRAII(name, settings.hostName, settings.databaseName, settings.userName,
                           settings.password);
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
    Logger::error(QStringLiteral("A recoverable error of type %1 has been detected for "
                                 "operation %2, but the retry limit has been reached, aborting.")
                      .arg(getErrorName(e))
                      .arg(operation));
}

void DBProvider::warnRecoverableError(
    const sql_error &e, const QString &operation, int retryDelay, int retryNumber, int maxRetries)
{
    Logger::warn(QStringLiteral("A recoverable error of type %1 has been detected for "
                                "operation %2. Retrying in %3 ms (%4/%5).")
                     .arg(getErrorName(e))
                     .arg(operation)
                     .arg(retryDelay)
                     .arg(retryNumber)
                     .arg(maxRetries));
}
