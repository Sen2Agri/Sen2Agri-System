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

ConfigurationParameterValueList
PersistenceManager::GetJobConfigurationParameters(int jobId, const QString &prefix)
{
    RunAsync([=]() { return dbProvider.GetJobConfigurationParameters(jobId, prefix); });

    return {};
}

KeyedMessageList
PersistenceManager::UpdateConfigurationParameters(const ConfigurationUpdateActionList &actions)
{
    RunAsync([=]() { return dbProvider.UpdateConfigurationParameters(actions); });

    return {};
}

KeyedMessageList PersistenceManager::UpdateJobConfigurationParameters(
    int jobId, const ConfigurationUpdateActionList &parameters)
{
    RunAsync([=]() { return dbProvider.UpdateJobConfigurationParameters(jobId, parameters); });

    return {};
}

void PersistenceManager::registerMetaTypes()
{
    ConfigurationSet::registerMetaTypes();

    ConfigurationParameterInfo::registerMetaTypes();
    ConfigurationParameterValue::registerMetaTypes();
    ConfigurationCategory::registerMetaTypes();
    Site::registerMetaTypes();

    ConfigurationUpdateAction::registerMetaTypes();

    KeyedMessage::registerMetaTypes();

    Product::registerMetaTypes();
}
