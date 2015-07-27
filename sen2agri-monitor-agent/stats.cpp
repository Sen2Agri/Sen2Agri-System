#include <utility>

#include <unistd.h>
#include <sys/statvfs.h>

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QJsonObject>
#include <QThread>

#include "logger.hpp"
#include "stats.hpp"

class MemInfo
{
public:
    uint64_t memTotalKb;
    uint64_t memUsedKb;
    uint64_t swapTotalKb;
    uint64_t swapUsedKb;

    MemInfo();
    MemInfo(uint64_t memTotalKb, uint64_t memUsedKb, uint64_t swapTotalKb, uint64_t swapUsedKb);
};

MemInfo::MemInfo() : memTotalKb(), memUsedKb(), swapTotalKb(), swapUsedKb()
{
}

MemInfo::MemInfo(uint64_t memTotalKb, uint64_t memUsedKb, uint64_t swapTotalKb, uint64_t swapUsedKb)
    : memTotalKb(memTotalKb), memUsedKb(memUsedKb), swapTotalKb(swapTotalKb), swapUsedKb(swapUsedKb)
{
}

class CpuInfo
{
public:
    double loadAvg1;
    double loadAvg5;
    double loadAvg15;

    CpuInfo();
    CpuInfo(double loadAvg1, double loadAvg5, double loadAvg15);
};

CpuInfo::CpuInfo()
{
}

CpuInfo::CpuInfo(double loadAvg1, double loadAvg5, double loadAvg15)
    : loadAvg1(loadAvg1), loadAvg5(loadAvg5), loadAvg15(loadAvg15)
{
}

class DiskInfo
{
public:
    uint64_t diskTotalBytes;
    uint64_t diskUsedBytes;

    DiskInfo();
    DiskInfo(uint64_t diskTotalBytes, uint64_t diskUsedBytes);
};

DiskInfo::DiskInfo() : diskTotalBytes(), diskUsedBytes()
{
}

DiskInfo::DiskInfo(uint64_t totalDiskBytes, uint64_t usedDiskBytes)
    : diskTotalBytes(totalDiskBytes), diskUsedBytes(usedDiskBytes)
{
}

class SystemInfo
{
public:
    QString hostname;
    double cpuUser;
    double cpuSystem;

    SystemInfo();
    SystemInfo(QString hostname, double cpuUser, double cpuSystem);
};

SystemInfo::SystemInfo() : cpuUser(), cpuSystem()
{
}

SystemInfo::SystemInfo(QString hostname, double cpuUser, double cpuSystem)
    : hostname(std::move(hostname)), cpuUser(cpuUser), cpuSystem(cpuSystem)
{
}

MemInfo getMemInfo()
{
    QFile file(QStringLiteral("/proc/meminfo"));
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error(file.errorString().toStdString());
    }

    QByteArray contents = file.readAll();
    file.close();

    uint64_t memTotal = 0;
    uint64_t memFree = 0;
    uint64_t buffers = 0;
    uint64_t cached = 0;
    uint64_t swapTotal = 0;
    uint64_t swapFree = 0;

    static QRegularExpression rex(QStringLiteral("(\\d+)"));
    QRegularExpression re(rex);

    QTextStream in(&contents);
    while (!in.atEnd()) {
        const auto &line = in.readLine();
        if (line.startsWith(QStringLiteral("MemTotal:"))) {
            memTotal = re.match(line).captured(1).toULongLong();
        } else if (line.startsWith(QStringLiteral("MemFree:"))) {
            memFree = re.match(line).captured(1).toULongLong();
        } else if (line.startsWith(QStringLiteral("Buffers:"))) {
            buffers = re.match(line).captured(1).toULongLong();
        } else if (line.startsWith(QStringLiteral("Cached:"))) {
            cached = re.match(line).captured(1).toULongLong();
        } else if (line.startsWith(QStringLiteral("SwapTotal:"))) {
            swapTotal = re.match(line).captured(1).toULongLong();
        } else if (line.startsWith(QStringLiteral("SwapFree:"))) {
            swapFree = re.match(line).captured(1).toULongLong();
        }
    }

    auto memUsed = memTotal - memFree - buffers - cached;
    auto swapUsed = swapTotal - swapFree;

    return { memTotal, memUsed, swapTotal, swapUsed };
}

CpuInfo getCpuInfo()
{
    QFile file(QStringLiteral("/proc/loadavg"));
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error(file.errorString().toStdString());
    }

    auto contents = file.readAll();
    file.close();

    QTextStream in(&contents);
    const auto &line = in.readLine();

    static QRegularExpression re(QStringLiteral("([0-9.]+) ([0-9.]+) ([0-9.]+)"));
    const auto &mi = QRegularExpression(re).match(line);
    return { mi.captured(1).toDouble(), mi.captured(2).toDouble(), mi.captured(3).toDouble() };
}

