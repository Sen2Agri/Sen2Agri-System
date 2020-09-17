#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "lairetrhandler_multidt_base.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"
#include "logger.hpp"

bool compareInfoTileFiles(const ProcessorHandlerHelper::InfoTileFile& infoTile1,const ProcessorHandlerHelper::InfoTileFile& infoTile2)
{
    return infoTile1.acquisitionDate < infoTile2.acquisitionDate;
}


void LaiRetrievalHandlerMultiDateBase::CreateTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                                    LAIProductFormatterParams &outProdFormatterParams,
                                                    bool bCompact, bool bRemoveTempFiles) {
    int initialTasksNo = outAllTasksList.size();
    // just create the tasks but with no information so far
    // first we add the tasks to be performed for each product
    outAllTasksList.append(TaskToSubmit{"lai-time-series-builder", {}});
    outAllTasksList.append(TaskToSubmit{"lai-err-time-series-builder", {}});
    outAllTasksList.append(TaskToSubmit{"lai-msk-flags-time-series-builder", {}});
    outAllTasksList.append(TaskToSubmit{"lai-reprocessing", {}});
    outAllTasksList.append(TaskToSubmit{"lai-reproc-splitter", {}});
    if(bRemoveTempFiles) {
        outAllTasksList.append(TaskToSubmit{ "files-remover", {} });
    }


    // now fill the tasks hierarchy infos - if bCompact = true, the first level does not exist
    //              ---------------------------------------------------------------------------------
    //              |                              |                              |                 |
    //      time-series-builder         err-time-series-builder   lai-msk-flags-time-series-builder |
    //              |                              |                              |                 |
    //              ---------------------------------------------------------------------------------
    //                                  |
    //              ---------------------------------------------
    //              |                                           |
    //      profile-reprocessing                fitted-profile-reprocessing
    //              |                                           |
    //      reprocessed-profile-splitter        fitted-reprocessed-profile-splitter
    //              |                                           |
    //              ---------------------------------------------
    //                                  |
    //                          product-formatter
    //
    // Specifies if the products creation should be chained or not.
    // TODO: This should be taken from the configuration
    bool bChainProducts = true;

    int nCurTaskIdx = initialTasksNo;
    int nTimeSeriesBuilderIdx = nCurTaskIdx++;

    int nErrTimeSeriesBuilderIdx;
    int nLaiMskFlgsTimeSeriesBuilderIdx;
    if (!bCompact) {
        nErrTimeSeriesBuilderIdx = nCurTaskIdx++;
        nLaiMskFlgsTimeSeriesBuilderIdx = nCurTaskIdx++;
    }

    // if we chain this product from another product
    if(bChainProducts && initialTasksNo > 0) {
        int prevProductLastTaskIdx = initialTasksNo-1;
        // we create a dependency to the last task of the previous product
        outAllTasksList[nTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[prevProductLastTaskIdx]);
        if (!bCompact) {
            outAllTasksList[nErrTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[prevProductLastTaskIdx]);
            outAllTasksList[nLaiMskFlgsTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[prevProductLastTaskIdx]);
        }
    }

    int nProfileReprocessingIdx = nTimeSeriesBuilderIdx;
    if (!bCompact) {
        //profile-reprocessing -> time-series-builder AND err-time-series-builder AND lai-msk-flags-time-series-builder
        nProfileReprocessingIdx = nCurTaskIdx++;
        outAllTasksList[nProfileReprocessingIdx].parentTasks.append(outAllTasksList[nTimeSeriesBuilderIdx]);
        outAllTasksList[nProfileReprocessingIdx].parentTasks.append(outAllTasksList[nErrTimeSeriesBuilderIdx]);
        outAllTasksList[nProfileReprocessingIdx].parentTasks.append(outAllTasksList[nLaiMskFlgsTimeSeriesBuilderIdx]);

    }
    //reprocessed-profile-splitter -> profile-reprocessing
    int nReprocessedProfileSplitterIdx = nCurTaskIdx++;
    outAllTasksList[nReprocessedProfileSplitterIdx].parentTasks.append(outAllTasksList[nProfileReprocessingIdx]);

    //product-formatter -> reprocessed-profile-splitter
    outProdFormatterParams.laiReprocParams.parentsTasksRef.append(outAllTasksList[nReprocessedProfileSplitterIdx]);

    if(bRemoveTempFiles) {
        // cleanup-intermediate-files -> reprocessed-profile-splitter
        outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nReprocessedProfileSplitterIdx]);
    }
}

