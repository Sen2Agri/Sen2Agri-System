#include "processorhandler.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>

#include "schedulingcontext.h"
#include "logger.hpp"

bool removeDir(const QString & dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}

bool compareProductDates(const QString& path1,const QString& path2)
{
  QDateTime dtProd1=ProcessorHandlerHelper::GetL2AProductDateFromPath(path1);
  QDateTime dtProd2=ProcessorHandlerHelper::GetL2AProductDateFromPath(path2);
  return (dtProd1 < dtProd2);
}

ProcessorHandler::~ProcessorHandler() {}

void ProcessorHandler::HandleProductAvailable(EventProcessingContext &ctx,
                                              const ProductAvailableEvent &event)
{
    HandleProductAvailableImpl(ctx, event);
}

void ProcessorHandler::HandleJobSubmitted(EventProcessingContext &ctx,
                                          const JobSubmittedEvent &event)
{
    HandleJobSubmittedImpl(ctx, event);
}

void ProcessorHandler::HandleTaskFinished(EventProcessingContext &ctx,
                                          const TaskFinishedEvent &event)
{
    HandleTaskFinishedImpl(ctx, event);
}

ProcessorJobDefinitionParams ProcessorHandler::GetProcessingDefinition(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                        const ConfigurationParameterValueMap &requestOverrideCfgValues) {
    return GetProcessingDefinitionImpl(ctx, siteId, scheduledDate, requestOverrideCfgValues);
}

void ProcessorHandler::HandleProductAvailableImpl(EventProcessingContext &,
                                                  const ProductAvailableEvent &)
{
}

QString ProcessorHandler::GetFinalProductFolder(EventProcessingContext &ctx, int jobId,
                                                int siteId) {
    auto configParameters = ctx.GetJobConfigurationParameters(jobId, PRODUCTS_LOCATION_CFG_KEY);
    QString siteName = ctx.GetSiteShortName(siteId);

    return GetFinalProductFolder(configParameters, siteName, processorDescr.shortName);
}

QString ProcessorHandler::GetFinalProductFolder(EventProcessingContext &ctx, int jobId, int siteId,
                                                const QString &productName) {
    auto configParameters = ctx.GetJobConfigurationParameters(jobId, PRODUCTS_LOCATION_CFG_KEY);
    QString siteName = ctx.GetSiteShortName(siteId);

    return GetFinalProductFolder(configParameters, siteName, productName);
}

QString ProcessorHandler::GetFinalProductFolder(const std::map<QString, QString> &cfgKeys, const QString &siteName,
                                                const QString &processorName) {
    auto it = cfgKeys.find(PRODUCTS_LOCATION_CFG_KEY);
    if (it == std::end(cfgKeys)) {
        throw std::runtime_error(QStringLiteral("No final product folder configured for site %1 and processor %2")
                                     .arg(siteName)
                                     .arg(processorName)
                                     .toStdString());
    }
    QString folderName = (*it).second;
    folderName = folderName.replace("{site}", siteName);
    folderName = folderName.replace("{processor}", processorName);

    return folderName;
}

bool ProcessorHandler::RemoveJobFolder(EventProcessingContext &ctx, int jobId, const QString &procName)
{
    QString strKey = "executor.processor." + procName + ".keep_job_folders";
    auto configParameters = ctx.GetJobConfigurationParameters(jobId, strKey);
    auto keepStr = configParameters[strKey];
    bool bRemove = true;
    if(keepStr == "1") bRemove = false;
    if(bRemove) {
        QString jobOutputPath = ctx.GetJobOutputPath(jobId);
        return removeDir(jobOutputPath);
    }
    return true;
}

QString ProcessorHandler::GetProductFormatterOutputProductPath(EventProcessingContext &ctx,
                                                        const TaskFinishedEvent &event) {
    QString prodFolderOutPath = ctx.GetOutputPath(event.jobId, event.taskId, event.module) +
            "/" + PRODUCT_FORMATTER_OUT_PROPS_FILE;
    QStringList fileLines = ProcessorHandlerHelper::GetTextFileLines(prodFolderOutPath);
    if(fileLines.size() > 0) {
        return fileLines[0].trimmed();
    }
    return "";
}

QString ProcessorHandler::GetProductFormatterProductName(EventProcessingContext &ctx,
                                                        const TaskFinishedEvent &event) {
    QString productPath = GetProductFormatterOutputProductPath(ctx, event);
    if(productPath.length() > 0) {
        QString name = ProcessorHandlerHelper::GetFileNameFromPath(productPath);
        if(name.trimmed() != "") {
            return name;
        }
    }
    return "";
}

