#include <utility>

#include <QDBusMetaType>
#include <QJsonDocument>
#include <QJsonObject>

#include "configurationparameter.hpp"

using std::move;

ConfigurationParameterInfo::ConfigurationParameterInfo()
{
}

ConfigurationParameterInfo::ConfigurationParameterInfo(
    QString key, int categoryId, QString friendlyName, QString dataType, bool isAdvanced)
    : key(move(key)),
      categoryId(categoryId),
      friendlyName(move(friendlyName)),
      dataType(dataType),
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
             << parameterInfo.dataType << parameterInfo.isAdvanced;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                ConfigurationParameterInfo &parameterInfo)
{
    argument.beginStructure();
    argument >> parameterInfo.key >> parameterInfo.categoryId >> parameterInfo.friendlyName >>
        parameterInfo.dataType >> parameterInfo.isAdvanced;
    argument.endStructure();

    return argument;
}

ConfigurationParameterValue::ConfigurationParameterValue()
{
}

ConfigurationParameterValue::ConfigurationParameterValue(QString key,
                                                         std::experimental::optional<int> siteId,
                                                         QString value)
    : key(move(key)), siteId(siteId), value(move(value))
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
    argument << parameterValue.key << parameterValue.siteId.value_or(0) << parameterValue.value;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                ConfigurationParameterValue &parameterValue)
{
    int siteId;

    argument.beginStructure();
    argument >> parameterValue.key >> siteId >> parameterValue.value;
    argument.endStructure();

    if (siteId) {
        parameterValue.siteId = siteId;
    } else {
        parameterValue.siteId = std::experimental::nullopt;
    }

    return argument;
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

Site::Site()
{
}

Site::Site(int siteId, QString name) : siteId(siteId), name(std::move(name))
{
}

void Site::registerMetaTypes()
{
    qRegisterMetaType<Site>("Site");
    qRegisterMetaType<SiteList>("SiteList");

    qDBusRegisterMetaType<Site>();
    qDBusRegisterMetaType<SiteList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const Site &site)
{
    argument.beginStructure();
    argument << site.siteId << site.name;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Site &site)
{
    argument.beginStructure();
    argument >> site.siteId >> site.name;
    argument.endStructure();

    return argument;
}

ConfigurationSet::ConfigurationSet()
{
}

ConfigurationSet::ConfigurationSet(ConfigurationCategoryList categories,
                                   ConfigurationParameterInfoList parameterInfo,
                                   ConfigurationParameterValueList parameterValues,
                                   SiteList sites)
    : categories(std::move(categories)),
      parameterInfo(std::move(parameterInfo)),
      parameterValues(std::move(parameterValues)),
      sites(std::move(sites))
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
    argument << configuration.categories << configuration.parameterInfo
             << configuration.parameterValues << configuration.sites;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ConfigurationSet &configuration)
{
    argument.beginStructure();
    argument >> configuration.categories >> configuration.parameterInfo >>
        configuration.parameterValues >> configuration.sites;
    argument.endStructure();

    return argument;
}

ConfigurationUpdateAction::ConfigurationUpdateAction()
{
}

ConfigurationUpdateAction::ConfigurationUpdateAction(QString key,
                                                     std::experimental::optional<int> siteId,
                                                     std::experimental::optional<QString> value)
    : key(std::move(key)), siteId(siteId), value(std::move(value))
{
}

void ConfigurationUpdateAction::registerMetaTypes()
{
    qRegisterMetaType<ConfigurationUpdateAction>("ConfigurationUpdateAction");
    qRegisterMetaType<ConfigurationUpdateActionList>("ConfigurationUpdateActionList");

    qDBusRegisterMetaType<ConfigurationUpdateAction>();
    qDBusRegisterMetaType<ConfigurationUpdateActionList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationUpdateAction &action)
{
    argument.beginStructure();
    argument << action.key << action.siteId.value_or(0);
    if (action.value) {
        argument << true << action.value.value();
    } else {
        argument << false << QString();
    }
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ConfigurationUpdateAction &action)
{
    int siteId;
    bool hasValue;
    QString value;

    argument.beginStructure();
    argument >> action.key >> siteId >> hasValue >> value;
    argument.endStructure();

    if (siteId) {
        action.siteId = siteId;
    } else {
        action.siteId = std::experimental::nullopt;
    }

    if (hasValue) {
        action.value = value;
    } else {
        action.value = std::experimental::nullopt;
    }

    return argument;
}
