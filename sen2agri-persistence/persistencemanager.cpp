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

ProductToArchiveList PersistenceManager::GetProductsToArchive()
{
    RunAsync([=]() { return dbProvider.GetProductsToArchive(); });

    return {};
}

void PersistenceManager::MarkProductsArchived(const ArchivedProductList &products)
{
    RunAsyncNoResult([=]() { return dbProvider.MarkProductsArchived(products); });
}

int PersistenceManager::SubmitJob(const NewJob &job)
{
    RunAsync([=]() { return dbProvider.SubmitJob(job); });

    return {};
}

void PersistenceManager::NotifyJobStepStarted(int jobId)
{
    RunAsyncNoResult([=]() { return dbProvider.NotifyJobStepStarted(jobId); });
}

void PersistenceManager::NotifyJobStepFinished(int jobId /*, resources */)
{
    RunAsyncNoResult([=]() { return dbProvider.NotifyJobStepFinished(jobId); });
}

void PersistenceManager::NotifyJobFinished(int jobId)
{
    RunAsyncNoResult([=]() { return dbProvider.NotifyJobFinished(jobId); });
}
