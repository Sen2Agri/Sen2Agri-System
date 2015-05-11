#include <memory>

#include <QJsonDocument>
#include <QThread>

#include "dbprovider.hpp"

DBProvider::DBProvider()
{
}

ConfigurationParameterList DBProvider::GetConfigurationParameters(const QString &)
{
    auto db = getDatabase();

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

void DBProvider::UpdateConfigurationParameters(const ConfigurationParameterList &parameters)
{
    auto db = getDatabase();

    return handleTransactionRetry(QStringLiteral("UpdateConfigurationParameters"), [&]() {
        //        auto query = db.prepareQuery("select
        //        sp_update_configuration_parameters(:parameters)");
        auto query = db.prepareQuery("select * from json_each_text(:parameters)");

        //       QString parameterArrayString = QStringLiteral("'{");
        QJsonObject doc;
        for (const auto &p : parameters) {
            doc[p.key] = p.value;
            //           parameterArrayString += "(\"" + p.key + "\",\"" + p.value + "\"),";
        }
        //       parameterArrayString[parameterArrayString.size() - 1] = '}';
        //       parameterArrayString += '\'';
        query.bindValue(QStringLiteral(":parameters"),
                        QString::fromUtf8(QJsonDocument(doc).toJson()));

        query.setForwardOnly(true);
        if (!query.exec()) {
            throw_query_error(query);
        }

        auto dataRecord = query.record();
        auto keyCol = dataRecord.indexOf(QStringLiteral("key"));
        auto valueCol = dataRecord.indexOf(QStringLiteral("value"));

        while (query.next()) {
            qDebug() << query.value(keyCol).toString() << query.value(valueCol).toString();
        }
        qDebug() << query.executedQuery();
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