NewStepList LaiRetrievalHandlerMultiDateBase::GetStepsForMultiDateReprocessing(
                const std::map<QString, QString> &configParameters, const TileTemporalFilesInfo &tileTemporalFilesInfo,
                QList<TaskToSubmit> &allTasksList, LAIProductFormatterParams &productFormatterParams,
               int tasksStartIdx, bool bRemoveTempFiles)
{
    NewStepList steps;
    QString reprocFileListFileName;
    QString reprocFlagsFileListFileName;
    QStringList cleanupTemporaryFilesList;

    QStringList monoDateMskFlagsLaiFileNames2;
    QStringList quantifiedLaiFileNames2;
    QStringList quantifiedErrLaiFileNames2;
    QString mainLaiImg;
    QString mainLaiErrImg;
    QString mainMsksImg;
    QStringList listDates;

    bool errFilesUsed = false;
    for(const ProcessorHandlerHelper::InfoTileFile &fileInfo: tileTemporalFilesInfo.temporalTilesFileInfos) {
        const QString &laiFileName = fileInfo.additionalFiles[LAI_RASTER_ADD_INFO_IDX];
        const QString &errFileName = fileInfo.additionalFiles[LAI_ERR_RASTER_ADD_INFO_IDX];
        const QString &mskFileName = fileInfo.additionalFiles[LAI_FLG_RASTER_ADD_INFO_IDX];

        // ensure that we have all files and we will have the same size for all lists
        if (laiFileName.size() > 0 && mskFileName.size() > 0) {
            quantifiedLaiFileNames2.append(laiFileName);
            if (errFileName.size() > 0) {
                errFilesUsed = true;
            }
            quantifiedErrLaiFileNames2.append(errFileName);
            monoDateMskFlagsLaiFileNames2.append(mskFileName);
            listDates.append(fileInfo.acquisitionDate);

            if(fileInfo.satId == tileTemporalFilesInfo.primarySatelliteId) {
                if(mainLaiImg.length() == 0) {
                    mainLaiImg = laiFileName;
                    mainLaiErrImg = errFileName;
                    mainMsksImg = mskFileName;
                }
            }
        }
    }
    int curTaskIdx = tasksStartIdx;
    TaskToSubmit &imgTimeSeriesBuilderTask = allTasksList[curTaskIdx++];
    TaskToSubmit &errTimeSeriesBuilderTask = allTasksList[curTaskIdx++];
    TaskToSubmit &mskFlagsTimeSeriesBuilderTask = allTasksList[curTaskIdx++];

    const auto & allLaiTimeSeriesFileName = imgTimeSeriesBuilderTask.GetFilePath("LAI_time_series.tif");
    QString allErrTimeSeriesFileName = errTimeSeriesBuilderTask.GetFilePath("Err_time_series.tif");
    const auto & allMskFlagsTimeSeriesFileName = mskFlagsTimeSeriesBuilderTask.GetFilePath("Mask_Flags_time_series.tif");

    const QStringList &timeSeriesBuilderArgs = GetTimeSeriesBuilderArgs(quantifiedLaiFileNames2, allLaiTimeSeriesFileName, mainLaiImg);
    QStringList errTimeSeriesBuilderArgs;
    if (errFilesUsed) {
        errTimeSeriesBuilderArgs = GetTimeSeriesBuilderArgs(quantifiedErrLaiFileNames2, allErrTimeSeriesFileName, mainLaiErrImg);
    } else {
        // we might have the situation when the L3B are created with INRA algorithm so we don't have err files.
        // do nothing in this case by executing the "true" command on linux that does nothing and will return success after the execution
        allErrTimeSeriesFileName = "";
        errTimeSeriesBuilderArgs.append("/usr/bin/true");
    }

    const QStringList &mskFlagsTimeSeriesBuilderArgs = GetTimeSeriesBuilderArgs(monoDateMskFlagsLaiFileNames2,
                                                                                 allMskFlagsTimeSeriesFileName, mainMsksImg, true);

    steps.append(imgTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", timeSeriesBuilderArgs));
    steps.append(errTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", errTimeSeriesBuilderArgs));
    steps.append(mskFlagsTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", mskFlagsTimeSeriesBuilderArgs));

    TaskToSubmit &profileReprocTask = allTasksList[curTaskIdx++];
    TaskToSubmit &profileReprocSplitTask = allTasksList[curTaskIdx++];

    const auto & reprocTimeSeriesFileName = profileReprocTask.GetFilePath("ReprocessedTimeSeries.tif");
    reprocFileListFileName = profileReprocSplitTask.GetFilePath("ReprocessedFilesList.txt");
    reprocFlagsFileListFileName = profileReprocSplitTask.GetFilePath("ReprocessedFlagsFilesList.txt");

    const QStringList &profileReprocessingArgs = GetReprocessingArgs(configParameters, allLaiTimeSeriesFileName,
                                                                         allErrTimeSeriesFileName, allMskFlagsTimeSeriesFileName,
                                                                         reprocTimeSeriesFileName, listDates);
    QStringList reprocProfileSplitterArgs = GetReprocProfileSplitterArgs(reprocTimeSeriesFileName, reprocFileListFileName,
                                                                         reprocFlagsFileListFileName, listDates);
    steps.append(profileReprocTask.CreateStep("ProfileReprocessing", profileReprocessingArgs));
    steps.append(profileReprocSplitTask.CreateStep("ReprocessedProfileSplitter2", reprocProfileSplitterArgs));

    if(bRemoveTempFiles) {
        TaskToSubmit &cleanupTemporaryFilesTask = allTasksList[curTaskIdx++];
        cleanupTemporaryFilesList.append(allLaiTimeSeriesFileName);
        cleanupTemporaryFilesList.append(allErrTimeSeriesFileName);
        cleanupTemporaryFilesList.append(allMskFlagsTimeSeriesFileName);
        cleanupTemporaryFilesList.append(reprocTimeSeriesFileName);
        // add also the cleanup step
        steps.append(cleanupTemporaryFilesTask.CreateStep("CleanupTemporaryFiles", cleanupTemporaryFilesList));
    }

    productFormatterParams.laiReprocParams.fileLaiReproc = reprocFileListFileName;
    productFormatterParams.laiReprocParams.fileLaiReprocFlgs = reprocFlagsFileListFileName;

    return steps;
}

NewStepList LaiRetrievalHandlerMultiDateBase::GetStepsForCompactMultiDateReprocessing(std::map<QString, QString> &configParameters,
                const TileTemporalFilesInfo &tileTemporalFilesInfo, QList<TaskToSubmit> &allTasksList,
                LAIProductFormatterParams &productFormatterParams,
               int tasksStartIdx, bool bRemoveTempFiles)
{
    NewStepList steps;
    QString reprocFileListFileName;
    QString reprocFlagsFileListFileName;

    QStringList cleanupTemporaryFilesList;

    QStringList monoDateMskFlagsLaiFileNames2;
    QStringList quantifiedLaiFileNames2;
    QStringList quantifiedErrLaiFileNames2;
    QString mainLaiImg;
    for(int i = 0; i< tileTemporalFilesInfo.temporalTilesFileInfos.size(); i++) {
        quantifiedLaiFileNames2.append(tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[LAI_RASTER_ADD_INFO_IDX]);
        quantifiedErrLaiFileNames2.append(tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[LAI_ERR_RASTER_ADD_INFO_IDX]);
        monoDateMskFlagsLaiFileNames2.append(tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[LAI_FLG_RASTER_ADD_INFO_IDX]);

        if(tileTemporalFilesInfo.temporalTilesFileInfos[i].satId == tileTemporalFilesInfo.primarySatelliteId) {
            if(mainLaiImg.length() == 0) {
                mainLaiImg = tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[LAI_RASTER_ADD_INFO_IDX];
            }
        }
    }

    int curTaskIdx = tasksStartIdx;
    TaskToSubmit &profileReprocTask = allTasksList[curTaskIdx++];
    TaskToSubmit &profileReprocSplitTask = allTasksList[curTaskIdx++];

    const QStringList &listDates = ProcessorHandlerHelper::GetTemporalTileAcquisitionDates(tileTemporalFilesInfo);
    const auto & reprocTimeSeriesFileName = profileReprocTask.GetFilePath("ReprocessedTimeSeries.tif");
    reprocFileListFileName = profileReprocSplitTask.GetFilePath("ReprocessedFilesList.txt");
    reprocFlagsFileListFileName = profileReprocSplitTask.GetFilePath("ReprocessedFlagsFilesList.txt");

    const QStringList &profileReprocessingArgs = GetCompactReprocessingArgs(configParameters, quantifiedLaiFileNames2,
                                                              quantifiedErrLaiFileNames2, monoDateMskFlagsLaiFileNames2, mainLaiImg,
                                                              reprocTimeSeriesFileName, listDates);

    const QStringList &reprocProfileSplitterArgs = GetReprocProfileSplitterArgs(reprocTimeSeriesFileName, reprocFileListFileName,
                                                                         reprocFlagsFileListFileName, listDates);
    steps.append(profileReprocTask.CreateStep("ProfileReprocessing", profileReprocessingArgs));
    steps.append(profileReprocSplitTask.CreateStep("ReprocessedProfileSplitter2", reprocProfileSplitterArgs));

    if(bRemoveTempFiles) {
        TaskToSubmit &cleanupTemporaryFilesTask = allTasksList[curTaskIdx++];
        cleanupTemporaryFilesList.append(reprocTimeSeriesFileName);
        // add also the cleanup step
        steps.append(cleanupTemporaryFilesTask.CreateStep("CleanupTemporaryFiles", cleanupTemporaryFilesList));
    }

    productFormatterParams.laiReprocParams.fileLaiReproc = reprocFileListFileName;
    productFormatterParams.laiReprocParams.fileLaiReprocFlgs = reprocFlagsFileListFileName;

    return steps;
}

void LaiRetrievalHandlerMultiDateBase::HandleNewTilesList(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                             const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                             LAIGlobalExecutionInfos &outGlobalExecInfos, bool bRemoveTempFiles) {

    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, GetProcessorDBPrefix());

    QList<TaskToSubmit> &allTasksList = outGlobalExecInfos.allTasksList;
    LAIProductFormatterParams &productFormatterParams = outGlobalExecInfos.prodFormatParams;

    int tasksStartIdx = allTasksList.size();

    // create the tasks
    bool useCompactReprocessing = IsReprocessingCompact(parameters, configParameters);
    CreateTasksForNewProducts(allTasksList, outGlobalExecInfos.prodFormatParams, useCompactReprocessing, bRemoveTempFiles);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    // submit all tasks
    SubmitTasks(ctx, event.jobId, allTasksListRef);

    NewStepList &steps = outGlobalExecInfos.allStepsList;

    if(useCompactReprocessing) {

        steps += GetStepsForCompactMultiDateReprocessing(configParameters, tileTemporalFilesInfo, allTasksList,
                                              productFormatterParams, tasksStartIdx, bRemoveTempFiles);
    } else {
        steps += GetStepsForMultiDateReprocessing(configParameters, tileTemporalFilesInfo, allTasksList,
                                              productFormatterParams, tasksStartIdx, bRemoveTempFiles);
    }
}

void LaiRetrievalHandlerMultiDateBase::WriteExecutionInfosFile(const QString &executionInfosPath,
                                               const std::map<QString, QString> &configParameters,
                                               const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles,
                                               const QStringList &listProducts) {
    std::ofstream executionInfosFile;
    try
    {
        executionInfosFile.open(executionInfosPath.toStdString().c_str(), std::ofstream::out);
        executionInfosFile << "<?xml version=\"1.0\" ?>" << std::endl;
        executionInfosFile << "<metadata>" << std::endl;
        executionInfosFile << "  <General>" << std::endl;
        executionInfosFile << "  </General>" << std::endl;

        WriteExecutionSpecificParamsValues(configParameters, executionInfosFile);

        executionInfosFile << "  <XML_files>" << std::endl;
        for (int i = 0; i<listProducts.size(); i++) {
            executionInfosFile << "    <XML_" << std::to_string(i) << ">" << listProducts[i].toStdString()
                               << "</XML_" << std::to_string(i) << ">" << std::endl;
        }
        executionInfosFile << "  </XML_files>" << std::endl;

        QStringList l3bMonoRasterFiles;
        QStringList l3bErrRasterFiles;
        QStringList l3bFlgsRasterFiles;
        for(const auto &tileId : l3bMapTiles.keys())
        {
           const TileTemporalFilesInfo &listTemporalTiles = l3bMapTiles.value(tileId);
           l3bMonoRasterFiles += GetL3BProductRasterFiles(listTemporalTiles, LAI_RASTER_ADD_INFO_IDX);
           l3bErrRasterFiles += GetL3BProductRasterFiles(listTemporalTiles, LAI_ERR_RASTER_ADD_INFO_IDX);
           l3bFlgsRasterFiles += GetL3BProductRasterFiles(listTemporalTiles, LAI_FLG_RASTER_ADD_INFO_IDX);
        }
        executionInfosFile << "  <L3B_files>" << std::endl;

        executionInfosFile << "    <L3B_MONO_files>" << std::endl;
        for (int i = 0; i<l3bMonoRasterFiles.size(); i++) {
            executionInfosFile << "    <MONO_" << std::to_string(i) << ">" << l3bMonoRasterFiles[i].toStdString()
                               << "</MONO_" << std::to_string(i) << ">" << std::endl;
        }
        executionInfosFile << "    </L3B_MONO_files>" << std::endl;

        executionInfosFile << "    <L3B_ERR_files>" << std::endl;
        for (int i = 0; i<l3bErrRasterFiles.size(); i++) {
            executionInfosFile << "    <ERR_" << std::to_string(i) << ">" << l3bErrRasterFiles[i].toStdString()
                               << "</ERR_" << std::to_string(i) << ">" << std::endl;
        }
        executionInfosFile << "    </L3B_ERR_files>" << std::endl;

        executionInfosFile << "    <L3B_FLG_files>" << std::endl;
        for (int i = 0; i<l3bFlgsRasterFiles.size(); i++) {
            executionInfosFile << "    <FLG_" << std::to_string(i) << ">" << l3bFlgsRasterFiles[i].toStdString()
                               << "</FLG_" << std::to_string(i) << ">" << std::endl;
        }
        executionInfosFile << "    </L3B_FLG_files>" << std::endl;

        executionInfosFile << "  </L3B_files>" << std::endl;
        executionInfosFile << "</metadata>" << std::endl;
        executionInfosFile.close();
    }
    catch(...)
    {

    }
}

bool LaiRetrievalHandlerMultiDateBase::GetL2AProductsInterval(const QMap<QString, QStringList> &mapTilesMeta,
                                                    QDateTime &startDate, QDateTime &endDate) {
    bool bDatesInitialized = false;
    for(const QString &prd: mapTilesMeta.keys()) {
        const QStringList &listFiles = mapTilesMeta.value(prd);
        if(listFiles.size() > 0) {
            const QString &tileMeta = listFiles.at(0);
            QDateTime dtPrdDate = ProcessorHandlerHelper::GetL2AProductDateFromPath(tileMeta);
            if(!bDatesInitialized) {
                startDate = dtPrdDate;
                endDate = dtPrdDate;
                bDatesInitialized = true;
            } else {
                if(startDate > dtPrdDate) {
                    startDate = dtPrdDate;
                }
                if(endDate < dtPrdDate) {
                    endDate = dtPrdDate;
                }
            }
        }

    }
    return (bDatesInitialized && startDate.isValid() && endDate.isValid());
}

/**
 * Get the L3B products from the received event. If the input products are L2A then the associated L3B products are
 * search and returned
 */
//TODO: This function should receive the Product and QList<Product> instead of just product path as these can be got from DB
//      The Product contains already the tiles, the full path and the acquisition date so can be avoided parsing files
QStringList LaiRetrievalHandlerMultiDateBase::GetL3BProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    bool inputsAreL3b = (GetIntParameterValue(parameters, "inputs_are_l3b", 0) == 1);

    QStringList filteredL3bProductList;
    if(inputsAreL3b) {
        Logger::debug(QStringLiteral("Inputs are L3B for job %1").arg(event.jobId));
        // fill the l3bMapTiles from the input L3B products
        const auto &inputProducts = parameters["input_products"].toArray();
        for (const auto &inputProduct : inputProducts) {
            const QString &prodAbsPath = ctx.GetProductAbsolutePath(event.siteId, inputProduct.toString());
            if(!filteredL3bProductList.contains(prodAbsPath)) {
                filteredL3bProductList.append(prodAbsPath);
            }
        }
    } else {
        Logger::debug(QStringLiteral("Inputs are L2A for job %1").arg(event.jobId));
        QMap<QString, QStringList> inputProductToTilesMap;
        const QStringList &listTilesMetaFiles = GetL2AInputProductsTiles(ctx, event, inputProductToTilesMap);

        // get the L3B products for the current product tiles
        QDateTime startDate;
        QDateTime endDate;
        if(GetL2AProductsInterval(inputProductToTilesMap, startDate, endDate)) {
            // we consider the end date until the end of day
            endDate = endDate.addSecs(SECONDS_IN_DAY-1);
            const ProductList &l3bProductList = ctx.GetProducts(event.siteId, (int)ProductType::L3BProductTypeId, startDate, endDate);
            for(const Product &l3bPrd: l3bProductList) {
                // now filter again the products according to the
                for(const QString &l2aTileFile: listTilesMetaFiles) {
                    if(ProcessorHandlerHelper::HighLevelPrdHasL2aSource(l3bPrd.fullPath, l2aTileFile)) {
                        if(!filteredL3bProductList.contains(l3bPrd.fullPath)) {
                            filteredL3bProductList.append(l3bPrd.fullPath);
                        }
                    }
                }
            }
        }
    }

    return filteredL3bProductList;
}

bool LaiRetrievalHandlerMultiDateBase::AddTileFileInfo(EventProcessingContext &ctx, TileTemporalFilesInfo &temporalTileInfo, const QString &l3bPrd, const QString &tileId,
                                             const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles,
                                             ProcessorHandlerHelper::SatelliteIdType satId, const QDateTime &curPrdMinDate)
{
    // Fill the tile information for the current tile from the current product
    const QMap<QString, QString> &mapL3BTiles = ProcessorHandlerHelper::GetHighLevelProductTilesDirs(l3bPrd);
    if(mapL3BTiles.size() == 0) {
        return false;
    }
    const QString &tileDir = mapL3BTiles[tileId];
    if(tileDir.length() == 0) {
        // get the primary satellite from the received tile and the received product
        ProcessorHandlerHelper::SatelliteIdType l3bPrdSatId = GetSatIdForTile(siteTiles, mapL3BTiles.keys().at(0));
        QList<ProcessorHandlerHelper::SatelliteIdType> listSatIds = {satId, l3bPrdSatId};
        ProcessorHandlerHelper::SatelliteIdType primarySatId = ProcessorHandlerHelper::GetPrimarySatelliteId(listSatIds);
        // if is primary satellite only, then add intersecting tiles from the product
        // otherwise, if secondary satellite, then use only products of its own type (with the same tile)
        if(primarySatId == satId) {
            // get the tiles for satellite l3bPrdSatId that intersect tileId
            auto it = temporalTileInfo.satIntersectingTiles.find(satId);
            const TileList &allIntersectingTiles = (it != temporalTileInfo.satIntersectingTiles.end()) ?
                (it.value()) :
                (ctx.GetIntersectingTiles(static_cast<Satellite>(satId), tileId));
            // set also the intersecting tiles in the temporalTileInfo
            if (it == temporalTileInfo.satIntersectingTiles.end()) {
                temporalTileInfo.satIntersectingTiles[satId] = allIntersectingTiles;
            }

            bool bAdded = false;
            // filter and add the secondary satellite tiles
            for(const QString &tileL3bPrd: mapL3BTiles.keys()) {
                for(const Tile &intersectingTile : allIntersectingTiles) {
                    if(((int)intersectingTile.satellite != (int)satId) && (intersectingTile.tileId  == tileL3bPrd)) {
                        const QString &newTileDir = mapL3BTiles[tileL3bPrd];
                        // this should not happen but is better to check
                        if(newTileDir.length() > 0) {
                            if (AddTileFileInfo(temporalTileInfo, l3bPrd, newTileDir, l3bPrdSatId, curPrdMinDate, &intersectingTile)) {
                                bAdded = true;
                            }
                        }
                    }
                }
            }
            return bAdded;
        } else {
            // if the satellite id of the tile is secondary compared with the l3bPrd received, then we do nothing
            return false;
        }
    }

    return AddTileFileInfo(temporalTileInfo, l3bPrd, tileDir, satId, curPrdMinDate);
}

bool LaiRetrievalHandlerMultiDateBase::AddTileFileInfo(TileTemporalFilesInfo &temporalTileInfo, const QString &l3bProdDir, const QString &l3bTileDir,
                                             ProcessorHandlerHelper::SatelliteIdType satId, const QDateTime &curPrdMinDate,
                                             const Tile *pIntersectingTile)
{
    if(l3bTileDir.length() > 0) {
        // fill the empty gaps for these lists
        ProcessorHandlerHelper::InfoTileFile l3bTileInfo;
        //update the sat id
        // update the files
        // Set the file to the tile dir
        if (pIntersectingTile) {
            l3bTileInfo.satId = ProcessorHandlerHelper::ConvertSatelliteType(pIntersectingTile->satellite);
            l3bTileInfo.file = ProcessorHandlerHelper::GetSourceL2AFromHighLevelProductIppFile(l3bProdDir, pIntersectingTile->tileId);
        } else {
            l3bTileInfo.satId = satId;
            l3bTileInfo.file = ProcessorHandlerHelper::GetSourceL2AFromHighLevelProductIppFile(l3bProdDir, temporalTileInfo.tileId);
        }
        l3bTileInfo.acquisitionDate = curPrdMinDate.toString("yyyyMMdd");
        const QString &laiFileName = ProcessorHandlerHelper::GetHigLevelProductTileFile(l3bTileDir, "SLAIMONO");
        const QString &errFileName = ProcessorHandlerHelper::GetHigLevelProductTileFile(l3bTileDir, "MLAIERR", true);
        const QString &mskFileName = ProcessorHandlerHelper::GetHigLevelProductTileFile(l3bTileDir, "MMONODFLG", true);
        if (laiFileName.size() > 0 && mskFileName.size() > 0) {
            l3bTileInfo.additionalFiles.append(laiFileName);
            l3bTileInfo.additionalFiles.append(errFileName);
            l3bTileInfo.additionalFiles.append(mskFileName);
            // add the sat id to the list of unique sat ids
            if(!temporalTileInfo.uniqueSatteliteIds.contains(l3bTileInfo.satId)) {
                 temporalTileInfo.uniqueSatteliteIds.append(l3bTileInfo.satId);
            }
            //add it to the temporal tile files info
            // NOTE: We add in front of the list in order to keep the list sorted ascending
            if (!temporalTileInfo.temporalTilesFileInfos.contains(l3bTileInfo)) {
                temporalTileInfo.temporalTilesFileInfos.prepend(l3bTileInfo);
                // sort also the list ascending just to be sure
                qSort(temporalTileInfo.temporalTilesFileInfos.begin(), temporalTileInfo.temporalTilesFileInfos.end(), compareInfoTileFiles);
                return true;
            }
        }
    }
    return false;
}

void LaiRetrievalHandlerMultiDateBase::SubmitEndOfLaiTask(EventProcessingContext &ctx,
                                                const JobSubmittedEvent &event,
                                                const QList<TaskToSubmit> &allTasksList) {
    // add the end of lai job that will perform the cleanup
    QList<std::reference_wrapper<const TaskToSubmit>> prdFormatterTasksListRef;
    for(const TaskToSubmit &task: allTasksList) {
        if(task.moduleName == "product-formatter") {
            prdFormatterTasksListRef.append(task);
        }
    }
    // we add a task in order to wait for all product formatter to finish.
    // This will allow us to mark the job as finished and to remove the job folder
    TaskToSubmit endOfJobDummyTask{"end-of-job", {}};
    endOfJobDummyTask.parentTasks.append(prdFormatterTasksListRef);
    SubmitTasks(ctx, event.jobId, {endOfJobDummyTask});
    ctx.SubmitSteps({endOfJobDummyTask.CreateStep("EndOfJob", QStringList())});
}

void LaiRetrievalHandlerMultiDateBase::SubmitL3BMapTiles(EventProcessingContext &ctx,
                                               const JobSubmittedEvent &event,
                                               const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles,
                                               bool bRemoveTempFiles,
                                               QList<TaskToSubmit> &allTasksList)
{
    QList<LAIProductFormatterParams> listParams;
    NewStepList allSteps;
    //container for all global execution infos
    QList<LAIGlobalExecutionInfos> allLaiGlobalExecInfos;

    // after retrieving the L3C products is possible to have only a subset of the original L2A products
    QStringList realL2AMetaFiles;
    for(const auto &tileId : l3bMapTiles.keys())
    {
       const TileTemporalFilesInfo &listTemporalTiles = l3bMapTiles.value(tileId);
       Logger::debug(QStringLiteral("Handling tile %1 from a number of %2 tiles").arg(tileId).arg(l3bMapTiles.size()));

       allLaiGlobalExecInfos.append(LAIGlobalExecutionInfos());
       LAIGlobalExecutionInfos &infosRef = allLaiGlobalExecInfos[allLaiGlobalExecInfos.size()-1];
       infosRef.prodFormatParams.tileId = GetProductFormatterTile(tileId);
       HandleNewTilesList(ctx, event, listTemporalTiles, infosRef, bRemoveTempFiles);
       if(infosRef.allTasksList.size() > 0 && infosRef.allStepsList.size() > 0) {
           listParams.append(infosRef.prodFormatParams);
           allTasksList.append(infosRef.allTasksList);
           allSteps.append(infosRef.allStepsList);
           realL2AMetaFiles += ProcessorHandlerHelper::GetTemporalTileFiles(listTemporalTiles);
       }
    }
    // submit only if we had something to execute but do not give an error
    if (realL2AMetaFiles.size() > 0) {
        // Create the product formatter tasks for the Reprocessed and/or Fitted Products (if needed)
        allTasksList.append({"product-formatter", {}});
        TaskToSubmit &productFormatterTask = allTasksList[allTasksList.size()-1];
        for(LAIProductFormatterParams params: listParams) {
            productFormatterTask.parentTasks.append(params.laiReprocParams.parentsTasksRef);
        }
        SubmitTasks(ctx, event.jobId, {productFormatterTask});
        const QStringList &productFormatterArgs = GetProductFormatterArgs(productFormatterTask, ctx,
                                              event, l3bMapTiles, realL2AMetaFiles, listParams);
        // add these steps to the steps list to be submitted
        allSteps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
        ctx.SubmitSteps(allSteps);
    } else {
        Logger::error(QStringLiteral("Request for executing but no products were found for execution. Ignored ..."));
    }
}

QMap<QString, TileTemporalFilesInfo> LaiRetrievalHandlerMultiDateBase::FilterSecondaryProductTiles(const QMap<QString, TileTemporalFilesInfo> &mapTiles,
                             const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles)
{
    QList<ProcessorHandlerHelper::SatelliteIdType> uniqueSatteliteIds;
    QMap<QString, ProcessorHandlerHelper::SatelliteIdType> mapTilesSats;
    // iterate the tiles of the newest L3B product
    for(const auto &tileId : mapTiles.keys()) {
        ProcessorHandlerHelper::SatelliteIdType tileSatId = GetSatIdForTile(siteTiles, tileId);
        // ignore tiles for which the satellite id cannot be determined
        if(tileSatId == ProcessorHandlerHelper::SATELLITE_ID_TYPE_UNKNOWN) {
            continue;
        }
        if(!uniqueSatteliteIds.contains(tileSatId)) {
             uniqueSatteliteIds.append(tileSatId);
        }
        //fill the map from sat id to tile id
        mapTilesSats[tileId] = tileSatId;
    }
    if(uniqueSatteliteIds.size() == 1) {
        return mapTiles;
    }

    QMap<QString, TileTemporalFilesInfo> newMapTiles;
    ProcessorHandlerHelper::SatelliteIdType primarySatelliteId = ProcessorHandlerHelper::GetPrimarySatelliteId(uniqueSatteliteIds);
    for(const auto &tileId : mapTilesSats.keys()) {
        ProcessorHandlerHelper::SatelliteIdType satId = mapTilesSats.value(tileId);
        if(satId == primarySatelliteId) {
            newMapTiles[tileId] = mapTiles.value(tileId);
        }
    }
    return newMapTiles;

}

void LaiRetrievalHandlerMultiDateBase::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    bool bRemoveTempFiles = NeedRemoveJobFolder(ctx, event.jobId, this->processorDescr.shortName);
    // get the list of the L3B products from the event
    const QStringList &listL3BProducts = GetL3BProducts(ctx, event);
    if (listL3BProducts.size() == 0) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No L3B products were found for the given event").toStdString());
    }

    const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles = GetSiteTiles(ctx, event.siteId);

    //container for all task
    QList<TaskToSubmit> allTasksList;
    const QList<QMap<QString, TileTemporalFilesInfo>> &l3bMapTilesList = ExtractL3BMapTiles(ctx, event, listL3BProducts, siteTiles);
    for (const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles : l3bMapTilesList) {
        if (l3bMapTiles.size() == 0) {
                ctx.MarkJobFailed(event.jobId);
                throw std::runtime_error(
                    QStringLiteral("No L3B products were found for the given event").toStdString());
            }
        SubmitL3BMapTiles(ctx, event, l3bMapTiles, bRemoveTempFiles, allTasksList);
    }

    // we add a task in order to wait for all product formatter to finish.
    // This will allow us to mark the job as finished and to remove the job folder
    SubmitEndOfLaiTask(ctx, event, allTasksList);
}

