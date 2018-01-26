#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "lairetrievalhandler_l3b_new.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"
#include "logger.hpp"
#include "maccshdrmeananglesreader.hpp"

void LaiRetrievalHandlerL3BNew::CreateTasksForNewProduct(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                         QList<TaskToSubmit> &outAllTasksList,
                                                    const QList<TileInfos> &tileInfosList,
                                                    bool bRemoveTempFiles) {
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");
    bool bGenNdvi = IsParamOrConfigKeySet(parameters, configParameters, "genlai", "processor.l3b.lai.produce_ndvi");
    bool bGenLai = IsParamOrConfigKeySet(parameters, configParameters, "genlai", "processor.l3b.lai.produce_lai");
    bool bGenFapar = IsParamOrConfigKeySet(parameters, configParameters, "genlai", "processor.l3b.lai.produce_fapar");
    bool bGenFCover = IsParamOrConfigKeySet(parameters, configParameters, "genlai", "processor.l3b.lai.produce_fcover");

    // in allTasksList we might have tasks from other products. We start from the first task of the current product
    int initialTasksNo = outAllTasksList.size();
    int nbLaiMonoProducts = tileInfosList.size();
    for(int i = 0; i<nbLaiMonoProducts; i++) {
        outAllTasksList.append(TaskToSubmit{"lai-processor-mask-flags", {}});
        if (bGenNdvi) {
            outAllTasksList.append(TaskToSubmit{"lai-processor-ndvi-extractor", {}});
        }
        if (bGenLai) {
            outAllTasksList.append(TaskToSubmit{"lai-processor", {}});
            outAllTasksList.append(TaskToSubmit{"lai-quantify-image", {}});
        }
        if (bGenFapar) {
            outAllTasksList.append(TaskToSubmit{"fapar-processor", {}});
            outAllTasksList.append(TaskToSubmit{"fapar-quantify-image", {}});
        }
        if (bGenFCover) {
            outAllTasksList.append(TaskToSubmit{"fcover-processor", {}});
            outAllTasksList.append(TaskToSubmit{"fcover-quantify-image", {}});
        }
    }
    outAllTasksList.append({"lai-processor-product-formatter", {}});
    if(bRemoveTempFiles) {
        outAllTasksList.append(TaskToSubmit{ "files-remover", {} });
    }

    //
    // NOTE: In this moment, the products in loop are not executed in parallel. To do this, the if(i > 0) below
    //      should be removed but in this case, the time-series-builders should wait for all the monodate images
    int i;
    QList<std::reference_wrapper<const TaskToSubmit>> productFormatterParentsRefs;

    // we execute in parallel and launch at once all processing chains for each product
    // for example, if we have genModels, we launch all bv-input-variable-generation for all products
    // if we do not have genModels, we launch all NDVIRVIExtraction in the same time for all products
    int nCurTaskIdx = initialTasksNo;

    // Specifies if the products creation should be chained or not.
    // TODO: This should be taken from the configuration
    bool bChainProducts = true;

    for(i = 0; i<nbLaiMonoProducts; i++) {
        // if we want chaining products and we have a previous product executed
        if(bChainProducts && initialTasksNo > 0) {
            // we create a dependency to the last task of the previous product
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
        }   // else  skip over the lai-processor-mask-flags as we run it with no previous dependency,
            // allowing running several products in parallel
        // increment the current index for ndvi-rvi-extraction
        nCurTaskIdx++;

        // lai-processor-ndvi-extraction, lai-processor, fapar-processor, fcover-processor -> lai-processor-mask-flags
        // all these are run in parallel
        int flagsTaskIdx = nCurTaskIdx-1;
        if (bGenNdvi) {
            int ndviRviExtrIdx = nCurTaskIdx++;
            outAllTasksList[ndviRviExtrIdx].parentTasks.append(outAllTasksList[flagsTaskIdx]);
            // add the ndvi task to the list of the product formatter corresponding to this product
            productFormatterParentsRefs.append(outAllTasksList[ndviRviExtrIdx]);
        }
        if (bGenLai) {
            int nLaiProcessorIdx = nCurTaskIdx++;
            outAllTasksList[nLaiProcessorIdx].parentTasks.append(outAllTasksList[flagsTaskIdx]);
            // lai-quantify-image -> lai-processor
            int nLaiQuantifyImageIdx = nCurTaskIdx++;
            outAllTasksList[nLaiQuantifyImageIdx].parentTasks.append(outAllTasksList[nLaiProcessorIdx]);
            // add the quantified task to the list of the product formatter corresponding to this product
            productFormatterParentsRefs.append(outAllTasksList[nLaiQuantifyImageIdx]);
        }

        if (bGenFapar) {
            int nFaparProcessorIdx = nCurTaskIdx++;
            outAllTasksList[nFaparProcessorIdx].parentTasks.append(outAllTasksList[flagsTaskIdx]);
            // fapar-quantify-image -> fapar-processor
            int nFaparQuantifyImageIdx = nCurTaskIdx++;
            outAllTasksList[nFaparQuantifyImageIdx].parentTasks.append(outAllTasksList[nFaparProcessorIdx]);
            // add the quantified task to the list of the product formatter corresponding to this product
            productFormatterParentsRefs.append(outAllTasksList[nFaparQuantifyImageIdx]);

        }

        if (bGenFCover) {
            int nFcoverProcessorIdx = nCurTaskIdx++;
            outAllTasksList[nFcoverProcessorIdx].parentTasks.append(outAllTasksList[flagsTaskIdx]);
            // fcover-quantify-image -> fcover-processor
            int nFCoverQuantifyImageIdx = nCurTaskIdx++;
            outAllTasksList[nFCoverQuantifyImageIdx].parentTasks.append(outAllTasksList[nFcoverProcessorIdx]);
            // add the quantified task to the list of the product formatter corresponding to this product
            productFormatterParentsRefs.append(outAllTasksList[nFCoverQuantifyImageIdx]);
        }
    }
    int productFormatterIdx = nCurTaskIdx++;
    outAllTasksList[productFormatterIdx].parentTasks.append(productFormatterParentsRefs);
    if(bRemoveTempFiles) {
        // cleanup-intermediate-files -> product formatter
        outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
    }

}

