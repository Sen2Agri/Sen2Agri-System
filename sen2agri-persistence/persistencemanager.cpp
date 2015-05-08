#include <QtSql>
#include <QDebug>

#include "persistencemanager.hpp"

PersistenceManager::PersistenceManager(QObject *parent) : QObject(parent)
{
}

ConfigurationParameterList PersistenceManager::GetConfigurationParameters(const QString &prefix)
{
    qDebug() << ':' << this->calledFromDBus();

    return dbProvider.GetConfigurationParameters(prefix);
}

void PersistenceManager::registerMetaTypes()
{
    ConfigurationParameter::registerMetaTypes();
}
