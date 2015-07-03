#include <stdexcept>
#include <memory>
#include <make_unique.hpp>
#include <QString>

#include <sys/types.h>
#include <grp.h>
#include <limits>
#include <unistd.h>

#include "credential_utils.hpp"

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

bool isUserInGroup(uid_t uid, const char *groupName)
{
    // don't check the group membership if the caller is root
    if (!uid) {
        return true;
    }

    auto pwdBufLen = getSysConf(_SC_GETPW_R_SIZE_MAX, 16384);
    auto pwdBuf(std::make_unique<char[]>(pwdBufLen));

    passwd pwd;
    passwd *pwdResult;
    if (auto r = getpwuid_r(uid, &pwd, pwdBuf.get(), pwdBufLen, &pwdResult)) {
        throw std::runtime_error(QStringLiteral("Unable to get user information: %1")
                                     .arg(getSystemErrorMessage(r))
                                     .toStdString());
    }

    if (!pwdResult) {
        return false;
    }

    auto grpBufLen = getSysConf(_SC_GETGR_R_SIZE_MAX, 16384);
    auto grpBuf(std::make_unique<char[]>(grpBufLen));

    group grp;
    group *grpResult;
    if (auto r = getgrnam_r(groupName, &grp, grpBuf.get(), grpBufLen, &grpResult)) {
        throw std::runtime_error(QStringLiteral("Unable to get group information: %1")
                                     .arg(getSystemErrorMessage(r))
                                     .toStdString());
    }

    if (!grpResult) {
        return false;
    }

    // getgrouplist() wants an int
    auto ngroupsl = getSysConf(_SC_NGROUPS_MAX, NGROUPS_MAX);
    Q_ASSERT(ngroupsl >= 0);
    Q_ASSERT(ngroupsl <= std::numeric_limits<int>::max());

    auto ngroups = static_cast<int>(ngroupsl);
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