NewStepList LaiRetrievalHandlerL3BNew::GetStepsForMonodateLai(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                    const QList<TileInfos> &prdTilesInfosList, QList<TaskToSubmit> &allTasksList,
                                                    bool bRemoveTempFiles, int tasksStartIdx)
{
    NewStepList steps;
    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");
    const auto &laiCfgFile = configParameters["processor.l3b.lai.laibandscfgfile"];
    bool bGenNdvi = IsParamOrConfigKeySet(parameters, configParameters, "genlai", "processor.l3b.lai.produce_ndvi");
    bool bGenLai = IsParamOrConfigKeySet(parameters, configParameters, "genlai", "processor.l3b.lai.produce_lai");
    bool bGenFapar = IsParamOrConfigKeySet(parameters, configParameters, "genlai", "processor.l3b.lai.produce_fapar");
    bool bGenFCover = IsParamOrConfigKeySet(parameters, configParameters, "genlai", "processor.l3b.lai.produce_fcover");

    // Get the resolution value
    int resolution = 0;
    if(!GetParameterValueAsInt(parameters, "resolution", resolution) ||
            resolution == 0) {
        resolution = 10;    // TODO: We should configure the default resolution in DB
    }
    const auto &resolutionStr = QString::number(resolution);

    // in allTasksList we might have tasks from other products. We start from the first task of the current product
    int curTaskIdx = tasksStartIdx;

    QStringList ndviList;
    QStringList laiList;
    QStringList faparList;
    QStringList fcoverList;
    QStringList laiFlgsList;
    QStringList cleanupTemporaryFilesList;
    for (int i = 0; i<prdTilesInfosList.size(); i++) {
        const auto &prdTileInfo = prdTilesInfosList[i];
        TaskToSubmit &genMonoDateMskFagsTask = allTasksList[curTaskIdx++];
        const auto & monoDateMskFlgsFileName = genMonoDateMskFagsTask.GetFilePath("LAI_mono_date_msk_flgs_img.tif");
        auto monoDateMskFlgsResFileName = genMonoDateMskFagsTask.GetFilePath("LAI_mono_date_msk_flgs_img_resampled.tif");
        QStringList genMonoDateMskFagsArgs = GetMonoDateMskFlagsArgs(prdTileInfo.tileFile, monoDateMskFlgsFileName,
                                                                     BuildProcessorOutputFileName(configParameters, monoDateMskFlgsResFileName),
                                                                     resolutionStr);
        // add these steps to the steps list to be submitted
        steps.append(genMonoDateMskFagsTask.CreateStep("GenerateLaiMonoDateMaskFlags", genMonoDateMskFagsArgs));
        laiFlgsList.append(monoDateMskFlgsResFileName);
        cleanupTemporaryFilesList.append(monoDateMskFlgsFileName);
        cleanupTemporaryFilesList.append(monoDateMskFlgsResFileName);

        if (bGenNdvi) {
            TaskToSubmit &ndviRviExtractorTask = allTasksList[curTaskIdx++];
            auto singleNdviFile = ndviRviExtractorTask.GetFilePath("single_ndvi.tif");
            const QStringList &ndviRviExtractionArgs = GetNdviRviExtractionNewArgs(prdTileInfo.tileFile, monoDateMskFlgsFileName,
                                                                         BuildProcessorOutputFileName(configParameters, singleNdviFile),
                                                                         resolutionStr, laiCfgFile);
            steps.append(ndviRviExtractorTask.CreateStep("NdviRviExtractionNew", ndviRviExtractionArgs));
            // save the file to be sent to product formatter
            ndviList.append(singleNdviFile);
            cleanupTemporaryFilesList.append(singleNdviFile);
        }
        if (bGenLai) {
            TaskToSubmit &laiProcessorTask = allTasksList[curTaskIdx++];
            TaskToSubmit &quantifyLaiImageTask = allTasksList[curTaskIdx++];
            const auto & monoDateLaiFileName = laiProcessorTask.GetFilePath("LAI_mono_date_img.tif");
            const auto & quantifiedLaiFileName = quantifyLaiImageTask.GetFilePath("LAI_mono_date_img_16.tif");
            const QStringList &laiProcessorArgs = GetLaiProcessorArgs(prdTileInfo.tileFile, resolutionStr, laiCfgFile,
                                                                      monoDateLaiFileName, "lai");
            steps.append(laiProcessorTask.CreateStep("BVLaiNewProcessor", laiProcessorArgs));
            const QStringList &quantifyLaiImageArgs = GetQuantifyImageArgs(configParameters, monoDateLaiFileName, quantifiedLaiFileName);
            steps.append(quantifyLaiImageTask.CreateStep("QuantifyImage", quantifyLaiImageArgs));
            // save the file to be sent to product formatter
            laiList.append(quantifiedLaiFileName);
            cleanupTemporaryFilesList.append(monoDateLaiFileName);
            cleanupTemporaryFilesList.append(quantifiedLaiFileName);
        }
        if (bGenFapar) {
            TaskToSubmit &faparProcessorTask = allTasksList[curTaskIdx++];
            TaskToSubmit &quantifyFaparImageTask = allTasksList[curTaskIdx++];
            const auto & faparFileName = faparProcessorTask.GetFilePath("FAPAR_mono_date_img.tif");
            const auto & quantifiedFaparFileName = quantifyFaparImageTask.GetFilePath("FAPAR_mono_date_img_16.tif");
            const QStringList &faparProcessorArgs = GetLaiProcessorArgs(prdTileInfo.tileFile, resolutionStr, laiCfgFile,
                                                                      faparFileName, "fapar");
            steps.append(faparProcessorTask.CreateStep("BVLaiNewProcessor", faparProcessorArgs));
            const QStringList &quantifyFaparImageArgs = GetQuantifyImageArgs(configParameters, faparFileName, quantifiedFaparFileName);
            steps.append(quantifyFaparImageTask.CreateStep("QuantifyImage", quantifyFaparImageArgs));
            // save the file to be sent to product formatter
            faparList.append(quantifiedFaparFileName);
            cleanupTemporaryFilesList.append(faparFileName);
            cleanupTemporaryFilesList.append(quantifiedFaparFileName);
        }
        if (bGenFCover) {
            TaskToSubmit &fcoverProcessorTask = allTasksList[curTaskIdx++];
            TaskToSubmit &quantifyFcoverImageTask = allTasksList[curTaskIdx++];
            const auto & fcoverFileName = fcoverProcessorTask.GetFilePath("FCOVER_mono_date_img.tif");
            const auto & quantifiedFcoverFileName = quantifyFcoverImageTask.GetFilePath("FCOVER_mono_date_img_16.tif");
            const QStringList &fcoverlaiProcessorArgs = GetLaiProcessorArgs(prdTileInfo.tileFile, resolutionStr, laiCfgFile,
                                                                      fcoverFileName, "fcover");
            steps.append(fcoverProcessorTask.CreateStep("BVLaiNewProcessor", fcoverlaiProcessorArgs));
            const QStringList &quantifyFcoverImageArgs = GetQuantifyImageArgs(configParameters, fcoverFileName, quantifiedFcoverFileName);
            steps.append(quantifyFcoverImageTask.CreateStep("QuantifyImage", quantifyFcoverImageArgs));
            // save the file to be sent to product formatter
            fcoverList.append(quantifiedFcoverFileName);
            cleanupTemporaryFilesList.append(fcoverFileName);
            cleanupTemporaryFilesList.append(quantifiedFcoverFileName);
        }
    }
    TaskToSubmit &laiMonoProductFormatterTask = allTasksList[curTaskIdx++];
    QStringList productFormatterArgs = GetLaiMonoProductFormatterArgs(
                laiMonoProductFormatterTask, ctx, event, prdTilesInfosList,
                ndviList, laiList, laiFlgsList, faparList, fcoverList);
    steps.append(laiMonoProductFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));

    if(bRemoveTempFiles) {
        TaskToSubmit &cleanupTemporaryFilesTask = allTasksList[curTaskIdx++];
        // add also the cleanup step
        steps.append(cleanupTemporaryFilesTask.CreateStep("CleanupTemporaryFiles", cleanupTemporaryFilesList));
    }

    return steps;
}
void LaiRetrievalHandlerL3BNew::WriteExecutionInfosFile(const QString &executionInfosPath,
                                               const QList<TileInfos> &tilesInfosList) {
    std::ofstream executionInfosFile;
    try
    {
        executionInfosFile.open(executionInfosPath.toStdString().c_str(), std::ofstream::out);
        executionInfosFile << "<?xml version=\"1.0\" ?>" << std::endl;
        executionInfosFile << "<metadata>" << std::endl;
        executionInfosFile << "  <General>" << std::endl;
        executionInfosFile << "  </General>" << std::endl;

        executionInfosFile << "  <XML_files>" << std::endl;
        for (int i = 0; i<tilesInfosList.size(); i++) {
            executionInfosFile << "    <XML_" << std::to_string(i) << ">" << tilesInfosList[i].tileFile.toStdString()
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

void LaiRetrievalHandlerL3BNew::HandleProduct(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                            const QList<TileInfos> &prdTilesInfosList, QList<TaskToSubmit> &allTasksList) {
    bool bRemoveTempFiles = NeedRemoveJobFolder(ctx, event.jobId, "l3b");

    int tasksStartIdx = allTasksList.size();
    // create the tasks
    CreateTasksForNewProduct(ctx, event, allTasksList, prdTilesInfosList, bRemoveTempFiles);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(int i = tasksStartIdx; i < allTasksList.size(); i++) {
        const TaskToSubmit &task = allTasksList.at(i);
        allTasksListRef.append((TaskToSubmit&)task);
    }
    // submit all tasks
    SubmitTasks(ctx, event.jobId, allTasksListRef);

    NewStepList steps;

    steps += GetStepsForMonodateLai(ctx, event, prdTilesInfosList, allTasksList, bRemoveTempFiles, tasksStartIdx);
    ctx.SubmitSteps(steps);
}

void LaiRetrievalHandlerL3BNew::SubmitEndOfLaiTask(EventProcessingContext &ctx,
                                                const JobSubmittedEvent &event,
                                                const QList<TaskToSubmit> &allTasksList) {
    // add the end of lai job that will perform the cleanup
    QList<std::reference_wrapper<const TaskToSubmit>> prdFormatterTasksListRef;
    for(const TaskToSubmit &task: allTasksList) {
        if(task.moduleName == "lai-processor-product-formatter") {
            prdFormatterTasksListRef.append(task);
        }
    }
    // we add a task in order to wait for all product formatter to finish.
    // This will allow us to mark the job as finished and to remove the job folder
    TaskToSubmit endOfJobDummyTask{"lai-processor-end-of-job", {}};
    endOfJobDummyTask.parentTasks.append(prdFormatterTasksListRef);
    SubmitTasks(ctx, event.jobId, {endOfJobDummyTask});
    ctx.SubmitSteps({endOfJobDummyTask.CreateStep("EndOfLAIDummy", QStringList())});

}

void LaiRetrievalHandlerL3BNew::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");
    const auto &modelsFolder = configParameters["processor.l3b.lai.modelsfolder"];

    bool bMonoDateLai = IsParamOrConfigKeySet(parameters, configParameters, "monolai", "processor.l3b.mono_date_lai");
    if(!bMonoDateLai) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("LAI mono-date processing needs to be defined").toStdString());
    }

    bool bGenNdvi = IsParamOrConfigKeySet(parameters, configParameters, "genndvi", "processor.l3b.lai.produce_ndvi");
    bool bGenLai = IsParamOrConfigKeySet(parameters, configParameters, "genlai", "processor.l3b.lai.produce_lai");
    bool bGenFapar = IsParamOrConfigKeySet(parameters, configParameters, "genfapar", "processor.l3b.lai.produce_fapar");
    bool bGenFCover = IsParamOrConfigKeySet(parameters, configParameters, "genfcover", "processor.l3b.lai.produce_fcover");
    if (!bGenNdvi && !bGenLai && !bGenFapar && !bGenFCover) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No index was configured to be generated").toStdString());
    }

    if(!QDir::root().mkpath(modelsFolder)) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
                    QStringLiteral("Unable to create path %1 for creating models!").arg(modelsFolder).toStdString());
    }

    // create and submit the tasks for the received products
    QMap<QString, QStringList> inputProductToTilesMap;
    QStringList listTilesMetaFiles = GetL2AInputProductsTiles(ctx, event, inputProductToTilesMap);
    if(listTilesMetaFiles.size() == 0) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No products provided at input or no products available in the specified interval").
                    toStdString());
    }

    // Group the products that belong to the same date
    // the tiles of products from secondary satellite are not included if they happen to be from the same date with tiles from
    // the same date
    QMap<QDate, QStringList> dateGroupedInputProductToTilesMap = ProcessorHandlerHelper::GroupL2AProductTilesByDate(inputProductToTilesMap);

    //container for all task
    QList<TaskToSubmit> allTasksList;
    const QSet<QString> &tilesFilter = GetTilesFilter(parameters, configParameters);
    for(const auto &key : dateGroupedInputProductToTilesMap.keys()) {
        const QStringList &prdTilesList = dateGroupedInputProductToTilesMap[key];
        // create structures providing the models for each tile
        QList<TileInfos> tilesInfosList;
        for(const QString &prdTile: prdTilesList) {
            if (FilterTile(tilesFilter, prdTile)) {
                TileInfos tileInfo;
                tileInfo.tileFile = prdTile;
                tilesInfosList.append(tileInfo);
            }
        }
        // Handle product only if we have at least one tile (we might have all of them filtered)
        if (tilesInfosList.size() > 0) {
            HandleProduct(ctx, event, tilesInfosList, allTasksList);
        }
    }

    // we add a task in order to wait for all product formatter to finish.
    // This will allow us to mark the job as finished and to remove the job folder
    SubmitEndOfLaiTask(ctx, event, allTasksList);
}

