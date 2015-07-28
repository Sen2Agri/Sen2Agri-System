#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>

class SqlDatabaseRAII
{
    QSqlDatabase db;
    bool isInitialized;
    bool inTransaction;

    void reset();

public:
    SqlDatabaseRAII(const QString &name,
                    const QString &hostName,
                    const QString &databaseName,
                    const QString &userName,
                    const QString &password);
    SqlDatabaseRAII(const SqlDatabaseRAII &) = delete;
    SqlDatabaseRAII(SqlDatabaseRAII &&other);
    SqlDatabaseRAII &operator=(const SqlDatabaseRAII &) = delete;
    SqlDatabaseRAII &operator=(SqlDatabaseRAII &&other);
    ~SqlDatabaseRAII();

    QSqlQuery createQuery();
    QSqlQuery prepareQuery(const QString &query);

    void transaction();
    void commit();
    void rollback();

    QSqlError lastError() const;
};

void throw_db_error(const SqlDatabaseRAII &db);
void throw_db_error(const QSqlDatabase &db);
void throw_query_error(SqlDatabaseRAII &db, const QSqlQuery &query);
