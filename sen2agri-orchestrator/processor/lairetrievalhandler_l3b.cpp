#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "lairetrievalhandler_l3b.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"
#include "logger.hpp"
#include "maccshdrmeananglesreader.hpp"

// The number of tasks that are executed for each product before executing time series tasks
#define LAI_TASKS_PER_PRODUCT       6
#define MODEL_GEN_TASKS_PER_PRODUCT 4

#define DEFAULT_GENERATED_SAMPLES_NO    "40000"
#define DEFAULT_NOISE_VAR               "0.01"
#define DEFAULT_BEST_OF                 "1"
#define DEFAULT_REGRESSOR               "nn"

void LaiRetrievalHandlerL3B::CreateTasksForNewProduct(QList<TaskToSubmit> &outAllTasksList,
                                                    const QList<TileInfos> &tileInfosList,
                                                    bool bForceGenModels, bool bRemoveTempFiles) {
    // in allTasksList we might have tasks from other products. We start from the first task of the current product
    int initialTasksNo = outAllTasksList.size();
    int nbLaiMonoProducts = tileInfosList.size();
    for(int i = 0; i<nbLaiMonoProducts; i++) {
        const TileInfos &tileInfo = tileInfosList[i];
        if(bForceGenModels || tileInfo.modelInfos.NeedsModelGeneration()) {
            outAllTasksList.append(TaskToSubmit{ "lai-bv-input-variable-generation", {} });
            outAllTasksList.append(TaskToSubmit{ "lai-prosail-simulator", {} });
            outAllTasksList.append(TaskToSubmit{ "lai-training-data-generator", {} });
            outAllTasksList.append(TaskToSubmit{ "lai-inverse-model-learning", {} });
        }
        outAllTasksList.append(TaskToSubmit{"lai-mono-date-mask-flags", {}});
        outAllTasksList.append(TaskToSubmit{"lai-ndvi-rvi-extractor", {}});
        outAllTasksList.append(TaskToSubmit{"lai-bv-image-invertion", {}});
        outAllTasksList.append(TaskToSubmit{"lai-bv-err-image-invertion", {}});
        outAllTasksList.append(TaskToSubmit{"lai-quantify-image", {}});
        outAllTasksList.append(TaskToSubmit{"lai-quantify-err-image", {}});
    }
    outAllTasksList.append({"lai-mono-date-product-formatter", {}});
    if(bRemoveTempFiles) {
        outAllTasksList.append(TaskToSubmit{ "files-remover", {} });
    }

    //   ----------------------------- LOOP --------------------------------------------
    //   |                      lai-create-tile-footprint      (optional)               |
    //   |                              |                                               |
    //   |                      bv-input-variable-generation   (optional)               |
    //   |                              |                                               |
    //   |                      prosail-simulator              (optional)               |
    //   |                              |                                               |
    //   |                      training-data-generator        (optional)               |
    //   |                              |                                               |
    //   |                      inverse-model-learning         (optional)               |
    //   |                              |                                               |
    //   |                      lai-mono-date-mask-flags                                |
    //   |                              |                                               |
    //   |                      ndvi-rvi-extraction                                     |
    //   |                              |                                               |
    //   |              ------------------------                                        |
    //   |              |                      |                                        |
    //   |      bv-image-inversion     bv-err-image-inversion                           |
    //   |              |                      |                                        |
    //   |      quantify-image          quantify-err-image                              |
    //   |              |                      |                                        |
    //   |              ------------------------                                        |
    //   |                              |                                               |
    //   -------------------------------------------------------------------------------
    //                                  |
    //                          product-formatter
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
        const TileInfos &tileInfo = tileInfosList[i];
        // add the tasks for generating models
        if(bForceGenModels || tileInfo.modelInfos.NeedsModelGeneration()) {
            // prosail simulator -> generate-vars
            // if we want chaining products and we have a previous product executed
            if(bChainProducts && initialTasksNo > 0) {
                // we create a dependency to the last task of the previous product
                outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
            }   // else skip over the  as we run the BVVariableGeneration with no previous dependency, allowing
                // running several products in parallel

            nCurTaskIdx++;
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
            nCurTaskIdx++;
            // trainig -> prosail simulator
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
            nCurTaskIdx++;
            // inverse model -> trainig
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
            nCurTaskIdx++;
            // now update the index for the lai-mono-date-mask-flags task and set its parent to the inverse-model-learning task
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
        } else {
            // if we want chaining products and we have a previous product executed
            if(bChainProducts && initialTasksNo > 0) {
                // we create a dependency to the last task of the previous product
                outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
            }   // else  skip over the lai-mono-date-mask-flags as we run it with no previous dependency,
                // allowing running several products in parallel
        }
        // increment the current index for ndvi-rvi-extraction
        nCurTaskIdx++;

        // ndvi-rvi-extraction -> lai-mono-date-mask-flags
        int ndviRviExtrIdx = nCurTaskIdx++;
        outAllTasksList[ndviRviExtrIdx].parentTasks.append(outAllTasksList[ndviRviExtrIdx-1]);

        // the others comme naturally updated
        // bv-image-inversion -> ndvi-rvi-extraction
        int nBVImageInversionIdx = nCurTaskIdx++;
        outAllTasksList[nBVImageInversionIdx].parentTasks.append(outAllTasksList[ndviRviExtrIdx]);

        // bv-err-image-inversion -> ndvi-rvi-extraction
        int nBVErrImageInversionIdx = nCurTaskIdx++;
        outAllTasksList[nBVErrImageInversionIdx].parentTasks.append(outAllTasksList[ndviRviExtrIdx]);

        // quantify-image -> bv-image-inversion
        int nQuantifyImageInversionIdx = nCurTaskIdx++;
        outAllTasksList[nQuantifyImageInversionIdx].parentTasks.append(outAllTasksList[nBVImageInversionIdx]);

        // quantify-err-image -> bv-err-image-inversion
        int nQuantifyErrImageInversionIdx = nCurTaskIdx++;
        outAllTasksList[nQuantifyErrImageInversionIdx].parentTasks.append(outAllTasksList[nBVErrImageInversionIdx]);

        // add the quantify image tasks to the list of the product formatter corresponding to this product
        productFormatterParentsRefs.append(outAllTasksList[nQuantifyImageInversionIdx]);
        productFormatterParentsRefs.append(outAllTasksList[nQuantifyErrImageInversionIdx]);
    }
    int productFormatterIdx = nCurTaskIdx++;
    outAllTasksList[productFormatterIdx].parentTasks.append(productFormatterParentsRefs);
    if(bRemoveTempFiles) {
        // cleanup-intermediate-files -> product formatter
        outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
    }

}

