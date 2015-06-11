#include <functional>

#include <QtSql>
#include <QDebug>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QThreadPool>

#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include "persistencemanager.hpp"
#include "make_unique.hpp"

// TODO error logging
static gid_t getGroupId(const char *name)
{
    group grp;
    group *result;

    auto buflen = sysconf(_SC_GETGR_R_SIZE_MAX);
    if (buflen == -1) {
        buflen = 16384;
    }

    auto buf(std::make_unique<char[]>(buflen));

    if (!getgrnam_r(name, &grp, buf.get(), buflen, &result)) {
        return grp.gr_gid;
    }

    return 0;
}

// TODO error logging
static bool isUserAdmin(uid_t uid)
{
    passwd pwd;
    passwd *result;

    auto buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (buflen == -1) {
        buflen = 16384;
    }

    auto buf(std::make_unique<char[]>(buflen));

    if (!getpwuid_r(uid, &pwd, buf.get(), buflen, &result)) {
        int ngroups = sysconf(_SC_NGROUPS_MAX);
        auto groups(std::make_unique<gid_t>(buflen));

        auto adminGroupId = getGroupId("sen2agri-admin");

        if (getgrouplist(pwd.pw_name, pwd.pw_gid, groups.get(), &ngroups) > 0) {
            for (int i = 0; i < ngroups; i++) {
                if (groups.get()[i] == adminGroupId) {
                    return true;
                }
            }
        }
    }

    return false;
}

PersistenceManager::PersistenceManager(const Settings &settings, QObject *parent)
    : QObject(parent), dbProvider(settings)
{
}

ConfigurationSet PersistenceManager::GetConfigurationSet()
{
    const auto &reply = connection().interface()->serviceUid(message().service());
    if (!reply.isValid()) {
    }

    auto isAdmin = isUserAdmin(reply.value());

    RunAsync([=]() { return dbProvider.GetConfigurationSet(isAdmin); });

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
