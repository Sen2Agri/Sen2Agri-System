#ifndef SCHEDULINGCONTEXT_H
#define SCHEDULINGCONTEXT_H

#include "executioncontextbase.hpp"

class SchedulingContext : public ExecutionContextBase
{
public:
    SchedulingContext(PersistenceManagerDBProvider &persistenceManager);
    ProductList GetProductsByInsertedTime(int siteId, int productTypeId, const QDateTime &startDate, const QDateTime &endDate);
    ConfigurationParameterValueMap GetConfigurationParameters(const QString &prefix, int siteId, const ConfigurationParameterValueMap &overrideValues);
    std::map<QString, QString> GetConfigurationParameterValues(const QString &prefix);
};

#endif // SCHEDULINGCONTEXT_H
