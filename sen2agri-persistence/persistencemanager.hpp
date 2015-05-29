#pragma once

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusMessage>

#include "persistencemanagerdbprovider.hpp"
#include "asyncdbustask.hpp"
#include "configurationparameter.hpp"
#include "keyedmessage.hpp"
#include "settings.hpp"

class PersistenceManager : public QObject, protected QDBusContext
{
    Q_OBJECT

    PersistenceManagerDBProvider dbProvider;
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

    template <typename F>
    void RunAsyncNoResult(F &&f)
    {
        setDelayedReply(true);

        if (auto task = makeAsyncDBusTaskNoResult(message(), connection, std::forward<F>(f))) {
            QThreadPool::globalInstance()->start(task);
        } else {
            sendErrorReply(QDBusError::NoMemory);
        }
    }

public:
    explicit PersistenceManager(QDBusConnection &connection,
                                const Settings &settings,
                                QObject *parent = 0);

    static void registerMetaTypes();

signals:

public slots:
    ConfigurationSet GetConfigurationSet();
    ConfigurationParameterValueList GetConfigurationParameters(const QString &prefix);
    ConfigurationParameterValueList GetJobConfigurationParameters(int jobId, const QString &prefix);
    KeyedMessageList
    UpdateConfigurationParameters(const ConfigurationUpdateActionList &actions);
    KeyedMessageList
    UpdateJobConfigurationParameters(int jobId, const ConfigurationUpdateActionList &parameters);
};
