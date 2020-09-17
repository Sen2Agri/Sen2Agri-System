#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "lairetrievalhandler_l3c.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"
#include "logger.hpp"

bool compareL3BProductDates(const QString& path1,const QString& path2)
{
    QFileInfo fileInfo1(path1);
    QString filename1(fileInfo1.fileName());
    QFileInfo fileInfo2(path2);
    QString filename2(fileInfo2.fileName());

    QDateTime minDate1, maxDate1;
    ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(filename1, minDate1, maxDate1);
    QDateTime minDate2, maxDate2;
    ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(filename2, minDate2, maxDate2);
    if(minDate1 == minDate2) {
        return QString::compare(filename1, filename2, Qt::CaseInsensitive);
    }
    return (minDate1 < minDate2);
}

QStringList LaiRetrievalHandlerL3C::GetSpecificReprocessingArgs(const std::map<QString, QString> &configParameters)
{
    const auto &localWindowBwr = ProcessorHandlerHelper::GetMapValue(configParameters, GetProcessorDBPrefix() + "localwnd.bwr");
    const auto &localWindowFwr = ProcessorHandlerHelper::GetMapValue(configParameters, GetProcessorDBPrefix() + "localwnd.fwr");
    QStringList specificArgs = {"-algo", "local",
                                "-algo.local.bwr", localWindowBwr,
                                "-algo.local.fwr", localWindowFwr};
    return specificArgs;
}

ProductType LaiRetrievalHandlerL3C::GetOutputProductType()
{
    return ProductType::L3CProductTypeId;
}

QString LaiRetrievalHandlerL3C::GetOutputProductShortName()
{
    return "L3C";
}

void LaiRetrievalHandlerL3C::WriteExecutionSpecificParamsValues(const std::map<QString, QString> &configParameters,
                                                                std::ofstream &stream)
{
    // Get the parameters from the configuration
    const auto &bwr = ProcessorHandlerHelper::GetMapValue(configParameters, GetProcessorDBPrefix() + "localwnd.bwr");
    const auto &fwr = ProcessorHandlerHelper::GetMapValue(configParameters, GetProcessorDBPrefix() + "localwnd.fwr");
    stream << "  <ProfileReprocessing_parameters>" << std::endl;
    stream << "    <bwr_for_algo_local_online_retrieval>" << bwr.toStdString() << "</bwr_for_algo_local_online_retrieval>" << std::endl;
    stream << "    <fwr_for_algo_local_online_retrieval>"<< fwr.toStdString() <<"</fwr_for_algo_local_online_retrieval>" << std::endl;
    stream << "  </ProfileReprocessing_parameters>" << std::endl;
}

QString LaiRetrievalHandlerL3C::GetPrdFormatterRasterFlagName()
{
    return "-processor.vegetation.filelaireproc";
}

QString LaiRetrievalHandlerL3C::GetPrdFormatterMskFlagName()
{
    return "-processor.vegetation.filelaireprocflgs";
}

QList<QMap<QString, TileTemporalFilesInfo>> LaiRetrievalHandlerL3C::ExtractL3BMapTiles(EventProcessingContext &ctx,
                                                   const JobSubmittedEvent &event,
                                                   const QStringList &listL3BProducts,
                                                   const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles)
{
    const std::map<QString, QString> &configParameters = ctx.GetJobConfigurationParameters(event.jobId, GetProcessorDBPrefix());
    const auto &localWindowBwr = ProcessorHandlerHelper::GetMapValue(configParameters, GetProcessorDBPrefix() + "localwnd.bwr");
    const auto &localWindowFwr = ProcessorHandlerHelper::GetMapValue(configParameters, GetProcessorDBPrefix() + "localwnd.fwr");

    int limitL3BPrdsPerTile = localWindowBwr.toInt() + localWindowFwr.toInt() + 1;

    QList<QMap<QString, TileTemporalFilesInfo>> retList;
    const QStringList &allL3BProductsList = GetL3BProductsSinceStartOfSeason(ctx, event.siteId, listL3BProducts);
    for(const QString &l3bProd: listL3BProducts) {
        const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles = GetL3BMapTiles(ctx, l3bProd, allL3BProductsList,
                                                                                 siteTiles, limitL3BPrdsPerTile);
        retList.append(l3bMapTiles);
    }
    return retList;
}

