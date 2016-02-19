#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "lairetrievalhandler.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"

// The number of tasks that are executed for each product before executing time series tasks
#define LAI_TASKS_PER_PRODUCT       4
#define MODEL_GEN_TASKS_PER_PRODUCT 4

#define DEFAULT_GENERATED_SAMPLES_NO    "40000"
#define DEFAULT_NOISE_VAR               "0.01"
#define DEFAULT_BEST_OF                 "1"
#define DEFAULT_REGRESSOR               "nn"

void LaiRetrievalHandler::CreateNewProductInJobTasks(QList<TaskToSubmit> &outAllTasksList, int nbProducts, bool bGenModels, bool bNDayReproc, bool bFittedReproc) {
    // just create the tasks but with no information so far
    // first we add the tasks to be performed for each product
    int TasksNoPerProduct = LAI_TASKS_PER_PRODUCT;
    if(bGenModels)
        TasksNoPerProduct += MODEL_GEN_TASKS_PER_PRODUCT;
    for(int i = 0; i<nbProducts; i++) {
        if(bGenModels) {
            outAllTasksList.append(TaskToSubmit{ "lai-bv-input-variable-generation", {} });
            outAllTasksList.append(TaskToSubmit{ "lai-prosail-simulator", {} });
            outAllTasksList.append(TaskToSubmit{ "lai-training-data-generator", {} });
            outAllTasksList.append(TaskToSubmit{ "lai-inverse-model-learning", {} });
        }
        outAllTasksList.append(TaskToSubmit{"lai-ndvi-rvi-extractor", {}});
        outAllTasksList.append(TaskToSubmit{"lai-bv-image-invertion", {}});
        outAllTasksList.append(TaskToSubmit{"lai-bv-err-image-invertion", {}});
        outAllTasksList.append(TaskToSubmit{"lai-mono-date-mask-flags", {}});
    }
    if(bNDayReproc || bFittedReproc) {
        outAllTasksList.append(TaskToSubmit{"lai-time-series-builder", {}});
        outAllTasksList.append(TaskToSubmit{"lai-err-time-series-builder", {}});
        outAllTasksList.append(TaskToSubmit{"lai-msk-flags-time-series-builder", {}});
        if(bNDayReproc) {
            outAllTasksList.append(TaskToSubmit{"lai-local-window-reprocessing", {}});
            outAllTasksList.append(TaskToSubmit{"lai-local-window-reproc-splitter", {}});
        }
        if(bFittedReproc) {
            outAllTasksList.append(TaskToSubmit{"lai-fitted-reprocessing", {}});
            outAllTasksList.append(TaskToSubmit{"lai-fitted-reproc-splitter", {}});
        }
    }
    outAllTasksList.append(TaskToSubmit{"product-formatter", {}});

    // now fill the tasks hierarchy infos

    //   ----------------------------- LOOP --------------------------------------------
    //   |                                                                              |
    //   |                      bv-input-variable-generation   (optional)               |
    //   |                              |                                               |
    //   |                      prosail-simulator              (optional)               |
    //   |                              |                                               |
    //   |                      training-data-generator        (optional)               |
    //   |                              |                                               |
    //   |                      inverse-model-learning         (optional)               |
    //   |                              |                                               |
    //   |                      ndvi-rvi-extraction                                     |
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
        int loopFirstIdx = i*TasksNoPerProduct;
        // initialize the ndviRvi task index
        int ndviRviExtrIdx = loopFirstIdx;
        // add the tasks for generating models
        if(bGenModels) {
            int prosailSimulatorIdx = loopFirstIdx+1;
            outAllTasksList[prosailSimulatorIdx].parentTasks.append(outAllTasksList[loopFirstIdx]);
            outAllTasksList[prosailSimulatorIdx+1].parentTasks.append(outAllTasksList[prosailSimulatorIdx]);
            outAllTasksList[prosailSimulatorIdx+2].parentTasks.append(outAllTasksList[prosailSimulatorIdx+1]);
            // now update the index for the ndviRvi task and set its parent to the inverse-model-learning task
            ndviRviExtrIdx += MODEL_GEN_TASKS_PER_PRODUCT;
            outAllTasksList[ndviRviExtrIdx].parentTasks.append(outAllTasksList[prosailSimulatorIdx+2]);
        }
        // the others comme naturally updated
        // bv-image-inversion -> ndvi-rvi-extraction
        outAllTasksList[ndviRviExtrIdx+1].parentTasks.append(outAllTasksList[ndviRviExtrIdx]);
        // bv-err-image-inversion -> ndvi-rvi-extraction
        outAllTasksList[ndviRviExtrIdx+2].parentTasks.append(outAllTasksList[ndviRviExtrIdx]);
        // lai-mono-date-mask-flags -> ndvi-rvi-extraction
        outAllTasksList[ndviRviExtrIdx+3].parentTasks.append(outAllTasksList[ndviRviExtrIdx]);
    }
    int nCurIdx = i*TasksNoPerProduct;
    if(bNDayReproc || bFittedReproc) {
        // time-series-builder -> last bv-image-inversion AND bv-err-image-inversion
        m_nTimeSeriesBuilderIdx = nCurIdx++;
        int nPrevBvErrImgInvIdx = (i-1)*TasksNoPerProduct + (TasksNoPerProduct-1);
        outAllTasksList[m_nTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx-1]);
        outAllTasksList[m_nTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx]);

        //err-time-series-builder -> last bv-image-inversion AND bv-err-image-inversion
        m_nErrTimeSeriesBuilderIdx = nCurIdx++;
        outAllTasksList[m_nErrTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx-1]);
        outAllTasksList[m_nErrTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx]);

        //lai-msk-flags-time-series-builder -> last bv-image-inversion AND bv-err-image-inversion
        m_nLaiMskFlgsTimeSeriesBuilderIdx = nCurIdx++;
        outAllTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx-1]);
        outAllTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx]);

        if(bNDayReproc) {
            //profile-reprocessing -> time-series-builder AND err-time-series-builder AND lai-msk-flags-time-series-builder
            m_nProfileReprocessingIdx = nCurIdx++;
            outAllTasksList[m_nProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nTimeSeriesBuilderIdx]);
            outAllTasksList[m_nProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nErrTimeSeriesBuilderIdx]);
            outAllTasksList[m_nProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx]);

            //reprocessed-profile-splitter -> profile-reprocessing
            m_nReprocessedProfileSplitterIdx = nCurIdx++;
            outAllTasksList[m_nReprocessedProfileSplitterIdx].parentTasks.append(outAllTasksList[m_nProfileReprocessingIdx]);
        }

        if(bFittedReproc) {
            //fitted-profile-reprocessing -> time-series-builder AND err-time-series-builder AND lai-msk-flags-time-series-builder
            m_nFittedProfileReprocessingIdx = nCurIdx++;
            outAllTasksList[m_nFittedProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nTimeSeriesBuilderIdx]);
            outAllTasksList[m_nFittedProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nErrTimeSeriesBuilderIdx]);
            outAllTasksList[m_nFittedProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx]);

            //fitted-reprocessed-profile-splitter -> fitted-profile-reprocessing
            m_nFittedProfileReprocessingSplitterIdx = nCurIdx++;
            outAllTasksList[m_nFittedProfileReprocessingSplitterIdx].parentTasks.append(outAllTasksList[m_nFittedProfileReprocessingIdx]);
        }
        //product-formatter -> reprocessed-profile-splitter OR fitted-reprocessed-profile-splitter (OR BOTH)
        m_nProductFormatterIdx = nCurIdx;
        if(bNDayReproc)
            outAllTasksList[m_nProductFormatterIdx].parentTasks.append(outAllTasksList[m_nReprocessedProfileSplitterIdx]);
        if(bFittedReproc)
            outAllTasksList[m_nProductFormatterIdx].parentTasks.append(outAllTasksList[m_nFittedProfileReprocessingSplitterIdx]);
    } else {
        //product-formatter -> last bv-image-inversion AND bv-err-image-inversion
        m_nProductFormatterIdx = nCurIdx;
        int nPrevBvErrImgInvIdx = (i-1)*TasksNoPerProduct + (TasksNoPerProduct-1);
        outAllTasksList[m_nProductFormatterIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx-1]);
        outAllTasksList[m_nProductFormatterIdx].parentTasks.append(outAllTasksList[nPrevBvErrImgInvIdx]);

    }
}