void LaiRetrievalHandlerMultiDateBase::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    if (event.module == "end-of-job") {
        ctx.MarkJobFinished(event.jobId);
        // Now remove the job folder containing temporary files
        RemoveJobFolder(ctx, event.jobId, this->processorDescr.shortName);
    }
    if (event.module == "product-formatter") {
        QString prodName = GetProductFormatterProductName(ctx, event);
        QString productFolder = GetProductFormatterOutputProductPath(ctx, event);
        if((prodName != "") && ProcessorHandlerHelper::IsValidHighLevelProduct(productFolder)) {
            QString quicklook = GetProductFormatterQuicklook(ctx, event);
            QString footPrint = GetProductFormatterFootprint(ctx, event);
            ProductType prodType = GetOutputProductType();

            const QStringList &prodTiles = ProcessorHandlerHelper::GetTileIdsFromHighLevelProduct(productFolder);

            // Insert the product into the database
            QDateTime minDate, maxDate;
            ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prodName, minDate, maxDate);
            int ret = ctx.InsertProduct({ prodType, event.processorId, event.siteId, event.jobId,
                                productFolder, maxDate, prodName,
                                quicklook, footPrint, std::experimental::nullopt, prodTiles });
            Logger::debug(QStringLiteral("InsertProduct for %1 returned %2").arg(prodName).arg(ret));

        } else {
            Logger::error(QStringLiteral("Cannot insert into database the product with name %1 and folder %2").arg(prodName).arg(productFolder));
            //ctx.MarkJobFailed(event.jobId);
        }
    }
}