DiskInfo getDiskInfo(const QString &path)
{
    struct statvfs buf;

    if (statvfs(path.toLocal8Bit().constData(), &buf) < 0) {
        throw std::runtime_error(strerror(errno));
    }

    return { buf.f_bsize * buf.f_blocks, buf.f_bsize * (buf.f_blocks - buf.f_bfree) };
}

QList<int64_t> getCpuStats()
{
    QFile file(QStringLiteral("/proc/stat"));
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error(file.errorString().toStdString());
    }

    auto contents = file.readAll();
    file.close();

    QTextStream in(&contents);

    const auto &values = in.readLine().split(' ', QString::SkipEmptyParts);

    QList<int64_t> result;
    auto sz = values.size();
    result.reserve(sz - 1);
    for (int i = 1; i < sz; i++) {
        result.push_back(values.at(i).toLongLong());
    }

    return result;
}

struct CpuUsage {
    int64_t user;
    int64_t system;
    int64_t total;
};

CpuUsage getCpuUsage()
{
    // cpu user nice system idle iowait irq softirq steal guest guest_nice
    // compute total
    // compute tuser as user + nice
    // compute tsystem as total - user - nice - idle
    const auto &values = getCpuStats();

    int64_t total = 0;
    for (auto val : values) {
        total += val;
    }

    auto user = values[0];
    auto nice = values[1];
    auto idle = values[3];

    auto tuser = user + nice;
    auto tsystem = total - tuser - idle;

    return { tuser, tsystem, total };
}

SystemInfo getSystemInfo()
{
    char buf[HOST_NAME_MAX];
    if (gethostname(buf, sizeof(buf)) < 0) {
        throw std::runtime_error("Unable to determine the hostname");
    }
    buf[sizeof(buf) - 1] = 0;

    auto cpuUsageOld = getCpuUsage();
    QThread::sleep(1);
    auto cpuUsageNew = getCpuUsage();

    // compute total_diff, user_diff, system_diff
    // compute user_cpu as user_diff / total_diff
    auto diff = CpuUsage{ cpuUsageNew.user - cpuUsageOld.user,
                          cpuUsageNew.system - cpuUsageOld.system,
                          cpuUsageNew.total - cpuUsageOld.total };

    double cpuUser = static_cast<double>(diff.user) / diff.total;
    double cpuSystem = static_cast<double>(diff.system) / diff.total;

    return { buf, cpuUser, cpuSystem };
}

QJsonDocument getStatsJson(const QString &diskPath) noexcept
{
    SystemInfo systemInfo;
    try {
        systemInfo = getSystemInfo();
    } catch (const std::runtime_error &e) {
        Logger::error(e.what());

        systemInfo.hostname = QStringLiteral("UNKNOWN");
    }

    MemInfo memInfo;
    try {
        memInfo = getMemInfo();
    } catch (const std::runtime_error &e) {
        Logger::error(e.what());
    }

    CpuInfo cpuInfo;
    try {
        cpuInfo = getCpuInfo();
    } catch (const std::runtime_error &e) {
        Logger::error(e.what());
    }

    DiskInfo diskInfo;
    try {
        diskInfo = getDiskInfo(diskPath);
    } catch (const std::runtime_error &e) {
        Logger::error(e.what());
    }

    QJsonObject obj;
    obj[QStringLiteral("hostname")] = systemInfo.hostname;
    obj[QStringLiteral("cpu_user")] = systemInfo.cpuUser;
    obj[QStringLiteral("cpu_system")] = systemInfo.cpuSystem;

    obj[QStringLiteral("mem_total_kb")] = static_cast<double>(memInfo.memTotalKb);
    obj[QStringLiteral("mem_used_kb")] = static_cast<double>(memInfo.memUsedKb);
    obj[QStringLiteral("swap_total_kb")] = static_cast<double>(memInfo.swapTotalKb);
    obj[QStringLiteral("swap_used_kb")] = static_cast<double>(memInfo.swapUsedKb);

    obj[QStringLiteral("load_avg_1m")] = cpuInfo.loadAvg1;
    obj[QStringLiteral("load_avg_5m")] = cpuInfo.loadAvg5;
    obj[QStringLiteral("load_avg_15m")] = cpuInfo.loadAvg15;

    obj[QStringLiteral("disk_total_bytes")] = static_cast<double>(diskInfo.diskTotalBytes);
    obj[QStringLiteral("disk_used_bytes")] = static_cast<double>(diskInfo.diskUsedBytes);

    return QJsonDocument(obj);
}
