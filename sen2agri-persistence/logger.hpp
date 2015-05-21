#pragma once

#include <QString>

class Logger
{
public:
    static void initialize();

    static void info(const QString &str);
    static void warn(const QString &str);
    static void error(const QString &str);
    static void fatal(const QString &str);

    static void info(const char *str);
    static void warn(const char *str);
    static void error(const char *str);
    static void fatal(const char *str);
};
