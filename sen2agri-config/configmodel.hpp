#pragma once

#include <QString>
#include <QMap>
#include <QDate>

#include "configurationparameter.hpp"

class ConfigModel
{
    ConfigurationSet configuration;

public:
    ConfigModel();
    ConfigModel(ConfigurationSet configuration);

    const ConfigurationCategoryList &categories() const;
    const ConfigurationParameterInfoList &parameters() const;
};
