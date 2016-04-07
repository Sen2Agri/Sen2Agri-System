#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "lairetrievalhandler.hpp"
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

void LaiRetrievalHandler::CreateTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                                    LAIProductFormatterParams &outProdFormatterParams,
                                                     int nbLaiMonoProducts, bool bGenModels, bool bMonoDateLai, bool bNDayReproc, bool bFittedReproc) {
    // just create the tasks but with no information so far
    // first we add the tasks to be performed for each product
    int TasksNoPerProduct = 0;
    if(bGenModels)
        TasksNoPerProduct += MODEL_GEN_TASKS_PER_PRODUCT;
    if(bMonoDateLai)
        TasksNoPerProduct += LAI_TASKS_PER_PRODUCT;

    for(int i = 0; i<nbLaiMonoProducts; i++) {
        if(bGenModels) {
            outAllTasksList.append(TaskToSubmit{ "lai-bv-input-variable-generation", {} });
            outAllTasksList.append(TaskToSubmit{ "lai-prosail-simulator", {} });
            outAllTasksList.append(TaskToSubmit{ "lai-training-data-generator", {} });
            outAllTasksList.append(TaskToSubmit{ "lai-inverse-model-learning", {} });
        }
        if(bMonoDateLai) {
            outAllTasksList.append(TaskToSubmit{"lai-mono-date-mask-flags", {}});
            outAllTasksList.append(TaskToSubmit{"lai-ndvi-rvi-extractor", {}});
            outAllTasksList.append(TaskToSubmit{"lai-bv-image-invertion", {}});
            outAllTasksList.append(TaskToSubmit{"lai-bv-err-image-invertion", {}});
            outAllTasksList.append(TaskToSubmit{"lai-quantify-image", {}});
            outAllTasksList.append(TaskToSubmit{"lai-quantify-err-image", {}});
        }
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
    //outAllTasksList.append(TaskToSubmit{"product-formatter", {}});

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
    QList<int> laiMonoDateFlgsIdxs;
    QList<int> bvImageInvIdxs;
    QList<int> bvErrImageInvIdxs;
    QList<int> quantifyImageInvIdxs;
    QList<int> quantifyErrImageInvIdxs;

    // we execute in parallel and launch at once all processing chains for each product
    // for example, if we have genModels, we launch all bv-input-variable-generation for all products
    // if we do not have genModels, we launch all NDVIRVIExtraction in the same time for all products
    for(i = 0; i<nbLaiMonoProducts; i++) {
        int loopFirstIdx = i*TasksNoPerProduct;
        // initialize the ndviRvi task index
        int genMasksIdx = loopFirstIdx;
        // add the tasks for generating models
        if(bGenModels) {
            int prosailSimulatorIdx = loopFirstIdx+1;
            outAllTasksList[prosailSimulatorIdx].parentTasks.append(outAllTasksList[loopFirstIdx]);
            outAllTasksList[prosailSimulatorIdx+1].parentTasks.append(outAllTasksList[prosailSimulatorIdx]);
            outAllTasksList[prosailSimulatorIdx+2].parentTasks.append(outAllTasksList[prosailSimulatorIdx+1]);
            // now update the index for the ndviRvi task and set its parent to the inverse-model-learning task
            genMasksIdx += MODEL_GEN_TASKS_PER_PRODUCT;
            outAllTasksList[genMasksIdx].parentTasks.append(outAllTasksList[prosailSimulatorIdx+2]);
        }
        if(bMonoDateLai) {
            laiMonoDateFlgsIdxs.append(genMasksIdx);

            // ndvi-rvi-extraction -> lai-mono-date-mask-flags
            int ndviRviExtrIdx = genMasksIdx+1;
            outAllTasksList[ndviRviExtrIdx].parentTasks.append(outAllTasksList[genMasksIdx]);

            // the others comme naturally updated
            // bv-image-inversion -> ndvi-rvi-extraction
            int nBVImageInversionIdx = ndviRviExtrIdx+1;
            outAllTasksList[nBVImageInversionIdx].parentTasks.append(outAllTasksList[ndviRviExtrIdx]);
            bvImageInvIdxs.append(nBVImageInversionIdx);

            // bv-err-image-inversion -> ndvi-rvi-extraction
            int nBVErrImageInversionIdx = nBVImageInversionIdx+1;
            outAllTasksList[nBVErrImageInversionIdx].parentTasks.append(outAllTasksList[ndviRviExtrIdx]);
            bvErrImageInvIdxs.append(nBVErrImageInversionIdx);

            // quantify-image -> bv-image-inversion
            int nQuantifyImageInversionIdx = nBVErrImageInversionIdx+1;
            outAllTasksList[nQuantifyImageInversionIdx].parentTasks.append(outAllTasksList[nBVImageInversionIdx]);
            quantifyImageInvIdxs.append(nQuantifyImageInversionIdx);

            // quantify-err-image -> bv-err-image-inversion
            int nQuantifyErrImageInversionIdx = nQuantifyImageInversionIdx+1;
            outAllTasksList[nQuantifyErrImageInversionIdx].parentTasks.append(outAllTasksList[nBVErrImageInversionIdx]);
            quantifyErrImageInvIdxs.append(nQuantifyErrImageInversionIdx);

            // add the quantify image tasks to the list of the product formatter corresponding to this product
            LAIMonoDateProductFormatterParams monoParams;
            monoParams.parentsTasksRef.append(outAllTasksList[nQuantifyImageInversionIdx]);
            monoParams.parentsTasksRef.append(outAllTasksList[nQuantifyErrImageInversionIdx]);
            outProdFormatterParams.listLaiMonoParams.append(monoParams);
        }
    }
    int nCurIdx = nbLaiMonoProducts*TasksNoPerProduct;


    if(bNDayReproc || bFittedReproc) {
        // time-series-builder -> ALL last bv-image-inversion/quantified-images
        m_nTimeSeriesBuilderIdx = nCurIdx++;
        for(int idx: quantifyImageInvIdxs) {
            outAllTasksList[m_nTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[idx]);
        }
        //err-time-series-builder -> ALL bv-err-image-inversion/quantified-err-images
        m_nErrTimeSeriesBuilderIdx = nCurIdx++;
        for(int idx: quantifyErrImageInvIdxs) {
            outAllTasksList[m_nErrTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[idx]);
        }

        //lai-msk-flags-time-series-builder -> ALL lai-mono-date-mask-flags
        m_nLaiMskFlgsTimeSeriesBuilderIdx = nCurIdx++;
        for(int idx: laiMonoDateFlgsIdxs) {
            outAllTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[idx]);
        }

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
        if(bNDayReproc) {
            outProdFormatterParams.laiReprocParams.parentsTasksRef.append(outAllTasksList[m_nReprocessedProfileSplitterIdx]);
        }
        if(bFittedReproc) {
            outProdFormatterParams.laiFitParams.parentsTasksRef.append(outAllTasksList[m_nFittedProfileReprocessingSplitterIdx]);
        }
    }
}

