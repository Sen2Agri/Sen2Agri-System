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

bool compareInfoTileFiles(const ProcessorHandlerHelper::InfoTileFile& infoTile1,const ProcessorHandlerHelper::InfoTileFile& infoTile2)
{
    return infoTile1.acquisitionDate < infoTile2.acquisitionDate;
}


void LaiRetrievalHandlerL3C::CreateTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                                    LAIProductFormatterParams &outProdFormatterParams,
                                                    bool bNDayReproc, bool bFittedReproc, bool bRemoveTempFiles) {
    int initialTasksNo = outAllTasksList.size();
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
    if(bRemoveTempFiles) {
        outAllTasksList.append(TaskToSubmit{ "files-remover", {} });
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
    // Specifies if the products creation should be chained or not.
    // TODO: This should be taken from the configuration
    bool bChainProducts = true;

    int nCurTaskIdx = initialTasksNo;
    int nTimeSeriesBuilderIdx = nCurTaskIdx++;
    int nErrTimeSeriesBuilderIdx = nCurTaskIdx++;
    int nLaiMskFlgsTimeSeriesBuilderIdx = nCurTaskIdx++;

    // if we chain this product from another product
    if(bChainProducts && initialTasksNo > 0) {
        int prevProductLastTaskIdx = initialTasksNo-1;
        // we create a dependency to the last task of the previous product
        outAllTasksList[nTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[prevProductLastTaskIdx]);
        outAllTasksList[nErrTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[prevProductLastTaskIdx]);
        outAllTasksList[nLaiMskFlgsTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[prevProductLastTaskIdx]);
    }

    //profile-reprocessing -> time-series-builder AND err-time-series-builder AND lai-msk-flags-time-series-builder
    int nProfileReprocessingIdx = nCurTaskIdx++;
    outAllTasksList[nProfileReprocessingIdx].parentTasks.append(outAllTasksList[nTimeSeriesBuilderIdx]);
    outAllTasksList[nProfileReprocessingIdx].parentTasks.append(outAllTasksList[nErrTimeSeriesBuilderIdx]);
    outAllTasksList[nProfileReprocessingIdx].parentTasks.append(outAllTasksList[nLaiMskFlgsTimeSeriesBuilderIdx]);

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

NewStepList LaiRetrievalHandlerL3C::GetStepsForMultiDateReprocessing(std::map<QString, QString> &configParameters,
                const TileTemporalFilesInfo &tileTemporalFilesInfo, QList<TaskToSubmit> &allTasksList,
                bool bNDayReproc, bool bFittedReproc, LAIProductFormatterParams &productFormatterParams,
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

    for(const ProcessorHandlerHelper::InfoTileFile &fileInfo: tileTemporalFilesInfo.temporalTilesFileInfos) {
        const QString &laiFileName = fileInfo.additionalFiles[LAI_RASTER_ADD_INFO_IDX];
        const QString &errFileName = fileInfo.additionalFiles[LAI_ERR_RASTER_ADD_INFO_IDX];
        const QString &mskFileName = fileInfo.additionalFiles[LAI_FLG_RASTER_ADD_INFO_IDX];

        // ensure that we have all files and we will have the same size for all lists
        if (laiFileName.size() > 0 && errFileName.size() > 0 && mskFileName.size() > 0) {
            quantifiedLaiFileNames2.append(laiFileName);
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
/*
    for(int i = 0; i< tileTemporalFilesInfo.temporalTilesFileInfos.size(); i++) {
        quantifiedLaiFileNames2.append(tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[LAI_RASTER_ADD_INFO_IDX]);
        quantifiedErrLaiFileNames2.append(tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[LAI_ERR_RASTER_ADD_INFO_IDX]);
        monoDateMskFlagsLaiFileNames2.append(tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[LAI_FLG_RASTER_ADD_INFO_IDX]);

        if(tileTemporalFilesInfo.temporalTilesFileInfos[i].satId == tileTemporalFilesInfo.primarySatelliteId) {
            if(mainLaiImg.length() == 0) {
                mainLaiImg = tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[LAI_RASTER_ADD_INFO_IDX];
                mainLaiErrImg = tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[LAI_ERR_RASTER_ADD_INFO_IDX];
                mainMsksImg = tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[LAI_FLG_RASTER_ADD_INFO_IDX];
            }
        }
    }
*/
    int curTaskIdx = tasksStartIdx;
    TaskToSubmit &imgTimeSeriesBuilderTask = allTasksList[curTaskIdx++];
    TaskToSubmit &errTimeSeriesBuilderTask = allTasksList[curTaskIdx++];
    TaskToSubmit &mskFlagsTimeSeriesBuilderTask = allTasksList[curTaskIdx++];

    const auto & allLaiTimeSeriesFileName = imgTimeSeriesBuilderTask.GetFilePath("LAI_time_series.tif");
    const auto & allErrTimeSeriesFileName = errTimeSeriesBuilderTask.GetFilePath("Err_time_series.tif");
    const auto & allMskFlagsTimeSeriesFileName = mskFlagsTimeSeriesBuilderTask.GetFilePath("Mask_Flags_time_series.tif");

    QStringList timeSeriesBuilderArgs = GetTimeSeriesBuilderArgs(quantifiedLaiFileNames2, allLaiTimeSeriesFileName, mainLaiImg);
    QStringList errTimeSeriesBuilderArgs = GetErrTimeSeriesBuilderArgs(quantifiedErrLaiFileNames2, allErrTimeSeriesFileName, mainLaiErrImg);
    QStringList mskFlagsTimeSeriesBuilderArgs = GetMskFlagsTimeSeriesBuilderArgs(monoDateMskFlagsLaiFileNames2, allMskFlagsTimeSeriesFileName, mainMsksImg);

    steps.append(imgTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", timeSeriesBuilderArgs));
    steps.append(errTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", errTimeSeriesBuilderArgs));
    steps.append(mskFlagsTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", mskFlagsTimeSeriesBuilderArgs));

//    const QStringList &listDates = ProcessorHandlerHelper::GetTemporalTileAcquisitionDates(tileTemporalFilesInfo);

    TaskToSubmit &profileReprocTask = allTasksList[curTaskIdx++];
    TaskToSubmit &profileReprocSplitTask = allTasksList[curTaskIdx++];

    const auto & reprocTimeSeriesFileName = profileReprocTask.GetFilePath("ReprocessedTimeSeries.tif");
    reprocFileListFileName = profileReprocSplitTask.GetFilePath("ReprocessedFilesList.txt");
    reprocFlagsFileListFileName = profileReprocSplitTask.GetFilePath("ReprocessedFlagsFilesList.txt");

    QStringList profileReprocessingArgs;
    if(bNDayReproc) {
        profileReprocessingArgs = GetProfileReprocessingArgs(configParameters, allLaiTimeSeriesFileName,
                                                                         allErrTimeSeriesFileName, allMskFlagsTimeSeriesFileName,
                                                                         reprocTimeSeriesFileName, listDates);
    } else if(bFittedReproc) {
        profileReprocessingArgs = GetFittedProfileReprocArgs(allLaiTimeSeriesFileName,
                                                             allErrTimeSeriesFileName, allMskFlagsTimeSeriesFileName,
                                                             reprocTimeSeriesFileName, listDates);
    }
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


void LaiRetrievalHandlerL3C::CreateTasksForNewProducts_New(QList<TaskToSubmit> &outAllTasksList,
                                                    LAIProductFormatterParams &outProdFormatterParams,
                                                    bool bNDayReproc, bool bRemoveTempFiles) {
    int initialTasksNo = outAllTasksList.size();
    // just create the tasks but with no information so far
    // first we add the tasks to be performed for each product
    if(bNDayReproc) {
        outAllTasksList.append(TaskToSubmit{"lai-local-window-reprocessing", {}});
        outAllTasksList.append(TaskToSubmit{"lai-local-window-reproc-splitter", {}});
    } else {
        outAllTasksList.append(TaskToSubmit{"lai-fitted-reprocessing", {}});
        outAllTasksList.append(TaskToSubmit{"lai-fitted-reproc-splitter", {}});
    }
    if(bRemoveTempFiles) {
        outAllTasksList.append(TaskToSubmit{ "files-remover", {} });
    }


    // now fill the tasks hierarchy infos
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
    int nProfileReprocessingIdx = nCurTaskIdx++;

    // if we chain this product from another product
    if(bChainProducts && initialTasksNo > 0) {
        int prevProductLastTaskIdx = initialTasksNo-1;
        // we create a dependency to the last task of the previous product
            outAllTasksList[nProfileReprocessingIdx].parentTasks.append(outAllTasksList[prevProductLastTaskIdx]);
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

NewStepList LaiRetrievalHandlerL3C::GetStepsForMultiDateReprocessing_New(std::map<QString, QString> &configParameters,
                const TileTemporalFilesInfo &tileTemporalFilesInfo, QList<TaskToSubmit> &allTasksList,
                bool bNDayReproc, LAIProductFormatterParams &productFormatterParams,
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

    QStringList profileReprocessingArgs;
    if(bNDayReproc) {
        profileReprocessingArgs = GetProfileReprocessingArgs_New(configParameters, quantifiedLaiFileNames2,
                                                              quantifiedErrLaiFileNames2, monoDateMskFlagsLaiFileNames2, mainLaiImg,
                                                              reprocTimeSeriesFileName, listDates);
    } else {
        profileReprocessingArgs = GetFittedProfileReprocArgs_New(quantifiedLaiFileNames2,
                                                             quantifiedErrLaiFileNames2, monoDateMskFlagsLaiFileNames2, mainLaiImg,
                                                             reprocTimeSeriesFileName, listDates);
    }

    QStringList reprocProfileSplitterArgs = GetReprocProfileSplitterArgs(reprocTimeSeriesFileName, reprocFileListFileName,
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




void LaiRetrievalHandlerL3C::HandleNewTilesList(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                             const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                             LAIGlobalExecutionInfos &outGlobalExecInfos, bool bRemoveTempFiles) {

    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");

    bool bNDayReproc = IsNDayReproc(parameters, configParameters);
    bool bFittedReproc = IsFittedReproc(parameters, configParameters);

    QList<TaskToSubmit> &allTasksList = outGlobalExecInfos.allTasksList;
    LAIProductFormatterParams &productFormatterParams = outGlobalExecInfos.prodFormatParams;

    int tasksStartIdx = allTasksList.size();

    // create the tasks
    bool useCompactReprocessing = IsReprocessingCompact(parameters, configParameters);
    if(useCompactReprocessing) {
        CreateTasksForNewProducts_New(allTasksList, outGlobalExecInfos.prodFormatParams, bNDayReproc, bRemoveTempFiles);
    } else {
        CreateTasksForNewProducts(allTasksList, outGlobalExecInfos.prodFormatParams,
                                  bNDayReproc, bFittedReproc, bRemoveTempFiles);
    }

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    // submit all tasks
    SubmitTasks(ctx, event.jobId, allTasksListRef);

    NewStepList &steps = outGlobalExecInfos.allStepsList;

    if(useCompactReprocessing) {

        steps += GetStepsForMultiDateReprocessing_New(configParameters, tileTemporalFilesInfo, allTasksList,
                                              bNDayReproc, productFormatterParams, tasksStartIdx, bRemoveTempFiles);
    } else {
        steps += GetStepsForMultiDateReprocessing(configParameters, tileTemporalFilesInfo, allTasksList,
                                              bNDayReproc, bFittedReproc, productFormatterParams,
                                              tasksStartIdx, bRemoveTempFiles);
    }
}

void LaiRetrievalHandlerL3C::WriteExecutionInfosFile(const QString &executionInfosPath,
                                               std::map<QString, QString> &configParameters,
                                               const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles,
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

bool LaiRetrievalHandlerL3C::GetL2AProductsInterval(const QMap<QString, QStringList> &mapTilesMeta,
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
 * Get all L3B products from the beginning of the season until now
 */
//TODO: This function should receive the Product and QList<Product> instead of just product path as these can be got from DB
//      The Product contains already the tiles, the full path and the acquisition date so can be avoided parsing files
QStringList LaiRetrievalHandlerL3C::GetL3BProductsSinceStartOfSeason(EventProcessingContext &ctx, int siteId, const QStringList &listExistingPrds)
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

/**
 * Get the L3B products from the received event. If the input products are L2A then the associated L3B products are
 * search and returned
 */
//TODO: This function should receive the Product and QList<Product> instead of just product path as these can be got from DB
//      The Product contains already the tiles, the full path and the acquisition date so can be avoided parsing files
QStringList LaiRetrievalHandlerL3C::GetL3BProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event)
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
        QStringList listTilesMetaFiles = GetL2AInputProductsTiles(ctx, event, inputProductToTilesMap);

        // get the L3B products for the current product tiles
        QDateTime startDate;
        QDateTime endDate;
        if(GetL2AProductsInterval(inputProductToTilesMap, startDate, endDate)) {
            // we consider the end date until the end of day
            endDate = endDate.addSecs(SECONDS_IN_DAY-1);
            ProductList l3bProductList = ctx.GetProducts(event.siteId, (int)ProductType::L3BProductTypeId, startDate, endDate);
            for(Product l3bPrd: l3bProductList) {
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

//TODO: This function should receive the Product and QList<Product> instead of just product path as these can be got from DB
//      The Product contains already the tiles, the full path and the acquisition date so can be avoided parsing files
QMap<QString, TileTemporalFilesInfo> LaiRetrievalHandlerL3C::GetL3BMapTiles(EventProcessingContext &ctx,
                                                                            const QStringList &l3bProducts,
                                                                            const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles)
{
    QMap<QString, TileTemporalFilesInfo> retL3bMapTiles;
    for(const QString &l3bProd: l3bProducts) {
//        QDateTime minDate, maxDate;
//        ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(l3bProd, minDate, maxDate);
        const QMap<QString, QString> &mapL3BTiles = ProcessorHandlerHelper::GetHighLevelProductTilesDirs(l3bProd);
        for(const auto &tileId : mapL3BTiles.keys()) {

            // TODO: see if a limitation is needed based on the satellite ID (only S2?)

            ProcessorHandlerHelper::SatelliteIdType tileSatId = GetSatIdForTile(siteTiles, tileId);
            if(!retL3bMapTiles.contains(tileId)) {
                TileTemporalFilesInfo newTileInfos;
                newTileInfos.tileId = tileId;
                // add the tile infos to the map
                retL3bMapTiles[tileId] = newTileInfos;
            }
            TileTemporalFilesInfo &tileInfo = retL3bMapTiles[tileId];
            for(const QString &curL3bPrd: l3bProducts) {
                QDateTime minDate, maxDate;
                ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(curL3bPrd, minDate, maxDate);
                // Fill the tile information for the current tile from the current product
                AddTileFileInfo(ctx, tileInfo, curL3bPrd, tileId, siteTiles, tileSatId, minDate);
            }
            if(tileInfo.temporalTilesFileInfos.size() > 0) {
                 // update the primary satellite information
                 tileInfo.primarySatelliteId = ProcessorHandlerHelper::GetPrimarySatelliteId(tileInfo.uniqueSatteliteIds);
            }
        }
    }
    // if primary and secondary satellites, then keep only the tiles from primary satellite
    // as we don't want to have in the resulted product combined primary and secondary satellites tiles
    //return retL3bMapTiles;
    // NOT NEEDED ANYMORE - filtering done in scheduled part
    return FilterSecondaryProductTiles(retL3bMapTiles, siteTiles);
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

bool LaiRetrievalHandlerL3C::AddTileFileInfo(EventProcessingContext &ctx, TileTemporalFilesInfo &temporalTileInfo, const QString &l3bPrd, const QString &tileId,
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
            const TileList &allIntersectingTiles = ctx.GetIntersectingTiles(static_cast<Satellite>(satId), tileId);

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

bool LaiRetrievalHandlerL3C::AddTileFileInfo(TileTemporalFilesInfo &temporalTileInfo, const QString &l3bProdDir, const QString &l3bTileDir,
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
        if (laiFileName.size() > 0 && errFileName.size() > 0 && mskFileName.size() > 0) {
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

void LaiRetrievalHandlerL3C::SubmitEndOfLaiTask(EventProcessingContext &ctx,
                                                const JobSubmittedEvent &event,
                                                const QList<TaskToSubmit> &allTasksList) {
    // add the end of lai job that will perform the cleanup
    QList<std::reference_wrapper<const TaskToSubmit>> prdFormatterTasksListRef;
    for(const TaskToSubmit &task: allTasksList) {
        if((task.moduleName == "lai-reproc-product-formatter") ||
                (task.moduleName == "lai-fitted-product-formatter")) {
            prdFormatterTasksListRef.append(task);
        }
    }
    // we add a task in order to wait for all product formatter to finish.
    // This will allow us to mark the job as finished and to remove the job folder
    TaskToSubmit endOfJobDummyTask{"lai-end-of-job", {}};
    endOfJobDummyTask.parentTasks.append(prdFormatterTasksListRef);
    SubmitTasks(ctx, event.jobId, {endOfJobDummyTask});
    ctx.SubmitSteps({endOfJobDummyTask.CreateStep("EndOfLAIDummy", QStringList())});
}

void LaiRetrievalHandlerL3C::SubmitL3BMapTiles(EventProcessingContext &ctx,
                                               const JobSubmittedEvent &event,
                                               const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles,
                                               bool bRemoveTempFiles, bool bNDayReproc,
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
        allTasksList.append({bNDayReproc ? "lai-reproc-product-formatter" : "lai-fitted-product-formatter", {}});
        TaskToSubmit &productFormatterTask = allTasksList[allTasksList.size()-1];
        for(LAIProductFormatterParams params: listParams) {
            productFormatterTask.parentTasks.append(params.laiReprocParams.parentsTasksRef);
        }
        SubmitTasks(ctx, event.jobId, {productFormatterTask});
        const QStringList &productFormatterArgs = GetReprocProductFormatterArgs(productFormatterTask, ctx,
                                              event, l3bMapTiles, realL2AMetaFiles, listParams, !bNDayReproc);
        // add these steps to the steps list to be submitted
        allSteps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
        ctx.SubmitSteps(allSteps);
    } else {
        Logger::error(QStringLiteral("Request for executing but no products were found for execution. Ignored ..."));
    }
}

QMap<QString, TileTemporalFilesInfo> LaiRetrievalHandlerL3C::FilterSecondaryProductTiles(const QMap<QString, TileTemporalFilesInfo> &mapTiles,
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

    bool bRemoveTempFiles = NeedRemoveJobFolder(ctx, event.jobId, "l3b");
    int limitL3BPrdsPerTileDefVal = (bNDayReproc ? 3 : -1);
    int limitL3BPrdsPerTile = GetIntParameterValue(parameters, "max_l3b_per_tile", limitL3BPrdsPerTileDefVal);
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
    if(bNDayReproc) {
        // get all the L3B products to be used for extraction of previous products
        const QStringList &allL3BProductsList = GetL3BProductsSinceStartOfSeason(ctx, event.siteId, listL3BProducts);
        for(const QString &l3bProd: listL3BProducts) {
            const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles = GetL3BMapTiles(ctx, l3bProd, allL3BProductsList, siteTiles, limitL3BPrdsPerTile);
            if (l3bMapTiles.size() == 0) {
                ctx.MarkJobFailed(event.jobId);
                throw std::runtime_error(
                    QStringLiteral("No L3B products were found for the given event").toStdString());
            }
            SubmitL3BMapTiles(ctx, event, l3bMapTiles, bRemoveTempFiles, bNDayReproc, allTasksList);
        }
    } else {
        const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles = GetL3BMapTiles(ctx, listL3BProducts, siteTiles);
        SubmitL3BMapTiles(ctx, event, l3bMapTiles, bRemoveTempFiles, bNDayReproc, allTasksList);
    }
    // we add a task in order to wait for all product formatter to finish.
    // This will allow us to mark the job as finished and to remove the job folder
    SubmitEndOfLaiTask(ctx, event, allTasksList);
}

void LaiRetrievalHandlerL3C::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    bool isReprocPf = false, isFittedPf = false;
    if (event.module == "lai-end-of-job") {
        ctx.MarkJobFinished(event.jobId);
        // Now remove the job folder containing temporary files
        RemoveJobFolder(ctx, event.jobId, "l3b");
    }
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

QStringList LaiRetrievalHandlerL3C::GetTimeSeriesBuilderArgs(const QStringList &monoDateLaiFileNames,
                                                             const QString &allLaiTimeSeriesFileName, const QString &mainImg) {
    QStringList timeSeriesBuilderArgs = { "TimeSeriesBuilder",
      "-out", "\"" + allLaiTimeSeriesFileName+"?gdal:co:COMPRESS=DEFLATE&gdal:co:BIGTIFF=YES\"",
      "-main", mainImg,
      "-il"
    };
    timeSeriesBuilderArgs += monoDateLaiFileNames;

    return timeSeriesBuilderArgs;
}

QStringList LaiRetrievalHandlerL3C::GetErrTimeSeriesBuilderArgs(const QStringList &monoDateErrLaiFileNames,
                                                                const QString &allErrTimeSeriesFileName, const QString &mainImg) {
    QStringList timeSeriesBuilderArgs = { "TimeSeriesBuilder",
      "-out", "\"" + allErrTimeSeriesFileName+"?gdal:co:COMPRESS=DEFLATE&gdal:co:BIGTIFF=YES\"",
      "-main", mainImg,
      "-il"
    };
    timeSeriesBuilderArgs += monoDateErrLaiFileNames;

    return timeSeriesBuilderArgs;
}

QStringList LaiRetrievalHandlerL3C::GetMskFlagsTimeSeriesBuilderArgs(const QStringList &monoDateMskFlagsLaiFileNames,
                                                                     const QString &allMskFlagsTimeSeriesFileName,  const QString &mainImg) {
    QStringList timeSeriesBuilderArgs = { "TimeSeriesBuilder",
      "-out", "\"" + allMskFlagsTimeSeriesFileName+"?gdal:co:COMPRESS=DEFLATE\"",
      "-main", mainImg,
      "-isflg", "1",
      "-il"
    };
    timeSeriesBuilderArgs += monoDateMskFlagsLaiFileNames;

    return timeSeriesBuilderArgs;
}

QStringList LaiRetrievalHandlerL3C::GetProfileReprocessingArgs(std::map<QString, QString> configParameters, const QString &allLaiTimeSeriesFileName,
                                       const QString &allErrTimeSeriesFileName, const QString &allMsksTimeSeriesFileName,
                                       const QString &reprocTimeSeriesFileName, const QStringList &listDates) {
    const auto &localWindowBwr = configParameters["processor.l3b.lai.localwnd.bwr"];
    const auto &localWindowFwr = configParameters["processor.l3b.lai.localwnd.fwr"];

    QStringList profileReprocessingArgs = { "ProfileReprocessing",
          "-lai", allLaiTimeSeriesFileName,
          "-err", allErrTimeSeriesFileName,
          "-msks", allMsksTimeSeriesFileName,
          "-opf", "\"" + reprocTimeSeriesFileName+"?gdal:co:COMPRESS=DEFLATE&gdal:co:BIGTIFF=YES\"",
          "-algo", "local",
          "-algo.local.bwr", localWindowBwr,
          "-algo.local.fwr", localWindowFwr,
          "-ildates"
    };
    profileReprocessingArgs += listDates;
    return profileReprocessingArgs;
}

QStringList LaiRetrievalHandlerL3C::GetProfileReprocessingArgs_New(std::map<QString, QString> configParameters,
                                       QStringList &monoDateLaiFileNames, QStringList &errFileNames, QStringList &flgsFileNames,
                                       const QString &mainImg, const QString &reprocTimeSeriesFileName, const QStringList &listDates) {
    const auto &localWindowBwr = configParameters["processor.l3b.lai.localwnd.bwr"];
    const auto &localWindowFwr = configParameters["processor.l3b.lai.localwnd.fwr"];

    QStringList profileReprocessingArgs = { "ProfileReprocessing",
          "-main", mainImg,
          "-opf", "\"" + reprocTimeSeriesFileName+"?gdal:co:COMPRESS=DEFLATE&gdal:co:BIGTIFF=YES\"",
          "-algo", "local",
          "-algo.local.bwr", localWindowBwr,
          "-algo.local.fwr", localWindowFwr
    };
    profileReprocessingArgs += "-illai"; profileReprocessingArgs += monoDateLaiFileNames;
    profileReprocessingArgs += "-ilerr"; profileReprocessingArgs += errFileNames;
    profileReprocessingArgs += "-ilmsks"; profileReprocessingArgs += flgsFileNames;

    profileReprocessingArgs += "-ildates"; profileReprocessingArgs += listDates;

    return profileReprocessingArgs;
}

QStringList LaiRetrievalHandlerL3C::GetFittedProfileReprocArgs_New(QStringList &monoDateLaiFileNames, QStringList &errFileNames, QStringList &flgsFileNames,
                                                                   const QString &mainImg, const QString &reprocTimeSeriesFileName, const QStringList &listDates) {
    QStringList profileReprocessingArgs = { "ProfileReprocessing",
          "-main", mainImg,
          "-opf", "\"" + reprocTimeSeriesFileName+"?gdal:co:COMPRESS=DEFLATE&gdal:co:BIGTIFF=YES\"",
          "-genall", "1",
          "-algo", "fit"
    };
    profileReprocessingArgs += "-illai"; profileReprocessingArgs += monoDateLaiFileNames;
    profileReprocessingArgs += "-ilerr"; profileReprocessingArgs += errFileNames;
    profileReprocessingArgs += "-ilmsks"; profileReprocessingArgs += flgsFileNames;

    profileReprocessingArgs += "-ildates"; profileReprocessingArgs += listDates;

    return profileReprocessingArgs;
}

QStringList LaiRetrievalHandlerL3C::GetReprocProfileSplitterArgs(const QString &reprocTimeSeriesFileName, const QString &reprocFileListFileName,
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

QStringList LaiRetrievalHandlerL3C::GetFittedProfileReprocArgs(const QString &allLaiTimeSeriesFileName, const QString &allErrTimeSeriesFileName,
                                       const QString &allMsksTimeSeriesFileName, const QString &fittedTimeSeriesFileName, const QStringList &ildates) {
    QStringList fittedProfileReprocArgs = { "ProfileReprocessing",
          "-lai", allLaiTimeSeriesFileName,
          "-err", allErrTimeSeriesFileName,
          "-msks", allMsksTimeSeriesFileName,
          "-opf", fittedTimeSeriesFileName,
          "-algo", "fit",
          "-genall", "1",
          "-ildates"
    };
    fittedProfileReprocArgs += ildates;
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

QStringList LaiRetrievalHandlerL3C::GetReprocProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx,
                                    const JobSubmittedEvent &event, const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles,
                                    const QStringList &listProducts, const QList<LAIProductFormatterParams> &productParams, bool isFitted) {

    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");

    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.xml");

    const auto &lutFile = configParameters["processor.l3b.lai.lut_path"];

    WriteExecutionInfosFile(executionInfosPath, configParameters, l3bMapTiles, listProducts, !isFitted);
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

    productFormatterArgs += (isFitted ? "-processor.vegetation.filelaifit" : "-processor.vegetation.filelaireproc");
    for(const LAIProductFormatterParams &params: productParams) {
        productFormatterArgs += params.tileId;
        productFormatterArgs += params.laiReprocParams.fileLaiReproc;
    }
    productFormatterArgs += (isFitted ? "-processor.vegetation.filelaifitflgs" : "-processor.vegetation.filelaireprocflgs");
    for(const LAIProductFormatterParams &params: productParams) {
        productFormatterArgs += params.tileId;
        productFormatterArgs += params.laiReprocParams.fileLaiReprocFlgs;
    }

    return productFormatterArgs;
}

int LaiRetrievalHandlerL3C::GetIntParameterValue(const QJsonObject &parameters, const QString &key, int defVal)
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

bool LaiRetrievalHandlerL3C::IsReprocessingCompact(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
    bool bReprocCompact = false;
    if(parameters.contains("reproc_compact")) {
        const auto &value = parameters["reproc_compact"];
        if(value.isDouble())
            bReprocCompact = (value.toInt() != 0);
        else if(value.isString()) {
            bReprocCompact = (value.toString() == "1");
        }
    } else {
        bReprocCompact = ((configParameters["processor.l3b.reproc_compact"]).toInt() != 0);
    }
    return bReprocCompact;
}


QStringList LaiRetrievalHandlerL3C::GetL3BProductRasterFiles(const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                                             LAI_RASTER_ADDITIONAL_INFO_IDX idx)
{
    QStringList retList;
    for(int i = 0; i< tileTemporalFilesInfo.temporalTilesFileInfos.size(); i++) {
        retList.append(tileTemporalFilesInfo.temporalTilesFileInfos[i].additionalFiles[idx]);
    }
    return retList;
}

QDate LaiRetrievalHandlerL3C::GetSiteFirstSeasonStartDate(EventProcessingContext &ctx,int siteId)
{
    const SeasonList &seasons = ctx.GetSiteSeasons(siteId);
    if (seasons.size() > 0) {
        return seasons[0].startDate;
    }
    return QDate();
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

ProcessorJobDefinitionParams LaiRetrievalHandlerL3C::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
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
        Logger::debug(QStringLiteral("Scheduler L3C/L3D: Error getting season start dates for site %1 for scheduled date %2!")
                      .arg(siteId)
                      .arg(qScheduledDate.toString()));
        return params;
    }

    Logger::debug(QStringLiteral("Scheduler L3C/L3D: season dates for site ID %1: start date %2, end date %3")
                  .arg(siteId)
                  .arg(seasonStartDate.toString())
                  .arg(seasonEndDate.toString()));

    QDateTime limitDate = seasonEndDate.addMonths(2);
    if(qScheduledDate > limitDate) {
        Logger::debug(QStringLiteral("Scheduler L3C: Error scheduled date %1 greater than the limit date %2 for site %3!")
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

    ConfigurationParameterValueMap mapCfg = ctx.GetConfigurationParameters(QString("processor.l3b."), siteId, requestOverrideCfgValues);

    // we might have an offset in days from starting the downloading products to start the L3C/L3D production
    int startSeasonOffset = mapCfg["processor.l3b.start_season_offset"].value.toInt();
    Logger::debug(QStringLiteral("Scheduler: start_season_offset for site ID %1: %2")
                  .arg(siteId)
                  .arg(startSeasonOffset));

    seasonStartDate = seasonStartDate.addDays(startSeasonOffset);
    Logger::debug(QStringLiteral("Scheduler: start_season_offset after ofsset for site ID %1: %2")
                  .arg(siteId)
                  .arg(seasonStartDate.toString()));

    int generateReprocess = false;
    int generateFitted = false;

    if(requestOverrideCfgValues.contains("product_type")) {
        const ConfigurationParameterValue &productType = requestOverrideCfgValues["product_type"];
        Logger::debug(QStringLiteral("Scheduler: productType for site ID %1: %2")
                      .arg(siteId)
                      .arg(productType.value));
        if(productType.value == "L3C") {
            generateReprocess = true;
            params.jsonParameters = "{ \"reproc\": \"1\", \"inputs_are_l3b\" : \"1\", \"max_l3b_per_tile\" : \"3\"}";
        } else if(productType.value == "L3D") {
            generateFitted = true;
            params.jsonParameters = "{ \"fitted\": \"1\", \"inputs_are_l3b\" : \"1\"}";
        }
    }
    // we need to have at least one flag set
    if(!generateReprocess && !generateFitted) {
        return params;
    }

    // by default the start date is the season start date
    QDateTime startDate = seasonStartDate;
    QDateTime endDate = qScheduledDate;
    Logger::debug(QStringLiteral("Scheduler: dates to retrieve products for site ID %1: start date %2, end date %3")
                  .arg(siteId)
                  .arg(startDate.toString())
                  .arg(endDate.toString()));


    if(generateReprocess) {
        int productionInterval = mapCfg["processor.l3b.reproc_production_interval"].value.toInt();
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
    }
    ProductList productList;
    if (generateReprocess) {
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
    } else {
        productList = ctx.GetProducts(siteId, (int)ProductType::L3BProductTypeId, startDate, endDate);
    }

    // we consider only Sentinel 2 products in generating final L3C/L3D products
    for(const Product &prd: productList) {
        const QString &l2aPrdHdrPath = ProcessorHandlerHelper::GetSourceL2AFromHighLevelProductIppFile(prd.fullPath);
        ProcessorHandlerHelper::SatelliteIdType satId = ProcessorHandlerHelper::GetL2ASatelliteFromTile(l2aPrdHdrPath);
        // in the case of L3C we filter here to have only the S2 products.
        // This is not the case for L3D where we send all products and we filter during job processing
        if((satId == ProcessorHandlerHelper::SATELLITE_ID_TYPE_S2) ||
           generateFitted) {
            params.productList.append(prd);
            Logger::debug(QStringLiteral("Scheduler: Using S2 product %1!")
                          .arg(prd.fullPath));
        } else {
            Logger::debug(QStringLiteral("Scheduler: Ignored product with satId %1 from path %2!")
                          .arg(satId)
                          .arg(prd.fullPath));
        }
    }

    // Normally, we need at least 3 product available in order to be able to create a L3C/L3D product
    // but if we do not return here, the schedule block waiting for products (that might never happen)
    bool waitForAvailProcInputs = (mapCfg["processor.l3b.sched_wait_proc_inputs"].value.toInt() != 0);
    if((waitForAvailProcInputs == false) || (params.productList.size() > 0)) {
        params.isValid = true;
        Logger::debug(QStringLiteral("Executing scheduled job. Scheduler extracted for L3C/L3D a number "
                                     "of %1 products for site ID %2 with start date %3 and end date %4!")
                      .arg(params.productList.size())
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    } else {
        Logger::debug(QStringLiteral("Scheduled job for L3C/L3D and site ID %1 with start date %2 and end date %3 "
                                     "will not be executed (no products)!")
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    }

    return params;
}