void LaiRetrievalHandlerL3BNew::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    if (event.module == "lai-processor-end-of-job") {
        ctx.MarkJobFinished(event.jobId);
        // Now remove the job folder containing temporary files
        RemoveJobFolder(ctx, event.jobId, "l3b");
    }
    if ((event.module == "lai-processor-product-formatter")) {
        QString prodName = GetProductFormatterProductName(ctx, event);
        QString productFolder = GetProductFormatterOutputProductPath(ctx, event);
        if((prodName != "") && ProcessorHandlerHelper::IsValidHighLevelProduct(productFolder)) {
            QString quicklook = GetProductFormatterQuicklook(ctx, event);
            QString footPrint = GetProductFormatterFootprint(ctx, event);
            ProductType prodType = ProductType::L3BProductTypeId;

            const QStringList &prodTiles = ProcessorHandlerHelper::GetTileIdsFromHighLevelProduct(productFolder);

            // get the satellite id for the product
            const QStringList &listL3BProdTiles = ProcessorHandlerHelper::GetTileIdsFromHighLevelProduct(productFolder);
            const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles = GetSiteTiles(ctx, event.siteId);
            ProcessorHandlerHelper::SatelliteIdType satId = ProcessorHandlerHelper::SATELLITE_ID_TYPE_UNKNOWN;
            for(const auto &tileId : listL3BProdTiles) {
                // we assume that all the tiles from the product are from the same satellite
                // in this case, we get only once the satellite Id for all tiles
                if(satId == ProcessorHandlerHelper::SATELLITE_ID_TYPE_UNKNOWN) {
                    satId = GetSatIdForTile(siteTiles, tileId);
                    // ignore tiles for which the satellite id cannot be determined
                    if(satId != ProcessorHandlerHelper::SATELLITE_ID_TYPE_UNKNOWN) {
                        break;
                    }
                }
            }

            // Insert the product into the database
            QDateTime minDate, maxDate;
            ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prodName, minDate, maxDate);
            int ret = ctx.InsertProduct({ prodType, event.processorId, static_cast<int>(satId), event.siteId, event.jobId,
                                productFolder, maxDate, prodName,
                                quicklook, footPrint, std::experimental::nullopt, prodTiles });
            Logger::debug(QStringLiteral("InsertProduct for %1 returned %2").arg(prodName).arg(ret));

            // submit a new job for the L3C product corresponding to this one
            SubmitL3CJobForL3BProduct(ctx, event, satId, prodName);
        } else {
            Logger::error(QStringLiteral("Cannot insert into database the product with name %1 and folder %2").arg(prodName).arg(productFolder));
            // We might have several L3B products, we should not mark it at failed here as
            // this will stop also all other L3B processings that might be successful
            //ctx.MarkJobFailed(event.jobId);
        }
    }
}

