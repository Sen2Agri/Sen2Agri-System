#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "lairetrievalhandler_l3c.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"
#include "logger.hpp"

// The number of tasks that are executed for each product before executing time series tasks
#define LAI_TASKS_PER_PRODUCT       6
#define MODEL_GEN_TASKS_PER_PRODUCT 4
#define CUT_TASKS_NO                5

#define DEFAULT_GENERATED_SAMPLES_NO    "40000"
#define DEFAULT_NOISE_VAR               "0.01"
#define DEFAULT_BEST_OF                 "1"
#define DEFAULT_REGRESSOR               "nn"

void LaiRetrievalHandlerL3C::CreateTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                                    LAIProductFormatterParams &outProdFormatterParams,
                                                    bool bNDayReproc, bool bFittedReproc) {
    // just create the tasks but with no information so far
    // first we add the tasks to be performed for each product
    outAllTasksList.append(TaskToSubmit{"lai-time-series-builder", {}});
    outAllTasksList.append(TaskToSubmit{"lai-err-time-series-builder", {}});
    outAllTasksList.append(TaskToSubmit{"lai-msk-flags-time-series-builder", {}});
    if(bNDayReproc) {
        outAllTasksList.append(TaskToSubmit{"lai-local-window-reprocessing", {}});
        outAllTasksList.append(TaskToSubmit{"lai-local-window-reproc-splitter", {}});
    } else if(bFittedReproc) {
        outAllTasksList.append(TaskToSubmit{"lai-fitted-reprocessing", {}});
        outAllTasksList.append(TaskToSubmit{"lai-fitted-reproc-splitter", {}});
    }

    // now fill the tasks hierarchy infos
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
    int nCurTaskIdx = 0;
    m_nTimeSeriesBuilderIdx = nCurTaskIdx++;
    m_nErrTimeSeriesBuilderIdx = nCurTaskIdx++;
    m_nLaiMskFlgsTimeSeriesBuilderIdx = nCurTaskIdx++;

    if(bNDayReproc) {
        //profile-reprocessing -> time-series-builder AND err-time-series-builder AND lai-msk-flags-time-series-builder
        m_nProfileReprocessingIdx = nCurTaskIdx++;
        outAllTasksList[m_nProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nTimeSeriesBuilderIdx]);
        outAllTasksList[m_nProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nErrTimeSeriesBuilderIdx]);
        outAllTasksList[m_nProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx]);

        //reprocessed-profile-splitter -> profile-reprocessing
        m_nReprocessedProfileSplitterIdx = nCurTaskIdx++;
        outAllTasksList[m_nReprocessedProfileSplitterIdx].parentTasks.append(outAllTasksList[m_nProfileReprocessingIdx]);

        //product-formatter -> reprocessed-profile-splitter OR fitted-reprocessed-profile-splitter (OR BOTH)
        outProdFormatterParams.laiReprocParams.parentsTasksRef.append(outAllTasksList[m_nReprocessedProfileSplitterIdx]);
    } else if(bFittedReproc) {
        //fitted-profile-reprocessing -> time-series-builder AND err-time-series-builder AND lai-msk-flags-time-series-builder
        m_nFittedProfileReprocessingIdx = nCurTaskIdx++;
        outAllTasksList[m_nFittedProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nTimeSeriesBuilderIdx]);
        outAllTasksList[m_nFittedProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nErrTimeSeriesBuilderIdx]);
        outAllTasksList[m_nFittedProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx]);

        //fitted-reprocessed-profile-splitter -> fitted-profile-reprocessing
        m_nFittedProfileReprocessingSplitterIdx = nCurTaskIdx++;
        outAllTasksList[m_nFittedProfileReprocessingSplitterIdx].parentTasks.append(outAllTasksList[m_nFittedProfileReprocessingIdx]);

        //product-formatter -> reprocessed-profile-splitter OR fitted-reprocessed-profile-splitter (OR BOTH)
        outProdFormatterParams.laiFitParams.parentsTasksRef.append(outAllTasksList[m_nFittedProfileReprocessingSplitterIdx]);
    }
}

