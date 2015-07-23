#include <QSettings>

#include "settings.hpp"

Settings::Settings(QString serviceUrl) : serviceUrl(std::move(serviceUrl))
{
}

Settings Settings::readSettings(const QString &path)
{
    QSettings settings(path, QSettings::IniFormat);

    return { settings.value("Monitor/ServiceUrl").toString() };
}