QStringList LaiRetrievalHandlerL3BNew::GetNdviRviExtractionNewArgs(const QString &inputProduct, const QString &msksFlagsFile,
                                                          const QString &ndviFile, const QString &resolution, const QString &laiBandsCfg) {
    return { "NdviRviExtractionNew",
           "-xml", inputProduct,
           "-msks", msksFlagsFile,
           "-ndvi", ndviFile,
           "-outres", resolution,
           "-laicfgs", laiBandsCfg
    };
}

QStringList LaiRetrievalHandlerL3BNew::GetLaiProcessorArgs(const QString &xmlFile, const QString &resolution, const QString &laiBandsCfg,
                                                   const QString &monoDateLaiFileName, const QString &indexName) {
    QString outParamName = QString("-out") + indexName;
    return { "BVLaiNewProcessor",
        "-xml", xmlFile,
        outParamName, monoDateLaiFileName,
        "-outres", resolution,
        "-laicfgs", laiBandsCfg
    };
}

QStringList LaiRetrievalHandlerL3BNew::GetQuantifyImageArgs(const std::map<QString, QString> &configParameters,
                                                            const QString &inFileName, const QString &outFileName)  {
    return { "QuantifyImage",
        "-in", inFileName,
        "-out", BuildProcessorOutputFileName(configParameters, outFileName)
    };
}