QStringList LaiRetrievalHandlerMultiDateBase::GetTimeSeriesBuilderArgs(const QStringList &monoDateFileNames,
                                                             const QString &allTimeSeriesFileName, const QString &mainImg,
                                                             bool bIsFlg) {
    QStringList timeSeriesBuilderArgs = { "TimeSeriesBuilder",
      "-out", (bIsFlg ? allTimeSeriesFileName : (allTimeSeriesFileName + "?gdal:co:BIGTIFF=YES")),
      "-main", mainImg,
      "-il"
    };
    timeSeriesBuilderArgs += monoDateFileNames;
    if (bIsFlg) {
        timeSeriesBuilderArgs += "-isflg";
        timeSeriesBuilderArgs += "1";
    }

    return timeSeriesBuilderArgs;
}

QStringList LaiRetrievalHandlerMultiDateBase::GetReprocessingArgs(const std::map<QString, QString> &configParameters,
                                       const QString &allLaiTimeSeriesFileName,
                                       const QString &allErrTimeSeriesFileName, const QString &allMsksTimeSeriesFileName,
                                       const QString &reprocTimeSeriesFileName, const QStringList &listDates) {
    QStringList profileReprocessingArgs = { "ProfileReprocessing",
                            "-lai", allLaiTimeSeriesFileName,
                            "-msks", allMsksTimeSeriesFileName,
                            "-opf", (reprocTimeSeriesFileName + "?gdal:co:BIGTIFF=YES"),
                      };
    const QStringList &specificArgs = GetSpecificReprocessingArgs(configParameters);
    profileReprocessingArgs += specificArgs;

    if (allErrTimeSeriesFileName.size() > 0) {
        profileReprocessingArgs.append("-err");
        profileReprocessingArgs.append(allErrTimeSeriesFileName);
    }
    profileReprocessingArgs.append("-ildates");
    profileReprocessingArgs += listDates;
    return profileReprocessingArgs;
}

