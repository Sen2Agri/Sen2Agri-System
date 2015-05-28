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
    for (const auto &p : this->configuration.parameterValues) {
        values[{ p.key, p.siteId }] = p.value;
    }

    originalValues = values;
}

bool ConfigModel::isSiteSpecific(const ParameterKey &parameter) const
{
    return values.find(parameter) != end(values);
}

QString ConfigModel::getValue(const ParameterKey &parameter, bool &fromGlobal) const
{
    auto it = values.find(parameter);
    if (it != end(values)) {
        fromGlobal = false;
        return it->second;
    } else {
        fromGlobal = true;
        return getGlobalValue(parameter);
    }
}

QString ConfigModel::getGlobalValue(const ParameterKey &parameter) const
{
    // should always be present
    return values.find({ parameter.key(), std::experimental::nullopt })->second;
}

void ConfigModel::setValue(const ParameterKey &parameter, const QString &value)
{
    values[parameter] = value;
    //    const auto &t = std::make_tuple(siteId, key);
    //    if (originalValues[t] != value) {
    //        newValues[t] = value;
    //    } else {
    //        newValues.erase(t);
    //    }
}

void ConfigModel::removeValue(const ParameterKey &parameter)
{
    values.erase(parameter);
}

ConfigurationParameterValueList ConfigModel::getChanges() const
{
    ConfigurationParameterValueList result;

    // TODO
    for (const auto &p : values) {
        result.append({ p.first.key(), p.first.siteId(), p.second });
    }

    return result;
}

bool ConfigModel::hasChanges() const
{
    // TODO
    return true;
}

ConfigurationUpdateActionList ConfigModel::getUpdateActions() const
{
    ConfigurationUpdateActionList result;

    for (const auto &p : values) {
        auto it = originalValues.find(p.first);
        if (p.second != it->second) {
            result.append({ p.first.key(), p.first.siteId(), p.second, false });
        }
    }

    auto valuesEnd = std::end(values);
    for (const auto &p : originalValues) {
        auto it = values.find(p.first);
        if (it == valuesEnd) {
            result.append({ p.first.key(), p.first.siteId(), QString(), true });
        }
    }

    return result;
}

const ConfigurationCategoryList &ConfigModel::categories() const
{
    return configuration.categories;
}

const SiteList &ConfigModel::sites() const
{
    return configuration.sites;
}

const ConfigurationParameterInfoList &ConfigModel::parameters() const
{
    return configuration.parameterInfo;
}