QStringList LaiRetrievalHandlerL3BNew::GetMonoDateMskFlagsArgs(const QString &inputProduct, const QString &monoDateMskFlgsFileName,
                                                         const QString &monoDateMskFlgsResFileName, const QString &resStr) {
    return { "GenerateLaiMonoDateMaskFlags",
      "-inxml", inputProduct,
      "-out", monoDateMskFlgsFileName,
      "-outres", resStr,
      "-outresampled", monoDateMskFlgsResFileName
    };
}

QStringList LaiRetrievalHandlerL3BNew::GetLaiMonoProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                                const QList<TileInfos> &prdTilesInfosList, const QStringList &ndviList,
                                                                const QStringList &laiList, const QStringList &laiFlgsList,
                                                                const QStringList &faparList,  const QStringList &fcoverList) {

    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");
    QStringList tileIdsList;
    for(const TileInfos &tileInfo: prdTilesInfosList) {
        ProcessorHandlerHelper::SatelliteIdType satId;
        QString tileId = ProcessorHandlerHelper::GetTileId(tileInfo.tileFile, satId);
        tileIdsList.append(tileId);
    }

    //const auto &targetFolder = productFormatterTask.GetFilePath("");
    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.xml");

    const auto &lutFile = configParameters["processor.l3b.lai.lut_path"];

    WriteExecutionInfosFile(executionInfosPath, prdTilesInfosList);

    QStringList productFormatterArgs = { "ProductFormatter",
                            "-destroot", targetFolder,
                            "-fileclass", "OPER",
                            "-level", "L3B",
                            "-baseline", "01.00",
                            "-siteid", QString::number(event.siteId),
                            "-processor", "vegetation",
                            "-gipp", executionInfosPath,
                            "-outprops", outPropsPath};
    productFormatterArgs += "-il";
    for(const TileInfos &tileInfo: prdTilesInfosList) {
        productFormatterArgs.append(tileInfo.tileFile);
    }

    if(lutFile.size() > 0) {
        productFormatterArgs += "-lut";
        productFormatterArgs += lutFile;
    }

    productFormatterArgs += "-processor.vegetation.laimdateflgs";
    for(int i = 0; i<tileIdsList.size(); i++) {
        productFormatterArgs += GetProductFormatterTile(tileIdsList[i]);
        productFormatterArgs += laiFlgsList[i];
    }

    if (ndviList.size() == tileIdsList.size()) {
        productFormatterArgs += "-processor.vegetation.laindvi";
        for(int i = 0; i<tileIdsList.size(); i++) {
            productFormatterArgs += GetProductFormatterTile(tileIdsList[i]);
            productFormatterArgs += ndviList[i];
        }
    }
    if (laiList.size() == tileIdsList.size()) {
        productFormatterArgs += "-processor.vegetation.laimonodate";
        for(int i = 0; i<tileIdsList.size(); i++) {
            productFormatterArgs += GetProductFormatterTile(tileIdsList[i]);
            productFormatterArgs += laiList[i];
        }
    }
    if (faparList.size() == tileIdsList.size()) {
        productFormatterArgs += "-processor.vegetation.faparmonodate";
        for(int i = 0; i<tileIdsList.size(); i++) {
            productFormatterArgs += GetProductFormatterTile(tileIdsList[i]);
            productFormatterArgs += faparList[i];
        }
    }
    if (fcoverList.size() == tileIdsList.size()) {
        productFormatterArgs += "-processor.vegetation.fcovermonodate";
        for(int i = 0; i<tileIdsList.size(); i++) {
            productFormatterArgs += GetProductFormatterTile(tileIdsList[i]);
            productFormatterArgs += fcoverList[i];
        }
    }

    return productFormatterArgs;
}