QString ProcessorHandler::GetProductFormatterQuicklook(EventProcessingContext &ctx,
                                                        const TaskFinishedEvent &event) {
    QString prodFolderOutPath = ctx.GetOutputPath(event.jobId, event.taskId, event.module) +
            "/" + PRODUCT_FORMATTER_OUT_PROPS_FILE;
    QStringList fileLines = ProcessorHandlerHelper::GetTextFileLines(prodFolderOutPath);
    QString quickLookName("");
    if(fileLines.size() > 0) {
        const QString &mainFolderName = fileLines[0];
        QString legacyFolder = mainFolderName;      // + "/LEGACY_DATA/";

        QDirIterator it(legacyFolder, QStringList() << "*.jpg", QDir::Files);
        // get the last shape file found
        QString quickLookFullName;
        while (it.hasNext()) {
            quickLookFullName = it.next();
            QFileInfo quickLookFileInfo(quickLookFullName);
            QString quickLookTmpName = quickLookFileInfo.fileName();
            if(quickLookTmpName.indexOf("_PVI_")) {
                return quickLookTmpName;
            }

        }
    }
    return quickLookName;
}

QString ProcessorHandler::GetProductFormatterFootprint(EventProcessingContext &ctx,
                                                        const TaskFinishedEvent &event) {
    QString prodFolderOutPath = ctx.GetOutputPath(event.jobId, event.taskId, event.module) +
            "/" + PRODUCT_FORMATTER_OUT_PROPS_FILE;
    QStringList fileLines = ProcessorHandlerHelper::GetTextFileLines(prodFolderOutPath);
    if(fileLines.size() > 0) {
        const QString &mainFolderName = fileLines[0];
        QString legacyFolder = mainFolderName + "/LEGACY_DATA/";

        QDirIterator it(legacyFolder, QStringList() << "*.xml", QDir::Files);
        // get the last shape file found
        QString footprintFullName;
        while (it.hasNext()) {
            footprintFullName = it.next();
            // check only the name if it contains MTD
            QFileInfo footprintFileInfo(footprintFullName);
            if(footprintFileInfo.fileName().indexOf("_MTD_") > 0) {
                // parse the XML file
                QFile inputFile(footprintFullName);
                if (inputFile.open(QIODevice::ReadOnly))
                {
                   QTextStream in(&inputFile);
                   while (!in.atEnd()) {
                        QString curLine = in.readLine();
                        // we assume we have only one line
                        QString startTag("<EXT_POS_LIST>");
                        int extposlistStartIdx = curLine.indexOf(startTag);
                        if(extposlistStartIdx >= 0) {
                            int extposlistEndIdxIdx = curLine.indexOf("</EXT_POS_LIST>");
                            if(extposlistEndIdxIdx >= 0) {
                                int startIdx = extposlistStartIdx + startTag.length();
                                QString extensionPointsStr = curLine.mid(startIdx,extposlistEndIdxIdx-startIdx);
                                QStringList extPointsList = extensionPointsStr.split(" ");
                                if((extPointsList.size() > 8) && (extPointsList.size() % 2) == 0) {
                                    QString footprint = "POLYGON((";
                                    for(int i = 0; i<extPointsList.size(); i++) {
                                        if(i > 0)
                                            footprint.append(", ");
                                        footprint.append(extPointsList[i]);

                                        footprint.append(" ");
                                        footprint.append(extPointsList[++i]);
                                    }
                                    footprint += "))";
                                    inputFile.close();
                                    return footprint;
                                }
                            }
                        }
                   }
                   inputFile.close();
                }
            }

        }
    }
    return "POLYGON((0.0 0.0, 0.0 0.0, 0.0 0.0, 0.0 0.0, 0.0 0.0))";
}