NewStepList LaiRetrievalHandlerL3C::GetStepsForMultiDateReprocessing(std::map<QString, QString> &configParameters,
                const TileTemporalFilesInfo &tileTemporalFilesInfo, QList<TaskToSubmit> &allTasksList,
                bool bNDayReproc, bool bFittedReproc, LAIProductFormatterParams &productFormatterParams)
{
    NewStepList steps;
    QString fittedFileListFileName;
    QString fittedFlagsFileListFileName;
    QString reprocFileListFileName;
    QString reprocFlagsFileListFileName;

    QStringList monoDateMskFlagsLaiFileNames2;
    QStringList quantifiedLaiFileNames2;
    QStringList quantifiedErrLaiFileNames2;
    QString mainLaiImg;
    QString mainLaiErrImg;
    QString mainMsksImg;
    for(int i = 0; i< tileTemporalFilesInfo.temporalTilesFileInfos.size(); i++) {
        quantifiedLaiFileNames2.append(tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[0]);
        quantifiedErrLaiFileNames2.append(tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[1]);
        monoDateMskFlagsLaiFileNames2.append(tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[2]);

        if(tileTemporalFilesInfo.temporalTilesFileInfos[i].satId == tileTemporalFilesInfo.primarySatelliteId) {
            if(mainLaiImg.length() == 0) {
                mainLaiImg = tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[0];
                mainLaiErrImg = tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[1];
                mainMsksImg = tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[2];
            }
        }
    }

    TaskToSubmit &imgTimeSeriesBuilderTask = allTasksList[m_nTimeSeriesBuilderIdx];
    TaskToSubmit &errTimeSeriesBuilderTask = allTasksList[m_nErrTimeSeriesBuilderIdx];
    TaskToSubmit &mskFlagsTimeSeriesBuilderTask = allTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx];

    const auto & allLaiTimeSeriesFileName = imgTimeSeriesBuilderTask.GetFilePath("LAI_time_series.tif");
    const auto & allErrTimeSeriesFileName = errTimeSeriesBuilderTask.GetFilePath("Err_time_series.tif");
    const auto & allMskFlagsTimeSeriesFileName = mskFlagsTimeSeriesBuilderTask.GetFilePath("Mask_Flags_time_series.tif");

    QStringList timeSeriesBuilderArgs = GetTimeSeriesBuilderArgs(quantifiedLaiFileNames2, allLaiTimeSeriesFileName, mainLaiImg);
    QStringList errTimeSeriesBuilderArgs = GetErrTimeSeriesBuilderArgs(quantifiedErrLaiFileNames2, allErrTimeSeriesFileName, mainLaiErrImg);
    QStringList mskFlagsTimeSeriesBuilderArgs = GetMskFlagsTimeSeriesBuilderArgs(monoDateMskFlagsLaiFileNames2, allMskFlagsTimeSeriesFileName, mainMsksImg);

    steps.append(imgTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", timeSeriesBuilderArgs));
    steps.append(errTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", errTimeSeriesBuilderArgs));
    steps.append(mskFlagsTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", mskFlagsTimeSeriesBuilderArgs));

    const QStringList &listProducts = ProcessorHandlerHelper::GetTemporalTileFiles(tileTemporalFilesInfo);

    if(bNDayReproc) {
        TaskToSubmit &profileReprocTask = allTasksList[m_nProfileReprocessingIdx];
        TaskToSubmit &profileReprocSplitTask = allTasksList[m_nReprocessedProfileSplitterIdx];

        const auto & reprocTimeSeriesFileName = profileReprocTask.GetFilePath("ReprocessedTimeSeries.tif");
        reprocFileListFileName = profileReprocSplitTask.GetFilePath("ReprocessedFilesList.txt");
        reprocFlagsFileListFileName = profileReprocSplitTask.GetFilePath("ReprocessedFlagsFilesList.txt");

        QStringList profileReprocessingArgs = GetProfileReprocessingArgs(configParameters, allLaiTimeSeriesFileName,
                                                                         allErrTimeSeriesFileName, allMskFlagsTimeSeriesFileName,
                                                                         reprocTimeSeriesFileName, listProducts);
        QStringList reprocProfileSplitterArgs = GetReprocProfileSplitterArgs(reprocTimeSeriesFileName, reprocFileListFileName,
                                                                             reprocFlagsFileListFileName, listProducts);
        steps.append(profileReprocTask.CreateStep("ProfileReprocessing", profileReprocessingArgs));
        steps.append(profileReprocSplitTask.CreateStep("ReprocessedProfileSplitter2", reprocProfileSplitterArgs));
        productFormatterParams.laiReprocParams.fileLaiReproc = reprocFileListFileName;
        productFormatterParams.laiReprocParams.fileLaiReprocFlgs = reprocFlagsFileListFileName;

    } else if(bFittedReproc) {
        TaskToSubmit &fittedProfileReprocTask = allTasksList[m_nFittedProfileReprocessingIdx];
        TaskToSubmit &fittedProfileReprocSplitTask = allTasksList[m_nFittedProfileReprocessingSplitterIdx];

        const auto & fittedTimeSeriesFileName = fittedProfileReprocTask.GetFilePath("FittedTimeSeries.tif");
        fittedFileListFileName = fittedProfileReprocSplitTask.GetFilePath("FittedFilesList.txt");
        fittedFlagsFileListFileName = fittedProfileReprocSplitTask.GetFilePath("FittedFlagsFilesList.txt");

        QStringList fittedProfileReprocArgs = GetFittedProfileReprocArgs(allLaiTimeSeriesFileName, allErrTimeSeriesFileName,
                                                                         allMskFlagsTimeSeriesFileName, fittedTimeSeriesFileName, listProducts);
        QStringList fittedProfileReprocSplitterArgs = GetFittedProfileReprocSplitterArgs(fittedTimeSeriesFileName, fittedFileListFileName,
                                                                                         fittedFlagsFileListFileName, listProducts);
        steps.append(fittedProfileReprocTask.CreateStep("ProfileReprocessing", fittedProfileReprocArgs));
        steps.append(fittedProfileReprocSplitTask.CreateStep("ReprocessedProfileSplitter2", fittedProfileReprocSplitterArgs));
        productFormatterParams.laiFitParams.fileLaiReproc = fittedFileListFileName;
        productFormatterParams.laiFitParams.fileLaiReprocFlgs = fittedFlagsFileListFileName;
    }

    return steps;
}

void LaiRetrievalHandlerL3C::HandleNewTilesList(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                             const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                             LAIGlobalExecutionInfos &outGlobalExecInfos) {

    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");

    bool bNDayReproc = IsNDayReproc(parameters, configParameters);
    bool bFittedReproc = IsFittedReproc(parameters, configParameters);

    QList<TaskToSubmit> &allTasksList = outGlobalExecInfos.allTasksList;
    LAIProductFormatterParams &productFormatterParams = outGlobalExecInfos.prodFormatParams;

    // create the tasks
    CreateTasksForNewProducts(allTasksList, outGlobalExecInfos.prodFormatParams,
                              bNDayReproc, bFittedReproc);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    // submit all tasks
    SubmitTasks(ctx, event.jobId, allTasksListRef);

    NewStepList &steps = outGlobalExecInfos.allStepsList;

    steps += GetStepsForMultiDateReprocessing(configParameters, tileTemporalFilesInfo, allTasksList,
                                              bNDayReproc, bFittedReproc, productFormatterParams);
}

void LaiRetrievalHandlerL3C::WriteExecutionInfosFile(const QString &executionInfosPath,
                                               std::map<QString, QString> &configParameters,
                                               const QStringList &listProducts, bool bIsReproc) {
    std::ofstream executionInfosFile;
    try
    {
        executionInfosFile.open(executionInfosPath.toStdString().c_str(), std::ofstream::out);
        executionInfosFile << "<?xml version=\"1.0\" ?>" << std::endl;
        executionInfosFile << "<metadata>" << std::endl;
        executionInfosFile << "  <General>" << std::endl;
        executionInfosFile << "  </General>" << std::endl;

        if(bIsReproc) {
            // Get the parameters from the configuration
            const auto &bwr = configParameters["processor.l3b.lai.localwnd.bwr"];
            const auto &fwr = configParameters["processor.l3b.lai.localwnd.fwr"];
            executionInfosFile << "  <ProfileReprocessing_parameters>" << std::endl;
            executionInfosFile << "    <bwr_for_algo_local_online_retrieval>" << bwr.toStdString() << "</bwr_for_algo_local_online_retrieval>" << std::endl;
            executionInfosFile << "    <fwr_for_algo_local_online_retrieval>"<< fwr.toStdString() <<"</fwr_for_algo_local_online_retrieval>" << std::endl;
            executionInfosFile << "  </ProfileReprocessing_parameters>" << std::endl;
        }
        executionInfosFile << "  <XML_files>" << std::endl;
        for (int i = 0; i<listProducts.size(); i++) {
            executionInfosFile << "    <XML_" << std::to_string(i) << ">" << listProducts[i].toStdString()
                               << "</XML_" << std::to_string(i) << ">" << std::endl;
        }
        executionInfosFile << "  </XML_files>" << std::endl;
        executionInfosFile << "</metadata>" << std::endl;
        executionInfosFile.close();
    }
    catch(...)
    {

    }
}

bool LaiRetrievalHandlerL3C::GetL2AProductsInterval(const QMap<QString, QStringList> &mapTilesMeta,
                                                    QDateTime &startDate, QDateTime &endDate) {
    bool bDatesInitialized = false;
    for(QString prd: mapTilesMeta.keys()) {
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

QStringList LaiRetrievalHandlerL3C::GetL3BProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                   const QMap<QString, QStringList> &inputProductToTilesMap) {
    QDateTime startDate;
    QDateTime endDate;
    QStringList filteredL3bProductList;
    if(GetL2AProductsInterval(inputProductToTilesMap, startDate, endDate)) {
        // we consider the end date until the end of day
        endDate = endDate.addSecs(SECONDS_IN_DAY-1);
        ProductList l3bProductList = ctx.GetProducts(event.siteId, (int)ProductType::L3BProductTypeId, startDate, endDate);
        for(Product l3bPrd: l3bProductList) {
            filteredL3bProductList.append(l3bPrd.fullPath);
        }
    }

    return filteredL3bProductList;
}

QMap<QString, TileTemporalFilesInfo> LaiRetrievalHandlerL3C::GetL3BMapTiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                                            QMap<QString, TileTemporalFilesInfo> mapTiles, QStringList &l3bProductsFiltered) {
    QMap<QString, QStringList> inputProductToTilesMap;
    QStringList listTilesMetaFiles = GetL2AInputProductsTiles(ctx, event, inputProductToTilesMap);
    if(listTilesMetaFiles.size() == 0) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No products provided at input or no products available in the specified interval").
                    toStdString());
    }

    // get the L3B products for the current product tiles
    QStringList filteredL3bProductList = GetL3BProducts(ctx, event, inputProductToTilesMap);

    // iterate through the L2A tiles and get the corresponding L3B product
    // Create a QMap<QString, TileTemporalFilesInfo> but with infos about L3B files and satelites
    // according to the input L2A tile file info
    QMap<QString, TileTemporalFilesInfo> l3bMapTiles;
    for(auto tileId : mapTiles.keys())
    {
       const TileTemporalFilesInfo &listTemporalTiles = mapTiles.value(tileId);
       TileTemporalFilesInfo newTileInfos;
       newTileInfos.tileId = tileId;
       // iterate the L2A tiles and for each tile, get the L3B files available
       for(ProcessorHandlerHelper::InfoTileFile l2aTileInfo: listTemporalTiles.temporalTilesFileInfos) {
           for(QString l3bPrd: filteredL3bProductList) {
               if(ProcessorHandlerHelper::HighLevelPrdHasL2aSource(l3bPrd, l2aTileInfo.file)) {
                   QMap<QString, QString> mapTiles = ProcessorHandlerHelper::GetHighLevelProductTilesDirs(l3bPrd);
                   // get the tiles and their folders for the L3B product
                   // search the tile Id (normally, we should have it)
                   QString tileDir;
                   if(mapTiles.contains(tileId)) {
                        tileDir = mapTiles[tileId];
                   } else {
                        // this is probably a secondary L8 product
                        if((l2aTileInfo.satId != listTemporalTiles.primarySatelliteId) && (mapTiles.size() == 1)) {
                            tileDir = mapTiles.first();
                        }
                   }
                   if(tileDir.length() > 0) {
                       // fill the empty gaps for these lists
                       ProcessorHandlerHelper::InfoTileFile l3bTileInfo;
                       //update the sat id
                       l3bTileInfo.satId = l2aTileInfo.satId;
                       // update the files
                       l3bTileInfo.file = l2aTileInfo.file;
                       l3bTileInfo.additionalFiles.append(ProcessorHandlerHelper::GetHigLevelProductTileFile(tileDir, "SLAIMONO"));
                       l3bTileInfo.additionalFiles.append(ProcessorHandlerHelper::GetHigLevelProductTileFile(tileDir, "MLAIERR", true));
                       l3bTileInfo.additionalFiles.append(ProcessorHandlerHelper::GetHigLevelProductTileFile(tileDir, "MMONODFLG", true));
                       // add the sat id to the list of unique sat ids
                       if(!newTileInfos.uniqueSatteliteIds.contains(l3bTileInfo.satId)) {
                            newTileInfos.uniqueSatteliteIds.append(l3bTileInfo.satId);
                       }
                       //add it to the temporal tile files info
                       newTileInfos.temporalTilesFileInfos.append(l3bTileInfo);
                       if(!l3bProductsFiltered.contains(l3bPrd)) {
                           l3bProductsFiltered.append(l3bPrd);
                       }
                   }
               }
           }
       }
       if(newTileInfos.temporalTilesFileInfos.size() > 0) {
            // update the other information
            newTileInfos.primarySatelliteId = ProcessorHandlerHelper::GetPrimarySatelliteId(newTileInfos.uniqueSatteliteIds);
            // add it to the returned map
            l3bMapTiles[tileId] = newTileInfos;
       }
    }
    return l3bMapTiles;
}