QStringList LaiRetrievalHandlerMultiDateBase::GetCompactReprocessingArgs(const std::map<QString, QString> &configParameters,
                                                                         QStringList &monoDateLaiFileNames, QStringList &errFileNames, QStringList &flgsFileNames,
                                                                         const QString &mainImg, const QString &reprocTimeSeriesFileName, const QStringList &listDates) {
    QStringList profileReprocessingArgs = { "ProfileReprocessing",
          "-main", mainImg,
          "-opf", (reprocTimeSeriesFileName + "?gdal:co:BIGTIFF=YES"),
    };
    const QStringList &specificArgs = GetSpecificReprocessingArgs(configParameters);
    profileReprocessingArgs += specificArgs;

    profileReprocessingArgs += "-illai"; profileReprocessingArgs += monoDateLaiFileNames;
    profileReprocessingArgs += "-ilerr"; profileReprocessingArgs += errFileNames;
    profileReprocessingArgs += "-ilmsks"; profileReprocessingArgs += flgsFileNames;

    profileReprocessingArgs += "-ildates"; profileReprocessingArgs += listDates;

    return profileReprocessingArgs;
}


QStringList LaiRetrievalHandlerMultiDateBase::GetReprocProfileSplitterArgs(const QString &reprocTimeSeriesFileName,
                                     const QString &reprocFileListFileName,
                                     const QString &reprocFlagsFileListFileName,
                                     const QStringList &listDates) {
    QStringList args = { "ReprocessedProfileSplitter2",
            "-in", reprocTimeSeriesFileName,
            "-outrlist", reprocFileListFileName,
            "-outflist", reprocFlagsFileListFileName,
            "-compress", "1",
            "-ildates"
    };
    args += listDates;
    return args;
}

