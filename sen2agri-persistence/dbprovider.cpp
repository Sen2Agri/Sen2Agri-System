#include "dbprovider.hpp"
#include "sql_error.hpp"

static void throw_query_error(const QSqlQuery &query);

DBProvider::DBProvider() : db(QSqlDatabase::addDatabase("QPSQL"))
{
    db.setHostName("sen2agri-dev");
    db.setDatabaseName("sen2agri");
    db.setUserName("admin");
    db.setPassword("sen2agri");

    bool ok = db.open();
    qDebug() << ok;
}

ConfigurationParameterList DBProvider::GetConfigurationParameters(const QString &)
{
    auto query = prepareQuery("select 'param1' as key, 'value1' as value");
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

QSqlQuery DBProvider::createQuery()
{
    return QSqlQuery(db);
}

QSqlQuery DBProvider::prepareQuery(const QString &query)
{
    auto q = createQuery();

    if (!q.prepare(query)) {
        throw_query_error(q);
    }

    return q;
}

void throw_query_error(const QSqlQuery &query)
{
    throw sql_error(query.lastError());
}