const QString& LaiRetrievalHandlerL3BNew::GetDefaultCfgVal(std::map<QString, QString> &configParameters, const QString &key, const QString &defVal) {
    auto search = configParameters.find(key);
    if(search != configParameters.end()) {
        return search->second;
    }
    return defVal;
}

bool LaiRetrievalHandlerL3BNew::IsParamOrConfigKeySet(const QJsonObject &parameters, std::map<QString, QString> &configParameters,
                                                const QString &cmdLineParamName, const QString & cfgParamKey, bool defVal) {
    bool bIsConfigKeySet = defVal;
    if(parameters.contains(cmdLineParamName)) {
        const auto &value = parameters[cmdLineParamName];
        if(value.isDouble())
            bIsConfigKeySet = (value.toInt() != 0);
        else if(value.isString()) {
            bIsConfigKeySet = (value.toString() == "1");
        }
    } else {
        if (cfgParamKey != "") {
            bIsConfigKeySet = ((configParameters[cfgParamKey]).toInt() != 0);
        }
    }
    return bIsConfigKeySet;
}

QSet<QString> LaiRetrievalHandlerL3BNew::GetTilesFilter(const QJsonObject &parameters, std::map<QString, QString> &configParameters)
{
    QString strTilesFilter;
    if(parameters.contains("tiles_filter")) {
        const auto &value = parameters["tiles_filter"];
        if(value.isString()) {
            strTilesFilter = value.toString();
        }
    }
    if (strTilesFilter.isEmpty()) {
        strTilesFilter = configParameters["processor.l3b.lai.tiles_filter"];
    }
    QSet<QString> retSet;
    // accept any of these separators
    const QStringList &tilesList = strTilesFilter.split(',');
    for (const QString &strTile: tilesList) {
        const QString &strTrimmedTile = strTile.trimmed();
        if(!strTrimmedTile.isEmpty()) {
            retSet.insert(strTrimmedTile);
        }
    }

    return retSet;
}