bool ProcessorHandler::GetSeasonStartEndDates(SchedulingContext &ctx, int siteId,
                                              QDateTime &startTime, QDateTime &endTime,
                                              const ConfigurationParameterValueMap &requestOverrideCfgValues) {
    QDate currentDate = QDate::currentDate();
    int curYear = currentDate.year();

    ConfigurationParameterValueMap seasonCfgValues = ctx.GetConfigurationParameters("downloader.", siteId, requestOverrideCfgValues);
    QDate startSummerSeasonDate = QDate::fromString(seasonCfgValues["downloader.summer-season.start"].value, "MMdd").addYears(curYear-1900);
    QDate endSummerSeasonDate = QDate::fromString(seasonCfgValues["downloader.summer-season.end"].value, "MMdd").addYears(curYear-1900);
    QDate startWinterSeasonDate = QDate::fromString(seasonCfgValues["downloader.winter-season.start"].value, "MMdd").addYears(curYear-1900);
    QDate endWinterSeasonDate = QDate::fromString(seasonCfgValues["downloader.winter-season.end"].value, "MMdd").addYears(curYear-1900);
    if(startSummerSeasonDate.isValid() && endSummerSeasonDate.isValid()) {
        // normally this should not happen for summer season but can happen for winter season
        if(endSummerSeasonDate < startSummerSeasonDate) {
            endSummerSeasonDate.addYears(-1);
        }
        if(currentDate >= startSummerSeasonDate && currentDate <= endSummerSeasonDate) {
            startTime = QDateTime(startSummerSeasonDate);
            endTime = QDateTime(endSummerSeasonDate);
            return true;
        }
    }
    if(startWinterSeasonDate.isValid() && endWinterSeasonDate.isValid()) {
        // this can happen for winter season
        if(endWinterSeasonDate < startWinterSeasonDate) {
            endWinterSeasonDate.addYears(-1);
        }

        if(currentDate >= startWinterSeasonDate && currentDate <= endWinterSeasonDate) {
            startTime = QDateTime(startWinterSeasonDate);
            endTime = QDateTime(endWinterSeasonDate);
            return true;
        }
    }

    // check the default values
    seasonCfgValues = ctx.GetConfigurationParameters("downloader.", -1, requestOverrideCfgValues);
    QDate defStartSummerSeasonDate = QDate::fromString(seasonCfgValues["downloader.summer-season.start"].value, "MMdd").addYears(curYear-1900);
    QDate defEndSummerSeasonDate = QDate::fromString(seasonCfgValues["downloader.summer-season.end"].value, "MMdd").addYears(curYear-1900);
    QDate defStartWinterSeasonDate = QDate::fromString(seasonCfgValues["downloader.winter-season.start"].value, "MMdd").addYears(curYear-1900);
    QDate defEndWinterSeasonDate = QDate::fromString(seasonCfgValues["downloader.winter-season.end"].value, "MMdd").addYears(curYear-1900);

    if(defStartSummerSeasonDate.isValid() && defEndSummerSeasonDate.isValid()) {
        // normally this should not happen for summer season but can happen for winter season
        if(defEndSummerSeasonDate < defStartSummerSeasonDate) {
            defEndSummerSeasonDate.addYears(-1);
        }

        if(currentDate >= defStartSummerSeasonDate && currentDate <= defEndSummerSeasonDate) {
            startTime = QDateTime(defStartSummerSeasonDate);
            endTime = QDateTime(defEndSummerSeasonDate);
            return true;
        }
    }
    if(defStartWinterSeasonDate.isValid() && defEndWinterSeasonDate.isValid()) {
        // this can happen for winter season
        if(defEndWinterSeasonDate < defStartWinterSeasonDate) {
            defEndWinterSeasonDate.addYears(-1);
        }

        if(currentDate >= defStartWinterSeasonDate && currentDate <= defEndWinterSeasonDate) {
            startTime = QDateTime(defStartWinterSeasonDate);
            endTime = QDateTime(defEndWinterSeasonDate);
            return true;
        }
    }
    return false;
}

