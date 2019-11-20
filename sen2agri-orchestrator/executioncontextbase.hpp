#ifndef EXECUTIONCONTEXTBASE_H
#define EXECUTIONCONTEXTBASE_H

#include "persistencemanager.hpp"
#include "model.hpp"

class ExecutionContextBase
{
public:
    ExecutionContextBase(PersistenceManagerDBProvider &persistenceManager);
    ProductList GetProducts(int siteId, int productTypeId, const QDateTime &startDate, const QDateTime &endDate);
    QString GetSiteName(int siteId);
    QString GetSiteShortName(int siteId);
    QString GetProcessorShortName(int processorId);

    SeasonList GetSiteSeasons(int siteId);

protected:
    PersistenceManagerDBProvider &persistenceManager;
};

#endif // EXECUTIONCONTEXTBASE_H
