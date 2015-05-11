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

void PersistenceManager::UpdateConfigurationParameters(const ConfigurationParameterList &parameters)
{
    return dbProvider.UpdateConfigurationParameters(parameters);
    //RunAsync([=]() { return dbProvider.UpdateConfigurationParameters(parameters); });
}

void PersistenceManager::registerMetaTypes()
{
    ConfigurationParameter::registerMetaTypes();
}
