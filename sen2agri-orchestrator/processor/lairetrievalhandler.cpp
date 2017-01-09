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
#define CUT_TASKS_NO                5

#define DEFAULT_GENERATED_SAMPLES_NO    "40000"
#define DEFAULT_NOISE_VAR               "0.01"
#define DEFAULT_BEST_OF                 "1"
#define DEFAULT_REGRESSOR               "nn"

void LaiRetrievalHandler::CreateTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList, const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                                    LAIProductFormatterParams &outProdFormatterParams,
                                                    const QStringList &listProducts, const QStringList &monoDateMskFlagsLaiFileNames,
                                                    bool bGenModels, bool bMonoDateLai, bool bNDayReproc, bool bFittedReproc) {
    // just create the tasks but with no information so far
    // first we add the tasks to be performed for each product
    //int TasksNoPerProduct = 0;
    //if(bGenModels)
    //    TasksNoPerProduct += MODEL_GEN_TASKS_PER_PRODUCT;

    bool bCreateFootprint = false;
    if(bMonoDateLai && (tileTemporalFilesInfo.uniqueSatteliteIds.size() > 1 && tileTemporalFilesInfo.shapePath == "")) {
        //TasksNoPerProduct += LAI_TASKS_PER_PRODUCT;
        // if we have multiple satellites and we don't have yet the shape file for this tile, then we must
        // create the footprint for the primary satellite ID
        outAllTasksList.append(TaskToSubmit{ "lai-create-tile-footprint", {} });
        bCreateFootprint = true;
    }

    int nbLaiMonoProducts = listProducts.size();
    for(int i = 0; i<nbLaiMonoProducts; i++) {
        // we might have here empty products as maybe we do not generate monodates just for the missing ones
        if(listProducts[i] != "") {
            if(bGenModels) {
                outAllTasksList.append(TaskToSubmit{ "lai-bv-input-variable-generation", {} });
                outAllTasksList.append(TaskToSubmit{ "lai-prosail-simulator", {} });
                outAllTasksList.append(TaskToSubmit{ "lai-training-data-generator", {} });
                outAllTasksList.append(TaskToSubmit{ "lai-inverse-model-learning", {} });
            }
            if(bMonoDateLai) {
                outAllTasksList.append(TaskToSubmit{"lai-mono-date-mask-flags", {}});
                outAllTasksList.append(TaskToSubmit{"lai-ndvi-rvi-extractor", {}});
                if(tileTemporalFilesInfo.temporalTilesFileInfos[i].satId != tileTemporalFilesInfo.primarySatelliteId) {
                    outAllTasksList.append(TaskToSubmit{"gdalwarp", {}});
                    outAllTasksList.append(TaskToSubmit{"compression", {}});
                    outAllTasksList.append(TaskToSubmit{"gdalwarp", {}});
                    outAllTasksList.append(TaskToSubmit{"compression", {}});
                    outAllTasksList.append(TaskToSubmit{"gdalwarp", {}});
                }
                outAllTasksList.append(TaskToSubmit{"lai-bv-image-invertion", {}});
                outAllTasksList.append(TaskToSubmit{"lai-bv-err-image-invertion", {}});
                outAllTasksList.append(TaskToSubmit{"lai-quantify-image", {}});
                outAllTasksList.append(TaskToSubmit{"lai-quantify-err-image", {}});
            }
        }
    }

    if(bNDayReproc || bFittedReproc) {
        for(int i = 0; i<nbLaiMonoProducts; i++) {
            if(monoDateMskFlagsLaiFileNames[i] != "") {
                if(tileTemporalFilesInfo.temporalTilesFileInfos[i].satId != tileTemporalFilesInfo.primarySatelliteId) {
                    outAllTasksList.append(TaskToSubmit{"gdalwarp", {}});
                    outAllTasksList.append(TaskToSubmit{"gdalwarp", {}});
                    outAllTasksList.append(TaskToSubmit{"gdalwarp", {}});
                }
            }
        }
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
    int nCurTaskIdx = bCreateFootprint ? 1 : 0;
    for(i = 0; i<nbLaiMonoProducts; i++) {
        // we might have here empty products as maybe we do not generate monodates just for the missing ones
        if(listProducts[i] != "") {
            // add the tasks for generating models
            if(bGenModels) {
                if(bCreateFootprint) {
                    // Do not start BVVariableGeneration until footprint is created
                    outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[0]);
                }
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
            } else if(bMonoDateLai && bCreateFootprint) {
                // Do not start until footprint is created
                outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[0]);
            }
            if(bMonoDateLai) {
                int genFlagsIdx = nCurTaskIdx++;

                // ndvi-rvi-extraction -> lai-mono-date-mask-flags
                int ndviRviExtrIdx = nCurTaskIdx++;
                outAllTasksList[ndviRviExtrIdx].parentTasks.append(outAllTasksList[ndviRviExtrIdx-1]);

                // Cut the NDVI/RVI and the flags images if necessary
                if(tileTemporalFilesInfo.temporalTilesFileInfos[i].satId != tileTemporalFilesInfo.primarySatelliteId) {
                    int cutFlagsTaksIdx = nCurTaskIdx++;
                    // add the task for cutting the flags
                    outAllTasksList[cutFlagsTaksIdx].parentTasks.append(outAllTasksList[cutFlagsTaksIdx-1]);
                    int compressCutFlagsTaksIdx = nCurTaskIdx++;
                    // add the task for cutting the flags
                    outAllTasksList[compressCutFlagsTaksIdx].parentTasks.append(outAllTasksList[cutFlagsTaksIdx]);
                    genFlagsIdx = compressCutFlagsTaksIdx;  // the flags index becomes this one

                    // add the task for cutting the single ndvi
                    int cutSingleNdviTaksIdx = nCurTaskIdx++;
                    outAllTasksList[cutSingleNdviTaksIdx].parentTasks.append(outAllTasksList[cutSingleNdviTaksIdx-1]);
                    int compressedCutSingleNdviTaksIdx = nCurTaskIdx++;
                    outAllTasksList[compressedCutSingleNdviTaksIdx].parentTasks.append(outAllTasksList[cutSingleNdviTaksIdx]);

                    // add the task for cutting the ndvi-rvi raster
                    int cutNdviRviTaksIdx = nCurTaskIdx++;
                    outAllTasksList[cutNdviRviTaksIdx].parentTasks.append(outAllTasksList[cutNdviRviTaksIdx-1]);
                    ndviRviExtrIdx = cutNdviRviTaksIdx; // the ndviRvi index now is the cutted NdviRvi
                                                        // so bv-image-inversion will wait after this one
                }
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
                LAIMonoDateProductFormatterParams monoParams;
                monoParams.parentsTasksRef.append(outAllTasksList[nQuantifyImageInversionIdx]);
                monoParams.parentsTasksRef.append(outAllTasksList[nQuantifyErrImageInversionIdx]);
                outProdFormatterParams.listLaiMonoParams.append(monoParams);
            }
        }
    }

    if(bNDayReproc || bFittedReproc) {
        m_nFirstReprocessingIdx = nCurTaskIdx;
        QList<int> tmpLaiMonoDateFlgsIdxs;
        QList<int> tmpQuantifyImageInvIdxs;
        QList<int> tmpQuantifyErrImageInvIdxs;

        for(i = 0; i<nbLaiMonoProducts; i++) {
            if(monoDateMskFlagsLaiFileNames[i] != "") {
                if(tileTemporalFilesInfo.temporalTilesFileInfos[i].satId != tileTemporalFilesInfo.primarySatelliteId) {
                    int cutFlagsTasksIdx = nCurTaskIdx++;
                    // the cut flags task in this case will wait (for simplicity) for all other flags generation
                    // NOTE: This raster can be cut multiple times if mono-date is also generated but this will
                    // not happen in production
                    for(int idx: laiMonoDateFlgsIdxs) {
                        outAllTasksList[cutFlagsTasksIdx].parentTasks.append(outAllTasksList[idx]);
                    }
                    // Make time series builder wait also for this task
                    tmpLaiMonoDateFlgsIdxs.append(cutFlagsTasksIdx);

                    int cutLaiTasksIdx = nCurTaskIdx++;
                    // the cut LAI task in this case will wait (for simplicity) for all other LAI generation
                    for(int idx: quantifyImageInvIdxs) {
                        outAllTasksList[cutLaiTasksIdx].parentTasks.append(outAllTasksList[idx]);
                    }
                    // Make time series builder wait also for this task
                    tmpQuantifyImageInvIdxs.append(cutLaiTasksIdx);

                    // the cut LAI ERR task in this case will wait (for simplicity) for all other LAI ERR generation
                    int cutLaiErrTasksIdx = nCurTaskIdx++;
                    for(int idx: quantifyErrImageInvIdxs) {
                        outAllTasksList[cutLaiErrTasksIdx].parentTasks.append(outAllTasksList[idx]);
                    }
                    // Make time series builder wait also for this task
                    tmpQuantifyErrImageInvIdxs.append(cutLaiErrTasksIdx);
                }
            }
        }
        // add the eventual cut tasks in the list of tasks for which time series builders are waiting
        for(int idx: tmpLaiMonoDateFlgsIdxs) {
            laiMonoDateFlgsIdxs.append(idx);
        }
        for(int idx: tmpQuantifyImageInvIdxs) {
            quantifyImageInvIdxs.append(idx);
        }
        for(int idx: tmpQuantifyErrImageInvIdxs) {
            quantifyErrImageInvIdxs.append(idx);
        }

        // time-series-builder -> ALL last bv-image-inversion/quantified-images
        m_nTimeSeriesBuilderIdx = nCurTaskIdx++;
        for(int idx: quantifyImageInvIdxs) {
            outAllTasksList[m_nTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[idx]);
        }
        //err-time-series-builder -> ALL bv-err-image-inversion/quantified-err-images
        m_nErrTimeSeriesBuilderIdx = nCurTaskIdx++;
        for(int idx: quantifyErrImageInvIdxs) {
            outAllTasksList[m_nErrTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[idx]);
        }

        //lai-msk-flags-time-series-builder -> ALL lai-mono-date-mask-flags
        m_nLaiMskFlgsTimeSeriesBuilderIdx = nCurTaskIdx++;
        for(int idx: laiMonoDateFlgsIdxs) {
            outAllTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx].parentTasks.append(outAllTasksList[idx]);
        }

        if(bNDayReproc) {
            //profile-reprocessing -> time-series-builder AND err-time-series-builder AND lai-msk-flags-time-series-builder
            m_nProfileReprocessingIdx = nCurTaskIdx++;
            outAllTasksList[m_nProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nTimeSeriesBuilderIdx]);
            outAllTasksList[m_nProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nErrTimeSeriesBuilderIdx]);
            outAllTasksList[m_nProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx]);

            //reprocessed-profile-splitter -> profile-reprocessing
            m_nReprocessedProfileSplitterIdx = nCurTaskIdx++;
            outAllTasksList[m_nReprocessedProfileSplitterIdx].parentTasks.append(outAllTasksList[m_nProfileReprocessingIdx]);
        }

        if(bFittedReproc) {
            //fitted-profile-reprocessing -> time-series-builder AND err-time-series-builder AND lai-msk-flags-time-series-builder
            m_nFittedProfileReprocessingIdx = nCurTaskIdx++;
            outAllTasksList[m_nFittedProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nTimeSeriesBuilderIdx]);
            outAllTasksList[m_nFittedProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nErrTimeSeriesBuilderIdx]);
            outAllTasksList[m_nFittedProfileReprocessingIdx].parentTasks.append(outAllTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx]);

            //fitted-reprocessed-profile-splitter -> fitted-profile-reprocessing
            m_nFittedProfileReprocessingSplitterIdx = nCurTaskIdx++;
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

