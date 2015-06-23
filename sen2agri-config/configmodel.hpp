#pragma once

#include <map>
#include <tuple>

#include <QString>
#include <QDate>

#include "model.hpp"
#include "parameterkey.hpp"

class ConfigModel
{
    ConfigurationSet configuration;
    std::map<int, QString> siteMap;
    std::map<ParameterKey, QString> originalValues;
    std::map<ParameterKey, QString> values;

public:
    ConfigModel();
    ConfigModel(ConfigurationSet configuration);

    bool isAdmin() const;

    bool isSiteSpecific(const ParameterKey &parameter) const;
    QString getValue(const ParameterKey &parameter, bool &fromGlobal) const;
    QString getGlobalValue(const ParameterKey &parameter) const;
    QString getSiteName(std::experimental::optional<int> siteId) const;

    void
    setValue(const ParameterKey &parameter, const QString &value);
    void removeValue(const ParameterKey &parameter);
    void reset();

    bool hasChanges() const;
    ConfigurationUpdateActionList getChanges() const;

    const ConfigurationCategoryList &categories() const;
    const SiteList &sites() const;
    const ConfigurationParameterInfoList &parameters() const;
};
