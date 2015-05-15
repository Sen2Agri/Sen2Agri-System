#include <utility>

#include <QJsonObject>
#include <QVariantMap>

#include "configmodel.hpp"

using std::move;

ConfigModel::ConfigModel()
{
}

ConfigModel::ConfigModel(ConfigurationSet configuration) : configuration(move(configuration))
{
}

const ConfigurationCategoryList &ConfigModel::categories() const
{
    return configuration.categories;
}

const ConfigurationParameterInfoList &ConfigModel::parameters() const
{
    return configuration.parameters;
}