NewStepList LaiRetrievalHandler::GetStepsToGenModel(std::map<QString, QString> &configParameters,
                                             const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                             bool bHasMonoDateLai,
                                             const QStringList &listProducts,
                                             QList<TaskToSubmit> &allTasksList)
{
    NewStepList steps;
    bool bCreateFootprint = false;
    if(tileTemporalFilesInfo.uniqueSatteliteIds.size() > 1 && tileTemporalFilesInfo.shapePath == "") {
        bCreateFootprint = true;
    }
    const auto &modelsFolder = configParameters["processor.l3b.lai.modelsfolder"];
    const auto &rsrCfgFile = configParameters["processor.l3b.lai.rsrcfgfile"];
    int curIdx = bCreateFootprint ? 1 : 0;
    for(int i = 0; i<listProducts.size(); i++) {
        const QString &curXml = listProducts[i];
        if(curXml != "") {
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
            if (bHasMonoDateLai) curIdx += LAI_TASKS_PER_PRODUCT;
            if(tileTemporalFilesInfo.temporalTilesFileInfos[i].satId != tileTemporalFilesInfo.primarySatelliteId) {
                curIdx += CUT_TASKS_NO;
            }
        }
    }

    return steps;
}

NewStepList LaiRetrievalHandler::GetStepsForMonodateLai(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                    const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                                    const QStringList &monoDateInputs, QList<TaskToSubmit> &allTasksList,
                                                    LAIProductFormatterParams &productFormatterParams,
                                                    QStringList &monoDateMskFlagsLaiFileNames,
                                                    QStringList &quantifiedLaiFileNames,
                                                    QStringList &quantifiedErrLaiFileNames)
{
    if((monoDateInputs.size() != monoDateMskFlagsLaiFileNames.size()) ||
            (monoDateInputs.size() != quantifiedLaiFileNames.size()) ||
            (monoDateInputs.size() != quantifiedErrLaiFileNames.size())) {
        throw std::runtime_error(
            QStringLiteral("The list of input products should have the same size as the L3B lists").toStdString());
    }

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
    bool bFootprintTaskCreated = false;
    QString shapePath = tileTemporalFilesInfo.shapePath;
    // if we have multiple satellites and we don't have yet the shape file for this tile, then we must
    // create the footprint for the primary satellite ID
    if(tileTemporalFilesInfo.uniqueSatteliteIds.size() > 1 && tileTemporalFilesInfo.shapePath == "") {
        // CreateFootprint should execute on the first primary product metadatafrom the tile
        QString primaryTileMetadata;
        for(int i = 0; i<tileTemporalFilesInfo.temporalTilesFileInfos.size(); i++) {
            if(tileTemporalFilesInfo.temporalTilesFileInfos[i].satId == tileTemporalFilesInfo.primarySatelliteId) {
                primaryTileMetadata = tileTemporalFilesInfo.temporalTilesFileInfos[i].file;
                break;
            }
        }
        // The create footprint task is the first task in the list
        TaskToSubmit &createFootprintTaskHandler = allTasksList[0];
        std::map<QString, QString> executorConfigParameters =
            ctx.GetJobConfigurationParameters(event.jobId, "executor.shapes_dir");
        QString shapeFilesFolder = executorConfigParameters["executor.shapes_dir"];
        shapePath = ProcessorHandlerHelper::BuildShapeName(shapeFilesFolder, tileTemporalFilesInfo.tileId,
                                                           event.jobId, createFootprintTaskHandler.taskId);
        QStringList createFootprintArgs = { "CreateFootprint", "-in", primaryTileMetadata,
                                            "-mode", "metadata", "-out", shapePath};
        steps.append(createFootprintTaskHandler.CreateStep("CreateFootprint", createFootprintArgs));
        bFootprintTaskCreated = true;
    }

    int curTaskIdx = bFootprintTaskCreated ? 1 : 0;
    int curValidProduct = 0;
    for (int i = 0; i<monoDateInputs.size(); i++) {
        const auto &inputProduct = monoDateInputs[i];
        if(inputProduct != "") {
            int flagsCutIdx = -1;
            int compressedFlagsCutIdx = -1;
            int singleNdviCutIdx = -1;
            int compressedNdviCutIdx = -1;
            int ndviRviCutIdx = -1;

            bool bIsSecondarySat = (tileTemporalFilesInfo.temporalTilesFileInfos[i].satId != tileTemporalFilesInfo.primarySatelliteId);
            if(bGenModels) {
                // now update the index for the ndviRvi task
                curTaskIdx += MODEL_GEN_TASKS_PER_PRODUCT;
            }
            TaskToSubmit &genMonoDateMskFagsTask = allTasksList[curTaskIdx++];
            TaskToSubmit &ndviRviExtractorTask = allTasksList[curTaskIdx++];
            if(bIsSecondarySat) {
                // if we have a secondary satellite, save the cut task indexes
                flagsCutIdx = curTaskIdx++;
                compressedFlagsCutIdx = curTaskIdx++;
                singleNdviCutIdx = curTaskIdx++;
                compressedNdviCutIdx = curTaskIdx++;
                ndviRviCutIdx = curTaskIdx++;
            }
            TaskToSubmit &bvImageInversionTask = allTasksList[curTaskIdx++];
            TaskToSubmit &bvErrImageInversionTask = allTasksList[curTaskIdx++];
            TaskToSubmit &quantifyImageTask = allTasksList[curTaskIdx++];
            TaskToSubmit &quantifyErrImageTask = allTasksList[curTaskIdx++];

            const auto & monoDateMskFlgsFileName = genMonoDateMskFagsTask.GetFilePath("LAI_mono_date_msk_flgs_img.tif");
            auto monoDateMskFlgsResFileName = genMonoDateMskFagsTask.GetFilePath("LAI_mono_date_msk_flgs_img_resampled.tif");
            auto singleNdviFile = ndviRviExtractorTask.GetFilePath("single_ndvi.tif");
            auto ftsFile = ndviRviExtractorTask.GetFilePath("ndvi_rvi.tif");
            const auto & monoDateLaiFileName = bvImageInversionTask.GetFilePath("LAI_mono_date_img.tif");
            const auto & monoDateErrFileName = bvErrImageInversionTask.GetFilePath("LAI_mono_date_ERR_img.tif");
            const auto & quantifiedLaiFileName = quantifyImageTask.GetFilePath("LAI_mono_date_img_16.tif");
            const auto & quantifiedErrFileName = quantifyErrImageTask.GetFilePath("LAI_mono_date_ERR_img_16.tif");

            QStringList genMonoDateMskFagsArgs = GetMonoDateMskFlagsArgs(inputProduct, monoDateMskFlgsFileName,
                                                                         "\"" + monoDateMskFlgsResFileName+"?gdal:co:COMPRESS=DEFLATE\"",
                                                                         resolutionStr);
            QStringList ndviRviExtractionArgs = GetNdviRviExtractionArgs(inputProduct, monoDateMskFlgsFileName,
                                                                         ftsFile, "\"" + singleNdviFile+"?gdal:co:COMPRESS=DEFLATE\"",
                                                                         resolutionStr);

            // add these steps to the steps list to be submitted
            steps.append(genMonoDateMskFagsTask.CreateStep("GenerateLaiMonoDateMaskFlags", genMonoDateMskFagsArgs));
            steps.append(ndviRviExtractorTask.CreateStep("NdviRviExtraction2", ndviRviExtractionArgs));

            // if we are a secondary satellite, then cut the Flags, Ndvi and NdviRvi images
            if(bIsSecondarySat) {
                TaskToSubmit &flagsCutTask = allTasksList[flagsCutIdx];
                TaskToSubmit &compressedFlagsCutTask = allTasksList[compressedFlagsCutIdx];
                TaskToSubmit &singleNdviCutTask = allTasksList[singleNdviCutIdx];
                TaskToSubmit &compressedSingleNdviCutTask = allTasksList[compressedNdviCutIdx];
                TaskToSubmit &ndviRviCutTask = allTasksList[ndviRviCutIdx];

                // Here cut the monodate and the ndvi flags
                const auto & cutMonoDateMskFlgsResFileName = flagsCutTask.GetFilePath("LAI_mono_date_msk_flgs_img_cut.tif");
                const auto & comprCutMonoDateMskFlgsResFileName = compressedFlagsCutTask.GetFilePath("LAI_mono_date_msk_flgs_img_cut_compr.tif");
                const auto & cutSingleNdviFile = singleNdviCutTask.GetFilePath("single_ndvi_cut.tif");
                const auto & comprCutSingleNdviFile = compressedSingleNdviCutTask.GetFilePath("single_ndvi_cut_compr.tif");
                const auto & cutFtsFile = ndviRviCutTask.GetFilePath("ndvi_rvi_cut.tif");

                QStringList cutFlgsArgs = GetCutImgArgs(shapePath, monoDateMskFlgsResFileName, cutMonoDateMskFlgsResFileName);
                QStringList comprCutFlgsArgs = GetCompressImgArgs(cutMonoDateMskFlgsResFileName, comprCutMonoDateMskFlgsResFileName);
                QStringList cutSingleNdviArgs = GetCutImgArgs(shapePath, singleNdviFile, cutSingleNdviFile);
                QStringList comprCutSingleNdviArgs = GetCompressImgArgs(cutSingleNdviFile, comprCutSingleNdviFile);
                QStringList cutNdviRviArgs = GetCutImgArgs(shapePath, ftsFile, cutFtsFile);
                steps.append(flagsCutTask.CreateStep("lai-gdal-flags", cutFlgsArgs));
                steps.append(compressedFlagsCutTask.CreateStep("lai-gdal-flags-compression", comprCutFlgsArgs));
                steps.append(singleNdviCutTask.CreateStep("lai-gdal-ndvi", cutSingleNdviArgs));
                steps.append(compressedSingleNdviCutTask.CreateStep("lai-gdal-ndvi-compression", comprCutSingleNdviArgs));
                steps.append(ndviRviCutTask.CreateStep("lai-gdal-ndvi-rvi", cutNdviRviArgs));

                // overwrite the value for the single ndvi file with the cut one
                monoDateMskFlgsResFileName = comprCutMonoDateMskFlgsResFileName;
                singleNdviFile = comprCutSingleNdviFile;
                ftsFile = cutFtsFile;
            }
            QStringList bvImageInvArgs = GetBvImageInvArgs(ftsFile, monoDateMskFlgsResFileName, inputProduct, modelsFolder, monoDateLaiFileName);
            QStringList bvErrImageInvArgs = GetBvErrImageInvArgs(ftsFile, monoDateMskFlgsResFileName, inputProduct, modelsFolder, monoDateErrFileName);
            QStringList quantifyImageArgs = GetQuantifyImageArgs(monoDateLaiFileName, quantifiedLaiFileName);
            QStringList quantifyErrImageArgs = GetQuantifyImageArgs(monoDateErrFileName, quantifiedErrFileName);

            // save the mono date LAI file name list
            productFormatterParams.listLaiMonoParams[curValidProduct].ndvi = singleNdviFile;
            productFormatterParams.listLaiMonoParams[curValidProduct].laiMonoDate = quantifiedLaiFileName;
            productFormatterParams.listLaiMonoParams[curValidProduct].laiMonoDateErr = quantifiedErrFileName;
            productFormatterParams.listLaiMonoParams[curValidProduct].laiMonoDateFlgs = monoDateMskFlgsResFileName;

            // Save the list for using in the Reprocessed task below (if needed).
            // We cannot use the lists in the productFormatterParams.listLaiMonoParams as these are not filled in the case
            // when we do not have Mono-date products creation (Here we might have just filling the
            // gaps for the missing L3B products)
            monoDateMskFlagsLaiFileNames[i] = monoDateMskFlgsResFileName;
            quantifiedLaiFileNames[i] = quantifiedLaiFileName;
            quantifiedErrLaiFileNames[i] = quantifiedErrFileName;

            steps.append(bvImageInversionTask.CreateStep("BVImageInversion", bvImageInvArgs));
            steps.append(bvErrImageInversionTask.CreateStep("BVImageInversion", bvErrImageInvArgs));
            steps.append(quantifyImageTask.CreateStep("QuantifyImage", quantifyImageArgs));
            steps.append(quantifyErrImageTask.CreateStep("QuantifyImage", quantifyErrImageArgs));
            curValidProduct++;
        }
    }

    return steps;
}