void LaiRetrievalHandler::HandleNewTilesList(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                             const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                             const QStringList &listL3BTiles,
                                             const TileTemporalFilesInfo &missingL3BTileTemporalFilesInfo,
                                             LAIGlobalExecutionInfos &outGlobalExecInfos) {

    QStringList listProducts = ProcessorHandlerHelper::GetTemporalTileFiles(tileTemporalFilesInfo);
    QStringList missingL3BInputs = ProcessorHandlerHelper::GetTemporalTileFiles(missingL3BTileTemporalFilesInfo);

    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");
    QStringList monoDateMskFlagsLaiFileNames;
    QStringList quantifiedLaiFileNames;
    QStringList quantifiedErrLaiFileNames;

    // Get the resolution value
    int resolution = 0;
    if(!GetParameterValueAsInt(parameters, "resolution", resolution) ||
            resolution == 0) {
        resolution = 10;    // TODO: We should configure the default resolution in DB
    }
    const auto &resolutionStr = QString::number(resolution);

    bool bGenModels = IsGenModels(parameters, configParameters);
    bool bMonoDateLai = IsGenMonoDate(parameters, configParameters);
    bool bNDayReproc = IsNDayReproc(parameters, configParameters);
    bool bFittedReproc = IsFittedReproc(parameters, configParameters);

    QStringList monoDateInputs = listProducts;
    if(!bMonoDateLai) {
        for(const QString &l3bTileFolder: listL3BTiles) {
            quantifiedLaiFileNames.append(ProcessorHandlerHelper::GetHigLevelProductTileFile(l3bTileFolder, "SLAIMONO"));
            quantifiedErrLaiFileNames.append(ProcessorHandlerHelper::GetHigLevelProductTileFile(l3bTileFolder, "MLAIERR", true));
            monoDateMskFlagsLaiFileNames.append(ProcessorHandlerHelper::GetHigLevelProductTileFile(l3bTileFolder, "MMONODFLG", true));
        }

        if(missingL3BInputs.size() > 0) {
            monoDateInputs = QStringList();
            for(const QString &inputProd: listProducts) {
                for(const QString &missingProd: missingL3BInputs) {
                    if(inputProd == missingProd) {
                        monoDateInputs.append(inputProd);
                    }
                }
            }
            // Force mono-date for the missing L3B inputs
            if(monoDateInputs.size() > 0)
                bMonoDateLai = true;
        }
        // if we have no monodate to produce and also we found no rasters for the existing L3B list, we create no task
        if(!bMonoDateLai && quantifiedLaiFileNames.size() == 0)
            return;
    }


    QList<TaskToSubmit> &allTasksList = outGlobalExecInfos.allTasksList;
    LAIProductFormatterParams &productFormatterParams = outGlobalExecInfos.prodFormatParams;

    // create the tasks
    CreateTasksForNewProducts(allTasksList, outGlobalExecInfos.prodFormatParams, monoDateInputs.size(), bGenModels, bMonoDateLai, bNDayReproc, bFittedReproc);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    // submit all tasks
    ctx.SubmitTasks(event.jobId, allTasksListRef);

    NewStepList &steps = outGlobalExecInfos.allStepsList;

    // first extract the model file names from the models folder
    int TasksNoPerProduct = bMonoDateLai ? LAI_TASKS_PER_PRODUCT : 0;
    if(bGenModels) {
        GetStepsToGenModel(configParameters, bMonoDateLai, monoDateInputs, allTasksList, steps);
        TasksNoPerProduct += MODEL_GEN_TASKS_PER_PRODUCT;
    }

    if(bMonoDateLai) {
        const auto &modelsFolder = configParameters["processor.l3b.lai.modelsfolder"];
        int i;
        for (i = 0; i<monoDateInputs.size(); i++) {
            const auto &inputProduct = monoDateInputs[i];
            // initialize the ndviRvi task index
            int genMsksIdx = i*TasksNoPerProduct;
            if(bGenModels) {
                // now update the index for the ndviRvi task
                genMsksIdx += MODEL_GEN_TASKS_PER_PRODUCT;
            }
            TaskToSubmit &genMonoDateMskFagsTask = allTasksList[genMsksIdx];
            TaskToSubmit &ndviRviExtractorTask = allTasksList[genMsksIdx+1];
            TaskToSubmit &bvImageInversionTask = allTasksList[genMsksIdx+2];
            TaskToSubmit &bvErrImageInversionTask = allTasksList[genMsksIdx+3];
            TaskToSubmit &quantifyImageTask = allTasksList[genMsksIdx+4];
            TaskToSubmit &quantifyErrImageTask = allTasksList[genMsksIdx+5];

            const auto & monoDateMskFlgsFileName = genMonoDateMskFagsTask.GetFilePath("LAI_mono_date_msk_flgs_img.tif");
            const auto & monoDateMskFlgsResFileName = genMonoDateMskFagsTask.GetFilePath("LAI_mono_date_msk_flgs_img_resampled.tif");
            const auto & singleNdviFile = ndviRviExtractorTask.GetFilePath("single_ndvi.tif");
            const auto & ftsFile = ndviRviExtractorTask.GetFilePath("ndvi_rvi.tif");
            const auto & monoDateLaiFileName = bvImageInversionTask.GetFilePath("LAI_mono_date_img.tif");
            const auto & monoDateErrFileName = bvErrImageInversionTask.GetFilePath("LAI_mono_date_ERR_img.tif");
            const auto & quantifiedLaiFileName = quantifyImageTask.GetFilePath("LAI_mono_date_img_16.tif");
            const auto & quantifiedErrFileName = quantifyErrImageTask.GetFilePath("LAI_mono_date_ERR_img_16.tif");

            // save the mono date LAI file name list
            productFormatterParams.listLaiMonoParams[i].ndvi = singleNdviFile;
            productFormatterParams.listLaiMonoParams[i].laiMonoDate = quantifiedLaiFileName;
            productFormatterParams.listLaiMonoParams[i].laiMonoDateErr = quantifiedErrFileName;
            productFormatterParams.listLaiMonoParams[i].laiMonoDateFlgs = monoDateMskFlgsResFileName;
            //ndviFileNames.append(singleNdviFile);
            //monoDateLaiFileNames.append(monoDateLaiFileName);
            //monoDateErrLaiFileNames.append(monoDateErrFileName);

            // Save the list for using in the Reprocessed task below (if needed).
            // We cannot use the lists in the productFormatterParams.listLaiMonoParams as these are not filled in the case
            // when we do not have Mono-date products creation.
            monoDateMskFlagsLaiFileNames.append(monoDateMskFlgsResFileName);
            quantifiedLaiFileNames.append(quantifiedLaiFileName);
            quantifiedErrLaiFileNames.append(quantifiedErrFileName);

            QStringList genMonoDateMskFagsArgs = GetMonoDateMskFlagsArgs(inputProduct, monoDateMskFlgsFileName, monoDateMskFlgsResFileName, resolutionStr);
            QStringList ndviRviExtractionArgs = GetNdviRviExtractionArgs(inputProduct, monoDateMskFlgsFileName, ftsFile, singleNdviFile, resolutionStr);
            QStringList bvImageInvArgs = GetBvImageInvArgs(ftsFile, inputProduct, modelsFolder, monoDateLaiFileName);
            QStringList bvErrImageInvArgs = GetBvErrImageInvArgs(ftsFile, inputProduct, modelsFolder, monoDateErrFileName);
            QStringList quantifyImageArgs = GetQuantifyImageArgs(monoDateLaiFileName, quantifiedLaiFileName);
            QStringList quantifyErrImageArgs = GetQuantifyImageArgs(monoDateErrFileName, quantifiedErrFileName);

            // add these steps to the steps list to be submitted
            steps.append(genMonoDateMskFagsTask.CreateStep("GenerateLaiMonoDateMaskFlags", genMonoDateMskFagsArgs));
            steps.append(ndviRviExtractorTask.CreateStep("NdviRviExtraction2", ndviRviExtractionArgs));
            steps.append(bvImageInversionTask.CreateStep("BVImageInversion", bvImageInvArgs));
            steps.append(bvErrImageInversionTask.CreateStep("BVImageInversion", bvErrImageInvArgs));
            steps.append(quantifyImageTask.CreateStep("QuantifyImage", quantifyImageArgs));
            steps.append(quantifyErrImageTask.CreateStep("QuantifyImage", quantifyErrImageArgs));
        }
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

        // TODO: In order to have consistent situation in the cases when we have only Reprocessing or (Monodate AND Reprocessing)
        // we should use here the list of the quantified values for the LAI monodate. Otherwise, we get different results in this situation
        QStringList timeSeriesBuilderArgs = GetTimeSeriesBuilderArgs(quantifiedLaiFileNames, allLaiTimeSeriesFileName);
        QStringList errTimeSeriesBuilderArgs = GetErrTimeSeriesBuilderArgs(quantifiedErrLaiFileNames, allErrTimeSeriesFileName);
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
            productFormatterParams.laiReprocParams.fileLaiReproc = reprocFileListFileName;
            productFormatterParams.laiReprocParams.fileLaiReprocFlgs = reprocFlagsFileListFileName;

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
            productFormatterParams.laiFitParams.fileLaiReproc = fittedFileListFileName;
            productFormatterParams.laiFitParams.fileLaiReprocFlgs = fittedFlagsFileListFileName;
        }
    }
}