void LaiRetrievalHandler::HandleNewProductInJob(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                const QStringList &listProducts) {

    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.lai.");

    // Get the resolution value
    const auto &resolution = QString::number(parameters["resolution"].toInt());
    bool bGenModels = IsGenerateModelNeeded(parameters);
    bool bNDayReproc = IsNDaysReprocessingNeeded(parameters);
    bool bFittedReproc = IsFittedReprocessingNeeded(parameters);

    QList<TaskToSubmit> allTasksList;
    CreateNewProductInJobTasks(allTasksList, listProducts.size(), bGenModels, bNDayReproc, bFittedReproc);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    // submit all tasks
    ctx.SubmitTasks(event.jobId, allTasksListRef);

    NewStepList steps;
    QStringList ndviFileNames;
    QStringList monoDateLaiFileNames;
    QStringList monoDateErrLaiFileNames;
    QStringList monoDateMskFlagsLaiFileNames;
    // first extract the model file names from the models folder
    int TasksNoPerProduct = LAI_TASKS_PER_PRODUCT;
    if(bGenModels) {
        GetStepsToGenModel(configParameters, listProducts, allTasksList, steps);
        TasksNoPerProduct += MODEL_GEN_TASKS_PER_PRODUCT;
    }

    const auto &modelsFolder = configParameters["processor.l3b.lai.modelsfolder"];

    int i;
    for (i = 0; i<listProducts.size(); i++) {
        const auto &inputProduct = listProducts[i];
        // initialize the ndviRvi task index
        int ndviRviExtrIdx = i*TasksNoPerProduct;
        if(bGenModels) {
            // now update the index for the ndviRvi task
            ndviRviExtrIdx += MODEL_GEN_TASKS_PER_PRODUCT;
        }
        TaskToSubmit &ndviRviExtractorTask = allTasksList[ndviRviExtrIdx];
        TaskToSubmit &bvImageInversionTask = allTasksList[ndviRviExtrIdx+1];
        TaskToSubmit &bvErrImageInversionTask = allTasksList[ndviRviExtrIdx+2];
        TaskToSubmit &genMonoDateMskFagsTask = allTasksList[ndviRviExtrIdx+3];

        const auto & ftsFile = ndviRviExtractorTask.GetFilePath("ndvi_rvi.tif");
        const auto & monoDateLaiFileName = bvImageInversionTask.GetFilePath("LAI_mono_date_img.tif");
        const auto & monoDateErrFileName = bvErrImageInversionTask.GetFilePath("LAI_mono_date_ERR_img.tif");
        const auto & monoDateMskFlgsFileName = genMonoDateMskFagsTask.GetFilePath("LAI_mono_date_msk_flgs_img.tif");

        // save the mono date LAI file name list
        ndviFileNames.append(ftsFile);
        monoDateLaiFileNames.append(monoDateLaiFileName);
        monoDateErrLaiFileNames.append(monoDateErrFileName);
        monoDateMskFlagsLaiFileNames.append(monoDateMskFlgsFileName);

        QStringList ndviRviExtractionArgs = GetNdviRviExtractionArgs(inputProduct, ftsFile, resolution);
        QStringList bvImageInvArgs = GetBvImageInvArgs(ftsFile, inputProduct, modelsFolder, monoDateLaiFileName);
        QStringList bvErrImageInvArgs = GetBvErrImageInvArgs(ftsFile, inputProduct, modelsFolder, monoDateErrFileName);
        QStringList genMonoDateMskFagsArgs = GetMonoDateMskFagsArgs(inputProduct, monoDateMskFlgsFileName);

        // add these steps to the steps list to be submitted
        steps.append(ndviRviExtractorTask.CreateStep("NdviRviExtraction2", ndviRviExtractionArgs));
        steps.append(bvImageInversionTask.CreateStep("BVImageInversion", bvImageInvArgs));
        steps.append(bvErrImageInversionTask.CreateStep("BVImageInversion", bvErrImageInvArgs));
        steps.append(genMonoDateMskFagsTask.CreateStep("GenerateLaiMonoDateMaskFlags", genMonoDateMskFagsArgs));
    }

    QString fittedFileListFileName;
    QString fittedFlagsFileListFileName;
    QString reprocFileListFileName;
    QString reprocFlagsFileListFileName;

    if(bNDayReproc || bFittedReproc) {
        TaskToSubmit &imgTimeSeriesBuilderTask = allTasksList[m_nTimeSeriesBuilderIdx];
        TaskToSubmit &errTimeSeriesBuilderTask = allTasksList[m_nErrTimeSeriesBuilderIdx];
        TaskToSubmit &mskFlagsTimeSeriesBuilderTask = allTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx];

        const auto & allLaiTimeSeriesFileName = imgTimeSeriesBuilderTask.GetFilePath("LAI_time_series.tif");
        const auto & allErrTimeSeriesFileName = errTimeSeriesBuilderTask.GetFilePath("Err_time_series.tif");
        const auto & allMskFlagsTimeSeriesFileName = mskFlagsTimeSeriesBuilderTask.GetFilePath("Mask_Flags_time_series.tif");

        QStringList timeSeriesBuilderArgs = GetTimeSeriesBuilderArgs(monoDateLaiFileNames, allLaiTimeSeriesFileName);
        QStringList errTimeSeriesBuilderArgs = GetErrTimeSeriesBuilderArgs(monoDateErrLaiFileNames, allErrTimeSeriesFileName);
        QStringList mskFlagsTimeSeriesBuilderArgs = GetMskFlagsTimeSeriesBuilderArgs(monoDateMskFlagsLaiFileNames, allMskFlagsTimeSeriesFileName);

        steps.append(imgTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", timeSeriesBuilderArgs));
        steps.append(errTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", errTimeSeriesBuilderArgs));
        steps.append(mskFlagsTimeSeriesBuilderTask.CreateStep("TimeSeriesBuilder", mskFlagsTimeSeriesBuilderArgs));

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
        }

        if(bFittedReproc) {
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
        }
    }

    // finally format the product
    TaskToSubmit &productFormatterTask = allTasksList[m_nProductFormatterIdx];

    // Get the tile ID from the product XML name. We extract it from the first product in the list as all
    // producs should be for the same tile
    QString tileId = ProcessorHandlerHelper::GetTileId(listProducts);
    QStringList productFormatterArgs = GetProductFormatterArgs(productFormatterTask, ctx, event, listProducts, ndviFileNames, monoDateLaiFileNames,
                                                               monoDateErrLaiFileNames, monoDateMskFlagsLaiFileNames, reprocFileListFileName,
                                                               reprocFlagsFileListFileName, fittedFileListFileName, fittedFlagsFileListFileName, tileId);

    // add these steps to the steps list to be submitted
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
    HandleNewProductInJob(ctx, event, listProducts);
}

void LaiRetrievalHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    if (event.module == "product-formatter") {
        ctx.MarkJobFinished(event.jobId);
        // Insert the product into the database
        ctx.InsertProduct({ ProductType::L3BLaiProductTypeId,
            event.processorId,
            event.taskId,
            ctx.GetOutputPath(event.jobId, event.taskId, "product-formatter"),
            QDateTime::currentDateTimeUtc() });

        // Now remove the job folder containing temporary files
        RemoveJobFolder(ctx, event.jobId);
    }
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

QStringList LaiRetrievalHandler::GetBvImageInvArgs(const QString &ftsFile, const QString &xmlFile, const QString &modelsFolder, const QString &monoDateLaiFileName) {
    return { "BVImageInversion",
        "-in", ftsFile,
        "-out", monoDateLaiFileName,
        "-xml", xmlFile,
        "-modelsfolder", modelsFolder,
        "-modelprefix", "Model_"
    };
}

QStringList LaiRetrievalHandler::GetBvErrImageInvArgs(const QString &ftsFile, const QString &xmlFile, const QString &modelsFolder, const QString &monoDateErrFileName)  {
    return { "BVImageInversion",
        "-in", ftsFile,
        "-out", monoDateErrFileName,
        "-xml", xmlFile,
        "-modelsfolder", modelsFolder,
        "-modelprefix", "Err_Est_Model_"
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

QStringList LaiRetrievalHandler::GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                    const QStringList &listProducts, const QStringList &listNdvis,
                                    const QStringList &listLaiMonoDate, const QStringList &listLaiMonoDateErr,
                                    const QStringList &listLaiMonoDateFlgs, const QString &fileLaiReproc,
                                    const QString &fileLaiReprocFlgs, const QString &fileLaiFit,
                                    const QString &fileLaiFitFlgs, const QString &tileId) {

    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.lai.");

    //const auto &targetFolder = productFormatterTask.GetFilePath("");
    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.parametersJson);
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
    if(IsNDaysReprocessingNeeded(parameters)) {
        productFormatterArgs += "-processor.vegetation.filelaireproc";
        productFormatterArgs += tileId;
        productFormatterArgs += fileLaiReproc;
        productFormatterArgs += "-processor.vegetation.filelaireprocflgs";
        productFormatterArgs += tileId;
        productFormatterArgs += fileLaiReprocFlgs;
    }
    if(IsFittedReprocessingNeeded(parameters)) {
        productFormatterArgs += "-processor.vegetation.filelaifit";
        productFormatterArgs += tileId;
        productFormatterArgs += fileLaiFit;
        productFormatterArgs += "-processor.vegetation.filelaifitflgs";
        productFormatterArgs += tileId;
        productFormatterArgs += fileLaiFitFlgs;
    }
    productFormatterArgs += "-gipp";
    productFormatterArgs += executionInfosPath;
    productFormatterArgs += "-il";
    productFormatterArgs += listProducts;

    return productFormatterArgs;
}

