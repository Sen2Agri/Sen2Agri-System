#include <iterator>
#include <utility>

#include <QJsonObject>
#include <QVariantMap>

#include "configmodel.hpp"

using std::begin;
using std::end;
using std::move;

ConfigModel::ConfigModel()
{
}

ConfigModel::ConfigModel(ConfigurationSet configuration) : configuration(move(configuration))
{
    for (const auto &p : parameters()) {
        originalValues[p.key] = p.value;
    }
}

void ConfigModel::setValue(const QString &key, const QString &value)
{
    if (originalValues[key] != value) {
        newValues[key] = value;
    } else {
        newValues.remove(key);
    }
}

ConfigurationParameterValueList ConfigModel::getChanges() const
{
    ConfigurationParameterValueList result;

    auto endNewValues = end(newValues);
    for (auto it = begin(newValues); it != endNewValues; ++it) {
        result.append({ it.key(), it.value() });
    }

    return result;
}
const ConfigurationCategoryList &ConfigModel::categories() const
{
    return configuration.categories;
}

const ConfigurationParameterInfoList &ConfigModel::parameters() const
{
    return configuration.parameters;
}