QStringList LaiRetrievalHandlerMultiDateBase::GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx,
                                    const JobSubmittedEvent &event, const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles,
                                    const QStringList &listProducts, const QList<LAIProductFormatterParams> &productParams) {

    const std::map<QString, QString> &configParameters = ctx.GetJobConfigurationParameters(event.jobId, GetProcessorDBPrefix());

    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.xml");

    WriteExecutionInfosFile(executionInfosPath, configParameters, l3bMapTiles, listProducts);

    QStringList productFormatterArgs = { "ProductFormatter",
                            "-destroot", GetFinalProductFolder(ctx, event.jobId, event.siteId),
                            "-fileclass", "OPER",
                            "-level", GetOutputProductShortName(),
                            "-baseline", "01.00",
                            "-siteid", QString::number(event.siteId),
                            "-processor", "vegetation",
                            "-gipp", executionInfosPath,
                            "-compress", "1",
                            "-outprops", outPropsPath};
    productFormatterArgs += "-il";
    productFormatterArgs += listProducts;

    const auto &lutFile = ProcessorHandlerHelper::GetMapValue(configParameters, GetProcessorDBPrefix() + "lut_path");
    if(lutFile.size() > 0) {
        productFormatterArgs += "-lut";
        productFormatterArgs += lutFile;
    }

    productFormatterArgs += GetPrdFormatterRasterFlagName();
    for(const LAIProductFormatterParams &params: productParams) {
        productFormatterArgs += params.tileId;
        productFormatterArgs += params.laiReprocParams.fileLaiReproc;
    }
    productFormatterArgs += GetPrdFormatterMskFlagName();
    for(const LAIProductFormatterParams &params: productParams) {
        productFormatterArgs += params.tileId;
        productFormatterArgs += params.laiReprocParams.fileLaiReprocFlgs;
    }

    if (IsCloudOptimizedGeotiff(configParameters)) {
        productFormatterArgs += "-cog";
        productFormatterArgs += "1";
    }

    return productFormatterArgs;
}

