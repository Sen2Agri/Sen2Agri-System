#include <stdio.h>
#include <syslog.h>

#include "logger.hpp"

void Logger::initialize()
{
    openlog(NULL, LOG_CONS | LOG_PID, LOG_DAEMON);
}

void Logger::info(const QString &str)
{
    const auto &msg = str.toLocal8Bit();
    fprintf(stderr, "%s\n", msg.constData());
    syslog(LOG_INFO, "%s", msg.constData());
}

void Logger::warn(const QString &str)
{
    const auto &msg = str.toLocal8Bit();
    fprintf(stderr, "%s\n", msg.constData());
    syslog(LOG_WARNING, "%s", msg.constData());
}

void Logger::error(const QString &str)
{
    const auto &msg = str.toLocal8Bit();
    fprintf(stderr, "%s\n", msg.constData());
    syslog(LOG_ERR, "%s", msg.constData());
}

void Logger::fatal(const QString &str)
{
    const auto &msg = str.toLocal8Bit();
    fprintf(stderr, "%s\n", msg.constData());
    syslog(LOG_CRIT, "%s", msg.constData());
}
