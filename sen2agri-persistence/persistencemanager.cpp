#include <functional>

#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <limits>
#include <unistd.h>

#include <QtSql>
#include <QDebug>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QThreadPool>

#include "persistencemanager.hpp"
#include "make_unique.hpp"

static QString getSystemErrorMessage(int err)
{
    char buf[1024];
    return strerror_r(err, buf, sizeof(buf));
}

static long getSysConf(int name, long fallback)
{
    auto value = sysconf(name);

    if (value <= 0) {
        value = fallback;
    }

    return value;
}

static bool getPasswordEntry(uid_t uid, passwd &pwd)
{
    passwd *result;

    auto buflen = getSysConf(_SC_GETPW_R_SIZE_MAX, 16384);
    auto buf(std::make_unique<char[]>(buflen));

    if (auto r = getpwuid_r(uid, &pwd, buf.get(), buflen, &result)) {
        throw std::runtime_error(QStringLiteral("Unable to get user information: %1")
                                     .arg(getSystemErrorMessage(r))
                                     .toStdString());
    }

    return result != nullptr;
}

static bool getGroupEntry(const char *name, group grp)
{
    group *result;

    auto buflen = getSysConf(_SC_GETGR_R_SIZE_MAX, 16384);
    auto buf(std::make_unique<char[]>(buflen));

    if (auto r = getgrnam_r(name, &grp, buf.get(), buflen, &result)) {
        throw std::runtime_error(QStringLiteral("Unable to get group information: %1")
                                     .arg(getSystemErrorMessage(r))
                                     .toStdString());
    }

    return result != nullptr;
}

static bool isUserAdmin(uid_t uid)
{
    // TODO what to do with this?
    static const char adminGroupName[] = "sen2agri-admin";

    // don't check the group membership if the caller is root
    if (!uid) {
        return true;
    }

    passwd pwd;
    if (!getPasswordEntry(uid, pwd)) {
        return false;
    }

    group grp;
    if (!getGroupEntry(adminGroupName, grp)) {
        return false;
    }

    int ngroups = getSysConf(_SC_NGROUPS_MAX, NGROUPS_MAX);
    auto groups(std::make_unique<gid_t[]>(ngroups));

    if (getgrouplist(pwd.pw_name, pwd.pw_gid, groups.get(), &ngroups) > 0) {
        for (int i = 0; i < ngroups; i++) {
            if (groups[i] == grp.gr_gid) {
                return true;
            }
        }
    }

    return false;
}

PersistenceManager::PersistenceManager(const Settings &settings, QObject *parent)
    : QObject(parent), dbProvider(settings)
{
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
        return isUserAdmin(reply.value());
    } catch (const std::runtime_error &e) {
        Logger::error(e.what());
        return false;
    }
}

ConfigurationSet PersistenceManager::GetConfigurationSet()
{
    auto isCallerAdmin = IsCallerAdmin();

    RunAsync([=]() {
        ConfigurationSet configuration = dbProvider.GetConfigurationSet();
        configuration.isAdmin = isCallerAdmin;

        return configuration;
    });

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
    auto isCallerAdmin = IsCallerAdmin();

    RunAsync([=]() { return dbProvider.UpdateConfigurationParameters(actions, isCallerAdmin); });

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

int PersistenceManager::SubmitTask(const NewTask &task)
{
    RunAsync([=]() { return dbProvider.SubmitTask(task); });

    return {};
}

void PersistenceManager::SubmitSteps(const NewStepList &steps)
{
    RunAsyncNoResult([=]() { dbProvider.SubmitSteps(steps); });
}

void PersistenceManager::MarkStepStarted(int taskId, const QString &name)
{
    RunAsyncNoResult([=]() { return dbProvider.MarkStepStarted(taskId, name); });
}

void PersistenceManager::MarkStepFinished(int taskId,
                                          const QString &name,
                                          const ExecutionStatistics &statistics)
{
    RunAsyncNoResult([=]() { return dbProvider.MarkStepFinished(taskId, name, statistics); });
}

void PersistenceManager::MarkJobFinished(int jobId)
{
    RunAsyncNoResult([=]() { return dbProvider.MarkJobFinished(jobId); });
}
