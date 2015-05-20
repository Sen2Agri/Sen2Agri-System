#include <functional>

#include <QtSql>
#include <QDebug>
#include <QDBusMessage>
#include <QThreadPool>

#include "persistencemanager.hpp"

PersistenceManager::PersistenceManager(QDBusConnection &connection,
                                       const Settings &settings,
                                       QObject *parent)
    : QObject(parent), dbProvider(settings), connection(connection)
{
}

ConfigurationSet PersistenceManager::GetConfigurationSet()
{
    RunAsync([=]() { return dbProvider.GetConfigurationSet(); });

    return {};
}

ConfigurationParameterValueList
PersistenceManager::GetConfigurationParameters(const QString &prefix)
{
    RunAsync([=]() { return dbProvider.GetConfigurationParameters(prefix); });

    return {};
}

KeyedMessageList
PersistenceManager::UpdateConfigurationParameters(const ConfigurationParameterValueList &parameters)
{
    RunAsync([=]() { return dbProvider.UpdateConfigurationParameters(parameters); });

    return {};
}

void PersistenceManager::registerMetaTypes()
{
    ConfigurationParameterInfo::registerMetaTypes();
    ConfigurationParameterValue::registerMetaTypes();
    ConfigurationCategory::registerMetaTypes();
    ConfigurationSet::registerMetaTypes();
    KeyedMessage::registerMetaTypes();
}
