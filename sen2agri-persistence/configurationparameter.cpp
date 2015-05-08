#include <utility>

#include <QDBusMetaType>

#include "configurationparameter.hpp"

using std::move;

ConfigurationParameter::ConfigurationParameter()
{
}

ConfigurationParameter::ConfigurationParameter(QString key, QString value)
    : key(move(key)), value(move(value))
{
}

void ConfigurationParameter::registerMetaTypes()
{
    qRegisterMetaType<ConfigurationParameter>("ConfigurationParameter");
    qRegisterMetaType<ConfigurationParameterList>("ConfigurationParameterList");

    qDBusRegisterMetaType<ConfigurationParameter>();
    qDBusRegisterMetaType<ConfigurationParameterList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationParameter &message)
{
    argument.beginStructure();
    argument << message.key << message.value;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ConfigurationParameter &message)
{
    argument.beginStructure();
    argument >> message.key >> message.value;
    argument.endStructure();

    return argument;
}
