#ifndef SCHEDULINGCONTEXT_H
#define SCHEDULINGCONTEXT_H

#include "persistencemanager.hpp"
#include "model.hpp"

class SchedulingContext
{
public:
    SchedulingContext(PersistenceManagerDBProvider &persistenceManager);
    ProductList GetProducts(int siteId, int productTypeId, const QDateTime &startDate, const QDateTime &endDate);
    ProductList GetProductsByInsertedTime(int siteId, int productTypeId, const QDateTime &startDate, const QDateTime &endDate);
    ConfigurationParameterValueMap GetConfigurationParameters(const QString &prefix, int siteId, const ConfigurationParameterValueMap &overrideValues);
    std::map<QString, QString> GetConfigurationParameterValues(const QString &prefix);
    QString GetSiteName(int siteId);
    QString GetSiteShortName(int siteId);
    QString GetProcessorShortName(int processorId);

    SeasonList GetSiteSeasons(int siteId);

private:
    PersistenceManagerDBProvider &persistenceManager;
};

#endif // SCHEDULINGCONTEXT_H
