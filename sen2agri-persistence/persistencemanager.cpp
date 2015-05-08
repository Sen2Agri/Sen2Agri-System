#include <QtSql>
#include <QDebug>

#include "persistencemanager.hpp"

PersistenceManager::PersistenceManager(QObject *parent) : QDBusAbstractAdaptor(parent)
{
}

ConfigurationParameterList PersistenceManager::GetConfigurationParameters(const QString &prefix)
{
    return dbProvider.GetConfigurationParameters(prefix);
}

void PersistenceManager::registerMetaTypes()
{
    ConfigurationParameter::registerMetaTypes();
}
