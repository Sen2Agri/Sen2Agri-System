#include <utility>

#include <QDBusMetaType>
#include <QJsonDocument>
#include <QJsonObject>

#include "configurationparameter.hpp"

using std::move;

ConfigurationParameterInfo::ConfigurationParameterInfo()
{
}

ConfigurationParameterInfo::ConfigurationParameterInfo(QString key,
                                                       int categoryId,
                                                       QString friendlyName,
                                                       QString dataType,
                                                       QString value,
                                                       bool isAdvanced)
    : key(move(key)),
      categoryId(categoryId),
      friendlyName(move(friendlyName)),
      dataType(dataType),
      value(move(value)),
      isAdvanced(isAdvanced)
{
}

void ConfigurationParameterInfo::registerMetaTypes()
{
    qRegisterMetaType<ConfigurationParameterInfo>("ConfigurationParameterInfo");
    qRegisterMetaType<ConfigurationParameterInfoList>("ConfigurationParameterInfoList");

    qDBusRegisterMetaType<ConfigurationParameterInfo>();
    qDBusRegisterMetaType<ConfigurationParameterInfoList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationParameterInfo &parameterInfo)
{
    argument.beginStructure();
    argument << parameterInfo.key << parameterInfo.categoryId << parameterInfo.friendlyName
             << parameterInfo.dataType << parameterInfo.value << parameterInfo.isAdvanced;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                ConfigurationParameterInfo &parameterInfo)
{
    argument.beginStructure();
    argument >> parameterInfo.key >> parameterInfo.categoryId >> parameterInfo.friendlyName >>
        parameterInfo.dataType >> parameterInfo.value >> parameterInfo.isAdvanced;
    argument.endStructure();

    return argument;
}

ConfigurationParameterValue::ConfigurationParameterValue()
{
}

ConfigurationParameterValue::ConfigurationParameterValue(QString key, QString value)
    : key(move(key)), value(move(value))
{
}

void ConfigurationParameterValue::registerMetaTypes()
{
    qRegisterMetaType<ConfigurationParameterValue>("ConfigurationParameterValue");
    qRegisterMetaType<ConfigurationParameterValueList>("ConfigurationParameterValueList");

    qDBusRegisterMetaType<ConfigurationParameterValue>();
    qDBusRegisterMetaType<ConfigurationParameterValueList>();
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const ConfigurationParameterValue &parameterValue)
{
    argument.beginStructure();
    argument << parameterValue.key << parameterValue.value;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                ConfigurationParameterValue &parameterValue)
{
    argument.beginStructure();
    argument >> parameterValue.key >> parameterValue.value;
    argument.endStructure();

    return argument;
}

QString toJson(const ConfigurationParameterValueList &parameters)
{
    QJsonObject node;
    for (const auto &p : parameters) {
        node[p.key] = p.value;
    }
    return QString::fromUtf8(QJsonDocument(node).toJson());
}

ConfigurationCategory::ConfigurationCategory()
{
}

ConfigurationCategory::ConfigurationCategory(int categoryId, QString name)
    : categoryId(move(categoryId)), name(move(name))
{
}

void ConfigurationCategory::registerMetaTypes()
{
    qRegisterMetaType<ConfigurationCategory>("ConfigurationCategory");
    qRegisterMetaType<ConfigurationCategoryList>("ConfigurationList");

    qDBusRegisterMetaType<ConfigurationCategory>();
    qDBusRegisterMetaType<ConfigurationCategoryList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationCategory &category)
{
    argument.beginStructure();
    argument << category.categoryId << category.name;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ConfigurationCategory &category)
{
    argument.beginStructure();
    argument >> category.categoryId >> category.name;
    argument.endStructure();

    return argument;
}

ConfigurationSet::ConfigurationSet()
{
}

ConfigurationSet::ConfigurationSet(ConfigurationCategoryList categories,
                                   ConfigurationParameterInfoList parameters)
    : categories(move(categories)), parameters(move(parameters))
{
}

void ConfigurationSet::registerMetaTypes()
{
    qRegisterMetaType<ConfigurationSet>("ConfigurationSet");

    qDBusRegisterMetaType<ConfigurationSet>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationSet &configuration)
{
    argument.beginStructure();
    argument << configuration.categories << configuration.parameters;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ConfigurationSet &configuration)
{
    argument.beginStructure();
    argument >> configuration.categories >> configuration.parameters;
    argument.endStructure();

    return argument;
}
