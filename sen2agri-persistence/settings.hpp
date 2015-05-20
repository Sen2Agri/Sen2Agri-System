#pragma once

#include <QString>

class Settings
{
public:
    Settings(QString hostName, QString databaseName, QString userName, QString password);

    QString hostName;
    QString databaseName;
    QString userName;
    QString password;

    static Settings readSettings(const QString &path);
};