void LaiRetrievalHandler::WriteExecutionInfosFile(const QString &executionInfosPath,
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

void LaiRetrievalHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");

    bool bMonoDateLai = IsGenMonoDate(parameters, configParameters);
    bool bNDayReproc = IsNDayReproc(parameters, configParameters);
    bool bFittedReproc = IsFittedReproc(parameters, configParameters);
    if(!bNDayReproc && !bFittedReproc && !bMonoDateLai) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("At least one processing needs to be defined (LAI mono-date,"
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

    // The list of input products that do not have a L3B LAI Monodate created
    QStringList listL3BProducts;
    QStringList missingL3BInputsTiles;
    QStringList missingL3BInputs;
    if(!bMonoDateLai) {
        for (const auto &tileMetaFile : listTilesMetaFiles) {
            // if it is reprocessing but we do not have mono-date, we need also the L3B products
            // if we have reprocessing and we have mono-date, the generated monodates will be internally used
            QDateTime dtStartDate = ProcessorHandlerHelper::GetL2AProductDateFromPath(tileMetaFile);
            QDateTime dtEndDate = dtStartDate.addSecs(SECONDS_IN_DAY-1);
            // get all the products from that day
            ProductList l3bProductList = ctx.GetProducts(event.siteId, (int)ProductType::L3BProductTypeId, dtStartDate, dtEndDate);
            if(l3bProductList.size() > 0) {
                // get the last of the products
                const QString &l3bProdPath = l3bProductList[l3bProductList.size()-1].fullPath;
                if(!listL3BProducts.contains(l3bProdPath)) {
                    listL3BProducts.append(l3bProdPath);
                    Logger::debug(QStringLiteral("Using existing L3B for reprocessing: %1").arg(l3bProdPath));
                }
            } else {
                missingL3BInputsTiles.append(tileMetaFile);
                QString productForTile = GetL2AProductForTileMetaFile(inputProductToTilesMap, tileMetaFile);
                if(!missingL3BInputs.contains(productForTile)) {
                    missingL3BInputs.append(productForTile);
                    Logger::debug(QStringLiteral("The L3B will be created for missing product: %1").arg(productForTile));
                }
            }
        }
    }

    QList<LAIProductFormatterParams> listParams;
    QMap<QString, LAIProductFormatterParams> mapTileToParams;
    NewStepList allSteps;
    //container for all task
    QList<TaskToSubmit> allTasksList;
    //container for all global execution infos
    QList<LAIGlobalExecutionInfos> allLaiGlobalExecInfos;
    // Create a map containing for each tile Id the list of the
    QMap<QString, TileTemporalFilesInfo> mapTiles = GroupTiles(ctx, event.jobId, listTilesMetaFiles,
                                                               ProductType::L2AProductTypeId);
    QMap<QString, TileTemporalFilesInfo> mapMissingL3BTiles = GroupTiles(ctx, event.jobId, missingL3BInputsTiles,
                                                                         ProductType::L2AProductTypeId);
    // In the case of Mono-lai we might have something in the listL3BProducts
    QMap<QString, QStringList> mapL3BTiles = ProcessorHandlerHelper::GroupHighLevelProductTiles(listL3BProducts);

    for(auto tileId : mapTiles.keys())
    {
       const TileTemporalFilesInfo &listTemporalTiles = mapTiles.value(tileId);
       const TileTemporalFilesInfo &listL3bMissingInputsTiles = mapMissingL3BTiles.value(tileId);
       QStringList listL3bTiles = mapL3BTiles.value(tileId);

       Logger::debug(QStringLiteral("Handling tile %1 from a number of %2 tiles").arg(tileId).arg(mapTiles.size()));

       allLaiGlobalExecInfos.append(LAIGlobalExecutionInfos());
       LAIGlobalExecutionInfos &infosRef = allLaiGlobalExecInfos[allLaiGlobalExecInfos.size()-1];
       infosRef.prodFormatParams.tileId = GetProductFormatterTile(tileId);
       HandleNewTilesList(ctx, event, listTemporalTiles, listL3bTiles, listL3bMissingInputsTiles, infosRef);
       if(infosRef.allTasksList.size() > 0 && infosRef.allStepsList.size() > 0) {
           listParams.append(infosRef.prodFormatParams);
           mapTileToParams[tileId] = infosRef.prodFormatParams;
           allTasksList.append(infosRef.allTasksList);
           allSteps.append(infosRef.allStepsList);
       }
    }

    // Create the product formatter tasks
    if(bMonoDateLai || missingL3BInputsTiles.size() != 0) {
        QStringList listMonoDateInputProducts = bMonoDateLai ? inputProductToTilesMap.keys() : missingL3BInputs;
        QMap<QString, TileTemporalFilesInfo> mapMonoDateInputProducts = bMonoDateLai ? mapTiles : mapMissingL3BTiles;
        // create the product formatters for each LAI monodate product
        for(int i = 0; i < listMonoDateInputProducts.size(); i++) {
            QStringList outProdTiles, outProdTilesMetaFiles;
            QList<LAIMonoDateProductFormatterParams> outProdParams;
            if(!GetMonoDateFormatterParamInfosForProduct(listMonoDateInputProducts[i], mapMonoDateInputProducts, mapTileToParams,
                                                         inputProductToTilesMap, outProdTiles, outProdParams, outProdTilesMetaFiles)) {
                ctx.MarkJobFailed(event.jobId);
                throw std::runtime_error(
                    QStringLiteral("Cannot process job - inconsistent tiles - product configuration (this is likely a bug)")
                            .toStdString());
            }
            QStringList ndviList;
            QStringList laiList;
            QStringList laiErrList;
            QStringList laiFlgsList;
            allTasksList.append({"lai-mono-date-product-formatter", {}});
            TaskToSubmit &laiMonoProductFormatterTask = allTasksList[allTasksList.size()-1];//laiMonoProductFormatterTask{"lai-mono-date-product-formatter", {}};
            for(int i = 0; i<outProdTiles.size(); i++) {
                ndviList.append(outProdParams[i].ndvi);
                laiList.append(outProdParams[i].laiMonoDate);
                laiErrList.append(outProdParams[i].laiMonoDateErr);
                laiFlgsList.append(outProdParams[i].laiMonoDateFlgs);
                laiMonoProductFormatterTask.parentTasks.append(outProdParams[i].parentsTasksRef);
            }
            ctx.SubmitTasks(event.jobId, {laiMonoProductFormatterTask});
            QStringList productFormatterArgs = GetLaiMonoProductFormatterArgs(
                        laiMonoProductFormatterTask, ctx, event, outProdTilesMetaFiles,
                        outProdTiles, ndviList, laiList, laiErrList, laiFlgsList);
            allSteps.append(laiMonoProductFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
            //allTasksList.append(laiMonoProductFormatterTask);
        }
    }

    // Create the product formatter tasks for the Reprocessed and/or Fitted Products (if needed)
    if(bNDayReproc) {
        allTasksList.append({"lai-reproc-product-formatter", {}});
        TaskToSubmit &laiReprocProductFormatterTask = allTasksList[allTasksList.size()-1];//{"lai-reproc-product-formatter", {}};
        for(LAIProductFormatterParams params: listParams) {
            laiReprocProductFormatterTask.parentTasks.append(params.laiReprocParams.parentsTasksRef);
        }
        ctx.SubmitTasks(event.jobId, {laiReprocProductFormatterTask});
        QStringList productFormatterArgs = GetReprocProductFormatterArgs(laiReprocProductFormatterTask, ctx, event, listTilesMetaFiles,
                                                                         listParams, false);
        // add these steps to the steps list to be submitted
        allSteps.append(laiReprocProductFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
        //allTasksList.append(laiReprocProductFormatterTask);
    }
    if(bFittedReproc) {
        allTasksList.append({"lai-fitted-product-formatter", {}});
        TaskToSubmit &laiFittedProductFormatterTask = allTasksList[allTasksList.size()-1];
        //TaskToSubmit laiFittedProductFormatterTask{"lai-fitted-product-formatter", {}};
        for(LAIProductFormatterParams params: listParams) {
            laiFittedProductFormatterTask.parentTasks.append(params.laiFitParams.parentsTasksRef);
        }
        ctx.SubmitTasks(event.jobId, {laiFittedProductFormatterTask});
        QStringList productFormatterArgs = GetReprocProductFormatterArgs(laiFittedProductFormatterTask, ctx, event, listTilesMetaFiles,
                                                                         listParams, true);
        // add these steps to the steps list to be submitted
        allSteps.append(laiFittedProductFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
        //allTasksList.append(laiFittedProductFormatterTask);
    }

    QList<std::reference_wrapper<const TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    TaskToSubmit endOfJobDummyTask{"lai-end-of-job", {}};
    endOfJobDummyTask.parentTasks.append(allTasksListRef);
    ctx.SubmitTasks(event.jobId, {endOfJobDummyTask});
    allSteps.append(endOfJobDummyTask.CreateStep("EndOfLAIDummy", QStringList()));

    ctx.SubmitSteps(allSteps);
}

void LaiRetrievalHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    if (event.module == "lai-end-of-job") {
        ctx.MarkJobFinished(event.jobId);
    }
    bool isMonoDatePf, isReprocPf = false, isFittedPf = false;
    if ((isMonoDatePf = (event.module == "lai-mono-date-product-formatter")) ||
         (isReprocPf = (event.module == "lai-reproc-product-formatter")) ||
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
                                quicklook, footPrint, std::experimental::nullopt, TileList() });
            Logger::debug(QStringLiteral("InsertProduct for %1 returned %2").arg(prodName).arg(ret));

            // Now remove the job folder containing temporary files
            //RemoveJobFolder(ctx, event.jobId);
        } else {
            Logger::error(QStringLiteral("Cannot insert into database the product with name %1 and folder %2").arg(prodName).arg(productFolder));
        }
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

QStringList LaiRetrievalHandler::GetNdviRviExtractionArgs(const QString &inputProduct, const QString &msksFlagsFile, const QString &ftsFile, const QString &ndviFile,
                                                          const QString &resolution) {
    return { "NdviRviExtraction2",
           "-xml", inputProduct,
           "-msks", msksFlagsFile,
           "-ndvi", ndviFile,
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

QStringList LaiRetrievalHandler::GetQuantifyImageArgs(const QString &inFileName, const QString &outFileName)  {
    return { "QuantifyImage",
        "-in", inFileName,
        "-out", outFileName
    };
}

QStringList LaiRetrievalHandler::GetMonoDateMskFlagsArgs(const QString &inputProduct, const QString &monoDateMskFlgsFileName,
                                                         const QString &monoDateMskFlgsResFileName, const QString &resStr) {
    return { "GenerateLaiMonoDateMaskFlags",
      "-inxml", inputProduct,
      "-out", monoDateMskFlgsFileName,
      "-outres", resStr,
      "-outresampled", monoDateMskFlgsResFileName

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

QStringList LaiRetrievalHandler::GetLaiMonoProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                                const QStringList &products, const QStringList &tileIdsList, const QStringList &ndviList,
                                                                const QStringList &laiList, const QStringList &laiErrList, const QStringList &laiFlgsList) {

    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");

    //const auto &targetFolder = productFormatterTask.GetFilePath("");
    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.xml");

    WriteExecutionInfosFile(executionInfosPath, configParameters, products, false);

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
    productFormatterArgs.append(products);

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

QStringList LaiRetrievalHandler::GetReprocProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                    const QStringList &listProducts, const QList<LAIProductFormatterParams> &productParams, bool isFitted) {

    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");

    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.xml");

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

void LaiRetrievalHandler::GetStepsToGenModel(std::map<QString, QString> &configParameters,
                                             bool bHasMonoDateLai,
                                             const QStringList &listProducts,
                                             QList<TaskToSubmit> &allTasksList,
                                             NewStepList &steps)
{
    const auto &modelsFolder = configParameters["processor.l3b.lai.modelsfolder"];
    const auto &rsrCfgFile = configParameters["processor.l3b.lai.rsrcfgfile"];
    int i = 0;
    int TasksNoPerProduct = MODEL_GEN_TASKS_PER_PRODUCT;
    if (bHasMonoDateLai) TasksNoPerProduct += LAI_TASKS_PER_PRODUCT;
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

bool LaiRetrievalHandler::IsGenModels(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
    bool bGenModels = false;
    if(parameters.contains("genmodel")) {
        bGenModels = (parameters["genmodel"].toInt() != 0);
    } else {
        bGenModels = ((configParameters["processor.l3b.generate_models"]).toInt() != 0);
    }
    return bGenModels;
}

bool LaiRetrievalHandler::IsGenMonoDate(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
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

bool LaiRetrievalHandler::IsNDayReproc(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
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

bool LaiRetrievalHandler::IsFittedReproc(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
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

ProcessorJobDefinitionParams LaiRetrievalHandler::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                          const ConfigurationParameterValueMap &requestOverrideCfgValues)
{
    ConfigurationParameterValueMap mapCfg = ctx.GetConfigurationParameters(QString("processor.l3b."), siteId, requestOverrideCfgValues);

    ProcessorJobDefinitionParams params;
    params.isValid = false;

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

    QDateTime seasonStartDate;
    QDateTime seasonEndDate;
    GetSeasonStartEndDates(ctx, siteId, seasonStartDate, seasonEndDate, requestOverrideCfgValues);
    if(!seasonStartDate.isValid()) {
        return params;
    }

    // by default the start date is the season start date
    QDateTime startDate = seasonStartDate;
    QDateTime endDate = QDateTime::fromTime_t(scheduledDate);

    if(generateLai || generateReprocess) {
        int productionInterval = mapCfg[generateLai ? "processor.l3b.production_interval":
                                                      "processor.l3b.reproc_production_interval"].value.toInt();
        startDate = endDate.addDays(-productionInterval);
        if(startDate < seasonStartDate) {
            startDate = seasonStartDate;
        }
    }

    params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    if (params.productList.size() > 0) {
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
    }

    return params;
}

bool LaiRetrievalHandler::GetMonoDateFormatterParamInfosForProduct(
        const QString &product,
        const QMap<QString, TileTemporalFilesInfo> &mapTiles,
        const QMap<QString, LAIProductFormatterParams> &mapTileToParams,
        const QMap<QString, QStringList> &inputProductToTilesMap,
        QStringList &outProductTiles,
        QList<LAIMonoDateProductFormatterParams> &outProductParams,
        QStringList &outProductTileMetaFiles) {

    for(auto tile : mapTiles.keys()) {
        QStringList listTemporalTiles = ProcessorHandlerHelper::GetTemporalTileFiles(mapTiles.value(tile));
        LAIProductFormatterParams params = mapTileToParams[tile];

        // normally we have the same number
        if(params.listLaiMonoParams.size() != listTemporalTiles.size()) {
            return false;
        }

        for(int i = 0; i<listTemporalTiles.size(); i++) {
            const QString &tileMetaFile = listTemporalTiles[i];
            QString productForTile = GetL2AProductForTileMetaFile(inputProductToTilesMap, tileMetaFile);
            if(product == productForTile) {
                // Normally, a product should occur only once for a tile. If multiple times, it will be handled as a distinct one
                outProductTiles.append(tile);
                outProductParams.append(params.listLaiMonoParams[i]);
                outProductTileMetaFiles.append(tileMetaFile);
            }
        }
    }
    return outProductTiles.size() > 0;
}
