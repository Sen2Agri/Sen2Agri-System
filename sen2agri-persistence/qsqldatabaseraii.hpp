#include <QSqlDatabase>
#include <QSqlQuery>

class QSqlDatabaseRAII
{
    QSqlDatabase db;
    bool isInitialized;

    void reset();

public:
    QSqlDatabaseRAII();
    QSqlDatabaseRAII(const QSqlDatabaseRAII &) = delete;
    QSqlDatabaseRAII(QSqlDatabaseRAII &&other);
    QSqlDatabaseRAII &operator=(const QSqlDatabaseRAII &) = delete;
    QSqlDatabaseRAII &operator=(QSqlDatabaseRAII &&other);
    ~QSqlDatabaseRAII();

    QSqlQuery createQuery();
    QSqlQuery prepareQuery(const QString &query);
};

void throw_query_error(const QSqlQuery &query);
