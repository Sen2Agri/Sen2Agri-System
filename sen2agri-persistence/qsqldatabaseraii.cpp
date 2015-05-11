#include <stdexcept>
#include <utility>

#include <QString>
#include <QSqlError>
#include <QThread>

#include "qsqldatabaseraii.hpp"
#include "sql_error.hpp"

using std::move;
using std::runtime_error;

// QSqlDatabase instances have thread affinity and can't be created without adding them to the
// global registry.
// Because of this, include the thread id in the connection name
SqlDatabaseRAII::SqlDatabaseRAII(const QString &name)
    : db(QSqlDatabase::addDatabase(
          QStringLiteral("QPSQL"),
          name + "@" +
              QString::number(reinterpret_cast<uintptr_t>(QThread::currentThreadId()), 16))),
      isInitialized(true)
{
    db.setHostName("sen2agri-dev");
    db.setDatabaseName("sen2agri");
    db.setUserName("admin");
    db.setPassword("sen2agri");

    if (!db.open()) {
        reset();

        throw runtime_error(db.lastError().text().toStdString());
    }
}

SqlDatabaseRAII::SqlDatabaseRAII(SqlDatabaseRAII &&other) : db(move(other.db))
{
    other.isInitialized = false;
}

SqlDatabaseRAII &SqlDatabaseRAII::operator=(SqlDatabaseRAII &&other)
{
    db = move(other.db);
    other.isInitialized = false;

    return *this;
}

SqlDatabaseRAII::~SqlDatabaseRAII()
{
    if (isInitialized) {
        // It was open after the constructor, but that might have changed if an error occurred
        if (db.isOpen()) {
            db.close();
        }

        reset();
    }
}

void SqlDatabaseRAII::reset()
{
    // QSqlDatabase::removeDatabase() will complain if there are outstanding references to the
    // QSqlDatabase instance, even if close() was called
    const auto &name = db.connectionName();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase(name);
}

QSqlQuery SqlDatabaseRAII::createQuery()
{
    return QSqlQuery(db);
}

QSqlQuery SqlDatabaseRAII::prepareQuery(const QString &query)
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
