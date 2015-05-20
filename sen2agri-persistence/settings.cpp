#include <QSettings>

#include "settings.hpp"

Settings::Settings(QString hostName, QString databaseName, QString userName, QString password)
    : hostName(std::move(hostName)),
      databaseName(std::move(databaseName)),
      userName(std::move(userName)),
      password(std::move(password))
{
}

Settings Settings::readSettings(const QString &path)
{
    QSettings settings(path, QSettings::IniFormat);

    return { settings.value("Database/HostName").toString(),
             settings.value("Database/DatabaseName").toString(),
             settings.value("Database/UserName").toString(),
             settings.value("Database/Password").toString() };
}

QString getSettingsFilePath()
{
    return QStringLiteral("/etc/sen2agri.conf");
}