QStringList ProcessorHandler::GetL2AInputProductsTiles(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event,
                                QMap<QString, QStringList> &mapProductToTilesMetaFiles) {
    QStringList listTilesMetaFiles;
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    const auto &inputProducts = parameters["input_products"].toArray();
    QStringList listProducts;

    // get the products from the input_products or based on date_start or date_end
    if(inputProducts.size() == 0) {
        const auto &startDate = QDateTime::fromString(parameters["date_start"].toString(), "yyyyMMdd");
        const auto &endDateStart = QDateTime::fromString(parameters["date_end"].toString(), "yyyyMMdd");
        // we consider the end of the end date day
        const auto endDate = endDateStart.addSecs(SECONDS_IN_DAY-1);
        ProductList productsList = ctx.GetProducts(event.siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
        for(const auto &product: productsList) {
            listProducts.append(product.fullPath);
        }
    } else {
        for (const auto &inputProduct : inputProducts) {
            listProducts.append(ctx.GetProductAbsolutePath(inputProduct.toString()));
        }
    }

    // for each product, get the valid tiles
    for(const QString &inPrd: listProducts) {
        QStringList tilesMetaFiles = ctx.findProductFiles(inPrd);
        QStringList listValidTilesMetaFiles;
        for(const QString &tileMetaFile: tilesMetaFiles) {
            if(ProcessorHandlerHelper::IsValidL2AMetadataFileName(tileMetaFile)) {
                listValidTilesMetaFiles.append(tileMetaFile);
                Logger::debug(QStringLiteral("Using L2A tile: %1").arg(tileMetaFile));
            }
        }
        if(listValidTilesMetaFiles.size() > 0) {
            listTilesMetaFiles.append(listValidTilesMetaFiles);
            mapProductToTilesMetaFiles[inPrd] = listValidTilesMetaFiles;
        }
    }

    // sort the input products according to their dates
    qSort(listTilesMetaFiles.begin(), listTilesMetaFiles.end(), compareProductDates);

    return listTilesMetaFiles;
}

QStringList ProcessorHandler::GetL2AInputProductsTiles(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) {
    QMap<QString, QStringList> map;
    Q_UNUSED(map);
    return GetL2AInputProductsTiles(ctx, event, map);
}

QString ProcessorHandler::GetL2AProductForTileMetaFile(const QMap<QString, QStringList> &mapProductToTilesMetaFiles,
                                     const QString &tileMetaFile) {
    for(const auto &prd : mapProductToTilesMetaFiles.keys()) {
        const QStringList &prdTilesList = mapProductToTilesMetaFiles[prd];
        for(const QString &strTileMeta: prdTilesList) {
            if(strTileMeta == tileMetaFile)
                return prd;
        }
    }
    return "";
}

bool ProcessorHandler::GetParameterValueAsInt(const QJsonObject &parameters, const QString &key,
                                              int &outVal) {
    bool bRet = false;
    if(parameters.contains(key)) {
        // first try to get it as string
        const auto &value = parameters[key];
        if(value.isDouble()) {
            outVal = value.toInt();
            bRet = true;
        }
        if(value.isString()) {
            outVal = value.toString().toInt(&bRet);
        }
    }
    return bRet;
}

QMap<QString, TileTemporalFilesInfo> ProcessorHandler::GroupTiles(
        EventProcessingContext &ctx, int jobId,
        const QStringList &listAllProductsTiles, ProductType productType)
{
    // perform a first iteration to see the satellites IDs in all tiles
    QList<ProcessorHandlerHelper::SatelliteIdType> satIds;
    // Get the primary satellite id
    ProcessorHandlerHelper::SatelliteIdType primarySatId;
    QMap<QString, TileTemporalFilesInfo>  mapTiles = ProcessorHandlerHelper::GroupTiles(listAllProductsTiles, satIds, primarySatId);
    if(productType != ProductType::L2AProductTypeId) return mapTiles;

    // if we have only one sattelite id for all tiles, then perform no filtering
    if(satIds.size() == 1) return mapTiles;

    std::map<QString, QString> configParameters =
        ctx.GetJobConfigurationParameters(jobId, "executor.shapes_dir");
    QString shapeFilesFolder = configParameters["executor.shapes_dir"];

    // second iteration: add for each primary satelitte tile the intersecting secondary products
    QMap<QString, TileTemporalFilesInfo>  retMapTiles;
    QMap<QString, TileTemporalFilesInfo>::iterator i;
    for (i = mapTiles.begin(); i != mapTiles.end(); ++i) {
        // this time, get a copy from the map and not the reference to info as we do not want to alter the input map
        TileTemporalFilesInfo info = i.value();
        bool isPrimarySatIdInfo = ((info.uniqueSatteliteIds.size() == 1) &&
                                   (primarySatId == info.uniqueSatteliteIds[0]));
        for(ProcessorHandlerHelper::SatelliteIdType satId: satIds) {
            // if is a secondary satellite id, then get the tiles from the database
            if(isPrimarySatIdInfo && (info.primarySatelliteId != satId)) {
                ProductList satSecProds = ctx.GetProductsForTile(info.tileId, productType, primarySatId, satId);
                // get the metadata tiles for all found products intersecting the current tile
                QStringList listProductsTiles;
                for(const Product &prod: satSecProds) {
                    listProductsTiles.append(ctx.findProductFiles(prod.fullPath));
                }

                // add the intersecting products for this satellite id to the current info
                ProcessorHandlerHelper::AddSatteliteIntersectingProducts(mapTiles, listProductsTiles, satId, info);
            }
        }
        // at this stage we know that the infos have only one unique satellite id
        if(isPrimarySatIdInfo) {
            // now search to see if we can find a shapefile already created for the current tile
            info.shapePath = ProcessorHandlerHelper::GetShapeForTile(shapeFilesFolder, info.tileId);

            // Sort the products by date as maybe we added secondary products at the end
            ProcessorHandlerHelper::SortTemporalTileInfoFiles(info);

            // add the tile info
            retMapTiles[info.tileId] = info;
        }
    }

    return retMapTiles;
}

QString ProcessorHandler::GetProductFormatterTile(const QString &tile) {
    if(tile.indexOf("TILE_") == 0)
        return tile;
    return ("TILE_" + tile);
}