bool LaiRetrievalHandler::IsNDaysReprocessingNeeded(const QJsonObject &parameters) {
    if(parameters.contains("reproc"))
        return true;
    return false;
}

bool LaiRetrievalHandler::IsFittedReprocessingNeeded(const QJsonObject &parameters) {
    if(parameters.contains("fitted"))
        return true;
    return false;
}

bool LaiRetrievalHandler::IsGenerateModelNeeded(const QJsonObject &parameters) {
    if(parameters.contains("genmodel"))
        return true;
    return false;
}

void LaiRetrievalHandler::GetStepsToGenModel(std::map<QString, QString> &configParameters,
                                             const QStringList &listProducts,
                                             QList<TaskToSubmit> &allTasksList,
                                             NewStepList &steps)
{
    const auto &modelsFolder = configParameters["processor.l3b.lai.modelsfolder"];
    const auto &rsrCfgFile = configParameters["processor.l3b.lai.rsrcfgfile"];
    int i = 0;
    int TasksNoPerProduct = LAI_TASKS_PER_PRODUCT + MODEL_GEN_TASKS_PER_PRODUCT;
    for(const QString& curXml : listProducts) {
        int loopFirstIdx = i*TasksNoPerProduct;
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
        i++;
    }
}

QStringList LaiRetrievalHandler::GetBVInputVariableGenerationArgs(std::map<QString, QString> &configParameters, const QString &strGenSampleFile) {
    QString samplesNo = GetDefaultCfgVal(configParameters, "processor.l3b.lai.models.samples", DEFAULT_GENERATED_SAMPLES_NO);
    return { "BVInputVariableGeneration",
                "-samples", samplesNo,
                "-out", strGenSampleFile
    };
}

