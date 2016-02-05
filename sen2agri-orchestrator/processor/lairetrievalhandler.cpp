#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "lairetrievalhandler.hpp"
#include "processorhandlerhelper.h"

// The number of tasks that are executed for each product before executing time series tasks
#define TasksNoPerProduct 5

void LaiRetrievalHandler::CreateNewProductInJobTasks(QList<TaskToSubmit> &outAllTasksList, int nbProducts) {
    // just create the tasks but with no information so far
    // first we add the tasks to be performed for each product
    for(int i = 0; i<nbProducts; i++) {
        outAllTasksList.append(TaskToSubmit{"lai-ndvi-rvi-extractor", {}});
        outAllTasksList.append(TaskToSubmit{"lai-models-extractor", {}});
        outAllTasksList.append(TaskToSubmit{"lai-bv-image-invertion", {}});
        outAllTasksList.append(TaskToSubmit{"lai-bv-err-image-invertion", {}});
        outAllTasksList.append(TaskToSubmit{"lai-mono-date-mask-flags", {}});
    }
    outAllTasksList.append(TaskToSubmit{"lai-time-series-builder", {}});
    outAllTasksList.append(TaskToSubmit{"lai-err-time-series-builder", {}});
    outAllTasksList.append(TaskToSubmit{"lai-msk-flags-time-series-builder", {}});
    outAllTasksList.append(TaskToSubmit{"lai-local-window-reprocessing", {}});
    outAllTasksList.append(TaskToSubmit{"lai-local-window-reproc-splitter", {}});
    outAllTasksList.append(TaskToSubmit{"lai-fitted-reprocessing", {}});
    outAllTasksList.append(TaskToSubmit{"lai-fitted-reproc-splitter", {}});
    outAllTasksList.append(TaskToSubmit{"product-formatter", {}});

    // now fill the tasks hierarchy infos

    //   ----------------------------- LOOP --------------------------------------------
    //   |                                                                              |
    //   |                      ndvi-rvi-extraction                                     |
    //   |                              |                                               |
    //   |                      get-lai-retrieval-model                                 |
    //   |                              |                                               |
    //   |              ---------------------------------------------------             |
    //   |              |                      |                           |            |
    //   |      bv-image-inversion     bv-err-image-inversion   lai-mono-date-mask-flags|
    //   |              |                      |                           |            |
    //   |              ---------------------------------------------------             |
    //   |                              |                                               |
    //   -------------------------------------------------------------------------------
    //                                  |
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

    // NOTE: In this moment, the products in loop are not executed in parallel. To do this, the if(i > 0) below
    //      should be removed but in this case, the time-series-builders should wait for all the monodate images
    int i;
    for(i = 0; i<nbProducts; i++) {
        if(i > 0) {
            // update the ndvi-rvi-extractor with the reference of the previous bv-err-image-inversion
            int nNdviRviExtractorIdx = i*TasksNoPerProduct;
            int nPrevBvErrImgInvIdx = (i-1)*TasksNoPerProduct + (TasksNoPerProduct-1);
            outAllTasksList[nNdviRviExtractorIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx-1]);
            outAllTasksList[nNdviRviExtractorIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx]);
        }
        // the others comme naturally updated
        // get-lai-retrieval-model -> ndvi-rvi-extraction
        outAllTasksList[i*TasksNoPerProduct+1].parentTasks.append(outAllTasksList[i*TasksNoPerProduct]);
        // bv-image-inversion -> get-lai-retrieval-model
        outAllTasksList[i*TasksNoPerProduct+2].parentTasks.append(outAllTasksList[i*TasksNoPerProduct+1]);
        // bv-err-image-inversion -> get-lai-retrieval-model
        outAllTasksList[i*TasksNoPerProduct+3].parentTasks.append(outAllTasksList[i*TasksNoPerProduct+1]);
        // lai-mono-date-mask-flags -> get-lai-retrieval-model
        outAllTasksList[i*TasksNoPerProduct+4].parentTasks.append(outAllTasksList[i*TasksNoPerProduct+1]);
    }

    // time-series-builder -> last bv-image-inversion AND bv-err-image-inversion
    m_nTimeSeriesBuilderIdx = i*TasksNoPerProduct;
    int nPrevBvErrImgInvIdx = (i-1)*TasksNoPerProduct + (TasksNoPerProduct-1);
    outAllTasksList[m_nTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx-1]);
    outAllTasksList[m_nTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx]);

    //err-time-series-builder -> last bv-image-inversion AND bv-err-image-inversion
    m_nErrTimeSeriesBuilderIdx = m_nTimeSeriesBuilderIdx+1;
    outAllTasksList[m_nErrTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx-1]);
    outAllTasksList[m_nErrTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx]);

    //lai-msk-flags-time-series-builder -> last bv-image-inversion AND bv-err-image-inversion
    m_nLaiMskFlgsTimeSeriesBuilderIdx = m_nErrTimeSeriesBuilderIdx+1;
    outAllTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx-1]);
    outAllTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx]);

    //profile-reprocessing -> time-series-builder AND err-time-series-builder AND lai-msk-flags-time-series-builder
    m_nProfileReprocessingIdx = m_nLaiMskFlgsTimeSeriesBuilderIdx+1;
    outAllTasksList[m_nProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nTimeSeriesBuilderIdx]);
    outAllTasksList[m_nProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nErrTimeSeriesBuilderIdx]);
    outAllTasksList[m_nProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx]);

    //reprocessed-profile-splitter -> profile-reprocessing
    m_nReprocessedProfileSplitterIdx = m_nProfileReprocessingIdx+1;
    outAllTasksList[m_nReprocessedProfileSplitterIdx].parentTasks.append(outAllTasksList[m_nProfileReprocessingIdx]);

    //fitted-profile-reprocessing -> time-series-builder AND err-time-series-builder AND lai-msk-flags-time-series-builder
    m_nFittedProfileReprocessingIdx = m_nReprocessedProfileSplitterIdx+1;
    outAllTasksList[m_nFittedProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nTimeSeriesBuilderIdx]);
    outAllTasksList[m_nFittedProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nErrTimeSeriesBuilderIdx]);
    outAllTasksList[m_nFittedProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx]);

    //fitted-reprocessed-profile-splitter -> fitted-profile-reprocessing
    m_nFittedProfileReprocessingSplitterIdx = m_nFittedProfileReprocessingIdx+1;
    outAllTasksList[m_nFittedProfileReprocessingSplitterIdx].parentTasks.append(outAllTasksList[m_nFittedProfileReprocessingIdx]);

    //product-formatter -> reprocessed-profile-splitter AND fitted-reprocessed-profile-splitter
    m_nProductFormatterIdx = m_nFittedProfileReprocessingSplitterIdx+1;
    outAllTasksList[m_nProductFormatterIdx].parentTasks.append(outAllTasksList[m_nReprocessedProfileSplitterIdx]);
    outAllTasksList[m_nProductFormatterIdx].parentTasks.append(outAllTasksList[m_nFittedProfileReprocessingSplitterIdx]);
}