void LaiRetrievalHandlerL3C::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");

    bool bNDayReproc = IsNDayReproc(parameters, configParameters);
    bool bFittedReproc = IsFittedReproc(parameters, configParameters);
    if(!bNDayReproc && !bFittedReproc) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("Only one processing needs to be defined ("
                           " LAI N-reprocessing or LAI Fitted)").toStdString());
    }

    QMap<QString, QStringList> inputProductToTilesMap;
    QStringList listTilesMetaFiles = GetL2AInputProductsTiles(ctx, event, inputProductToTilesMap);
    if(listTilesMetaFiles.size() == 0) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No products provided at input or no products available in the specified interval").
                    toStdString());
    }
    // perform the L3B tile to temporal files mapping
    QMap<QString, TileTemporalFilesInfo> mapTiles = GroupTiles(ctx, event.jobId, listTilesMetaFiles,
                                                               ProductType::L2AProductTypeId);
    // perform the L3B tile to temporal files mapping
    QStringList filteredL3bPrds;
    QMap<QString, TileTemporalFilesInfo> l3bMapTiles = GetL3BMapTiles(ctx, event, mapTiles, filteredL3bPrds);

    QList<LAIProductFormatterParams> listParams;
    NewStepList allSteps;
    //container for all task
    QList<TaskToSubmit> allTasksList;
    //container for all global execution infos
    QList<LAIGlobalExecutionInfos> allLaiGlobalExecInfos;

    // after retrieving the L3C products is possible to have only a subset of the original L2A products
    QStringList realL2AMetaFiles;
    for(auto tileId : l3bMapTiles.keys())
    {
       const TileTemporalFilesInfo &listTemporalTiles = l3bMapTiles.value(tileId);
       Logger::debug(QStringLiteral("Handling tile %1 from a number of %2 tiles").arg(tileId).arg(l3bMapTiles.size()));

       allLaiGlobalExecInfos.append(LAIGlobalExecutionInfos());
       LAIGlobalExecutionInfos &infosRef = allLaiGlobalExecInfos[allLaiGlobalExecInfos.size()-1];
       infosRef.prodFormatParams.tileId = GetProductFormatterTile(tileId);
       HandleNewTilesList(ctx, event, listTemporalTiles, infosRef);
       if(infosRef.allTasksList.size() > 0 && infosRef.allStepsList.size() > 0) {
           listParams.append(infosRef.prodFormatParams);
           allTasksList.append(infosRef.allTasksList);
           allSteps.append(infosRef.allStepsList);
           realL2AMetaFiles += ProcessorHandlerHelper::GetTemporalTileFiles(listTemporalTiles);
       }
    }

    // Create the product formatter tasks for the Reprocessed and/or Fitted Products (if needed)
    allTasksList.append({"", {}});
    TaskToSubmit &productFormatterTask = allTasksList[allTasksList.size()-1];
    if(bNDayReproc) {
        productFormatterTask.moduleName = "lai-reproc-product-formatter";
        for(LAIProductFormatterParams params: listParams) {
            productFormatterTask.parentTasks.append(params.laiReprocParams.parentsTasksRef);
        }
    } else if(bFittedReproc) {
        productFormatterTask.moduleName = "lai-fitted-product-formatter";
        TaskToSubmit &laiFittedProductFormatterTask = allTasksList[allTasksList.size()-1];
        for(LAIProductFormatterParams params: listParams) {
            laiFittedProductFormatterTask.parentTasks.append(params.laiFitParams.parentsTasksRef);
        }
    }
    SubmitTasks(ctx, event.jobId, {productFormatterTask});
    QStringList productFormatterArgs = GetReprocProductFormatterArgs(productFormatterTask, ctx,
                                          event, realL2AMetaFiles, listParams, !bNDayReproc);
    // add these steps to the steps list to be submitted
    allSteps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
    ctx.SubmitSteps(allSteps);
}

