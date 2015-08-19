#include <iterator>
#include <utility>

#include <QJsonObject>
#include <QVariantMap>

#include "configmodel.hpp"

using std::begin;
using std::end;
using std::move;

ConfigModel::ConfigModel() {}

ConfigModel::ConfigModel(ConfigurationSet configuration) : configuration(move(configuration))
{
    for (const auto &p : this->configuration.parameterValues) {
        values[{ p.key, p.siteId }] = p.value;
    }

    for (const auto &s : this->configuration.sites) {
        siteMap.emplace(s.siteId, s.name);
    }

    originalValues = values;
}

bool ConfigModel::isAdmin() const { return configuration.isAdmin; }

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

QString ConfigModel::getSiteName(std::experimental::optional<int> siteId) const
{
    if (!siteId) {
        return QStringLiteral("Global");
    }

    return siteMap.find(siteId.value())->second;
}

void ConfigModel::setValue(const ParameterKey &parameter, const QString &value)
{
    values[parameter] = value;
}

void ConfigModel::removeValue(const ParameterKey &parameter) { values.erase(parameter); }

void ConfigModel::reset() { originalValues = values; }

bool ConfigModel::hasChanges() const { return !getChanges().empty(); }

ConfigurationUpdateActionList ConfigModel::getChanges() const
{
    ConfigurationUpdateActionList result;

    auto originalValuesEnd = std::end(originalValues);
    for (const auto &p : values) {
        auto it = originalValues.find(p.first);
        if (it == originalValuesEnd || p.second != it->second) {
            result.append({ p.first.key(), p.first.siteId(), p.second });
        }
    }

    auto valuesEnd = std::end(values);
    for (const auto &p : originalValues) {
        auto it = values.find(p.first);
        if (it == valuesEnd) {
            result.append({ p.first.key(), p.first.siteId(), std::experimental::nullopt });
        }
    }

    return result;
}

const ConfigurationCategoryList &ConfigModel::categories() const
{
    return configuration.categories;
}

const SiteList &ConfigModel::sites() const { return configuration.sites; }

const ConfigurationParameterInfoList &ConfigModel::parameters() const
{
    return configuration.parameterInfo;
}

const ConfigurationSet &ConfigModel::configurationSet() const { return configuration; }
