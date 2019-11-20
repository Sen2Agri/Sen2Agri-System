#include "schedulingcontext.h"

SchedulingContext::SchedulingContext(PersistenceManagerDBProvider &persistenceManager)
    :ExecutionContextBase(persistenceManager)
{
}

ProductList SchedulingContext::GetProductsByInsertedTime(int siteId, int productTypeId, const QDateTime &startDate, const QDateTime &endDate)
{
    return persistenceManager.GetProductsByInsertedTime(siteId, productTypeId, startDate, endDate);
}

ConfigurationParameterValueMap SchedulingContext::GetConfigurationParameters(const QString &prefix, int siteId,
                                                                             const ConfigurationParameterValueMap &overrideValues)
{
    QMap<QString, ConfigurationParameterValue> retMap;
    const ConfigurationParameterValueList & cfgList = persistenceManager.GetConfigurationParameters(prefix);
    ConfigurationParameterValueList defaultValues;
    for(const ConfigurationParameterValue &value : cfgList) {
        // a siteId with value less than or equals with 0 means the default values
        // if we have (the received siteId AND the value siteId less than or equal with 0) OR
        //    we have (the received and value site id equal)
        if(((siteId <= 0) && (value.siteId <= 0)) || (value.siteId == siteId)) {
            if(overrideValues.contains(value.key)) {
                retMap[value.key] = overrideValues[value.key];
            } else {
                retMap[value.key] = value;
            }
        }
        if(value.siteId <= 0) {
            defaultValues.append(value);
        }
    }
    // now check if we have the keys for the default values. If not, add them
    for(const ConfigurationParameterValue &defValue: defaultValues) {
        if(!retMap.contains(defValue.key)) {
            retMap[defValue.key] = defValue;
        }
    }
    return retMap;
}

std::map<QString, QString> SchedulingContext::GetConfigurationParameterValues(const QString &prefix)
{
    const ConfigurationParameterValueList & cfgList = persistenceManager.GetConfigurationParameters(prefix);
    std::map<QString, QString> result;
    for (const auto &p : cfgList) {
        result.emplace(p.key, p.value);
    }

    return result;

}
