#include <functional>

#include <QtSql>
#include <QDebug>
#include <QDBusMessage>
#include <QThreadPool>

#include "persistencemanager.hpp"

PersistenceManager::PersistenceManager(QDBusConnection &connection, QObject *parent)
    : QObject(parent), connection(connection)
{
}

ConfigurationParameterList PersistenceManager::GetConfigurationParameters(const QString &prefix)
{
    RunAsync([=]() { return dbProvider.GetConfigurationParameters(prefix); });

    return {};
}

KeyedMessageList
PersistenceManager::UpdateConfigurationParameters(const ConfigurationParameterList &parameters)
{
    RunAsync([=]() { return dbProvider.UpdateConfigurationParameters(parameters); });

    return {};
}

void PersistenceManager::registerMetaTypes()
{
    ConfigurationParameter::registerMetaTypes();
    KeyedMessage::registerMetaTypes();
}
