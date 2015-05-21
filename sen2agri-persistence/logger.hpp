#include <QString>

class Logger
{
public:
    static void initialize();

    static void info(const QString &str);
    static void warn(const QString &str);
    static void error(const QString &str);
    static void fatal(const QString &str);
};
