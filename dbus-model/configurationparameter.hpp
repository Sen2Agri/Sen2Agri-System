#pragma once

#include <QString>
#include <QDBusArgument>
#include <QMetaType>

#include "../Optional/optional.hpp"

class ConfigurationParameterInfo
{
public:
    QString key;
    int categoryId;
    std::experimental::optional<int> siteId;
    QString friendlyName;
    QString dataType;
    QString value;
    bool isAdvanced;

    ConfigurationParameterInfo();
    ConfigurationParameterInfo(QString key,
                               int categoryId,
                               std::experimental::optional<int> siteId,
                               QString friendlyName,
                               QString dataType,
                               QString value,
                               bool isAdvanced);

    static void registerMetaTypes();
};

typedef QList<ConfigurationParameterInfo> ConfigurationParameterInfoList;

Q_DECLARE_METATYPE(ConfigurationParameterInfo)
Q_DECLARE_METATYPE(ConfigurationParameterInfoList)

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationParameterInfo &parameter);
const QDBusArgument &operator>>(const QDBusArgument &argument,
                                ConfigurationParameterInfo &parameter);

class ConfigurationParameterValue
{
public:
    QString key;
    QString value;

    ConfigurationParameterValue();
    ConfigurationParameterValue(QString key, QString value);

    static void registerMetaTypes();
};

typedef QList<ConfigurationParameterValue> ConfigurationParameterValueList;

Q_DECLARE_METATYPE(ConfigurationParameterValue)
Q_DECLARE_METATYPE(ConfigurationParameterValueList)

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationParameterValue &parameter);
const QDBusArgument &operator>>(const QDBusArgument &argument,
                                ConfigurationParameterValue &parameter);

class ConfigurationCategory
{
public:
    int categoryId;
    QString name;

    ConfigurationCategory();
    ConfigurationCategory(int categoryId, QString name);

    static void registerMetaTypes();
};

typedef QList<ConfigurationCategory> ConfigurationCategoryList;

Q_DECLARE_METATYPE(ConfigurationCategory);
Q_DECLARE_METATYPE(ConfigurationCategoryList);

class ConfigurationSet
{
public:
    ConfigurationCategoryList categories;
    ConfigurationParameterInfoList parameters;

    ConfigurationSet();
    ConfigurationSet(ConfigurationCategoryList categories,
                     ConfigurationParameterInfoList parameters);

    static void registerMetaTypes();
};

Q_DECLARE_METATYPE(ConfigurationSet);

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationSet &parameter);
const QDBusArgument &operator>>(const QDBusArgument &argument, ConfigurationSet &parameter);
