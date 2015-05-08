#include <stdexcept>
#include <utility>

#include <QString>
#include <QSqlError>
#include <QThread>

#include "qsqldatabaseraii.hpp"
#include "sql_error.hpp"

using std::move;
using std::runtime_error;

QSqlDatabaseRAII::QSqlDatabaseRAII()
    : db(QSqlDatabase::addDatabase(
          QStringLiteral("QPSQL"),
          QStringLiteral("@") +
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

QSqlDatabaseRAII::QSqlDatabaseRAII(QSqlDatabaseRAII &&other) : db(move(other.db))
{
    other.isInitialized = false;
}

QSqlDatabaseRAII &QSqlDatabaseRAII::operator=(QSqlDatabaseRAII &&other)
{
    db = move(other.db);
    other.isInitialized = false;

    return *this;
}

QSqlDatabaseRAII::~QSqlDatabaseRAII()
{
    if (isInitialized) {
        if (db.isOpen()) {
            db.close();
        }

        reset();
    }
}

void QSqlDatabaseRAII::reset()
{
    const auto &name = db.connectionName();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase(name);
}

QSqlQuery QSqlDatabaseRAII::createQuery()
{
    return QSqlQuery(db);
}

QSqlQuery QSqlDatabaseRAII::prepareQuery(const QString &query)
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