int LaiRetrievalHandlerMultiDateBase::GetIntParameterValue(const QJsonObject &parameters, const QString &key, int defVal)
{
    int retVal = defVal;
    if(parameters.contains(key)) {
        const auto &value = parameters[key];
        if(value.isDouble())
            retVal = value.toInt();
        else if(value.isString()) {
            retVal = value.toString().toInt();
        }
    }
    return retVal;
}

bool LaiRetrievalHandlerMultiDateBase::IsReprocessingCompact(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
    bool bReprocCompact = false;
    if(parameters.contains("reproc_compact")) {
        const auto &value = parameters["reproc_compact"];
        if(value.isDouble())
            bReprocCompact = (value.toInt() != 0);
        else if(value.isString()) {
            bReprocCompact = (value.toString() == "1");
        }
    } else {
        bReprocCompact = ((configParameters[GetProcessorDBPrefix() + "reproc_compact"]).toInt() != 0);
    }
    return bReprocCompact;
}


QStringList LaiRetrievalHandlerMultiDateBase::GetL3BProductRasterFiles(const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                                             LAI_RASTER_ADDITIONAL_INFO_IDX idx)
{
    QStringList retList;
    for(int i = 0; i< tileTemporalFilesInfo.temporalTilesFileInfos.size(); i++) {
        retList.append(tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[idx]);
    }
    return retList;
}

