#include <functional>

#include <QtSql>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QThreadPool>

#include "make_unique.hpp"
#include "credential_utils.hpp"
#include "persistencemanager.hpp"

PersistenceManager::PersistenceManager(const Settings &settings, QObject *parent)
    : QObject(parent), dbProvider(settings)
{
    dbProvider.TestConnection();
}

bool PersistenceManager::IsCallerAdmin()
{
    const auto &reply = connection().interface()->serviceUid(message().service());
    if (!reply.isValid()) {
        Logger::error(QStringLiteral("Unable to determine caller credentials: %1")
                          .arg(reply.error().message()));

        return false;
    }

    try {
        // TODO what to do with this?
        static const char adminGroupName[] = "sen2agri-admin";

        return isUserInGroup(reply.value(), adminGroupName);
    } catch (const std::runtime_error &e) {
        Logger::error(e.what());
        return false;
    }
}

ConfigurationSet PersistenceManager::GetConfigurationSet()
{
    auto isCallerAdmin = IsCallerAdmin();

    RunAsync([this, isCallerAdmin] {
        auto configuration = dbProvider.GetConfigurationSet();
        configuration.isAdmin = isCallerAdmin;

        return configuration;
    });

    return {};
}

ConfigurationParameterValueList PersistenceManager::GetConfigurationParameters(QString prefix)
{
    // a bit of overkill to avoid copying the argument
    // using c++14 generalized capture would avoid the need to use std::bind
    RunAsync(std::bind([](PersistenceManagerDBProvider &dbProvider, const QString &prefix) {
        return dbProvider.GetConfigurationParameters(prefix);
    }, std::ref(dbProvider), std::move(prefix)));

    return {};
}

ConfigurationParameterValueList PersistenceManager::GetJobConfigurationParameters(int jobId,
                                                                                  QString prefix)
{
    RunAsync(
        std::bind([](PersistenceManagerDBProvider &dbProvider, int jobId, const QString &prefix) {
            return dbProvider.GetJobConfigurationParameters(jobId, prefix);
        }, std::ref(dbProvider), jobId, std::move(prefix)));

    return {};
}

KeyedMessageList
PersistenceManager::UpdateConfigurationParameters(ConfigurationUpdateActionList parameters)
{
    RunAsync(std::bind(
        [](PersistenceManagerDBProvider &dbProvider,
           const ConfigurationUpdateActionList &parameters, bool isCallerAdmin) {
            return dbProvider.UpdateConfigurationParameters(parameters, isCallerAdmin);
        },
        std::ref(dbProvider), std::move(parameters), IsCallerAdmin()));

    return {};
}

KeyedMessageList
PersistenceManager::UpdateJobConfigurationParameters(int jobId,
                                                     ConfigurationUpdateActionList parameters)
{
    RunAsync(std::bind(
        [](PersistenceManagerDBProvider &dbProvider, int jobId,
           const ConfigurationUpdateActionList &parameters) {
            return dbProvider.UpdateJobConfigurationParameters(jobId, parameters);
        },
        std::ref(dbProvider), jobId, std::move(parameters)));

    return {};
}

ProductToArchiveList PersistenceManager::GetProductsToArchive()
{
    RunAsync([this] { return dbProvider.GetProductsToArchive(); });

    return {};
}

void PersistenceManager::MarkProductsArchived(ArchivedProductList products)
{
    RunAsync(std::bind(
        [](PersistenceManagerDBProvider &dbProvider, const ArchivedProductList &products) {
            dbProvider.MarkProductsArchived(products);
        },
        std::ref(dbProvider), std::move(products)));
}

int PersistenceManager::SubmitJob(NewJob job)
{
    RunAsync(std::bind([](PersistenceManagerDBProvider &dbProvider, const NewJob &job) {
        return dbProvider.SubmitJob(job);
    }, std::ref(dbProvider), std::move(job)));

    return {};
}

int PersistenceManager::SubmitTask(NewTask task)
{
    RunAsync(std::bind([](PersistenceManagerDBProvider &dbProvider, const NewTask &task) {
        return dbProvider.SubmitTask(task);
    }, std::ref(dbProvider), std::move(task)));

    return {};
}

void PersistenceManager::SubmitSteps(int taskId, NewStepList steps)
{
    RunAsync(
        std::bind([](PersistenceManagerDBProvider &dbProvider, int taskId,
                     const NewStepList &steps) { return dbProvider.SubmitSteps(taskId, steps); },
                  std::ref(dbProvider), taskId, std::move(steps)));
}

void PersistenceManager::MarkStepPendingStart(int taskId, QString name)
{
    RunAsync(
        std::bind([](PersistenceManagerDBProvider &dbProvider, int taskId, const QString &name) {
            dbProvider.MarkStepPendingStart(taskId, name);
        }, std::ref(dbProvider), taskId, std::move(name)));
}

void PersistenceManager::MarkStepStarted(int taskId, QString name)
{
    RunAsync(
        std::bind([](PersistenceManagerDBProvider &dbProvider, int taskId, const QString &name) {
            dbProvider.MarkStepStarted(taskId, name);
        }, std::ref(dbProvider), taskId, std::move(name)));
}

bool PersistenceManager::MarkStepFinished(int taskId, QString name, ExecutionStatistics statistics)
{
    RunAsync(std::bind(
        [](PersistenceManagerDBProvider &dbProvider, int taskId, const QString &name,
           const ExecutionStatistics &statistics) {
            if (statistics.exitCode) {
                dbProvider.MarkStepFailed(taskId, name, statistics);
                return true;
            } else {
                return dbProvider.MarkStepFinished(taskId, name, statistics);
            }
        },
        std::ref(dbProvider), taskId, std::move(name), std::move(statistics)));

    return {};
}