void LaiRetrievalHandlerL3C::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    bool isReprocPf = false, isFittedPf = false;
    if ((isReprocPf = (event.module == "lai-reproc-product-formatter")) ||
         (isFittedPf = (event.module == "lai-fitted-product-formatter"))) {
        QString prodName = GetProductFormatterProductName(ctx, event);
        QString productFolder = GetProductFormatterOutputProductPath(ctx, event);
        if((prodName != "") && ProcessorHandlerHelper::IsValidHighLevelProduct(productFolder)) {
            QString quicklook = GetProductFormatterQuicklook(ctx, event);
            QString footPrint = GetProductFormatterFootprint(ctx, event);
            ProductType prodType = ProductType::L3BProductTypeId;
            if(isReprocPf) prodType = ProductType::L3CProductTypeId;
            else if (isFittedPf) prodType = ProductType::L3DProductTypeId;

            // Insert the product into the database
            QDateTime minDate, maxDate;
            ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prodName, minDate, maxDate);
            int ret = ctx.InsertProduct({ prodType, event.processorId, event.siteId, event.jobId,
                                productFolder, maxDate, prodName,
                                quicklook, footPrint, std::experimental::nullopt, TileIdList() });
            Logger::debug(QStringLiteral("InsertProduct for %1 returned %2").arg(prodName).arg(ret));

        } else {
            Logger::error(QStringLiteral("Cannot insert into database the product with name %1 and folder %2").arg(prodName).arg(productFolder));
            ctx.MarkJobFailed(event.jobId);
        }
        // Now remove the job folder containing temporary files
        RemoveJobFolder(ctx, event.jobId, "l3b");
    }
}