ProcessorJobDefinitionParams LaiRetrievalHandlerMultiDateBase::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                          const ConfigurationParameterValueMap &requestOverrideCfgValues)
{
    ProcessorJobDefinitionParams params;
    params.isValid = false;

    QDateTime seasonStartDate;
    QDateTime seasonEndDate;
    // extract the scheduled date
    QDateTime qScheduledDate = QDateTime::fromTime_t(scheduledDate);
    bool success = GetSeasonStartEndDates(ctx, siteId, seasonStartDate, seasonEndDate, qScheduledDate, requestOverrideCfgValues);
    // if cannot get the season dates
    if(!success) {
        Logger::debug(QStringLiteral("Scheduler %1: Error getting season start dates for site %2 for scheduled date %3!")
                      .arg(processorDescr.shortName)
                      .arg(siteId)
                      .arg(qScheduledDate.toString()));
        return params;
    }

    Logger::debug(QStringLiteral("Scheduler %1: season dates for site ID %2: start date %3, end date %4")
                  .arg(processorDescr.shortName)
                  .arg(siteId)
                  .arg(seasonStartDate.toString())
                  .arg(seasonEndDate.toString()));

    QDateTime limitDate = seasonEndDate.addMonths(2);
    if(qScheduledDate > limitDate) {
        Logger::debug(QStringLiteral("Scheduler %1: Error scheduled date %2 greater than the limit date %3 for site %4!")
                      .arg(processorDescr.shortName)
                      .arg(qScheduledDate.toString())
                      .arg(limitDate.toString())
                      .arg(siteId));
        return params;
    }
    if(!seasonStartDate.isValid()) {
        Logger::error(QStringLiteral("Season start date for site ID %1 is invalid in the database!")
                      .arg(siteId));
        return params;
    }

    ConfigurationParameterValueMap mapCfg = ctx.GetConfigurationParameters(GetProcessorDBPrefix(), siteId, requestOverrideCfgValues);

    // we might have an offset in days from starting the downloading products to start the L3C/L3D production
    int startSeasonOffset = mapCfg[GetProcessorDBPrefix() + "start_season_offset"].value.toInt();
    Logger::debug(QStringLiteral("Scheduler: start_season_offset for site ID %1: %2")
                  .arg(siteId)
                  .arg(startSeasonOffset));

    seasonStartDate = seasonStartDate.addDays(startSeasonOffset);
    Logger::debug(QStringLiteral("Scheduler: start_season_offset after ofsset for site ID %1: %2")
                  .arg(siteId)
                  .arg(seasonStartDate.toString()));

    params.jsonParameters = "{ \"inputs_are_l3b\" : \"1\"}";

    // by default the start date is the season start date
    QDateTime startDate = seasonStartDate;
    QDateTime endDate = qScheduledDate;
    Logger::debug(QStringLiteral("Scheduler: dates to retrieve products for site ID %1: start date %2, end date %3")
                  .arg(siteId)
                  .arg(startDate.toString())
                  .arg(endDate.toString()));


    ProductList productList = GetScheduledJobProductList(ctx, siteId, seasonStartDate, seasonEndDate,
                                                         qScheduledDate, requestOverrideCfgValues);

    // we consider only Sentinel 2 products in generating final L3C/L3D products
    for(const Product &prd: productList) {
        const QString &l2aPrdHdrPath = ProcessorHandlerHelper::GetSourceL2AFromHighLevelProductIppFile(prd.fullPath);
        ProcessorHandlerHelper::SatelliteIdType satId = ProcessorHandlerHelper::GetL2ASatelliteFromTile(l2aPrdHdrPath);
        // in the case of L3C we filter here to have only the S2 products.
        // This is not the case for L3D where we send all products and we filter during job processing

        if(AcceptSchedJobProduct(l2aPrdHdrPath, satId)) {
            params.productList.append(prd);
            Logger::debug(QStringLiteral("Scheduler %1: Using S2 product %2!")
                          .arg(processorDescr.shortName)
                          .arg(prd.fullPath));
        } else {
            Logger::debug(QStringLiteral("Scheduler: Ignored product with satId %1 from path %2!")
                          .arg(satId)
                          .arg(prd.fullPath));
        }
    }

    // Normally, we need at least 3 product available in order to be able to create a L3C/L3D product
    // but if we do not return here, the schedule block waiting for products (that might never happen)
    bool waitForAvailProcInputs = (mapCfg[GetProcessorDBPrefix() + "sched_wait_proc_inputs"].value.toInt() != 0);
    if((waitForAvailProcInputs == false) || (params.productList.size() > 0)) {
        params.isValid = true;
        Logger::debug(QStringLiteral("Executing scheduled job. Scheduler extracted for %1 a number "
                                     "of %2 products for site ID %3 with start date %4 and end date %5!")
                      .arg(processorDescr.shortName)
                      .arg(params.productList.size())
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    } else {
        Logger::debug(QStringLiteral("Scheduled job for %1 and site ID %2 with start date %3 and end date %4 "
                                     "will not be executed (no products)!")
                      .arg(processorDescr.shortName)
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    }

    return params;
}


QString LaiRetrievalHandlerMultiDateBase::GetProcessorDBPrefix()
{
    if (m_procDbPrefix.size() == 0) {
        m_procDbPrefix = QString("processor.") + this->processorDescr.shortName + ".";
    }
    return m_procDbPrefix;
}
