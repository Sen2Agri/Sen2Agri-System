#pragma once

#include <map>
#include <tuple>

#include <QString>
#include <QDate>

#include "configurationparameter.hpp"
#include "parameterkey.hpp"

class ConfigModel
{
    ConfigurationSet configuration;
    std::map<ParameterKey, QString> originalValues;
    std::map<ParameterKey, QString> values;

public:
    ConfigModel();
    ConfigModel(ConfigurationSet configuration);

    bool isSiteSpecific(const ParameterKey &parameter) const;
    QString getValue(const ParameterKey &parameter, bool &fromGlobal) const;
    QString getGlobalValue(const ParameterKey &parameter) const;

    void
    setValue(const ParameterKey &parameter, const QString &value);
    void removeValue(const ParameterKey &parameter);

    ConfigurationParameterValueList getChanges() const;
    bool hasChanges() const;
    ConfigurationUpdateActionList getUpdateActions() const;

    const ConfigurationCategoryList &categories() const;
    const SiteList &sites() const;
    const ConfigurationParameterInfoList &parameters() const;
};