QStringList LaiRetrievalHandlerL3C::GetTimeSeriesBuilderArgs(const QStringList &monoDateLaiFileNames,
                                                             const QString &allLaiTimeSeriesFileName, const QString &mainImg) {
    QStringList timeSeriesBuilderArgs = { "TimeSeriesBuilder",
      "-out", allLaiTimeSeriesFileName,
      "-main", mainImg,
      "-il"
    };
    timeSeriesBuilderArgs += monoDateLaiFileNames;

    return timeSeriesBuilderArgs;
}

QStringList LaiRetrievalHandlerL3C::GetErrTimeSeriesBuilderArgs(const QStringList &monoDateErrLaiFileNames,
                                                                const QString &allErrTimeSeriesFileName, const QString &mainImg) {
    QStringList timeSeriesBuilderArgs = { "TimeSeriesBuilder",
      "-out", allErrTimeSeriesFileName,
      "-main", mainImg,
      "-il"
    };
    timeSeriesBuilderArgs += monoDateErrLaiFileNames;

    return timeSeriesBuilderArgs;
}

QStringList LaiRetrievalHandlerL3C::GetMskFlagsTimeSeriesBuilderArgs(const QStringList &monoDateMskFlagsLaiFileNames,
                                                                     const QString &allMskFlagsTimeSeriesFileName,  const QString &mainImg) {
    QStringList timeSeriesBuilderArgs = { "TimeSeriesBuilder",
      "-out", allMskFlagsTimeSeriesFileName,
      "-main", mainImg,
      "-isflg", "1",
      "-il"
    };
    timeSeriesBuilderArgs += monoDateMskFlagsLaiFileNames;

    return timeSeriesBuilderArgs;
}

