#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "lairetrievalhandler_l3b.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"
#include "logger.hpp"

// The number of tasks that are executed for each product before executing time series tasks
#define LAI_TASKS_PER_PRODUCT       6
#define MODEL_GEN_TASKS_PER_PRODUCT 4

#define DEFAULT_GENERATED_SAMPLES_NO    "40000"
#define DEFAULT_NOISE_VAR               "0.01"
#define DEFAULT_BEST_OF                 "1"
#define DEFAULT_REGRESSOR               "nn"

void LaiRetrievalHandlerL3B::CreateTasksForNewProduct(QList<TaskToSubmit> &outAllTasksList,
                                                    const QStringList &listProducts,
                                                    bool bGenModels, bool bRemoveTempFiles) {
    int nbLaiMonoProducts = listProducts.size();
    for(int i = 0; i<nbLaiMonoProducts; i++) {
        if(bGenModels) {
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
    QList<int> laiMonoDateFlgsIdxs;
    QList<int> bvImageInvIdxs;
    QList<int> bvErrImageInvIdxs;
    QList<int> quantifyImageInvIdxs;
    QList<int> quantifyErrImageInvIdxs;

    QList<std::reference_wrapper<const TaskToSubmit>> productFormatterParentsRefs;

    // we execute in parallel and launch at once all processing chains for each product
    // for example, if we have genModels, we launch all bv-input-variable-generation for all products
    // if we do not have genModels, we launch all NDVIRVIExtraction in the same time for all products
    int nCurTaskIdx = 0;
    for(i = 0; i<nbLaiMonoProducts; i++) {
        // add the tasks for generating models
        if(bGenModels) {
            // prosail simulator -> generate-vars
            // skip over the BVVariableGeneration
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
            //genMasksIdx += MODEL_GEN_TASKS_PER_PRODUCT;
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
        }

        int genFlagsIdx = nCurTaskIdx++;

        // ndvi-rvi-extraction -> lai-mono-date-mask-flags
        int ndviRviExtrIdx = nCurTaskIdx++;
        outAllTasksList[ndviRviExtrIdx].parentTasks.append(outAllTasksList[ndviRviExtrIdx-1]);

        // we add it here in list as we might have a cut before
        laiMonoDateFlgsIdxs.append(genFlagsIdx);

        // the others comme naturally updated
        // bv-image-inversion -> ndvi-rvi-extraction
        int nBVImageInversionIdx = nCurTaskIdx++;
        outAllTasksList[nBVImageInversionIdx].parentTasks.append(outAllTasksList[ndviRviExtrIdx]);
        bvImageInvIdxs.append(nBVImageInversionIdx);

        // bv-err-image-inversion -> ndvi-rvi-extraction
        int nBVErrImageInversionIdx = nCurTaskIdx++;
        outAllTasksList[nBVErrImageInversionIdx].parentTasks.append(outAllTasksList[ndviRviExtrIdx]);
        bvErrImageInvIdxs.append(nBVErrImageInversionIdx);

        // quantify-image -> bv-image-inversion
        int nQuantifyImageInversionIdx = nCurTaskIdx++;
        outAllTasksList[nQuantifyImageInversionIdx].parentTasks.append(outAllTasksList[nBVImageInversionIdx]);
        quantifyImageInvIdxs.append(nQuantifyImageInversionIdx);

        // quantify-err-image -> bv-err-image-inversion
        int nQuantifyErrImageInversionIdx = nCurTaskIdx++;
        outAllTasksList[nQuantifyErrImageInversionIdx].parentTasks.append(outAllTasksList[nBVErrImageInversionIdx]);
        quantifyErrImageInvIdxs.append(nQuantifyErrImageInversionIdx);

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
                                             const QStringList &listProducts,
                                             QList<TaskToSubmit> &allTasksList)
{
    NewStepList steps;
    const auto &modelsFolder = configParameters["processor.l3b.lai.modelsfolder"];
    const auto &rsrCfgFile = configParameters["processor.l3b.lai.rsrcfgfile"];
    int curIdx = 0;
    for(int i = 0; i<listProducts.size(); i++) {
        const QString &curXml = listProducts[i];
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


        QStringList BVInputVariableGenerationArgs = GetBVInputVariableGenerationArgs(configParameters, generatedSampleFile);
        QStringList ProSailSimulatorArgs = GetProSailSimulatorArgs(curXml, generatedSampleFile, rsrCfgFile, simuReflsFile, anglesFile, configParameters);
        QStringList TrainingDataGeneratorArgs = GetTrainingDataGeneratorArgs(curXml, generatedSampleFile, simuReflsFile, trainingFile);
        QStringList InverseModelLearningArgs = GetInverseModelLearningArgs(trainingFile, curXml, modelFile, errEstModelFile, modelsFolder, configParameters);

        steps.append(bvInputVariableGenerationTask.CreateStep("BVInputVariableGeneration", BVInputVariableGenerationArgs));
        steps.append(prosailSimulatorTask.CreateStep("ProSailSimulator", ProSailSimulatorArgs));
        steps.append(trainingDataGeneratorTask.CreateStep("TrainingDataGenerator", TrainingDataGeneratorArgs));
        steps.append(inverseModelLearningTask.CreateStep("InverseModelLearning", InverseModelLearningArgs));
        curIdx += MODEL_GEN_TASKS_PER_PRODUCT;
        curIdx += LAI_TASKS_PER_PRODUCT;
    }

    return steps;
}

NewStepList LaiRetrievalHandlerL3B::GetStepsForMonodateLai(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                    const QStringList &prdTilesList, QList<TaskToSubmit> &allTasksList,
                                                    bool bRemoveTempFiles)
{
    NewStepList steps;
    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");

    // Get the resolution value
    int resolution = 0;
    if(!GetParameterValueAsInt(parameters, "resolution", resolution) ||
            resolution == 0) {
        resolution = 10;    // TODO: We should configure the default resolution in DB
    }
    const auto &resolutionStr = QString::number(resolution);

    bool bGenModels = IsGenModels(parameters, configParameters);

    const auto &modelsFolder = configParameters["processor.l3b.lai.modelsfolder"];
    int curTaskIdx = 0;

    QStringList ndviList;
    QStringList laiList;
    QStringList laiErrList;
    QStringList laiFlgsList;
    QStringList cleanupTemporaryFilesList;
    for (int i = 0; i<prdTilesList.size(); i++) {
        const auto &prdTile = prdTilesList[i];
        if(bGenModels) {
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

        QStringList genMonoDateMskFagsArgs = GetMonoDateMskFlagsArgs(prdTile, monoDateMskFlgsFileName,
                                                                     "\"" + monoDateMskFlgsResFileName+"?gdal:co:COMPRESS=DEFLATE\"",
                                                                     resolutionStr);
        QStringList ndviRviExtractionArgs = GetNdviRviExtractionArgs(prdTile, monoDateMskFlgsFileName,
                                                                     ftsFile, "\"" + singleNdviFile+"?gdal:co:COMPRESS=DEFLATE\"",
                                                                     resolutionStr);

        // add these steps to the steps list to be submitted
        steps.append(genMonoDateMskFagsTask.CreateStep("GenerateLaiMonoDateMaskFlags", genMonoDateMskFagsArgs));
        steps.append(ndviRviExtractorTask.CreateStep("NdviRviExtraction2", ndviRviExtractionArgs));

        QStringList bvImageInvArgs = GetBvImageInvArgs(ftsFile, monoDateMskFlgsResFileName, prdTile, modelsFolder, monoDateLaiFileName);
        QStringList bvErrImageInvArgs = GetBvErrImageInvArgs(ftsFile, monoDateMskFlgsResFileName, prdTile, modelsFolder, monoDateErrFileName);
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
                laiMonoProductFormatterTask, ctx, event, prdTilesList,
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
                                               const QStringList &listProducts) {
    std::ofstream executionInfosFile;
    try
    {
        executionInfosFile.open(executionInfosPath.toStdString().c_str(), std::ofstream::out);
        executionInfosFile << "<?xml version=\"1.0\" ?>" << std::endl;
        executionInfosFile << "<metadata>" << std::endl;
        executionInfosFile << "  <General>" << std::endl;
        executionInfosFile << "  </General>" << std::endl;

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

void LaiRetrievalHandlerL3B::HandleProduct(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                            const QStringList &prdTilesList) {

    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");

    bool bGenModels = IsGenModels(parameters, configParameters);
    bool bRemoveTempFiles = NeedRemoveJobFolder(ctx, event.jobId, "l3b");

    QList<TaskToSubmit> allTasksList;
    // create the tasks
    CreateTasksForNewProduct(allTasksList, prdTilesList, bGenModels, bRemoveTempFiles);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    // submit all tasks
    SubmitTasks(ctx, event.jobId, allTasksListRef);

    NewStepList steps;

    // first extract the model file names from the models folder
    if(bGenModels) {
        steps += GetStepsToGenModel(configParameters, prdTilesList, allTasksList);
    }

    steps += GetStepsForMonodateLai(ctx, event, prdTilesList, allTasksList, bRemoveTempFiles);
    ctx.SubmitSteps(steps);
}

void LaiRetrievalHandlerL3B::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");

    bool bMonoDateLai = IsGenMonoDate(parameters, configParameters);
    if(!bMonoDateLai) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("LAI mono-date processing needs to be defined").toStdString());
    }
    
    bool bGenModels = IsGenModels(parameters, configParameters);
    if(bGenModels) {
        const auto &modelsFolder = configParameters["processor.l3b.lai.modelsfolder"];
        if(!QDir::root().mkpath(modelsFolder)) {
            ctx.MarkJobFailed(event.jobId);
            throw std::runtime_error(
                        QStringLiteral("Unable to create path %1 for creating models!").arg(modelsFolder).toStdString());
        }
    }

    QMap<QString, QStringList> inputProductToTilesMap;
    QStringList listTilesMetaFiles = GetL2AInputProductsTiles(ctx, event, inputProductToTilesMap);
    if(listTilesMetaFiles.size() == 0) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No products provided at input or no products available in the specified interval").
                    toStdString());
    }

    for(const auto &prd : inputProductToTilesMap.keys()) {
        const QStringList &prdTilesList = inputProductToTilesMap[prd];
        HandleProduct(ctx, event, prdTilesList);
    }
}

void LaiRetrievalHandlerL3B::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    bool isMonoDatePf;
    if ((isMonoDatePf = (event.module == "lai-mono-date-product-formatter"))) {
        QString prodName = GetProductFormatterProductName(ctx, event);
        QString productFolder = GetProductFormatterOutputProductPath(ctx, event);
        if((prodName != "") && ProcessorHandlerHelper::IsValidHighLevelProduct(productFolder)) {
            QString quicklook = GetProductFormatterQuicklook(ctx, event);
            QString footPrint = GetProductFormatterFootprint(ctx, event);
            ProductType prodType = ProductType::L3BProductTypeId;

            // Insert the product into the database
            QDateTime minDate, maxDate;
            ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prodName, minDate, maxDate);
            int ret = ctx.InsertProduct({ prodType, event.processorId, event.siteId, event.jobId,
                                productFolder, maxDate, prodName,
                                quicklook, footPrint, std::experimental::nullopt, TileList() });
            Logger::debug(QStringLiteral("InsertProduct for %1 returned %2").arg(prodName).arg(ret));
            ctx.MarkJobFinished(event.jobId);
        } else {
            Logger::error(QStringLiteral("Cannot insert into database the product with name %1 and folder %2").arg(prodName).arg(productFolder));
            ctx.MarkJobFailed(event.jobId);
        }
        // Now remove the job folder containing temporary files
        //RemoveJobFolder(ctx, event.jobId, "l3b");
    }
}


