#include <stdio.h>
#include <syslog.h>

#include "logger.hpp"

void Logger::initialize()
{
    openlog(NULL, LOG_CONS | LOG_PID, LOG_DAEMON);
}

void Logger::info(const QString &str)
{
    info(str.toLocal8Bit().constData());
}

void Logger::warn(const QString &str)
{
    warn(str.toLocal8Bit().constData());
}

void Logger::error(const QString &str)
{
    error(str.toLocal8Bit().constData());
}

void Logger::fatal(const QString &str)
{
    fatal(str.toLocal8Bit().constData());
}

void Logger::info(const char *str)
{
    fprintf(stderr, "%s\n", str);
    syslog(LOG_INFO, "%s", str);
}

void Logger::warn(const char *str)
{
    fprintf(stderr, "%s\n", str);
    syslog(LOG_WARNING, "%s", str);
}

void Logger::error(const char *str)
{
    fprintf(stderr, "%s\n", str);
    syslog(LOG_ERR, "%s", str);
}

void Logger::fatal(const char *str)
{
    fprintf(stderr, "%s\n", str);
    syslog(LOG_CRIT, "%s", str);
}