QStringList LaiRetrievalHandlerL3C::GetProfileReprocessingArgs(std::map<QString, QString> configParameters, const QString &allLaiTimeSeriesFileName,
                                       const QString &allErrTimeSeriesFileName, const QString &allMsksTimeSeriesFileName,
                                       const QString &reprocTimeSeriesFileName, const QStringList &listProducts) {
    const auto &localWindowBwr = configParameters["processor.l3b.lai.localwnd.bwr"];
    const auto &localWindowFwr = configParameters["processor.l3b.lai.localwnd.fwr"];

    QStringList profileReprocessingArgs = { "ProfileReprocessing",
          "-lai", allLaiTimeSeriesFileName,
          "-err", allErrTimeSeriesFileName,
          "-msks", allMsksTimeSeriesFileName,
          "-opf", reprocTimeSeriesFileName,
          "-algo", "local",
          "-algo.local.bwr", localWindowBwr,
          "-algo.local.fwr", localWindowFwr,
          "-ilxml"
    };
    profileReprocessingArgs += listProducts;
    return profileReprocessingArgs;
}

QStringList LaiRetrievalHandlerL3C::GetReprocProfileSplitterArgs(const QString &reprocTimeSeriesFileName, const QString &reprocFileListFileName,
                                                              const QString &reprocFlagsFileListFileName,
                                                              const QStringList &allXmlsFileName) {
    QStringList args = { "ReprocessedProfileSplitter2",
            "-in", reprocTimeSeriesFileName,
            "-outrlist", reprocFileListFileName,
            "-outflist", reprocFlagsFileListFileName,
            "-compress", "1",
            "-ilxml"
    };
    args += allXmlsFileName;
    return args;
}