bool LaiRetrievalHandlerL3BNew::FilterTile(const QSet<QString> &tilesSet, const QString &prdTileFile)
{
    ProcessorHandlerHelper::SatelliteIdType satId;
    const QString &tileId = ProcessorHandlerHelper::GetTileId(prdTileFile, satId);
    return (tilesSet.empty() || tilesSet.contains(tileId));
}


ProcessorJobDefinitionParams LaiRetrievalHandlerL3BNew::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
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
        Logger::debug(QStringLiteral("Scheduler L3B: Error getting season start dates for site %1 for scheduled date %2!")
                      .arg(siteId)
                      .arg(qScheduledDate.toString()));
        return params;
    }

    QDateTime limitDate = seasonEndDate.addMonths(2);
    if(qScheduledDate > limitDate) {
        Logger::debug(QStringLiteral("Scheduler L3B: Error scheduled date %1 greater than the limit date %2 for site %3!")
                      .arg(qScheduledDate.toString())
                      .arg(limitDate.toString())
                      .arg(siteId));
        return params;
    }
    if(!seasonStartDate.isValid()) {
        Logger::error(QStringLiteral("Scheduler L3B: Season start date for site ID %1 is invalid in the database!")
                      .arg(siteId));
        return params;
    }

    ConfigurationParameterValueMap mapCfg = ctx.GetConfigurationParameters(QString("processor.l3b."), siteId, requestOverrideCfgValues);

    // we might have an offset in days from starting the downloading products to start the L3B production
    int startSeasonOffset = mapCfg["processor.l3b.start_season_offset"].value.toInt();
    seasonStartDate = seasonStartDate.addDays(startSeasonOffset);

    int generateLai = false;
    if(requestOverrideCfgValues.contains("product_type")) {
        const ConfigurationParameterValue &productType = requestOverrideCfgValues["product_type"];
        if(productType.value == "L3B") {
            generateLai = true;
            params.jsonParameters = "{ \"monolai\": \"1\"}";
        }
    }
    // we need to have at least one flag set
    if(!generateLai) {
        return params;
    }

    // by default the start date is the season start date
    QDateTime startDate = seasonStartDate;
    QDateTime endDate = qScheduledDate;

    int productionInterval = mapCfg["processor.l3b.production_interval"].value.toInt();
    startDate = endDate.addDays(-productionInterval);
    // Use only the products after the configured start season date
    if(startDate < seasonStartDate) {
        startDate = seasonStartDate;
    }

    bool bOnceExecution = false;
    if(requestOverrideCfgValues.contains("task_repeat_type")) {
        const ConfigurationParameterValue &repeatType = requestOverrideCfgValues["task_repeat_type"];
        if (repeatType.value == "0") {
            bOnceExecution = true;
        }
    }
    const QDateTime &curDateTime = QDateTime::currentDateTime();
    if ((curDateTime > seasonEndDate) || bOnceExecution) {
        // processing of a past season, that was already finished
        params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    } else {
        // processing of a season in progress, we get the products inserted in the last interval since the last scheduling
        params.productList = ctx.GetProductsByInsertedTime(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    }
    // TODO: Maybe we should perform also a filtering by the creation date, to be inside the season to avoid creation for the
    // products that are outside the season
    // Normally, we need at least 1 product available in order to be able to create a L3B product
    // but if we do not return here, the schedule block waiting for products (that might never happen)
    bool waitForAvailProcInputs = (mapCfg["processor.l3b.sched_wait_proc_inputs"].value.toInt() != 0);
    if((waitForAvailProcInputs == false) || (params.productList.size() > 0)) {
        params.isValid = true;
        Logger::debug(QStringLiteral("Executing scheduled job. Scheduler extracted for L3B a number "
                                     "of %1 products for site ID %2 with start date %3 and end date %4!")
                      .arg(params.productList.size())
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    } else {
        Logger::debug(QStringLiteral("Scheduled job for L3B and site ID %1 with start date %2 and end date %3 "
                                     "will not be executed (no products)!")
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    }

    return params;
}

void LaiRetrievalHandlerL3BNew::SubmitL3CJobForL3BProduct(EventProcessingContext &ctx, const TaskFinishedEvent &event,
                                                       const ProcessorHandlerHelper::SatelliteIdType &satId, const QString &l3bProdName)
{
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.lai.link_l3c_to_l3b");
    bool bLinkL3CToL3B = ((configParameters["processor.l3b.lai.link_l3c_to_l3b"]).toInt() == 1);
    // generate automatically only for Sentinel2
    if (bLinkL3CToL3B) {
        if(satId == ProcessorHandlerHelper::SATELLITE_ID_TYPE_S2) {

            NewJob newJob;
            newJob.processorId = event.processorId;  //here we have for now the same processor ID
            newJob.siteId = event.siteId;
            newJob.startType = JobStartType::Triggered;

            QJsonObject processorParamsObj;
            QJsonArray prodsJsonArray;
            prodsJsonArray.append(l3bProdName);
            processorParamsObj["input_products"] = prodsJsonArray;
            processorParamsObj["resolution"] = "10";
            processorParamsObj["reproc"] = "1";
            processorParamsObj["inputs_are_l3b"] = "1";
            processorParamsObj["max_l3b_per_tile"] = "3";
            newJob.parametersJson = jsonToString(processorParamsObj);

            ctx.SubmitJob(newJob);
        }
    }
}

