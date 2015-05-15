#pragma once

#include <QString>
#include <QMap>
#include <QDate>

#include "configurationparameter.hpp"

class ConfigModel
{
    ConfigurationSet configuration;
    QMap<QString, QString> originalValues;
    QMap<QString, QString> newValues;

public:
    ConfigModel();
    ConfigModel(ConfigurationSet configuration);

    void setValue(const QString &key, const QString &value);
    ConfigurationParameterValueList getChanges() const;

    const ConfigurationCategoryList &categories() const;
    const ConfigurationParameterInfoList &parameters() const;
};
