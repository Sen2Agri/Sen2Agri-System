#include "schedulingcontext.h"

SchedulingContext::SchedulingContext(PersistenceManagerDBProvider &persistenceManager)
    :persistenceManager(persistenceManager)
{
}

ProductList SchedulingContext::GetProducts(int siteId, int productTypeId, const QDateTime &startDate, const QDateTime &endDate)
{
    //QDateTime startDateTime = QDateTime::fromString(startDate, "yyyyMMdd");
    //QDateTime endDateTime = QDateTime::fromString(endDate, "yyyyMMdd");
    return persistenceManager.GetProducts(siteId, productTypeId, startDate, endDate);
}

ConfigurationParameterValueMap SchedulingContext::GetConfigurationParameters(const QString &prefix, int siteId,
                                                                             const ConfigurationParameterValueMap &overrideValues)
{
    QMap<QString, ConfigurationParameterValue> retMap;
    const ConfigurationParameterValueList & cfgList = persistenceManager.GetConfigurationParameters(prefix);
    for(const ConfigurationParameterValue &value : cfgList) {
        if((siteId <= 0) || (value.siteId <= 0) || (value.siteId == siteId)) {
            if(overrideValues.contains(value.key)) {
                retMap[value.key] = overrideValues[value.key];
            } else {
                retMap[value.key] = value;
            }
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

QString SchedulingContext::GetProcessorShortName(int processorId)
{
    return persistenceManager.GetProcessorShortName(processorId);
}

QString SchedulingContext::GetSiteShortName(int siteId) {
    return persistenceManager.GetSiteShortName(siteId);
}

QString SchedulingContext::GetSiteName(int siteId)
{
    return persistenceManager.GetSiteName(siteId);
}


