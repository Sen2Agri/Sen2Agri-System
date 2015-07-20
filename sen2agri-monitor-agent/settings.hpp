#pragma once

#include <QString>

class Settings
{
public:
    Settings(QString serviceUrl);

    QString serviceUrl;

    static Settings readSettings(const QString &path);
};
