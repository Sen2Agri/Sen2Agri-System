#include <utility>

#include <QDBusMetaType>
#include <QJsonDocument>
#include <QJsonObject>

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

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationParameter &parameter)
{
    argument.beginStructure();
    argument << parameter.key << parameter.value;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ConfigurationParameter &parameter)
{
    argument.beginStructure();
    argument >> parameter.key >> parameter.value;
    argument.endStructure();

    return argument;
}

QString toJson(const ConfigurationParameterList &parameters)
{
    QJsonObject node;
    for (const auto &p : parameters) {
        node[p.key] = p.value;
    }
    return QString::fromUtf8(QJsonDocument(node).toJson());
}
