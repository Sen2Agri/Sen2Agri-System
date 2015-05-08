#include <memory>

#include <QThread>

#include "dbprovider.hpp"

DBProvider::DBProvider()
{
}

ConfigurationParameterList DBProvider::GetConfigurationParameters(const QString &)
{
    QSqlDatabaseRAII db = getDatabase();

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
}

QSqlDatabaseRAII DBProvider::getDatabase()
{
    return QSqlDatabaseRAII();
}