NewStepList LaiRetrievalHandler::GetStepsForMultiDateReprocessing(std::map<QString, QString> &configParameters,
                const TileTemporalFilesInfo &tileTemporalFilesInfo, const QStringList &listProducts, QList<TaskToSubmit> &allTasksList,
                bool bNDayReproc, bool bFittedReproc, LAIProductFormatterParams &productFormatterParams,
                QStringList &monoDateMskFlagsLaiFileNames, QStringList &quantifiedLaiFileNames, QStringList &quantifiedErrLaiFileNames)
{
    NewStepList steps;
    QString fittedFileListFileName;
    QString fittedFlagsFileListFileName;
    QString reprocFileListFileName;
    QString reprocFlagsFileListFileName;

    QStringList monoDateMskFlagsLaiFileNames2;
    QStringList quantifiedLaiFileNames2;
    QStringList quantifiedErrLaiFileNames2;
    int nCurTaskIdx = m_nFirstReprocessingIdx;

    // Check that quantifiedLaiFileNames size is equal with listProducts size (normally it should)
    if((listProducts.size() != monoDateMskFlagsLaiFileNames.size()) ||
            (listProducts.size() != quantifiedLaiFileNames.size()) ||
            (listProducts.size() != quantifiedErrLaiFileNames.size())) {
        throw std::runtime_error(
            QStringLiteral("The list of input products should have the same size as the L3B lists").toStdString());
    }

    for(int i = 0; i< listProducts.size(); i++) {
        // if we are a secondary satellite, then cut the Flags, Ndvi and NdviRvi images
        if(monoDateMskFlagsLaiFileNames[i] != "") {
            if(tileTemporalFilesInfo.temporalTilesFileInfos[i].satId != tileTemporalFilesInfo.primarySatelliteId) {
                TaskToSubmit &flagsCutTask = allTasksList[nCurTaskIdx++];
                TaskToSubmit &laiCutTask = allTasksList[nCurTaskIdx++];
                TaskToSubmit &laiErrCutTask = allTasksList[nCurTaskIdx++];

                // Here cut the monodate and the ndvi flags
                const auto & cutFlgsFileName = flagsCutTask.GetFilePath("flags_cut.tif");
                const auto & cutLaiFile = laiCutTask.GetFilePath("lai_cut.tif");
                const auto & cutLaiErrFile = laiErrCutTask.GetFilePath("lai_err_cut.tif");

                QStringList cutFlgsArgs = GetCutImgArgs(tileTemporalFilesInfo.shapePath, monoDateMskFlagsLaiFileNames[i], cutFlgsFileName);
                QStringList cutLaiArgs = GetCutImgArgs(tileTemporalFilesInfo.shapePath, quantifiedLaiFileNames[i], cutLaiFile);
                QStringList cutLaiErrArgs = GetCutImgArgs(tileTemporalFilesInfo.shapePath, quantifiedErrLaiFileNames[i], cutLaiErrFile);
                steps.append(flagsCutTask.CreateStep("lai-gdal-flags", cutFlgsArgs));
                steps.append(laiCutTask.CreateStep("lai-gdal-lai", cutLaiArgs));
                steps.append(laiErrCutTask.CreateStep("lai-gdal-err-lai", cutLaiErrArgs));

                monoDateMskFlagsLaiFileNames2.append(cutFlgsFileName);
                quantifiedLaiFileNames2.append(cutLaiFile);
                quantifiedErrLaiFileNames2.append(cutLaiErrFile);
            } else {
                monoDateMskFlagsLaiFileNames2.append(monoDateMskFlagsLaiFileNames[i]);
                quantifiedLaiFileNames2.append(quantifiedLaiFileNames[i]);
                quantifiedErrLaiFileNames2.append(quantifiedErrLaiFileNames[i]);
            }
        }
    }

    TaskToSubmit &imgTimeSeriesBuilderTask = allTasksList[m_nTimeSeriesBuilderIdx];
    TaskToSubmit &errTimeSeriesBuilderTask = allTasksList[m_nErrTimeSeriesBuilderIdx];
    TaskToSubmit &mskFlagsTimeSeriesBuilderTask = allTasksList[m_nLaiMskFlgsTimeSeriesBuilderIdx];

    const auto & allLaiTimeSeriesFileName = imgTimeSeriesBuilderTask.GetFilePath("LAI_time_series.tif");
    const auto & allErrTimeSeriesFileName = errTimeSeriesBuilderTask.GetFilePath("Err_time_series.tif");
    const auto & allMskFlagsTimeSeriesFileName = mskFlagsTimeSeriesBuilderTask.GetFilePath("Mask_Flags_time_series.tif");

    // TODO: In order to have consistent situation in the cases when we have only Reprocessing or (Monodate AND Reprocessing)
    // we should use here the list of the quantified values for the LAI monodate. Otherwise, we get different results in this situation
    QStringList timeSeriesBuilderArgs = GetTimeSeriesBuilderArgs(quantifiedLaiFileNames2, allLaiTimeSeriesFileName);
    QStringList errTimeSeriesBuilderArgs = GetErrTimeSeriesBuilderArgs(quantifiedErrLaiFileNames2, allErrTimeSeriesFileName);
    QStringList mskFlagsTimeSeriesBuilderArgs = GetMskFlagsTimeSeriesBuilderArgs(monoDateMskFlagsLaiFileNames2, allMskFlagsTimeSeriesFileName);

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

    return steps;
}