void MarkJobPaused(int jobId);
void MarkJobCancelled(int jobId);
void MarkJobFinished(int jobId);
void MarkJobFailed(int jobId);
void MarkJobNeedsInput(int jobId);

void PersistenceManager::MarkJobPaused(int jobId)
{
    RunAsync([this, jobId]() { dbProvider.MarkJobPaused(jobId); });
}

void PersistenceManager::MarkJobResumed(int jobId)
{
    RunAsync([this, jobId]() { dbProvider.MarkJobResumed(jobId); });
}

void PersistenceManager::MarkJobCancelled(int jobId)
{
    RunAsync([this, jobId]() { dbProvider.MarkJobCancelled(jobId); });
}

void PersistenceManager::MarkJobFinished(int jobId)
{
    RunAsync([this, jobId]() { dbProvider.MarkJobFinished(jobId); });
}

void PersistenceManager::MarkJobFailed(int jobId)
{
    RunAsync([this, jobId]() { dbProvider.MarkJobFailed(jobId); });
}

void PersistenceManager::MarkJobNeedsInput(int jobId)
{
    RunAsync([this, jobId]() { dbProvider.MarkJobNeedsInput(jobId); });
}

TaskIdList PersistenceManager::GetJobTasksByStatus(int jobId, ExecutionStatusList statusList)
{
    RunAsync(std::bind(
        [](PersistenceManagerDBProvider &dbProvider, int jobId,
           const ExecutionStatusList &statusList) {
            return dbProvider.GetJobTasksByStatus(jobId, statusList);
        },
        std::ref(dbProvider), jobId, std::move(statusList)));

    return {};
}

JobStepToRunList PersistenceManager::GetJobStepsForResume(int jobId)
{
    RunAsync(std::bind([](PersistenceManagerDBProvider &dbProvider, int jobId) {
        return dbProvider.GetJobStepsForResume(jobId);
    }, std::ref(dbProvider), jobId));

    return {};
}

void PersistenceManager::InsertTaskAddedEvent(TaskAddedEvent event)
{
    RunAsync(std::bind([](PersistenceManagerDBProvider &dbProvider, const TaskAddedEvent &event) {
        dbProvider.InsertEvent(event);
    }, std::ref(dbProvider), std::move(event)));
}

void PersistenceManager::InsertTaskFinishedEvent(TaskFinishedEvent event)
{
    RunAsync(
        std::bind([](PersistenceManagerDBProvider &dbProvider, const TaskFinishedEvent &event) {
            dbProvider.InsertEvent(event);
        }, std::ref(dbProvider), std::move(event)));
}

void PersistenceManager::InsertProductAvailableEvent(ProductAvailableEvent event)
{
    RunAsync(
        std::bind([](PersistenceManagerDBProvider &dbProvider, const ProductAvailableEvent &event) {
            dbProvider.InsertEvent(event);
        }, std::ref(dbProvider), std::move(event)));
}

void PersistenceManager::InsertJobCancelledEvent(JobCancelledEvent event)
{
    RunAsync(
        std::bind([](PersistenceManagerDBProvider &dbProvider, const JobCancelledEvent &event) {
            dbProvider.InsertEvent(event);
        }, std::ref(dbProvider), std::move(event)));
}

void PersistenceManager::InsertJobPausedEvent(JobPausedEvent event)
{
    RunAsync(std::bind([](PersistenceManagerDBProvider &dbProvider, const JobPausedEvent &event) {
        dbProvider.InsertEvent(event);
    }, std::ref(dbProvider), std::move(event)));
}

void PersistenceManager::InsertJobResumedEvent(JobResumedEvent event)
{
    RunAsync(std::bind([](PersistenceManagerDBProvider &dbProvider, const JobResumedEvent &event) {
        dbProvider.InsertEvent(event);
    }, std::ref(dbProvider), std::move(event)));
}

void PersistenceManager::InsertJobSubmittedEvent(JobSubmittedEvent event)
{
    RunAsync(
        std::bind([](PersistenceManagerDBProvider &dbProvider, const JobSubmittedEvent &event) {
            dbProvider.InsertEvent(event);
        }, std::ref(dbProvider), std::move(event)));
}

UnprocessedEventList PersistenceManager::GetNewEvents()
{
    RunAsync([this] { return dbProvider.GetNewEvents(); });

    return {};
}

void PersistenceManager::MarkEventProcessingStarted(int eventId)
{
    RunAsync([this, eventId] { return dbProvider.MarkEventProcessingStarted(eventId); });
}

void PersistenceManager::MarkEventProcessingComplete(int eventId)
{
    RunAsync([this, eventId] { return dbProvider.MarkEventProcessingComplete(eventId); });
}

void PersistenceManager::InsertNodeStatistics(NodeStatistics statistics)
{
    RunAsync(
        std::bind([](PersistenceManagerDBProvider &dbProvider, const NodeStatistics &statistics) {
            dbProvider.InsertNodeStatistics(statistics);
        }, std::ref(dbProvider), std::move(statistics)));
}

QString PersistenceManager::GetDashboardData(QDate since)
{
    RunAsync(std::bind([](PersistenceManagerDBProvider &dbProvider, const QDate &since) {
        return dbProvider.GetDashboardData(since);
    }, std::ref(dbProvider), std::move(since)));

    return {};
}