QStringList LaiRetrievalHandlerL3B::GetCompressImgArgs(const QString &inFile, const QString &outFile) {
    return { "-in", inFile,
             "-out", "\"" + outFile+"?gdal:co:COMPRESS=DEFLATE\""
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
                                                                const QStringList &prdTilesList, const QStringList &ndviList,
                                                                const QStringList &laiList, const QStringList &laiErrList, const QStringList &laiFlgsList) {

    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");
    QStringList tileIdsList;
    for(const QString &metaFile: prdTilesList) {
        ProcessorHandlerHelper::SatelliteIdType satId;
        QString tileId = ProcessorHandlerHelper::GetTileId(metaFile, satId);
        tileIdsList.append(tileId);
    }

    //const auto &targetFolder = productFormatterTask.GetFilePath("");
    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.xml");

    const auto &lutFile = configParameters["processor.l3b.lai.lut_path"];

    WriteExecutionInfosFile(executionInfosPath, prdTilesList);

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
    productFormatterArgs.append(prdTilesList);

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

QStringList LaiRetrievalHandlerL3B::GetProSailSimulatorArgs(const QString &product, const QString &bvFileName, const QString &rsrCfgFileName,
                                                         const QString &outSimuReflsFile, const QString &outAngles, std::map<QString, QString> &configParameters) {
    QString noiseVar = GetDefaultCfgVal(configParameters, "processor.l3b.lai.models.noisevar", DEFAULT_NOISE_VAR);
    return { "ProSailSimulator",
                "-xml", product,
                "-bvfile", bvFileName,
                "-rsrcfg", rsrCfgFileName,
                "-out", outSimuReflsFile,
                "-outangles", outAngles,
                "-noisevar", noiseVar
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

bool LaiRetrievalHandlerL3B::IsGenModels(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
    bool bGenModels = false;
    if(parameters.contains("genmodel")) {
        bGenModels = (parameters["genmodel"].toInt() != 0);
    } else {
        bGenModels = ((configParameters["processor.l3b.generate_models"]).toInt() != 0);
    }
    return bGenModels;
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
