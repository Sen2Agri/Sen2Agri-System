#pragma once

#include <QString>
#include <QDBusArgument>
#include <QMetaType>

class ConfigurationParameter
{
public:
    QString key;
    QString value;

    ConfigurationParameter();
    ConfigurationParameter(QString key, QString value);

    static void registerMetaTypes();
};

typedef QList<ConfigurationParameter> ConfigurationParameterList;

Q_DECLARE_METATYPE(ConfigurationParameter)
Q_DECLARE_METATYPE(ConfigurationParameterList)

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationParameter &parameter);
const QDBusArgument &operator>>(const QDBusArgument &argument, ConfigurationParameter &parameter);

QString toJson(const ConfigurationParameterList &parameters);