QStringList LaiRetrievalHandlerL3C::GetFittedProfileReprocArgs(const QString &allLaiTimeSeriesFileName, const QString &allErrTimeSeriesFileName,
                                       const QString &allMsksTimeSeriesFileName, const QString &fittedTimeSeriesFileName, const QStringList &listProducts) {
    QStringList fittedProfileReprocArgs = { "ProfileReprocessing",
          "-lai", allLaiTimeSeriesFileName,
          "-err", allErrTimeSeriesFileName,
          "-msks", allMsksTimeSeriesFileName,
          "-opf", fittedTimeSeriesFileName,
          "-algo", "fit",
          "-ilxml"
    };
    fittedProfileReprocArgs += listProducts;
    return fittedProfileReprocArgs;
}

QStringList LaiRetrievalHandlerL3C::GetFittedProfileReprocSplitterArgs(const QString &fittedTimeSeriesFileName, const QString &fittedFileListFileName,
                                                                    const QString &fittedFlagsFileListFileName,
                                                                    const QStringList &allXmlsFileName) {
    QStringList args = { "ReprocessedProfileSplitter2",
                "-in", fittedTimeSeriesFileName,
                "-outrlist", fittedFileListFileName,
                "-outflist", fittedFlagsFileListFileName,
                "-compress", "1",
                "-ilxml"
    };
    args += allXmlsFileName;
    return args;
}

QStringList LaiRetrievalHandlerL3C::GetReprocProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                    const QStringList &listProducts, const QList<LAIProductFormatterParams> &productParams, bool isFitted) {

    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");

    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.xml");

    const auto &lutFile = configParameters["processor.l3b.lai.lut_path"];

    WriteExecutionInfosFile(executionInfosPath, configParameters, listProducts, !isFitted);
    QString l3ProductType = isFitted ? "L3D" : "L3C";
    QString productShortName = isFitted ? "l3d_fitted": "l3c_reproc";
    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId, productShortName);

    QStringList productFormatterArgs = { "ProductFormatter",
                            "-destroot", targetFolder,
                            "-fileclass", "OPER",
                            "-level", l3ProductType,
                            "-baseline", "01.00",
                            "-siteid", QString::number(event.siteId),
                            "-processor", "vegetation",
                            "-gipp", executionInfosPath,
                            "-outprops", outPropsPath};
    productFormatterArgs += "-il";
    productFormatterArgs += listProducts;

    if(lutFile.size() > 0) {
        productFormatterArgs += "-lut";
        productFormatterArgs += lutFile;
    }

    if(isFitted) {
        productFormatterArgs += "-processor.vegetation.filelaifit";
        for(const LAIProductFormatterParams &params: productParams) {
            productFormatterArgs += params.tileId;
            productFormatterArgs += params.laiFitParams.fileLaiReproc;
        }
        productFormatterArgs += "-processor.vegetation.filelaifitflgs";
        for(const LAIProductFormatterParams &params: productParams) {
            productFormatterArgs += params.tileId;
            productFormatterArgs += params.laiFitParams.fileLaiReproc;
        }
    } else {
        productFormatterArgs += "-processor.vegetation.filelaireproc";
        for(const LAIProductFormatterParams &params: productParams) {
            productFormatterArgs += params.tileId;
            productFormatterArgs += params.laiReprocParams.fileLaiReproc;
        }
        productFormatterArgs += "-processor.vegetation.filelaireprocflgs";
        for(const LAIProductFormatterParams &params: productParams) {
            productFormatterArgs += params.tileId;
            productFormatterArgs += params.laiReprocParams.fileLaiReprocFlgs;
        }
    }

    return productFormatterArgs;
}


bool LaiRetrievalHandlerL3C::IsNDayReproc(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
    bool bNDayReproc = false;
    if(parameters.contains("reproc")) {
        const auto &value = parameters["reproc"];
        if(value.isDouble())
            bNDayReproc = (value.toInt() != 0);
        else if(value.isString()) {
            bNDayReproc = (value.toString() == "1");
        }
    } else {
        bNDayReproc = ((configParameters["processor.l3b.reprocess"]).toInt() != 0);
    }
    return bNDayReproc;
}

