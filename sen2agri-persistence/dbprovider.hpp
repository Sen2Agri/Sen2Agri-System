#pragma once

#include <utility>

#include <QString>
#include <QtSql>

#include "qsqldatabaseraii.hpp"
#include "sql_error.hpp"
#include "configurationparameter.hpp"
#include "keyedmessage.hpp"

class DBProvider
{
public:
    DBProvider();

    QSqlDatabaseRAII getDatabase() const;

    bool shouldRetryTransaction(const sql_error &e);
    QString getErrorName(const sql_error &e);
    void warnRecoverableErrorAbort(const sql_error &e, const QString &operation);
    void warnRecoverableError(const sql_error &e,
                              const QString &operation,
                              int retryDelay,
                              int retryNumber,
                              int maxNetries);

    template <typename F>
    auto handleTransactionRetry(const QString &operation, F &&f) -> decltype(f())
    {
        static const int maxTxnRetries = 10;
        static const int initialRetryDelay = 1000;
        static const int maxRetryDelay = 8000;

        int retryDelay = 0;
        int txnRetryNumber = 0;
        while (true) {
            try {
                return f();
            } catch (const sql_error &e) {
                if (!shouldRetryTransaction(e)) {
                    throw;
                } else if (++txnRetryNumber > maxTxnRetries) {
                    warnRecoverableErrorAbort(e, operation);
                    throw;
                } else {
                    if (!retryDelay) {
                        retryDelay = initialRetryDelay;
                    } else if (retryDelay * 2 <= maxRetryDelay) {
                        retryDelay *= 2;
                    }

                    warnRecoverableError(e, operation, retryDelay, txnRetryNumber, maxTxnRetries);

                    QThread::msleep(retryDelay);
                }
            }
        }
    }
};

class PersistenceManagerDBProvider
{
    DBProvider provider;

public:
    ConfigurationParameterList GetConfigurationParameters(const QString &prefix);
    KeyedMessageList UpdateConfigurationParameters(const ConfigurationParameterList &parameters);
};