QStringList LaiRetrievalHandler::GetProSailSimulatorArgs(const QString &product, const QString &bvFileName, const QString &rsrCfgFileName,
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

QStringList LaiRetrievalHandler::GetTrainingDataGeneratorArgs(const QString &product, const QString &biovarsFile,
                                                              const QString &simuReflsFile, const QString &outTrainingFile) {
    return { "TrainingDataGenerator",
                "-xml", product,
                "-biovarsfile", biovarsFile,
                "-simureflsfile", simuReflsFile,
                "-outtrainfile", outTrainingFile,
                "-addrefls", "1"
    };
}

QStringList LaiRetrievalHandler::GetInverseModelLearningArgs(const QString &trainingFile, const QString &product, const QString &modelFile,
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

const QString& LaiRetrievalHandler::GetDefaultCfgVal(std::map<QString, QString> &configParameters, const QString &key, const QString &defVal) {
    auto search = configParameters.find(key);
    if(search != configParameters.end()) {
        return search->second;
    }
    return defVal;
}


QString LaiRetrievalHandler::GetProcessingDefinitionJsonImpl(const QJsonObject &procInfoParams,
                                                      const ProductList &listProducts,
                                                      bool &bIsValid)
{
    bIsValid = false;
    if(!procInfoParams.contains("resolution")) {
        return "Cannot execute LAIRetrieval processor. The parameters should contain the resolution!";
    }

    // we need no parameters from the processor info parameters
    ProductList usedProductList;
    for(const Product &product: listProducts) {
        if(product.productTypeId == ProductType::L2AProductTypeId) {
            usedProductList.append(product);
        }
    }
    if(usedProductList.size() == 0) {
        return QString("Cannot execute LaiRetrieval processor. No L2A product was found!");
    }

    // for this we can check the RSR file presence or other things
    //bool bGenModels = IsGenerateModelNeeded(procInfoParams);
    bool bNDayReproc = IsNDaysReprocessingNeeded(procInfoParams);
    bool bFittedReproc = IsFittedReprocessingNeeded(procInfoParams);
    if(bNDayReproc) {
        if(usedProductList.size() < 3) {
            return QString("Cannot execute LaiRetrieval processor. Reprocessing requested but have only %1 products available!")
                           .arg(usedProductList.size());
        }
    }

    if(bFittedReproc) {
        if(usedProductList.size() < 4) {
            return QString("Cannot execute LaiRetrieval processor. Fitted reprocessing requested but have only %1 products available!").
                           arg(usedProductList.size());
        }
    }

    QJsonObject mainObj(procInfoParams);
    QJsonArray inputProductsArr;
    for (const auto &p : usedProductList) {
        inputProductsArr.append(p.fullPath);
    }
    mainObj[QStringLiteral("input_products")] = inputProductsArr;
    bIsValid = true;

    return jsonToString(mainObj);
}