bool LaiRetrievalHandlerL3C::IsFittedReproc(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
    bool bFittedReproc = false;
    if(parameters.contains("fitted")) {
        const auto &value = parameters["fitted"];
        if(value.isDouble())
            bFittedReproc = (value.toInt() != 0);
        else if(value.isString()) {
            bFittedReproc = (value.toString() == "1");
        }
    } else {
        bFittedReproc = ((configParameters["processor.l3b.fitted"]).toInt() != 0);
    }
    return bFittedReproc;
}

ProcessorJobDefinitionParams LaiRetrievalHandlerL3C::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                          const ConfigurationParameterValueMap &requestOverrideCfgValues)
{
    ProcessorJobDefinitionParams params;
    params.isValid = false;

    QDateTime seasonStartDate;
    QDateTime seasonEndDate;
    // extract the scheduled date
    QDateTime qScheduledDate = QDateTime::fromTime_t(scheduledDate);
    GetSeasonStartEndDates(ctx, siteId, seasonStartDate, seasonEndDate, qScheduledDate, requestOverrideCfgValues);
    QDateTime limitDate = seasonEndDate.addMonths(2);
    if(qScheduledDate > limitDate) {
        return params;
    }
    if(!seasonStartDate.isValid()) {
        Logger::error(QStringLiteral("Season start date for site ID %1 is invalid in the database!")
                      .arg(siteId));
        return params;
    }

    ConfigurationParameterValueMap mapCfg = ctx.GetConfigurationParameters(QString("processor.l3b."), siteId, requestOverrideCfgValues);

    // we might have an offset in days from starting the downloading products to start the L3B/L3C/L3D production
    int startSeasonOffset = mapCfg["processor.l3b.start_season_offset"].value.toInt();
    seasonStartDate = seasonStartDate.addDays(startSeasonOffset);

    int generateLai = false;
    int generateReprocess = false;
    int generateFitted = false;

    if(requestOverrideCfgValues.contains("product_type")) {
        const ConfigurationParameterValue &productType = requestOverrideCfgValues["product_type"];
        if(productType.value == "L3B") {
            generateLai = true;
            params.jsonParameters = "{ \"monolai\": \"1\"}";
        } else if(productType.value == "L3C") {
            generateReprocess = true;
            params.jsonParameters = "{ \"reproc\": \"1\"}";
        } else if(productType.value == "L3D") {
            generateFitted = true;
            params.jsonParameters = "{ \"fitted\": \"1\"}";
        }
    }
    // we need to have at least one flag set
    if(!generateLai && !generateReprocess && !generateFitted) {
        return params;
    }

    // by default the start date is the season start date
    QDateTime startDate = seasonStartDate;
    QDateTime endDate = qScheduledDate;

    if(generateLai || generateReprocess) {
        int productionInterval = mapCfg[generateLai ? "processor.l3b.production_interval":
                                                      "processor.l3b.reproc_production_interval"].value.toInt();
        startDate = endDate.addDays(-productionInterval);
        // Use only the products after the configured start season date
        if(startDate < seasonStartDate) {
            startDate = seasonStartDate;
        }
    }

    params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    // Normally, we need at least 1 product available in order to be able to create a L3B/L3C/L3D product
    // but if we do not return here, the schedule block waiting for products (that might never happen)
    bool waitForAvailProcInputs = (mapCfg["processor.l3b.sched_wait_proc_inputs"].value.toInt() != 0);
    if((waitForAvailProcInputs == false) || (params.productList.size() > 0)) {
        params.isValid = true;
//        if(generateLai) {
//            params.isValid = true;
//        } else if(generateFitted) {
//            if(params.productList.size() > 4) {
//                params.isValid = true;
//            }
//        } else if (generateReprocess > 2) {
//            params.isValid = true;
//        }
        Logger::debug(QStringLiteral("Executing scheduled job. Scheduler extracted for L3B/L3C/L3D a number "
                                     "of %1 products for site ID %2 with start date %3 and end date %4!")
                      .arg(params.productList.size())
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    } else {
        Logger::debug(QStringLiteral("Scheduled job for L3B/L3C/L3D and site ID %1 with start date %2 and end date %3 "
                                     "will not be executed (no products)!")
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    }

    return params;
}

