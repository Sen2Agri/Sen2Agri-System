#include "executioncontextbase.hpp"

ExecutionContextBase::ExecutionContextBase(PersistenceManagerDBProvider &persistenceManager)
    :persistenceManager(persistenceManager)
{
}

ProductList ExecutionContextBase::GetProducts(int siteId, int productTypeId, const QDateTime &startDate, const QDateTime &endDate)
{
    //QDateTime startDateTime = QDateTime::fromString(startDate, "yyyyMMdd");
    //QDateTime endDateTime = QDateTime::fromString(endDate, "yyyyMMdd");
    return persistenceManager.GetProducts(siteId, productTypeId, startDate, endDate);
}

L1CProductList ExecutionContextBase::GetL1CProducts(int siteId, const SatellitesList &satelliteIds, const StatusesList &statusIds,
                                                    const QDateTime &startDate, const QDateTime &endDate)
{
    return persistenceManager.GetL1CProducts(siteId, satelliteIds, statusIds, startDate, endDate);
}

QString ExecutionContextBase::GetProcessorShortName(int processorId)
{
    return persistenceManager.GetProcessorShortName(processorId);
}

QString ExecutionContextBase::GetSiteShortName(int siteId) {
    return persistenceManager.GetSiteShortName(siteId);
}

QString ExecutionContextBase::GetSiteName(int siteId)
{
    return persistenceManager.GetSiteName(siteId);
}

SeasonList ExecutionContextBase::GetSiteSeasons(int siteId)
{
    return persistenceManager.GetSiteSeasons(siteId);
}
