#include <stdexcept>
#include <utility>

#include <QString>
#include <QSqlError>
#include <QThread>

#include "sqldatabaseraii.hpp"
#include "sql_error.hpp"

using std::move;
using std::runtime_error;

// QSqlDatabase instances have thread affinity and can't be created without adding them to the
// global registry.
// Because of this, include the thread id in the connection name
SqlDatabaseRAII::SqlDatabaseRAII(const QString &name,
                                 const QString &hostName,
                                 const QString &databaseName,
                                 const QString &userName,
                                 const QString &password)
    : db(QSqlDatabase::addDatabase(
          QStringLiteral("QPSQL"),
          name + "@" +
              QString::number(reinterpret_cast<uintptr_t>(QThread::currentThreadId()), 16))),
      isInitialized(true),
      inTransaction()
{
    db.setHostName(hostName);
    db.setDatabaseName(databaseName);
    db.setUserName(userName);
    db.setPassword(password);

    if (!db.open()) {
        const auto &message = QStringLiteral("Unable to connect to the database: %1")
                                  .arg(db.lastError().text())
                                  .toStdString();

        reset();

        throw runtime_error(message);
    }
}

SqlDatabaseRAII::SqlDatabaseRAII(SqlDatabaseRAII &&other)
    : db(move(other.db)), isInitialized(other.isInitialized), inTransaction(other.inTransaction)
{
    other.isInitialized = false;
}

SqlDatabaseRAII &SqlDatabaseRAII::operator=(SqlDatabaseRAII &&other)
{
    isInitialized = other.isInitialized;
    inTransaction = other.inTransaction;

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

QSqlQuery SqlDatabaseRAII::createQuery() { return QSqlQuery(db); }

QSqlQuery SqlDatabaseRAII::prepareQuery(const QString &query)
{
    auto q = createQuery();

    if (!q.prepare(query)) {
        throw_query_error(*this, q);
    }

    return q;
}

void SqlDatabaseRAII::transaction()
{
    if (!db.transaction()) {
        throw_db_error(db);
    }

    inTransaction = true;
}

void SqlDatabaseRAII::commit()
{
    if (!db.commit()) {
        throw_db_error(db);
    }

    inTransaction = false;
}

void SqlDatabaseRAII::rollback()
{
    if (inTransaction && !db.rollback()) {
        inTransaction = false;

        throw_db_error(db);
    }
}

void throw_db_error(const QSqlDatabase &db) { throw sql_error(db.lastError()); }

void throw_query_error(SqlDatabaseRAII &db, const QSqlQuery &query)
{
    // NOTE: Postgres doesn't roll back aborted transactions, but won't execute anything else until
    // the end of the transaction block.
    // NOTE: normally we'd handle the abort in handle_transaction_retry(), but the QSqlQuery
    // destructor would deallocate the prepared statement before the transaction rollback, making
    // Postgres complain in the log. To avoid confusing any sysadmins, we roll back the transaction
    // before unwinding the stack. In any case, it's good hygiene to use check for errors using this
    // function and most of the callers don't care about transactions.
    db.rollback();

    throw sql_error(query.lastError());
}