NewStepList LaiRetrievalHandlerL3B::GetStepsToGenModel(std::map<QString, QString> &configParameters,
                                             const QList<TileInfos> &listPrdTiles, QList<TaskToSubmit> &allTasksList,
                                             int tasksStartIdx, bool bForceGenModels)
{
    NewStepList steps;
    const auto &modelsFolder = configParameters["processor.l3b.lai.modelsfolder"];
    const auto &rsrCfgFile = configParameters["processor.l3b.lai.rsrcfgfile"];
    const auto &globalSampleFile = configParameters["processor.l3b.lai.global_bv_samples_file"];
    bool useLaiBandsCfg = ((configParameters["processor.l3b.lai.use_lai_bands_cfg"]).toInt() != 0);
    const auto &laiCfgFile = configParameters["processor.l3b.lai.laibandscfgfile"];

    // in allTasksList we might have tasks from other products. We start from the first task of the current product
    int curIdx = tasksStartIdx;
    for(int i = 0; i<listPrdTiles.size(); i++) {
        const TileInfos &tileInfo = listPrdTiles[i];
        if(bForceGenModels || tileInfo.modelInfos.NeedsModelGeneration()) {
            const QString &curXml = tileInfo.tileFile;
            int loopFirstIdx = curIdx;
            TaskToSubmit &bvInputVariableGenerationTask = allTasksList[loopFirstIdx];
            TaskToSubmit &prosailSimulatorTask = allTasksList[loopFirstIdx+1];
            TaskToSubmit &trainingDataGeneratorTask = allTasksList[loopFirstIdx+2];
            TaskToSubmit &inverseModelLearningTask = allTasksList[loopFirstIdx+3];

            const auto & generatedSampleFile = bvInputVariableGenerationTask.GetFilePath("out_bv_dist_samples.txt");
            const auto & simuReflsFile = prosailSimulatorTask.GetFilePath("out_simu_refls.txt");
            const auto & anglesFile = prosailSimulatorTask.GetFilePath("out_angles.txt");
            const auto & trainingFile = trainingDataGeneratorTask.GetFilePath("out_training.txt");
            const auto & modelFile = inverseModelLearningTask.GetFilePath("out_model.txt");
            const auto & errEstModelFile = inverseModelLearningTask.GetFilePath("out_err_est_model.txt");

            const QStringList &BVInputVariableGenerationArgs = GetBVInputVariableGenerationArgs(configParameters, generatedSampleFile);
            QString bvSamplesFile = generatedSampleFile;
            // if configured a global samples file, then use it instead of the current generated one
            if(globalSampleFile != "") {
                bvSamplesFile = globalSampleFile;
            }
            const QStringList &InverseModelLearningArgs = GetInverseModelLearningArgs(trainingFile, curXml, modelFile, errEstModelFile, modelsFolder, configParameters);
            steps.append(bvInputVariableGenerationTask.CreateStep("BVInputVariableGeneration", BVInputVariableGenerationArgs));

            // Use the new configurable version or the old one
            if (useLaiBandsCfg) {
                const QStringList &ProSailSimulatorArgs = GetProSailSimulatorNewArgs(curXml, bvSamplesFile, rsrCfgFile, simuReflsFile,
                                                                                     anglesFile, configParameters, laiCfgFile);
                steps.append(prosailSimulatorTask.CreateStep("ProSailSimulatorNew", ProSailSimulatorArgs));

                const QStringList &TrainingDataGeneratorArgs = GetTrainingDataGeneratorNewArgs(curXml, bvSamplesFile, simuReflsFile,
                                                                                               trainingFile, laiCfgFile);
                steps.append(trainingDataGeneratorTask.CreateStep("TrainingDataGeneratorNew", TrainingDataGeneratorArgs));
            } else {
                const QStringList &ProSailSimulatorArgs = GetProSailSimulatorArgs(curXml, bvSamplesFile, rsrCfgFile, simuReflsFile, anglesFile, configParameters);
                steps.append(prosailSimulatorTask.CreateStep("ProSailSimulator", ProSailSimulatorArgs));

                const QStringList &TrainingDataGeneratorArgs = GetTrainingDataGeneratorArgs(curXml, bvSamplesFile, simuReflsFile, trainingFile);
                steps.append(trainingDataGeneratorTask.CreateStep("TrainingDataGenerator", TrainingDataGeneratorArgs));
            }

            steps.append(inverseModelLearningTask.CreateStep("InverseModelLearning", InverseModelLearningArgs));
            curIdx += MODEL_GEN_TASKS_PER_PRODUCT;
        }
        curIdx += LAI_TASKS_PER_PRODUCT;
    }

    return steps;
}

