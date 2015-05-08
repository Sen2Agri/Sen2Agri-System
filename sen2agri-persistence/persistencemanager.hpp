#pragma once

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusMessage>

#include "dbprovider.hpp"
#include "configurationparameter.hpp"
#include "asyncdbustask.hpp"

class PersistenceManager : public QObject, protected QDBusContext
{
    Q_OBJECT

    DBProvider dbProvider;
    QDBusConnection connection;

    template <typename F>
    void RunAsync(F &&f)
    {
        setDelayedReply(true);

        if (auto task = makeAsyncDBusTask(message(), connection, std::forward<F>(f))) {
            QThreadPool::globalInstance()->start(task);
        } else {
            sendErrorReply(QDBusError::NoMemory);
        }
    }

public:
    explicit PersistenceManager(QDBusConnection &connection, QObject *parent = 0);

    static void registerMetaTypes();

signals:

public slots:
    ConfigurationParameterList GetConfigurationParameters(const QString &prefix);
};