void LaiRetrievalHandler::HandleNewTilesList(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                             const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                             const QList<L2AToL3B> &listL2AToL3BProducts,
                                             const TileTemporalFilesInfo &missingL3BTileTemporalFilesInfo,
                                             LAIGlobalExecutionInfos &outGlobalExecInfos) {

    QStringList listProducts = ProcessorHandlerHelper::GetTemporalTileFiles(tileTemporalFilesInfo);
    QStringList missingL3BInputs = ProcessorHandlerHelper::GetTemporalTileFiles(missingL3BTileTemporalFilesInfo);

    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");
    QStringList monoDateMskFlagsLaiFileNames;
    QStringList quantifiedLaiFileNames;
    QStringList quantifiedErrLaiFileNames;

    bool bGenModels = IsGenModels(parameters, configParameters);
    bool bMonoDateLai = IsGenMonoDate(parameters, configParameters);
    bool bNDayReproc = IsNDayReproc(parameters, configParameters);
    bool bFittedReproc = IsFittedReproc(parameters, configParameters);

    QStringList monoDateInputs = listProducts;
    // make the quantifiedLaiFileNames to have the same dimmension as the input products.
    // Only the existing ones will be filled
    for(int i = 0; i<listProducts.size(); i++) {
        quantifiedLaiFileNames.append("");
        quantifiedErrLaiFileNames.append("");
        monoDateMskFlagsLaiFileNames.append("");
    }
    if(!bMonoDateLai) {
        ExtractExistingL3BProductsFiles(listProducts, tileTemporalFilesInfo, listL2AToL3BProducts, monoDateMskFlagsLaiFileNames,
                                        quantifiedLaiFileNames, quantifiedErrLaiFileNames);

        if(missingL3BInputs.size() > 0) {
            // fill the monodate inputs with the products that are empty
            // but filling with empty string the products that already have monodate products
            monoDateInputs = QStringList();
            int addedMonodateProds = 0;
            for(const QString &inputProd: listProducts) {
                bool bAdded = false;
                for(const QString &missingProd: missingL3BInputs) {
                    if(inputProd == missingProd) {
                        monoDateInputs.append(inputProd);
                        addedMonodateProds++;
                        bAdded = true;
                        break;
                    }
                }
                if(!bAdded) {
                    monoDateInputs.append("");
                }
            }
            // Force mono-date for the missing L3B inputs
            if(addedMonodateProds > 0)
                bMonoDateLai = true;
        }
        // if we have no monodate to produce and also we found no rasters for the existing L3B list, we create no task
        if(!bMonoDateLai && quantifiedLaiFileNames.size() == 0)
            return;
    }
    // no need to generate models if no monodate LAI generation
    if(!bMonoDateLai) {
        bGenModels = false;
    }

    QList<TaskToSubmit> &allTasksList = outGlobalExecInfos.allTasksList;
    LAIProductFormatterParams &productFormatterParams = outGlobalExecInfos.prodFormatParams;

    // create the tasks
    CreateTasksForNewProducts(allTasksList, tileTemporalFilesInfo, outGlobalExecInfos.prodFormatParams, monoDateInputs,
                              monoDateMskFlagsLaiFileNames, bGenModels, bMonoDateLai, bNDayReproc, bFittedReproc);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    // submit all tasks
    SubmitTasks(ctx, event.jobId, allTasksListRef);

    NewStepList &steps = outGlobalExecInfos.allStepsList;

    // first extract the model file names from the models folder
    if(bGenModels) {
        steps += GetStepsToGenModel(configParameters, tileTemporalFilesInfo, bMonoDateLai, monoDateInputs, allTasksList);
    }

    if(bMonoDateLai) {
        steps += GetStepsForMonodateLai(ctx, event, tileTemporalFilesInfo, monoDateInputs, allTasksList, productFormatterParams,
                               monoDateMskFlagsLaiFileNames, quantifiedLaiFileNames, quantifiedErrLaiFileNames);
    }

    if(bNDayReproc || bFittedReproc) {
        steps += GetStepsForMultiDateReprocessing(configParameters, tileTemporalFilesInfo, listProducts, allTasksList, bNDayReproc, bFittedReproc, productFormatterParams,
                                                  monoDateMskFlagsLaiFileNames, quantifiedLaiFileNames, quantifiedErrLaiFileNames);
    }
}