void LaiRetrievalHandler::HandleNewProductInJob(EventProcessingContext &ctx, int jobId,
                                                const QString &jsonParams, const QStringList &listProducts) {

    const QJsonObject &parameters = QJsonDocument::fromJson(jsonParams.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(jobId, "processor.l3b.lai.");

    // Get the resolution value
    const auto &resolution = QString::number(parameters["resolution"].toInt());
    bool bGenerateMonodate = false;
    bool bGenerateReproc = false;
    bool bGenerateFitted = false;

    if(parameters.contains("monodate"))
        bGenerateMonodate = true;
    if(parameters.contains("reproc"))
        bGenerateReproc = true;
    if(parameters.contains("fitted"))
        bGenerateFitted = true;
    // if no flag was set, then set them all
    if(!bGenerateMonodate && !bGenerateReproc && !bGenerateFitted) {
        bGenerateMonodate = true;
        bGenerateReproc = true;
        bGenerateFitted = true;
    }

    QList<TaskToSubmit> allTasksList;
    CreateNewProductInJobTasks(allTasksList, listProducts.size());

    NewStepList steps;
    QStringList ndviFileNames;
    QStringList monoDateLaiFileNames;
    QStringList monoDateErrLaiFileNames;
    QStringList monoDateMskFlagsLaiFileNames;

    // first extract the model file names from the models folder
    QStringList modelsList;
    QStringList errModelsList;
    GetModelFileList(modelsList, "Model_*.txt", configParameters);
    GetModelFileList(errModelsList, "Err_Est_Model_*.txt", configParameters);

    int i;
    for (i = 0; i<listProducts.size(); i++) {
        const auto &inputProduct = listProducts[i];

        TaskToSubmit &ndviRviExtractorTask = allTasksList[i*TasksNoPerProduct];
        TaskToSubmit &getLaiRetrievalModelTask = allTasksList[i*TasksNoPerProduct+1];
        TaskToSubmit &bvImageInversionTask = allTasksList[i*TasksNoPerProduct+2];
        TaskToSubmit &bvErrImageInversionTask = allTasksList[i*TasksNoPerProduct+3];
        TaskToSubmit &genMonoDateMskFagsTask = allTasksList[i*TasksNoPerProduct+4];

        ctx.SubmitTasks(jobId, { ndviRviExtractorTask,
                                 getLaiRetrievalModelTask,
                                 bvImageInversionTask,
                                 bvErrImageInversionTask,
                                 genMonoDateMskFagsTask
        });

        const auto & ftsFile = ndviRviExtractorTask.GetFilePath("ndvi_rvi.tif");
        const auto & modelFileName = getLaiRetrievalModelTask.GetFilePath("model_file.txt");
        const auto & errModelFileName = getLaiRetrievalModelTask.GetFilePath("err_model_file.txt");
        const auto & monoDateLaiFileName = bvImageInversionTask.GetFilePath("LAI_mono_date_img.tif");
        const auto & monoDateErrFileName = bvErrImageInversionTask.GetFilePath("LAI_mono_date_ERR_img.tif");
        const auto & monoDateMskFlgsFileName = genMonoDateMskFagsTask.GetFilePath("LAI_mono_date_msk_flgs_img.tif");

        // save the mono date LAI file name list
        ndviFileNames.append(ftsFile);
        monoDateLaiFileNames.append(monoDateLaiFileName);
        monoDateErrLaiFileNames.append(monoDateErrFileName);
        monoDateMskFlagsLaiFileNames.append(monoDateMskFlgsFileName);

        QStringList ndviRviExtractionArgs = GetNdviRviExtractionArgs(inputProduct, ftsFile, resolution);
        QStringList getLaiModelArgs = GetLaiModelExtractorArgs(inputProduct, modelsList, errModelsList, modelFileName, errModelFileName);
        QStringList bvImageInvArgs = GetBvImageInvArgs(ftsFile, modelFileName, monoDateLaiFileName);
        QStringList bvErrImageInvArgs = GetBvErrImageInvArgs(ftsFile, errModelFileName, monoDateErrFileName);
        QStringList genMonoDateMskFagsArgs = GetMonoDateMskFagsArgs(inputProduct, monoDateMskFlgsFileName);

        // add these steps to the steps list to be submitted
        steps.append(ndviRviExtractorTask.CreateStep("NdviRviExtraction2", ndviRviExtractionArgs));
        steps.append(getLaiRetrievalModelTask.CreateStep("GetLaiRetrievalModel", getLaiModelArgs));
        steps.append(bvImageInversionTask.CreateStep("BVImageInversion", bvImageInvArgs));
        steps.append(bvErrImageInversionTask.CreateStep("BVImageInversion", bvErrImageInvArgs));
        steps.append(genMonoDateMskFagsTask.CreateStep("GenerateLaiMonoDateMaskFlags", genMonoDateMskFagsArgs));
    }
    TaskToSubmit &imgTimeSeriesBuilderTask = allTasksList[m_nTimeSeriesBuilderIdx];
    TaskToSubmit &errTimeSeriesBuilderTask = allTasksList[m_nErrTimeSeriesBuilderIdx];
    TaskToSubmit &mskFlagsTimeSeriesBuilderTask = allTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx];
    TaskToSubmit &profileReprocTask = allTasksList[m_nProfileReprocessingIdx];
    TaskToSubmit &profileReprocSplitTask = allTasksList[m_nReprocessedProfileSplitterIdx];
    TaskToSubmit &fittedProfileReprocTask = allTasksList[m_nFittedProfileReprocessingIdx];
    TaskToSubmit &fittedProfileReprocSplitTask = allTasksList[m_nFittedProfileReprocessingSplitterIdx];
    TaskToSubmit &productFormatterTask = allTasksList[m_nProductFormatterIdx];

    ctx.SubmitTasks(jobId, { imgTimeSeriesBuilderTask,
                             errTimeSeriesBuilderTask,
                             mskFlagsTimeSeriesBuilderTask,
                             profileReprocTask,
                             profileReprocSplitTask,
                             fittedProfileReprocTask,
                             fittedProfileReprocSplitTask,
                             productFormatterTask
    });

    const auto & allLaiTimeSeriesFileName = imgTimeSeriesBuilderTask.GetFilePath("LAI_time_series.tif");
    const auto & allErrTimeSeriesFileName = errTimeSeriesBuilderTask.GetFilePath("Err_time_series.tif");
    const auto & allMskFlagsTimeSeriesFileName = mskFlagsTimeSeriesBuilderTask.GetFilePath("Mask_Flags_time_series.tif");

    const auto & reprocTimeSeriesFileName = profileReprocTask.GetFilePath("ReprocessedTimeSeries.tif");
    const auto & fittedTimeSeriesFileName = fittedProfileReprocTask.GetFilePath("FittedTimeSeries.tif");

    const auto & fittedFileListFileName = fittedProfileReprocSplitTask.GetFilePath("FittedFilesList.txt");
    const auto & fittedFlagsFileListFileName = fittedProfileReprocSplitTask.GetFilePath("FittedFlagsFilesList.txt");

    const auto & reprocFileListFileName = profileReprocSplitTask.GetFilePath("ReprocessedFilesList.txt");
    const auto & reprocFlagsFileListFileName = profileReprocSplitTask.GetFilePath("ReprocessedFlagsFilesList.txt");

    QStringList timeSeriesBuilderArgs = GetTimeSeriesBuilderArgs(monoDateLaiFileNames, allLaiTimeSeriesFileName);
    QStringList errTimeSeriesBuilderArgs = GetErrTimeSeriesBuilderArgs(monoDateErrLaiFileNames, allErrTimeSeriesFileName);
    QStringList mskFlagsTimeSeriesBuilderArgs = GetMskFlagsTimeSeriesBuilderArgs(monoDateMskFlagsLaiFileNames, allMskFlagsTimeSeriesFileName);

    QStringList profileReprocessingArgs = GetProfileReprocessingArgs(configParameters, allLaiTimeSeriesFileName,
                                                                     allErrTimeSeriesFileName, allMskFlagsTimeSeriesFileName,
                                                                     reprocTimeSeriesFileName, listProducts);
    QStringList reprocProfileSplitterArgs = GetReprocProfileSplitterArgs(reprocTimeSeriesFileName, reprocFileListFileName,
                                                                         reprocFlagsFileListFileName, listProducts);
    QStringList fittedProfileReprocArgs = GetFittedProfileReprocArgs(allLaiTimeSeriesFileName, allErrTimeSeriesFileName,
                                                                     allMskFlagsTimeSeriesFileName, fittedTimeSeriesFileName, listProducts);
    QStringList fittedProfileReprocSplitterArgs = GetFittedProfileReprocSplitterArgs(fittedTimeSeriesFileName, fittedFileListFileName,
                                                                                     fittedFlagsFileListFileName, listProducts);

    // Get the tile ID from the product XML name. We extract it from the first product in the list as all
    // producs should be for the same tile
    QString tileId = ProcessorHandlerHelper::GetTileId(listProducts);
    QStringList productFormatterArgs = GetProductFormatterArgs(productFormatterTask, configParameters,
                                                               listProducts, ndviFileNames, monoDateLaiFileNames, monoDateErrLaiFileNames,
                                                               monoDateMskFlagsLaiFileNames, fittedFileListFileName, fittedFlagsFileListFileName,
                                                               reprocFileListFileName, reprocFlagsFileListFileName, tileId);

    // add these steps to the steps list to be submitted
    steps.append(imgTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", timeSeriesBuilderArgs));
    steps.append(errTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", errTimeSeriesBuilderArgs));
    steps.append(mskFlagsTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", mskFlagsTimeSeriesBuilderArgs));
    steps.append(profileReprocTask.CreateStep("ProfileReprocessing", profileReprocessingArgs));
    steps.append(profileReprocSplitTask.CreateStep("ReprocessedProfileSplitter", reprocProfileSplitterArgs));
    steps.append(fittedProfileReprocTask.CreateStep("ProfileReprocessing", fittedProfileReprocArgs));
    steps.append(fittedProfileReprocSplitTask.CreateStep("ReprocessedProfileSplitter2", fittedProfileReprocSplitterArgs));
    steps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));

    ctx.SubmitSteps(steps);
}

void LaiRetrievalHandler::WriteExecutionInfosFile(const QString &executionInfosPath,
                                               std::map<QString, QString> &configParameters,
                                               const QStringList &listProducts) {
    std::ofstream executionInfosFile;
    try
    {
        // Get the parameters from the configuration
        const auto &bwr = configParameters["processor.l3b.lai.localwnd.bwr"];
        const auto &fwr = configParameters["processor.l3b.lai.localwnd.fwr"];

        executionInfosFile.open(executionInfosPath.toStdString().c_str(), std::ofstream::out);
        executionInfosFile << "<?xml version=\"1.0\" ?>" << std::endl;
        executionInfosFile << "<metadata>" << std::endl;
        executionInfosFile << "  <General>" << std::endl;
        executionInfosFile << "  </General>" << std::endl;

        executionInfosFile << "  <ProfileReprocessing_parameters>" << std::endl;
        executionInfosFile << "    <bwr_for_algo_local_online_retrieval>" << bwr.toStdString() << "</bwr_for_algo_local_online_retrieval>" << std::endl;
        executionInfosFile << "    <fwr_for_algo_local_online_retrieval>"<< fwr.toStdString() <<"</fwr_for_algo_local_online_retrieval>" << std::endl;
        executionInfosFile << "  </ProfileReprocessing_parameters>" << std::endl;

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

void LaiRetrievalHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    const auto &inputProducts = parameters["input_products"].toArray();

    QStringList listProducts;
    for (const auto &inputProduct : inputProducts) {
        listProducts.append(ctx.findProductFile(inputProduct.toString()));
    }
    // process the received L2A products in the current job
    HandleNewProductInJob(ctx, event.jobId, event.parametersJson, listProducts);
}

void LaiRetrievalHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    if (event.module == "product-formatter") {
        ctx.MarkJobFinished(event.jobId);
    }

    //    ctx.InsertProduct({ ProductType::TestProduct,
    //                        event.processorId,
    //                        event.taskId,
    //                        ctx.GetOutputPath(event.jobId, event.taskId),
    //                        QDateTime::currentDateTimeUtc() });
}

void LaiRetrievalHandler::GetModelFileList(QStringList &outListModels, const QString &strPattern, std::map<QString, QString> &configParameters)
{
    // Get the models folder name
    const auto &modelsFolder = configParameters["processor.l3b.lai.modelsfolder"];
    QStringList nameFilters = {strPattern};
    QDirIterator it(modelsFolder, nameFilters, QDir::Files, QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        it.next();
        if (QFileInfo(it.filePath()).isFile()) {
            outListModels.append(it.filePath());
        }
    }
}


QStringList LaiRetrievalHandler::GetNdviRviExtractionArgs(const QString &inputProduct, const QString &ftsFile, const QString &resolution) {
    return { "NdviRviExtraction2",
           "-xml", inputProduct,
           "-fts", ftsFile,
           "-outres", resolution
    };
}

QStringList LaiRetrievalHandler::GetLaiModelExtractorArgs(const QString &inputProduct, const QStringList &modelsList, const QStringList &errModelsList,
                                     const QString &modelFileName, const QString &errModelFileName) {
    QStringList laiModelExtractorArgs = { "GetLaiRetrievalModel",
                                "-xml", inputProduct,
                                "-outm", modelFileName,
                                "-outerr", errModelFileName,
                                "-ilmodels"};
    // append the list of models
    laiModelExtractorArgs += modelsList;
    laiModelExtractorArgs.append("-ilerrmodels");
    laiModelExtractorArgs += errModelsList;
    return laiModelExtractorArgs;
}

QStringList LaiRetrievalHandler::GetBvImageInvArgs(const QString &ftsFile, const QString &modelFileName, const QString &monoDateLaiFileName) {
    return { "BVImageInversion",
        "-in", ftsFile,
        "-modelfile", modelFileName,
        "-out", monoDateLaiFileName
    };
}

QStringList LaiRetrievalHandler::GetBvErrImageInvArgs(const QString &ftsFile, const QString &errModelFileName, const QString &monoDateErrFileName)  {
    return { "BVImageInversion",
      "-in", ftsFile,
      "-modelfile", errModelFileName,
      "-out", monoDateErrFileName
    };
}

QStringList LaiRetrievalHandler::GetMonoDateMskFagsArgs(const QString &inputProduct, const QString &monoDateMskFlgsFileName) {
    return { "GenerateLaiMonoDateMaskFlags",
      "-inxml", inputProduct,
      "-out", monoDateMskFlgsFileName
    };
}


QStringList LaiRetrievalHandler::GetTimeSeriesBuilderArgs(const QStringList &monoDateLaiFileNames, const QString &allLaiTimeSeriesFileName) {
    QStringList timeSeriesBuilderArgs = { "TimeSeriesBuilder",
      "-out", allLaiTimeSeriesFileName,
      "-il"
    };
    timeSeriesBuilderArgs += monoDateLaiFileNames;

    return timeSeriesBuilderArgs;
}

QStringList LaiRetrievalHandler::GetErrTimeSeriesBuilderArgs(const QStringList &monoDateErrLaiFileNames, const QString &allErrTimeSeriesFileName) {
    QStringList timeSeriesBuilderArgs = { "TimeSeriesBuilder",
      "-out", allErrTimeSeriesFileName,
      "-il"
    };
    timeSeriesBuilderArgs += monoDateErrLaiFileNames;

    return timeSeriesBuilderArgs;
}

QStringList LaiRetrievalHandler::GetMskFlagsTimeSeriesBuilderArgs(const QStringList &monoDateMskFlagsLaiFileNames, const QString &allMskFlagsTimeSeriesFileName) {
    QStringList timeSeriesBuilderArgs = { "TimeSeriesBuilder",
      "-out", allMskFlagsTimeSeriesFileName,
      "-il"
    };
    timeSeriesBuilderArgs += monoDateMskFlagsLaiFileNames;

    return timeSeriesBuilderArgs;
}

QStringList LaiRetrievalHandler::GetProfileReprocessingArgs(std::map<QString, QString> configParameters, const QString &allLaiTimeSeriesFileName,
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

QStringList LaiRetrievalHandler::GetReprocProfileSplitterArgs(const QString &reprocTimeSeriesFileName, const QString &reprocFileListFileName,
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

QStringList LaiRetrievalHandler::GetFittedProfileReprocArgs(const QString &allLaiTimeSeriesFileName, const QString &allErrTimeSeriesFileName,
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

QStringList LaiRetrievalHandler::GetFittedProfileReprocSplitterArgs(const QString &fittedTimeSeriesFileName, const QString &fittedFileListFileName,
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

QStringList LaiRetrievalHandler::GetProductFormatterArgs(TaskToSubmit &productFormatterTask, std::map<QString, QString> configParameters,
                                    const QStringList &listProducts, const QStringList &listNdvis,
                                    const QStringList &listLaiMonoDate, const QStringList &listLaiMonoDateErr,
                                    const QStringList &listLaiMonoDateFlgs, const QString &fileLaiReproc,
                                    const QString &fileLaiReprocFlgs, const QString &fileLaiFit,
                                    const QString &fileLaiFitFlgs, const QString &tileId) {

    const auto &targetFolder = productFormatterTask.GetFilePath("");
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.xml");

    WriteExecutionInfosFile(executionInfosPath, configParameters, listProducts);

    QStringList productFormatterArgs = { "ProductFormatter",
                            "-destroot", targetFolder,
                            "-fileclass", "SVT1",
                            "-level", "L3B",
                            "-baseline", "01.00",
                            "-processor", "vegetation",
                            "-processor.vegetation.laindvi"};
    productFormatterArgs += tileId;
    productFormatterArgs += listNdvis;
    productFormatterArgs += "-processor.vegetation.laimonodate";
    productFormatterArgs += tileId;
    productFormatterArgs += listLaiMonoDate;
    productFormatterArgs += "-processor.vegetation.laimonodateerr";
    productFormatterArgs += tileId;
    productFormatterArgs += listLaiMonoDateErr;
    productFormatterArgs += "-processor.vegetation.laimdateflgs";
    productFormatterArgs += tileId;
    productFormatterArgs += listLaiMonoDateFlgs;
    productFormatterArgs += "-processor.vegetation.filelaireproc";
    productFormatterArgs += tileId;
    productFormatterArgs += fileLaiReproc;
    productFormatterArgs += "-processor.vegetation.filelaireprocflgs";
    productFormatterArgs += tileId;
    productFormatterArgs += fileLaiReprocFlgs;
    productFormatterArgs += "-processor.vegetation.filelaifit";
    productFormatterArgs += tileId;
    productFormatterArgs += fileLaiFit;
    productFormatterArgs += "-processor.vegetation.filelaifitflgs";
    productFormatterArgs += tileId;
    productFormatterArgs += fileLaiFitFlgs;
    productFormatterArgs += "-gipp";
    productFormatterArgs += executionInfosPath;
    productFormatterArgs += "-il";
    productFormatterArgs += listProducts;

    return productFormatterArgs;
}

bool IsMonoDateProcessingNeeded() {
    return false;
}

bool IsNDaysReprocessingNeeded() {
    return false;
}