NewStepList LaiRetrievalHandlerL3B::GetStepsForMonodateLai(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                    const QList<TileInfos> &prdTilesInfosList, QList<TaskToSubmit> &allTasksList,
                                                    bool bRemoveTempFiles, int tasksStartIdx)
{
    NewStepList steps;
    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");
    bool useLaiBandsCfg = ((configParameters["processor.l3b.lai.use_lai_bands_cfg"]).toInt() != 0);
    const auto &laiCfgFile = configParameters["processor.l3b.lai.laibandscfgfile"];

    // Get the resolution value
    int resolution = 0;
    if(!GetParameterValueAsInt(parameters, "resolution", resolution) ||
            resolution == 0) {
        resolution = 10;    // TODO: We should configure the default resolution in DB
    }
    const auto &resolutionStr = QString::number(resolution);

    bool bForceGenModels = IsForceGenModels(parameters, configParameters);

    const auto &modelsFolder = configParameters["processor.l3b.lai.modelsfolder"];
    // in allTasksList we might have tasks from other products. We start from the first task of the current product
    int curTaskIdx = tasksStartIdx;

    QStringList ndviList;
    QStringList laiList;
    QStringList laiErrList;
    QStringList laiFlgsList;
    QStringList cleanupTemporaryFilesList;
    for (int i = 0; i<prdTilesInfosList.size(); i++) {
        const auto &prdTileInfo = prdTilesInfosList[i];
        if(bForceGenModels || prdTileInfo.modelInfos.NeedsModelGeneration()) {
            // now update the index for the ndviRvi task
            curTaskIdx += MODEL_GEN_TASKS_PER_PRODUCT;
        }
        TaskToSubmit &genMonoDateMskFagsTask = allTasksList[curTaskIdx++];
        TaskToSubmit &ndviRviExtractorTask = allTasksList[curTaskIdx++];
        TaskToSubmit &bvImageInversionTask = allTasksList[curTaskIdx++];
        TaskToSubmit &bvErrImageInversionTask = allTasksList[curTaskIdx++];
        TaskToSubmit &quantifyImageTask = allTasksList[curTaskIdx++];
        TaskToSubmit &quantifyErrImageTask = allTasksList[curTaskIdx++];

        const auto & monoDateMskFlgsFileName = genMonoDateMskFagsTask.GetFilePath("LAI_mono_date_msk_flgs_img.tif");
        auto ftsFile = ndviRviExtractorTask.GetFilePath("ndvi_rvi.tif");
        const auto & monoDateLaiFileName = bvImageInversionTask.GetFilePath("LAI_mono_date_img.tif");
        const auto & monoDateErrFileName = bvErrImageInversionTask.GetFilePath("LAI_mono_date_ERR_img.tif");

        auto singleNdviFile = ndviRviExtractorTask.GetFilePath("single_ndvi.tif");
        auto monoDateMskFlgsResFileName = genMonoDateMskFagsTask.GetFilePath("LAI_mono_date_msk_flgs_img_resampled.tif");
        const auto & quantifiedLaiFileName = quantifyImageTask.GetFilePath("LAI_mono_date_img_16.tif");
        const auto & quantifiedErrFileName = quantifyErrImageTask.GetFilePath("LAI_mono_date_ERR_img_16.tif");

        QStringList genMonoDateMskFagsArgs = GetMonoDateMskFlagsArgs(prdTileInfo.tileFile, monoDateMskFlgsFileName,
                                                                     "\"" + monoDateMskFlgsResFileName+"?gdal:co:COMPRESS=DEFLATE\"",
                                                                     resolutionStr);

        // add these steps to the steps list to be submitted
        steps.append(genMonoDateMskFagsTask.CreateStep("GenerateLaiMonoDateMaskFlags", genMonoDateMskFagsArgs));

        if (useLaiBandsCfg) {
            const QStringList &ndviRviExtractionArgs = GetNdviRviExtractionNewArgs(prdTileInfo.tileFile, monoDateMskFlgsFileName,
                                                                         ftsFile, "\"" + singleNdviFile+"?gdal:co:COMPRESS=DEFLATE\"",
                                                                         resolutionStr, laiCfgFile);
            steps.append(ndviRviExtractorTask.CreateStep("NdviRviExtractionNew", ndviRviExtractionArgs));
        } else {
            const QStringList &ndviRviExtractionArgs = GetNdviRviExtractionArgs(prdTileInfo.tileFile, monoDateMskFlgsFileName,
                                                                         ftsFile, "\"" + singleNdviFile+"?gdal:co:COMPRESS=DEFLATE\"",
                                                                         resolutionStr);
            steps.append(ndviRviExtractorTask.CreateStep("NdviRviExtraction2", ndviRviExtractionArgs));
        }

        QStringList bvImageInvArgs = GetBvImageInvArgs(ftsFile, monoDateMskFlgsResFileName, prdTileInfo.tileFile, modelsFolder, monoDateLaiFileName);
        QStringList bvErrImageInvArgs = GetBvErrImageInvArgs(ftsFile, monoDateMskFlgsResFileName, prdTileInfo.tileFile, modelsFolder, monoDateErrFileName);
        QStringList quantifyImageArgs = GetQuantifyImageArgs(monoDateLaiFileName, quantifiedLaiFileName);
        QStringList quantifyErrImageArgs = GetQuantifyImageArgs(monoDateErrFileName, quantifiedErrFileName);

        // save the mono date LAI file name list
        ndviList.append(singleNdviFile);
        laiList.append(quantifiedLaiFileName);
        laiErrList.append(quantifiedErrFileName);
        laiFlgsList.append(monoDateMskFlgsResFileName);

        steps.append(bvImageInversionTask.CreateStep("BVImageInversion", bvImageInvArgs));
        steps.append(bvErrImageInversionTask.CreateStep("BVImageInversion", bvErrImageInvArgs));
        steps.append(quantifyImageTask.CreateStep("QuantifyImage", quantifyImageArgs));
        steps.append(quantifyErrImageTask.CreateStep("QuantifyImage", quantifyErrImageArgs));

        cleanupTemporaryFilesList.append(monoDateMskFlgsFileName);
        cleanupTemporaryFilesList.append(ftsFile);
        cleanupTemporaryFilesList.append(monoDateLaiFileName);
        cleanupTemporaryFilesList.append(monoDateErrFileName);
        cleanupTemporaryFilesList.append(singleNdviFile);
        cleanupTemporaryFilesList.append(monoDateMskFlgsResFileName);
        cleanupTemporaryFilesList.append(quantifiedLaiFileName);
        cleanupTemporaryFilesList.append(quantifiedErrFileName);
    }
    TaskToSubmit &laiMonoProductFormatterTask = allTasksList[curTaskIdx++];
    QStringList productFormatterArgs = GetLaiMonoProductFormatterArgs(
                laiMonoProductFormatterTask, ctx, event, prdTilesInfosList,
                ndviList, laiList, laiErrList, laiFlgsList);
    steps.append(laiMonoProductFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));

    if(bRemoveTempFiles) {
        TaskToSubmit &cleanupTemporaryFilesTask = allTasksList[curTaskIdx++];
        // add also the cleanup step
        steps.append(cleanupTemporaryFilesTask.CreateStep("CleanupTemporaryFiles", cleanupTemporaryFilesList));
    }

    return steps;
}
void LaiRetrievalHandlerL3B::WriteExecutionInfosFile(const QString &executionInfosPath,
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

void LaiRetrievalHandlerL3B::HandleProduct(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                            const QList<TileInfos> &prdTilesInfosList, QList<TaskToSubmit> &allTasksList) {

    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");

    bool bForceGenModels = IsForceGenModels(parameters, configParameters);
    bool bRemoveTempFiles = NeedRemoveJobFolder(ctx, event.jobId, "l3b");

    int tasksStartIdx = allTasksList.size();
    // create the tasks
    CreateTasksForNewProduct(allTasksList, prdTilesInfosList, bForceGenModels, bRemoveTempFiles);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(int i = tasksStartIdx; i < allTasksList.size(); i++) {
        const TaskToSubmit &task = allTasksList.at(i);
        allTasksListRef.append((TaskToSubmit&)task);
    }
    // submit all tasks
    SubmitTasks(ctx, event.jobId, allTasksListRef);

    NewStepList steps;

    // first extract the model file names from the models folder
    steps += GetStepsToGenModel(configParameters, prdTilesInfosList, allTasksList, tasksStartIdx, bForceGenModels);

    steps += GetStepsForMonodateLai(ctx, event, prdTilesInfosList, allTasksList, bRemoveTempFiles, tasksStartIdx);
    ctx.SubmitSteps(steps);
}

void LaiRetrievalHandlerL3B::SubmitEndOfLaiTask(EventProcessingContext &ctx,
                                                const JobSubmittedEvent &event,
                                                const QList<TaskToSubmit> &allTasksList) {
    // add the end of lai job that will perform the cleanup
    QList<std::reference_wrapper<const TaskToSubmit>> prdFormatterTasksListRef;
    for(const TaskToSubmit &task: allTasksList) {
        if(task.moduleName == "lai-mono-date-product-formatter") {
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

void LaiRetrievalHandlerL3B::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");
    const auto &modelsFolder = configParameters["processor.l3b.lai.modelsfolder"];

    bool bMonoDateLai = IsGenMonoDate(parameters, configParameters);
    if(!bMonoDateLai) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("LAI mono-date processing needs to be defined").toStdString());
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
    QStringList allModels;
    QStringList allErrModels;
    bool bForceGenModels = IsForceGenModels(parameters, configParameters);
    // if we are forced to generate the models, then do not extract anymore
    // the list of models
    if(!bForceGenModels) {
        GetModelFileList(modelsFolder, "Model_", allModels);
        GetModelFileList(modelsFolder, "Err_Est_Model_", allErrModels);
    }

    //container for all task
    QList<TaskToSubmit> allTasksList;
    for(const auto &key : dateGroupedInputProductToTilesMap.keys()) {
        const QStringList &prdTilesList = dateGroupedInputProductToTilesMap[key];
        // create structures providing the models for each tile
        QList<TileInfos> tilesInfosList;
        for(const QString &prdTile: prdTilesList) {
            TileInfos tileInfo;
            tileInfo.tileFile = prdTile;
            tileInfo.modelInfos.model = GetExistingModelForTile(allModels, prdTile);
            tileInfo.modelInfos.errModel = GetExistingModelForTile(allErrModels, prdTile);
            tilesInfosList.append(tileInfo);
        }
        HandleProduct(ctx, event, tilesInfosList, allTasksList);
    }

    // we add a task in order to wait for all product formatter to finish.
    // This will allow us to mark the job as finished and to remove the job folder
    SubmitEndOfLaiTask(ctx, event, allTasksList);
}

void LaiRetrievalHandlerL3B::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    if (event.module == "lai-end-of-job") {
        ctx.MarkJobFinished(event.jobId);
        // Now remove the job folder containing temporary files
        RemoveJobFolder(ctx, event.jobId, "l3b");
    }
    if ((event.module == "lai-mono-date-product-formatter")) {
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

QStringList LaiRetrievalHandlerL3B::GetCompressImgArgs(const QString &inFile, const QString &outFile) {
    return { "-in", inFile,
             "-out", "\"" + outFile+"?gdal:co:COMPRESS=DEFLATE\""
           };
}

QStringList LaiRetrievalHandlerL3B::GetNdviRviExtractionNewArgs(const QString &inputProduct, const QString &msksFlagsFile, const QString &ftsFile,
                                                          const QString &ndviFile, const QString &resolution, const QString &laiBandsCfg) {
    return { "NdviRviExtractionNew",
           "-xml", inputProduct,
           "-msks", msksFlagsFile,
           "-ndvi", ndviFile,
           "-fts", ftsFile,
           "-outres", resolution,
           "-laicfgs", laiBandsCfg
    };
}

QStringList LaiRetrievalHandlerL3B::GetNdviRviExtractionArgs(const QString &inputProduct, const QString &msksFlagsFile, const QString &ftsFile,
                                                          const QString &ndviFile, const QString &resolution) {
    return { "NdviRviExtraction2",
           "-xml", inputProduct,
           "-msks", msksFlagsFile,
           "-ndvi", ndviFile,
           "-fts", ftsFile,
           "-outres", resolution
    };
}

QStringList LaiRetrievalHandlerL3B::GetBvImageInvArgs(const QString &ftsFile, const QString &msksFlagsFile, const QString &xmlFile,
                                                   const QString &modelsFolder, const QString &monoDateLaiFileName) {
    return { "BVImageInversion",
        "-in", ftsFile,
        "-msks", msksFlagsFile,
        "-out", monoDateLaiFileName,
        "-xml", xmlFile,
        "-modelsfolder", modelsFolder,
        "-modelprefix", "Model_"
    };
}

QStringList LaiRetrievalHandlerL3B::GetBvErrImageInvArgs(const QString &ftsFile, const QString &msksFlagsFile, const QString &xmlFile,
                                                      const QString &modelsFolder, const QString &monoDateErrFileName)  {
    return { "BVImageInversion",
        "-in", ftsFile,
        "-msks", msksFlagsFile,
        "-out", monoDateErrFileName,
        "-xml", xmlFile,
        "-modelsfolder", modelsFolder,
        "-modelprefix", "Err_Est_Model_"
    };
}

QStringList LaiRetrievalHandlerL3B::GetQuantifyImageArgs(const QString &inFileName, const QString &outFileName)  {
    return { "QuantifyImage",
        "-in", inFileName,
        "-out", "\"" + outFileName+"?gdal:co:COMPRESS=DEFLATE\""
    };
}

QStringList LaiRetrievalHandlerL3B::GetMonoDateMskFlagsArgs(const QString &inputProduct, const QString &monoDateMskFlgsFileName,
                                                         const QString &monoDateMskFlgsResFileName, const QString &resStr) {
    return { "GenerateLaiMonoDateMaskFlags",
      "-inxml", inputProduct,
      "-out", monoDateMskFlgsFileName,
      "-outres", resStr,
      "-outresampled", monoDateMskFlgsResFileName
    };
}

QStringList LaiRetrievalHandlerL3B::GetLaiMonoProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                                const QList<TileInfos> &prdTilesInfosList, const QStringList &ndviList,
                                                                const QStringList &laiList, const QStringList &laiErrList, const QStringList &laiFlgsList) {

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

    productFormatterArgs += "-processor.vegetation.laindvi";
    for(int i = 0; i<tileIdsList.size(); i++) {
        productFormatterArgs += GetProductFormatterTile(tileIdsList[i]);
        productFormatterArgs += ndviList[i];
    }

    productFormatterArgs += "-processor.vegetation.laimonodate";
    for(int i = 0; i<tileIdsList.size(); i++) {
        productFormatterArgs += GetProductFormatterTile(tileIdsList[i]);
        productFormatterArgs += laiList[i];
    }

    productFormatterArgs += "-processor.vegetation.laimonodateerr";
    for(int i = 0; i<tileIdsList.size(); i++) {
        productFormatterArgs += GetProductFormatterTile(tileIdsList[i]);
        productFormatterArgs += laiErrList[i];
    }
    productFormatterArgs += "-processor.vegetation.laimdateflgs";
    for(int i = 0; i<tileIdsList.size(); i++) {
        productFormatterArgs += GetProductFormatterTile(tileIdsList[i]);
        productFormatterArgs += laiFlgsList[i];
    }

    return productFormatterArgs;
}

QStringList LaiRetrievalHandlerL3B::GetBVInputVariableGenerationArgs(std::map<QString, QString> &configParameters, const QString &strGenSampleFile) {
    QString samplesNo = GetDefaultCfgVal(configParameters, "processor.l3b.lai.models.samples", DEFAULT_GENERATED_SAMPLES_NO);
    return { "BVInputVariableGeneration",
                "-samples", samplesNo,
                "-out", strGenSampleFile
    };
}

QStringList LaiRetrievalHandlerL3B::GetProSailSimulatorNewArgs(const QString &product, const QString &bvFileName, const QString &rsrCfgFileName,
                                                         const QString &outSimuReflsFile, const QString &outAngles,
                                                         std::map<QString, QString> &configParameters, const QString &laiBandsCfg) {
    //QString noiseVar = GetDefaultCfgVal(configParameters, "processor.l3b.lai.models.noisevar", DEFAULT_NOISE_VAR);
    return { "ProSailSimulatorNew",
                "-xml", product,
                "-bvfile", bvFileName,
                "-rsrcfg", rsrCfgFileName,
                "-out", outSimuReflsFile,
                "-outangles", outAngles,
                "-laicfgs", laiBandsCfg
//                "-noisevar", noiseVar
    };
}

QStringList LaiRetrievalHandlerL3B::GetProSailSimulatorArgs(const QString &product, const QString &bvFileName, const QString &rsrCfgFileName,
                                                         const QString &outSimuReflsFile, const QString &outAngles, std::map<QString, QString> &configParameters) {
    //QString noiseVar = GetDefaultCfgVal(configParameters, "processor.l3b.lai.models.noisevar", DEFAULT_NOISE_VAR);
    return { "ProSailSimulator",
                "-xml", product,
                "-bvfile", bvFileName,
                "-rsrcfg", rsrCfgFileName,
                "-out", outSimuReflsFile,
                "-outangles", outAngles
//                "-noisevar", noiseVar
    };
}

QStringList LaiRetrievalHandlerL3B::GetTrainingDataGeneratorNewArgs(const QString &product, const QString &biovarsFile,
                                                              const QString &simuReflsFile, const QString &outTrainingFile,
                                                              const QString &laiBandsCfg) {
    return { "TrainingDataGeneratorNew",
                "-xml", product,
                "-biovarsfile", biovarsFile,
                "-simureflsfile", simuReflsFile,
                "-outtrainfile", outTrainingFile,
                "-laicfgs", laiBandsCfg
    };
}

QStringList LaiRetrievalHandlerL3B::GetTrainingDataGeneratorArgs(const QString &product, const QString &biovarsFile,
                                                              const QString &simuReflsFile, const QString &outTrainingFile) {
    return { "TrainingDataGenerator",
                "-xml", product,
                "-biovarsfile", biovarsFile,
                "-simureflsfile", simuReflsFile,
                "-outtrainfile", outTrainingFile,
                "-addrefls", "1"
    };
}

QStringList LaiRetrievalHandlerL3B::GetInverseModelLearningArgs(const QString &trainingFile, const QString &product, const QString &modelFile,
                                                             const QString &errEstFile, const QString &modelsFolder,
                                                             std::map<QString, QString> &configParameters) {
    QString bestOf = GetDefaultCfgVal(configParameters, "processor.l3b.lai.models.bestof", DEFAULT_BEST_OF);
    QString regressor = GetDefaultCfgVal(configParameters, "processor.l3b.lai.models.regressor", DEFAULT_REGRESSOR);

    return { "InverseModelLearning",
                "-training", trainingFile,
                "-out", modelFile,
                "-errest", errEstFile,
                "-regression", regressor,
                "-bestof", bestOf,
                "-xml", product,
                "-newnamesoutfolder", modelsFolder
    };
}

const QString& LaiRetrievalHandlerL3B::GetDefaultCfgVal(std::map<QString, QString> &configParameters, const QString &key, const QString &defVal) {
    auto search = configParameters.find(key);
    if(search != configParameters.end()) {
        return search->second;
    }
    return defVal;
}

bool LaiRetrievalHandlerL3B::IsForceGenModels(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
    bool bForceGenModels = false;
    if(parameters.contains("genmodel")) {
        bForceGenModels = (parameters["genmodel"].toInt() != 0);
    } else {
        bForceGenModels = ((configParameters["processor.l3b.generate_models"]).toInt() != 0);
    }
    return bForceGenModels;
}

bool LaiRetrievalHandlerL3B::IsGenMonoDate(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
    bool bMonoDateLai = true;
    if(parameters.contains("monolai")) {
        const auto &value = parameters["monolai"];
        if(value.isDouble())
            bMonoDateLai = (value.toInt() != 0);
        else if(value.isString()) {
            bMonoDateLai = (value.toString() == "1");
        }
    } else {
        bMonoDateLai = ((configParameters["processor.l3b.mono_date_lai"]).toInt() != 0);
    }
    return bMonoDateLai;
}

ProcessorJobDefinitionParams LaiRetrievalHandlerL3B::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
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

    params.productList = ctx.GetProductsByInsertedTime(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
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

void LaiRetrievalHandlerL3B::SubmitL3CJobForL3BProduct(EventProcessingContext &ctx, const TaskFinishedEvent &event,
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

void LaiRetrievalHandlerL3B::GetModelFileList(const QString &folderName, const QString &modelPrefix, QStringList &outModelsList) {
    QDirIterator it(folderName, QStringList() << modelPrefix + "*.txt", QDir::Files/*, QDirIterator::NoIteratorFlags*/);
    while (it.hasNext()) {
        it.next();
        if (QFileInfo(it.filePath()).isFile()) {
            outModelsList.append(it.filePath());
        }
    }
}

bool LaiRetrievalHandlerL3B::InRange(double middle, double distance, double value) {
      if((value >= (middle - distance)) &&
         (value <= (middle + distance))) {
            return true;
      }

      return false;
}

bool LaiRetrievalHandlerL3B::ParseModelFileName(const QString &qtModelFileName, double &solarZenith, double &sensorZenith, double &relAzimuth) {
    //The expected file name format is:
    // [FILEPREFIX_]THETA_S_<solarzenith>_THETA_V_<sensorzenith>_REL_PHI_%f
    std::string modelFileName = qtModelFileName.toStdString();
    std::size_t nThetaSPos = modelFileName.find("_THETA_S_");
    if (nThetaSPos == std::string::npos)
      return false;

    std::size_t nThetaVPos = modelFileName.find("_THETA_V_");
    if (nThetaVPos == std::string::npos)
      return false;

    std::size_t nRelPhiPos = modelFileName.find("_REL_PHI_");
    if (nRelPhiPos == std::string::npos)
      return false;

    std::size_t nExtPos = modelFileName.find(".txt");
    if (nExtPos == std::string::npos)
      return false;

    int nThetaSStartIdx = nThetaSPos + strlen("_THETA_S_");
    const std::string &strThetaS = modelFileName.substr(nThetaSStartIdx, nThetaVPos - nThetaSStartIdx);

    int nThetaVStartIdx = nThetaVPos + strlen("_THETA_V_");
    const std::string &strThetaV = modelFileName.substr(nThetaVStartIdx, nRelPhiPos - nThetaVStartIdx);

    int nRelPhiStartIdx = nRelPhiPos + strlen("_REL_PHI_");
    const std::string &strRelPhi = modelFileName.substr(nRelPhiStartIdx, nExtPos - nRelPhiStartIdx);

    solarZenith = ::atof(strThetaS.c_str());
    sensorZenith = ::atof(strThetaV.c_str());
    relAzimuth = ::atof(strRelPhi.c_str());

    return true;
}

QString LaiRetrievalHandlerL3B::GetExistingModelForTile(const QStringList &modelsList, const QString &tileFile) {
    // no need to parse the tile file as we have nothing to compare with
    if(modelsList.size() == 0) {
        return "";
    }

    double fSolarZenith;
    double fSensorZenith;
    double fRelAzimuth;

    MaccsHdrMeanAnglesReader reader(tileFile);
    MaccsHdrMeanAngles anglesInfos = reader.read();
    if(!anglesInfos.IsValid()) {
        // if the angles were not successfully loaded, then exit
        return "";
    }

    MeanAngles solarAngles = anglesInfos.sunMeanAngles;
    double relativeAzimuth = anglesInfos.GetRelativeAzimuthAngle();

    for(int i = 0; i<(int)modelsList.size(); i++) {
        const QString &modelName = modelsList[i];

        if(ParseModelFileName(modelName, fSolarZenith, fSensorZenith, fRelAzimuth)) {

            int nTotalBandsNo = anglesInfos.meanViewingAnglesList.size();
            for(int j = 0; j<nTotalBandsNo; j++) {
                const MeanAngles &sensorBandAngles = anglesInfos.meanViewingAnglesList[j].meanViewingAngles;
                if(InRange(fSolarZenith, 0.5, solarAngles.zenith) &&
                   InRange(fSensorZenith, 0.5, sensorBandAngles.zenith) &&
                   InRange(fRelAzimuth, 0.5, relativeAzimuth)) {
                    return modelName;
                }
            }
        }
    }

    return "";
}