// fill the monoDateMskFlagsLaiFileNames,quantifiedLaiFileNames, quantifiedErrLaiFileNames for the existing products
// or let empty string if not possible to find these files
// It is assumed these lists have the same size with the product list
void LaiRetrievalHandler::ExtractExistingL3BProductsFiles(const QStringList &listProducts,
                                     const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                     const QList<L2AToL3B> &listL2AToL3BProducts, QStringList &monoDateMskFlagsLaiFileNames,
                                     QStringList &quantifiedLaiFileNames,
                                     QStringList &quantifiedErrLaiFileNames) {
    ProcessorHandlerHelper::SatelliteIdType satId;
    if((listProducts.size() != monoDateMskFlagsLaiFileNames.size()) ||
            (listProducts.size() != quantifiedLaiFileNames.size()) ||
            (listProducts.size() != quantifiedErrLaiFileNames.size())) {
        throw std::runtime_error(
            QStringLiteral("The list of input products should have the same size as the L3B lists").toStdString());
    }
    for(int i = 0; i<listProducts.size(); i++) {
        const QString &productMeta = listProducts[i];
        // Get the L3B product folder for this product metadata
        //  if it does not exists, add empty quantifiedLaiFileNames
        // else
        //  Get the tile id for this L2A product metadata
        //  Search in the tiles map the L3B product for the found tile
        //  if it exists, get the quantifiedLaiFileNames
        //  otherwise, add empty string
        //
        QString l3bProd;
        for(const L2AToL3B &l2aToL3BProd: listL2AToL3BProducts) {
            if(l2aToL3BProd.L2A == productMeta) {
                l3bProd = l2aToL3BProd.L3B;
                break;
            }
        }
        if(l3bProd != "") {
            // we have an L3B product folder for this metadata
            QString tileId = ProcessorHandlerHelper::GetTileId(productMeta, satId);
            QMap<QString, QString> mapTiles = ProcessorHandlerHelper::GetHighLevelProductTilesDirs(l3bProd);
            if(tileId == tileTemporalFilesInfo.tileId) {
                // get the tiles and their folders for the L3B product
                // search the tile Id (normally, we should have it)
                QString tileDir = mapTiles.contains(tileId) ? mapTiles[tileId] : "";
                if(tileDir != "") {
                    // fill the empty gaps for these lists
                    quantifiedLaiFileNames[i] = ProcessorHandlerHelper::GetHigLevelProductTileFile(tileDir, "SLAIMONO");
                    quantifiedErrLaiFileNames[i] = ProcessorHandlerHelper::GetHigLevelProductTileFile(tileDir, "MLAIERR", true);
                    monoDateMskFlagsLaiFileNames[i] = ProcessorHandlerHelper::GetHigLevelProductTileFile(tileDir, "MMONODFLG", true);
                }
            } else {
                // we have probably a secondary satellite
                // we could check here the sat Id with the primary sat ID
                // Here we might have the following situations:
                //  - either the product was created from a pure secondary product and in this case it has a product Id specific to that satellite
                //  - the L3B product was created and cut from a secondary satellite product but for the tiles from a primary satellite
                // Search the primary tile id into the product tiles
                QString tileDir = mapTiles.contains(tileTemporalFilesInfo.tileId) ?
                            mapTiles[tileTemporalFilesInfo.tileId] :
                            (mapTiles.contains(tileId) ? mapTiles[tileId] : "");
                if(tileDir != "") {
                    // fill the empty gaps for these lists
                    quantifiedLaiFileNames[i] = ProcessorHandlerHelper::GetHigLevelProductTileFile(tileDir, "SLAIMONO");
                    quantifiedErrLaiFileNames[i] = ProcessorHandlerHelper::GetHigLevelProductTileFile(tileDir, "MLAIERR", true);
                    monoDateMskFlagsLaiFileNames[i] = ProcessorHandlerHelper::GetHigLevelProductTileFile(tileDir, "MMONODFLG", true);
                }
            }
        }
        // if not added, the lists quantifiedLaiFileNames will remain with the default value (empty string)
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

void LaiRetrievalHandler::ExtractExistingL3BProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event, const QStringList &listTilesMetaFiles,
                                                     const QMap<QString, QStringList> &inputProductToTilesMap,
                                                     QList<L2AToL3B> &listL2AToL3BProducts,
                                                     QStringList &listL3BProducts, QStringList &missingL3BInputsTiles,
                                                     QStringList &missingL3BInputs) {
    QStringList tempMissingL3BInputsTiles;
    QList<ProcessorHandlerHelper::SatelliteIdType> satIds;
    for (const auto &tileMetaFile : listTilesMetaFiles) {
        ProcessorHandlerHelper::SatelliteIdType satId;
        ProcessorHandlerHelper::GetTileId(tileMetaFile, satId);
        if(!satIds.contains(satId)) {
            satIds.append(satId);
        }
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
            // add the meta file regardless if the l3b already appears as we might have meta files for several
            // tiles that were encapsulated in the same product
            listL2AToL3BProducts.append({tileMetaFile, l3bProdPath});
        } else {
            tempMissingL3BInputsTiles.append(tileMetaFile);
        }
    }
    // generating missing L3B is supported only when not in combinations with Sentinel2 due to complications
    // that occur
    if(satIds.size() == 1) {
        for (const auto &tileMetaFile : tempMissingL3BInputsTiles) {
            missingL3BInputsTiles.append(tileMetaFile);
            QString productForTile = GetL2AProductForTileMetaFile(inputProductToTilesMap, tileMetaFile);
            if(!missingL3BInputs.contains(productForTile)) {
                missingL3BInputs.append(productForTile);
                Logger::debug(QStringLiteral("The L3B will be created for missing product: %1").arg(productForTile));
            }
        }
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

    // The list of input products that do not have a L3B LAI Monodate created
    QList<L2AToL3B> listL2AToL3BProducts;
    QStringList listL3BProducts;
    QStringList missingL3BInputsTiles;
    QStringList missingL3BInputs;
    if(!bMonoDateLai) {
        ExtractExistingL3BProducts(ctx, event, listTilesMetaFiles, inputProductToTilesMap, listL2AToL3BProducts,
                                   listL3BProducts, missingL3BInputsTiles, missingL3BInputs);
    }

    QList<LAIProductFormatterParams> listParams;
    QMap<QString, LAIProductFormatterParams> mapTileToParams;
    NewStepList allSteps;
    //container for all task
    QList<TaskToSubmit> allTasksList;
    //container for all global execution infos
    QList<LAIGlobalExecutionInfos> allLaiGlobalExecInfos;
    // Create a map containing for each tile Id the list of the temporal file infos
    // NOTE: This map contains only the determined primary satellite tiles
    QMap<QString, TileTemporalFilesInfo> mapTiles = GroupTiles(ctx, event.siteId, event.jobId, listTilesMetaFiles,
                                                               ProductType::L2AProductTypeId);

    //TODO: if the missing tiles are only from the secondary satellite ID (we have in the map tiles 2 satellites),
    //      this will produce later an error as it will consider the secondary satellite as primary and will keep their TileIds
    //      THIS Should be updated to receive the primary satellite ID
    QMap<QString, TileTemporalFilesInfo> mapMissingL3BTiles = GroupTiles(ctx, event.siteId, event.jobId, missingL3BInputsTiles,
                                                                         ProductType::L2AProductTypeId);
    for(auto tileId : mapTiles.keys())
    {
       const TileTemporalFilesInfo &listTemporalTiles = mapTiles.value(tileId);
       const TileTemporalFilesInfo &listL3bMissingInputsTiles = mapMissingL3BTiles.value(tileId);

       Logger::debug(QStringLiteral("Handling tile %1 from a number of %2 tiles").arg(tileId).arg(mapTiles.size()));

       allLaiGlobalExecInfos.append(LAIGlobalExecutionInfos());
       LAIGlobalExecutionInfos &infosRef = allLaiGlobalExecInfos[allLaiGlobalExecInfos.size()-1];
       infosRef.prodFormatParams.tileId = GetProductFormatterTile(tileId);
       HandleNewTilesList(ctx, event, listTemporalTiles, listL2AToL3BProducts, listL3bMissingInputsTiles, infosRef);
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
        bool noValidMonoLai = true;
        for(int i = 0; i < listMonoDateInputProducts.size(); i++) {
            QStringList outProdTiles, outProdTilesMetaFiles;
            QList<LAIMonoDateProductFormatterParams> outProdParams;
            if(!GetMonoDateFormatterParamInfosForProduct(listMonoDateInputProducts[i], mapMonoDateInputProducts, mapTileToParams,
                                                         inputProductToTilesMap, outProdTiles, outProdParams, outProdTilesMetaFiles)) {
                continue;
            }
            QStringList ndviList;
            QStringList laiList;
            QStringList laiErrList;
            QStringList laiFlgsList;
            allTasksList.append({"lai-mono-date-product-formatter", {}});
            TaskToSubmit &laiMonoProductFormatterTask = allTasksList[allTasksList.size()-1];
            for(int i = 0; i<outProdTiles.size(); i++) {
                ndviList.append(outProdParams[i].ndvi);
                laiList.append(outProdParams[i].laiMonoDate);
                laiErrList.append(outProdParams[i].laiMonoDateErr);
                laiFlgsList.append(outProdParams[i].laiMonoDateFlgs);
                laiMonoProductFormatterTask.parentTasks.append(outProdParams[i].parentsTasksRef);
            }
            SubmitTasks(ctx, event.jobId, {laiMonoProductFormatterTask});
            QStringList productFormatterArgs = GetLaiMonoProductFormatterArgs(
                        laiMonoProductFormatterTask, ctx, event, outProdTilesMetaFiles,
                        outProdTiles, ndviList, laiList, laiErrList, laiFlgsList);
            allSteps.append(laiMonoProductFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
            noValidMonoLai = false;
        }
        if(noValidMonoLai) {
            ctx.MarkJobFailed(event.jobId);
            throw std::runtime_error(
                QStringLiteral("Cannot process job - inconsistent tiles - product configuration (this is likely a bug)")
                        .toStdString());
        }
    }

    // Create the product formatter tasks for the Reprocessed and/or Fitted Products (if needed)
    if(bNDayReproc) {
        allTasksList.append({"lai-reproc-product-formatter", {}});
        TaskToSubmit &laiReprocProductFormatterTask = allTasksList[allTasksList.size()-1];
        for(LAIProductFormatterParams params: listParams) {
            laiReprocProductFormatterTask.parentTasks.append(params.laiReprocParams.parentsTasksRef);
        }
        SubmitTasks(ctx, event.jobId, {laiReprocProductFormatterTask});
        QStringList productFormatterArgs = GetReprocProductFormatterArgs(laiReprocProductFormatterTask, ctx, event, listTilesMetaFiles,
                                                                         listParams, false);
        // add these steps to the steps list to be submitted
        allSteps.append(laiReprocProductFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
    }
    if(bFittedReproc) {
        allTasksList.append({"lai-fitted-product-formatter", {}});
        TaskToSubmit &laiFittedProductFormatterTask = allTasksList[allTasksList.size()-1];
        for(LAIProductFormatterParams params: listParams) {
            laiFittedProductFormatterTask.parentTasks.append(params.laiFitParams.parentsTasksRef);
        }
        SubmitTasks(ctx, event.jobId, {laiFittedProductFormatterTask});
        QStringList productFormatterArgs = GetReprocProductFormatterArgs(laiFittedProductFormatterTask, ctx, event, listTilesMetaFiles,
                                                                         listParams, true);
        // add these steps to the steps list to be submitted
        allSteps.append(laiFittedProductFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
    }

    QList<std::reference_wrapper<const TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    TaskToSubmit endOfJobDummyTask{"lai-end-of-job", {}};
    endOfJobDummyTask.parentTasks.append(allTasksListRef);
    SubmitTasks(ctx, event.jobId, {endOfJobDummyTask});
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
                                quicklook, footPrint, std::experimental::nullopt, TileIdList() });
            Logger::debug(QStringLiteral("InsertProduct for %1 returned %2").arg(prodName).arg(ret));

            // Now remove the job folder containing temporary files
            RemoveJobFolder(ctx, event.jobId, "l3b");
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

QStringList LaiRetrievalHandler::GetCutImgArgs(const QString &shapePath, const QString &inFile, const QString &outFile) {
    return { "-dstnodata", "0", "-overwrite",
             "-cutline", shapePath,
             "-crop_to_cutline",
             inFile,
             outFile
           };
}

QStringList LaiRetrievalHandler::GetCompressImgArgs(const QString &inFile, const QString &outFile) {
    return { "-in", inFile,
             "-out", "\"" + outFile+"?gdal:co:COMPRESS=DEFLATE\""
           };
}

QStringList LaiRetrievalHandler::GetNdviRviExtractionArgs(const QString &inputProduct, const QString &msksFlagsFile, const QString &ftsFile,
                                                          const QString &ndviFile, const QString &resolution) {
    return { "NdviRviExtraction2",
           "-xml", inputProduct,
           "-msks", msksFlagsFile,
           "-ndvi", ndviFile,
           "-fts", ftsFile,
           "-outres", resolution
    };
}

QStringList LaiRetrievalHandler::GetBvImageInvArgs(const QString &ftsFile, const QString &msksFlagsFile, const QString &xmlFile,
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

QStringList LaiRetrievalHandler::GetBvErrImageInvArgs(const QString &ftsFile, const QString &msksFlagsFile, const QString &xmlFile,
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

QStringList LaiRetrievalHandler::GetQuantifyImageArgs(const QString &inFileName, const QString &outFileName)  {
    return { "QuantifyImage",
        "-in", inFileName,
        "-out", "\"" + outFileName+"?gdal:co:COMPRESS=DEFLATE\""
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

    const auto &lutFile = configParameters["processor.l3b.lai.lut_path"];

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

QStringList LaiRetrievalHandler::GetReprocProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
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