ProductList LaiRetrievalHandlerL3C::GetScheduledJobProductList(SchedulingContext &ctx, int siteId, const QDateTime &seasonStartDate,
                                                               const QDateTime &seasonEndDate, const QDateTime &qScheduledDate,
                                                               const ConfigurationParameterValueMap &requestOverrideCfgValues)
{
    QDateTime startDate = seasonStartDate;
    QDateTime endDate = qScheduledDate;
    const ConfigurationParameterValueMap &mapCfg = ctx.GetConfigurationParameters(GetProcessorDBPrefix(), siteId, requestOverrideCfgValues);

    int productionInterval = 30;
    const QString &reprocIntKey = GetProcessorDBPrefix() + "reproc_production_interval";
    if(requestOverrideCfgValues.contains(reprocIntKey)) {
        productionInterval = mapCfg[reprocIntKey].value.toInt();
    }
    Logger::debug(QStringLiteral("Scheduler: production interval for site ID %1: %2")
                  .arg(siteId)
                  .arg(productionInterval));
    startDate = endDate.addDays(-productionInterval);
    Logger::debug(QStringLiteral("Scheduler: start date after production interval for site ID %1: %2")
                  .arg(siteId)
                  .arg(startDate.toString()));

    // Use only the products after the configured start season date
    if(startDate < seasonStartDate) {
        startDate = seasonStartDate;
        Logger::debug(QStringLiteral("Scheduler: start date after correction to season start date for site ID %1: %2")
                      .arg(siteId)
                      .arg(startDate.toString()));
    }
    ProductList productList;
    bool bOnceExecution = false;
    if(requestOverrideCfgValues.contains("task_repeat_type")) {
        const ConfigurationParameterValue &repeatType = requestOverrideCfgValues["task_repeat_type"];
        if (repeatType.value == "0") {
            bOnceExecution = true;
        }
    }
    const QDateTime &curDateTime = QDateTime::currentDateTime();
    if (curDateTime > seasonEndDate || bOnceExecution) {
        // processing of a past season, that was already finished
        productList = ctx.GetProducts(siteId, (int)ProductType::L3BProductTypeId, startDate, endDate);
    } else {
        // processing of a season in progress, we get the products inserted in the last interval since the last scheduling
        productList = ctx.GetProductsByInsertedTime(siteId, (int)ProductType::L3BProductTypeId, startDate, endDate);
    }
    return productList;
}

bool LaiRetrievalHandlerL3C::AcceptSchedJobProduct(const QString &, ProcessorHandlerHelper::SatelliteIdType satId)
{
    return (satId == ProcessorHandlerHelper::SATELLITE_ID_TYPE_S2);
}

/**
 * Get all L3B products from the beginning of the season until now
 */
//TODO: This function should receive the Product and QList<Product> instead of just product path as these can be got from DB
//      The Product contains already the tiles, the full path and the acquisition date so can be avoided parsing files
QStringList LaiRetrievalHandlerL3C::GetL3BProductsSinceStartOfSeason(EventProcessingContext &ctx, int siteId,
                                                                     const QStringList &listExistingPrds)
{
    // extract the start and end dates
    const QDate &startSeasonDate = GetSiteFirstSeasonStartDate(ctx, siteId);
    const QDateTime &startDateTime = QDateTime(startSeasonDate);
    const QDateTime &lastPrdsTime = GetL3BLastAcqDate(listExistingPrds);

    // Get all products since the start of the first season
    const ProductList &prdsList = ctx.GetProducts(siteId, (int)ProductType::L3BProductTypeId, startDateTime, lastPrdsTime);
    QStringList retList;
    for(const Product &prd: prdsList) {
        retList.append(prd.fullPath);
    }

    // sort ascending the list according to the acquisition time and the name if dates equal
    qSort(retList.begin(), retList.end(), compareL3BProductDates);

    return retList;
}

//TODO: This function should receive the Product and QList<Product> instead of just product path as these can be got from DB
//      The Product contains already the tiles, the full path and the acquisition date so can be avoided parsing files
QMap<QString, TileTemporalFilesInfo> LaiRetrievalHandlerL3C::GetL3BMapTiles(EventProcessingContext &ctx, const QString &newestL3BProd,
                                                                            const QStringList &l3bProducts,
                                                                            const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles,
                                                                            int limitL3BPrdsPerTile)
{
    QMap<QString, TileTemporalFilesInfo> retL3bMapTiles;
    const QStringList &listNewestL3BProdTiles = ProcessorHandlerHelper::GetTileIdsFromHighLevelProduct(newestL3BProd);
    if (listNewestL3BProdTiles.size() == 0) {
        Logger::debug(QStringLiteral("No tiles ID found for product %1").arg(newestL3BProd));
        return retL3bMapTiles;
    }

    QDateTime minDate, maxDate;
    ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(newestL3BProd, minDate, maxDate);
    ProcessorHandlerHelper::SatelliteIdType tileSatId = ProcessorHandlerHelper::SATELLITE_ID_TYPE_UNKNOWN;

    // iterate the tiles of the newest L3B product
    for(const auto &tileId : listNewestL3BProdTiles) {
        // we assume that all the tiles from the product are from the same satellite
        // in this case, we get only once the satellite Id for all tiles
        if(tileSatId == ProcessorHandlerHelper::SATELLITE_ID_TYPE_UNKNOWN) {
            tileSatId = GetSatIdForTile(siteTiles, tileId);
            // ignore tiles for which the satellite id cannot be determined
            if(tileSatId == ProcessorHandlerHelper::SATELLITE_ID_TYPE_UNKNOWN) {
                Logger::debug(QStringLiteral("The satellite ID cannot be extracted for tileId %1 (nb. site tiles is %2)").arg(tileId).arg(siteTiles.size()));
                continue;
            }
        }

        // add the new tile info if missing
        if(!retL3bMapTiles.contains(tileId)) {
            TileTemporalFilesInfo newTileInfos;
            newTileInfos.tileId = tileId;
            newTileInfos.primarySatelliteId = tileSatId;
            newTileInfos.uniqueSatteliteIds.append(tileSatId);

            // Fill the tile information for the current tile from the current product
            AddTileFileInfo(ctx, newTileInfos, newestL3BProd, tileId, siteTiles, tileSatId, minDate);

            // add the tile infos to the map
            retL3bMapTiles[tileId] = newTileInfos;
            Logger::debug(QStringLiteral("Added tile id %1 from product %2").arg(tileId).arg(newestL3BProd));
        }
        TileTemporalFilesInfo &tileInfo = retL3bMapTiles[tileId];
        // NOTE: we assume the products are sorted ascending
        for(int i = l3bProducts.size(); i --> 0; ) {
            const QString &l3bPrd = l3bProducts[i];
            // If we have a limit of maximum temporal products per tile and we reached this limit,
            // then ignore the other products for this tile
            if(HasSufficientProducts(tileInfo, tileSatId, limitL3BPrdsPerTile)) {
                break;
            }

            // check if the current product date is greater than the one of the reference product
            // if so, ignore it
            QDateTime curPrdMinDate, curPrdMaxDate;
            ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(l3bPrd, curPrdMinDate, curPrdMaxDate);
            if(curPrdMinDate.date() >= minDate.date()) {
                continue;
            }
            // Fill the tile information for the current tile from the current product
            AddTileFileInfo(ctx, tileInfo, l3bPrd, tileId, siteTiles, tileSatId, curPrdMinDate);
        }

        Logger::debug(QStringLiteral("Using for tile %1 a number of %2 tiles").arg(tileId).arg(tileInfo.temporalTilesFileInfos.size()));
        if(tileInfo.temporalTilesFileInfos.size() > 0) {
             // update the primary satellite information
             tileInfo.primarySatelliteId = ProcessorHandlerHelper::GetPrimarySatelliteId(tileInfo.uniqueSatteliteIds);
        }
    }
    // if primary and secondary satellites, then keep only the tiles from primary satellite
    // as we don't want to have in the resulted product combined primary and secondary satellites tiles
    return retL3bMapTiles;
    // NOT NEEDED ANYMORE - filtering done in scheduled part
    //return FilterSecondaryProductTiles(retL3bMapTiles, siteTiles);
}

QDateTime LaiRetrievalHandlerL3C::GetL3BLastAcqDate(const QStringList &listL3bPrds)
{
    QDateTime curDate = QDateTime();
    for (const QString &prd: listL3bPrds) {
        QDateTime minDate;
        QDateTime maxDate;
        if (ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prd, minDate, maxDate)) {
            if (!curDate.isValid() || (curDate.isValid() && curDate < maxDate)) {
                curDate = maxDate;
            }
        }
    }
    return curDate;
}

QDate LaiRetrievalHandlerL3C::GetSiteFirstSeasonStartDate(EventProcessingContext &ctx,int siteId)
{
    const SeasonList &seasons = ctx.GetSiteSeasons(siteId);
    if (seasons.size() > 0) {
        return seasons[0].startDate;
    }
    return QDate();
}

bool LaiRetrievalHandlerL3C::HasSufficientProducts(const TileTemporalFilesInfo &tileInfo,
                                                   const ProcessorHandlerHelper::SatelliteIdType &tileSatId,
                                                   int limitL3BPrdsPerTile)
{
    // TODO: Should we consider here also the orbit???
    if(limitL3BPrdsPerTile > 0) {
        int cntSameSat = 0;
        // We must have at least limitL3BPrdsPerTile for the primary satellite
        // As we do not add in the temp info file the intersecting tiles of the same satellite
        // it is OK to count the occurences of the tile infos having the main satellite
        // as this means it is the same tile ID
        for(const ProcessorHandlerHelper::InfoTileFile &tempInfoFile: tileInfo.temporalTilesFileInfos) {
            if(tempInfoFile.satId == tileSatId) {
                cntSameSat++;
                if(cntSameSat > limitL3BPrdsPerTile) {
                    return true;
                }
            }
        }
    }
    return false;
}


