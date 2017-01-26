#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "compositehandler.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"
#include "logger.hpp"

void CompositeHandler::CreateTasksForNewProducts(const CompositeJobConfig &cfg, QList<TaskToSubmit> &outAllTasksList,
                                                QList<std::reference_wrapper<const TaskToSubmit>> &outProdFormatterParentsList,
                                                const TileTemporalFilesInfo &tileTemporalFilesInfo)
{
    bool bFootprintTaskPresent = false;
    // if we have multiple satellites and we don't have yet the shape file for this tile, then we must
    // create the footprint for the primary satellite ID
    if(tileTemporalFilesInfo.uniqueSatteliteIds.size() > 1 && tileTemporalFilesInfo.shapePath == "") {
        // add the task for creating the footprint
        outAllTasksList.append(TaskToSubmit{ "composite-create-footprint", {} });
        bFootprintTaskPresent = true;
    }

    int nbProducts = tileTemporalFilesInfo.temporalTilesFileInfos.size();
    // just create the tasks but with no information so far
    for (int i = 0; i < nbProducts; i++) {
        outAllTasksList.append(TaskToSubmit{ "composite-mask-handler", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-preprocessing", {} });

        // if it is a secondary satellite, then we must cut the masks and the image with
        // gdalwarp according to the primary satellite shape
/*        if(tileTemporalFilesInfo.temporalTilesFileInfos[i].satId != tileTemporalFilesInfo.primarySatelliteId) {
            outAllTasksList.append(TaskToSubmit{ "gdalwarp", {} });
            outAllTasksList.append(TaskToSubmit{ "gdalwarp", {} });
            outAllTasksList.append(TaskToSubmit{ "gdalwarp", {} });
            outAllTasksList.append(TaskToSubmit{ "gdalwarp", {} });
            outAllTasksList.append(TaskToSubmit{ "gdalwarp", {} });
        }
*/
        outAllTasksList.append(TaskToSubmit{ "composite-weight-aot", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-weight-on-clouds", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-total-weight", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-update-synthesis", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-splitter", {} });
        if(!cfg.keepJobFiles) {
            outAllTasksList.append(TaskToSubmit{ "files-remover", {} });
        }
    }

    // now fill the tasks hierarchy infos
    int i;
    int nCurTaskIdx = 0;

    for (i = 0; i < nbProducts; i++) {
        if (i > 0) {
            // update the mask handler with the reference of the previous composite splitter
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
            nCurTaskIdx++;
        } else {
            nCurTaskIdx++;
            // if it is the case, add the CreateFootprint as parent to first maskhandler
            if(bFootprintTaskPresent) {
                outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
                nCurTaskIdx++;
            }
        }
        // the others comme naturally updated
        // composite-preprocessing -> mask-handler
        outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
        nCurTaskIdx++;

        // check if we need to cut (according to primary shape) the product components if we have secondary satellite product
/*        if(tileTemporalFilesInfo.temporalTilesFileInfos[i].satId != tileTemporalFilesInfo.primarySatelliteId) {
            // launch in parallel all the cutting gdalwarp tasks
            int prevTaskId = nCurTaskIdx-1;
            int imgCutIdx = nCurTaskIdx++;
            outAllTasksList[imgCutIdx].parentTasks.append(outAllTasksList[prevTaskId]);
            int aotCutIdx = nCurTaskIdx++;
            outAllTasksList[aotCutIdx].parentTasks.append(outAllTasksList[prevTaskId]);
            int cldCutIdx = nCurTaskIdx++;
            outAllTasksList[cldCutIdx].parentTasks.append(outAllTasksList[prevTaskId]);
            int watCutIdx = nCurTaskIdx++;
            outAllTasksList[watCutIdx].parentTasks.append(outAllTasksList[prevTaskId]);
            int snowCutIdx = nCurTaskIdx++;
            outAllTasksList[snowCutIdx].parentTasks.append(outAllTasksList[prevTaskId]);

            // weigh-aot -> composite-cut-aot
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[aotCutIdx]);
            nCurTaskIdx++;
            // weigh-on-clouds -> composite-cut-clouds
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[cldCutIdx]);
            nCurTaskIdx++;

            // total-weight -> weigh-aot and weigh-on-clouds
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-2]);
            nCurTaskIdx++;

            // update-synthesis -> total-weight & composite-cut-img & composite-cut-snow & composite-cut-wat
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[imgCutIdx]);
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[watCutIdx]);
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[snowCutIdx]);
            nCurTaskIdx++;
        } else {
*/            // this is the case when we need to do no cutting (we are either primary satellite or we have only one satellite)
            // weigh-aot -> composite-preprocessing
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
            nCurTaskIdx++;
            // weigh-on-clouds -> composite-preprocessing
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-2]);
            nCurTaskIdx++;
            // total-weight -> weigh-aot and weigh-on-clouds
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-2]);
            nCurTaskIdx++;
            // update-synthesis -> total-weight
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
            nCurTaskIdx++;
//        }
        // composite-splitter -> update-synthesis
        outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
        nCurTaskIdx++;
        if(!cfg.keepJobFiles) {
            // cleanup-intermediate-files -> composite-splitter
            outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
            nCurTaskIdx++;
        }

    }
    // product-formatter -> the last composite-splitter
    outProdFormatterParentsList.append(outAllTasksList[outAllTasksList.size() - 1]);
}

void CompositeHandler::HandleNewTilesList(EventProcessingContext &ctx,
                                          const CompositeJobConfig &cfg,
                                          const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                          CompositeGlobalExecutionInfos &globalExecInfos,
                                          int resolution)
{
    QStringList listProducts = ProcessorHandlerHelper::GetTemporalTileFiles(tileTemporalFilesInfo);
    QString bandsMapping = DeductBandsMappingFile(listProducts, cfg.bandsMapping, resolution);
    const auto &resolutionStr = QString::number(resolution);
    QString scatCoeffs = ((resolution == 10) ? cfg.scatCoeffs10M : cfg.scatCoeffs20M);

    QList<TaskToSubmit> &allTasksList = globalExecInfos.allTasksList;
    QList<std::reference_wrapper<const TaskToSubmit>> &prodFormParTsksList = globalExecInfos.prodFormatParams.parentsTasksRef;
    CreateTasksForNewProducts(cfg, allTasksList, prodFormParTsksList, tileTemporalFilesInfo);

    NewStepList &steps = globalExecInfos.allStepsList;
    int nCurTaskIdx = 0;

    QString shapePath = tileTemporalFilesInfo.shapePath;
    QString primaryTileMetadata;
    for(int i = 0; i<tileTemporalFilesInfo.temporalTilesFileInfos.size(); i++) {
        if(tileTemporalFilesInfo.temporalTilesFileInfos[i].satId == tileTemporalFilesInfo.primarySatelliteId) {
            primaryTileMetadata = tileTemporalFilesInfo.temporalTilesFileInfos[i].file;
            break;
        }
    }
    // if we have multiple satellites and we don't have yet the shape file for this tile, then we must
    // create the footprint for the primary satellite ID
    if(tileTemporalFilesInfo.uniqueSatteliteIds.size() > 1 && tileTemporalFilesInfo.shapePath == "") {
        // CreateFootprint should execute on the first primary product metadatafrom the tile
        TaskToSubmit &createFootprintTaskHandler = allTasksList[nCurTaskIdx++];
        SubmitTasks(ctx, cfg.jobId, {createFootprintTaskHandler});
        shapePath = ProcessorHandlerHelper::BuildShapeName(cfg.shapeFilesFolder, tileTemporalFilesInfo.tileId,
                                                           cfg.jobId, createFootprintTaskHandler.taskId);
        QStringList createFootprintArgs = { "CreateFootprint", "-in", primaryTileMetadata,
                                            "-mode", "metadata", "-out", shapePath};
        steps.append(createFootprintTaskHandler.CreateStep("CreateFootprint", createFootprintArgs));
    }

    QString prevL3AProdRefls;
    QString prevL3AProdWeights;
    QString prevL3AProdFlags;
    QString prevL3AProdDates;
    QString prevL3ARgbFile;
    for (int i = 0; i < listProducts.size(); i++) {
        QStringList cleanupTemporaryFilesList;// = {"-f"};
        const auto &inputProduct = listProducts[i];
        // Mask Handler Step
        TaskToSubmit &maskHandler = allTasksList[nCurTaskIdx++];
        SubmitTasks(ctx, cfg.jobId, {maskHandler});
        const auto &masksFile = maskHandler.GetFilePath("all_masks_file.tif");
        QStringList maskHandlerArgs = { "MaskHandler", "-xml",         inputProduct, "-out",
                                        masksFile,     "-sentinelres", resolutionStr };
        steps.append(maskHandler.CreateStep("MaskHandler", maskHandlerArgs));
        cleanupTemporaryFilesList.append(masksFile);

        TaskToSubmit &compositePreprocessing = allTasksList[nCurTaskIdx++];
        SubmitTasks(ctx, cfg.jobId, {compositePreprocessing});
        // Composite preprocessing Step
        auto outResImgBands = compositePreprocessing.GetFilePath("img_res_bands.tif");
        auto cldResImg = compositePreprocessing.GetFilePath("cld_res.tif");
        auto waterResImg = compositePreprocessing.GetFilePath("water_res.tif");
        auto snowResImg = compositePreprocessing.GetFilePath("snow_res.tif");
        auto aotResImg = compositePreprocessing.GetFilePath("aot_res.tif");
        QStringList compositePreprocessingArgs = { "CompositePreprocessing2", "-xml", inputProduct,
                                                   "-bmap", bandsMapping, "-res", resolutionStr,
                                                   "-msk", masksFile, "-outres", outResImgBands,
                                                   "-outcmres", cldResImg, "-outwmres", waterResImg,
                                                   "-outsmres", snowResImg, "-outaotres", aotResImg };

        cleanupTemporaryFilesList.append(outResImgBands);
        cleanupTemporaryFilesList.append(cldResImg);
        cleanupTemporaryFilesList.append(waterResImg);
        cleanupTemporaryFilesList.append(snowResImg);
        cleanupTemporaryFilesList.append(aotResImg);

        if(scatCoeffs.length() > 0) {
            compositePreprocessingArgs.append("-scatcoef");
            compositePreprocessingArgs.append(scatCoeffs);
        }
        if((primaryTileMetadata != "") &&
           (tileTemporalFilesInfo.temporalTilesFileInfos[i].satId != tileTemporalFilesInfo.primarySatelliteId)) {
            compositePreprocessingArgs.append("-pmxml");
            compositePreprocessingArgs.append(primaryTileMetadata);
        }
        steps.append(compositePreprocessing.CreateStep("CompositePreprocessing", compositePreprocessingArgs));

/*        if(tileTemporalFilesInfo.temporalTilesFileInfos[i].satId != tileTemporalFilesInfo.primarySatelliteId) {
            TaskToSubmit &cutImgTask = allTasksList[nCurTaskIdx++];
            SubmitTasks(ctx, cfg.jobId, {cutImgTask});

            const auto &cutImgFile = cutImgTask.GetFilePath("img_res_bands_clipped.tif");
            QStringList gdalWarpArgs = GetGdalWarpArgs(outResImgBands, cutImgFile, "-10000", "2048", shapePath, resolutionStr);
            steps.append(cutImgTask.CreateStep("gdalwarp-img", gdalWarpArgs));
            outResImgBands = cutImgFile;
            cleanupTemporaryFilesList.append(cutImgFile);

            TaskToSubmit &cutAotTask = allTasksList[nCurTaskIdx++];
            SubmitTasks(ctx, cfg.jobId, {cutAotTask});
            const auto &cutAotFile = cutAotTask.GetFilePath("aot_res_clipped.tif");
            QStringList gdalWarpAotArgs = GetGdalWarpArgs(aotResImg, cutAotFile, "-10000", "2048", shapePath, resolutionStr);
            steps.append(cutAotTask.CreateStep("gdalwarp-aot", gdalWarpAotArgs));
            aotResImg = cutAotFile;
            cleanupTemporaryFilesList.append(cutAotFile);

            TaskToSubmit &cutCldTask = allTasksList[nCurTaskIdx++];
            SubmitTasks(ctx, cfg.jobId, {cutCldTask});
            const auto &cutCldFile = cutCldTask.GetFilePath("cld_res_clipped.tif");
            QStringList gdalWarpCldArgs = GetGdalWarpArgs(cldResImg, cutCldFile, "-10000", "2048", shapePath, resolutionStr);
            steps.append(cutCldTask.CreateStep("gdalwarp-cld", gdalWarpCldArgs));
            cldResImg = cutCldFile;
            cleanupTemporaryFilesList.append(cutCldFile);

            TaskToSubmit &cutWatTask = allTasksList[nCurTaskIdx++];
            SubmitTasks(ctx, cfg.jobId, {cutWatTask});
            const auto &cutWatFile = cutWatTask.GetFilePath("water_res_clipped.tif");
            QStringList gdalWarpWatArgs = GetGdalWarpArgs(waterResImg, cutWatFile, "-10000", "2048", shapePath, resolutionStr);
            steps.append(cutWatTask.CreateStep("gdalwarp-wat", gdalWarpWatArgs));
            waterResImg = cutWatFile;
            cleanupTemporaryFilesList.append(cutWatFile);

            TaskToSubmit &cutSnowTask = allTasksList[nCurTaskIdx++];
            SubmitTasks(ctx, cfg.jobId, {cutSnowTask});
            const auto &cutSnowFile = cutSnowTask.GetFilePath("snow_res_clipped.tif");
            QStringList gdalWarpSnowArgs = GetGdalWarpArgs(snowResImg, cutSnowFile, "-10000", "2048", shapePath, resolutionStr);
            steps.append(cutSnowTask.CreateStep("gdalwarp-snow", gdalWarpSnowArgs));
            snowResImg = cutSnowFile;
            cleanupTemporaryFilesList.append(cutSnowFile);
        }
*/
        TaskToSubmit &weightAot = allTasksList[nCurTaskIdx++];
        SubmitTasks(ctx, cfg.jobId, {weightAot});
        TaskToSubmit &weightOnClouds = allTasksList[nCurTaskIdx++];
        SubmitTasks(ctx, cfg.jobId, {weightOnClouds});
        TaskToSubmit &totalWeight = allTasksList[nCurTaskIdx++];
        SubmitTasks(ctx, cfg.jobId, {totalWeight});
        TaskToSubmit &updateSynthesis = allTasksList[nCurTaskIdx++];
        SubmitTasks(ctx, cfg.jobId, {updateSynthesis});
        TaskToSubmit &compositeSplitter = allTasksList[nCurTaskIdx++];
        SubmitTasks(ctx, cfg.jobId, {compositeSplitter});

        // Weight AOT Step
        const auto &outWeightAotFile = weightAot.GetFilePath("weight_aot.tif");
        QStringList weightAotArgs = { "WeightAOT",     "-xml",     inputProduct, "-in",
                                      aotResImg,       "-waotmin", cfg.weightAOTMin, "-waotmax",
                                      cfg.weightAOTMax,    "-aotmax",  cfg.AOTMax,       "-out",
                                      outWeightAotFile };
        steps.append(weightAot.CreateStep("WeightAOT", weightAotArgs));
        cleanupTemporaryFilesList.append(outWeightAotFile);

        // Weight on clouds Step
        const auto &outWeightCldFile = weightOnClouds.GetFilePath("weight_cloud.tif");
        QStringList weightOnCloudArgs = { "WeightOnClouds", "-inxml",         inputProduct,
                                          "-incldmsk",      cldResImg,        "-coarseres",
                                          cfg.coarseRes,        "-sigmasmallcld", cfg.sigmaSmallCloud,
                                          "-sigmalargecld", cfg.sigmaLargeCloud,  "-out", outWeightCldFile };
        steps.append(weightOnClouds.CreateStep("WeightOnClouds", weightOnCloudArgs));
        cleanupTemporaryFilesList.append(outWeightCldFile);

        // Total weight Step
        const auto &outTotalWeighFile = totalWeight.GetFilePath("weight_total.tif");
        QStringList totalWeightArgs = { "TotalWeight",    "-xml",           inputProduct,
                                        "-waotfile",      outWeightAotFile, "-wcldfile",
                                        outWeightCldFile, "-l3adate",       cfg.l3aSynthesisDate,
                                        "-halfsynthesis", cfg.synthalf,         "-wdatemin",
                                        cfg.weightDateMin,    "-out",           outTotalWeighFile };
        steps.append(totalWeight.CreateStep("TotalWeight", totalWeightArgs));
        cleanupTemporaryFilesList.append(outTotalWeighFile);

        // Update Synthesis Step
        const auto &outL3AResultFile = updateSynthesis.GetFilePath("L3AResult.tif");
        QStringList updateSynthesisArgs = { "UpdateSynthesis", "-in",   outResImgBands,    "-bmap",
                                            bandsMapping,      "-xml",  inputProduct,      "-csm",
                                            cldResImg,         "-wm",   waterResImg,       "-sm",
                                            snowResImg,        "-wl2a", outTotalWeighFile, "-out",
                                            outL3AResultFile };
        if (i > 0) {
            updateSynthesisArgs.append("-prevl3aw");
            updateSynthesisArgs.append(prevL3AProdWeights);
            updateSynthesisArgs.append("-prevl3ad");
            updateSynthesisArgs.append(prevL3AProdDates);
            updateSynthesisArgs.append("-prevl3ar");
            updateSynthesisArgs.append(prevL3AProdRefls);
            updateSynthesisArgs.append("-prevl3af");
            updateSynthesisArgs.append(prevL3AProdFlags);
            // remove at this step the previous files
            cleanupTemporaryFilesList.append(prevL3AProdWeights);
            cleanupTemporaryFilesList.append(prevL3AProdDates);
            cleanupTemporaryFilesList.append(prevL3AProdRefls);
            cleanupTemporaryFilesList.append(prevL3AProdFlags);
            // normally this will not be created in intermedaiate steps
            //cleanupTemporaryFilesList.append(prevL3ARgbFile);
        }
        steps.append(updateSynthesis.CreateStep("UpdateSynthesis", updateSynthesisArgs));
        cleanupTemporaryFilesList.append(outL3AResultFile);

        // Composite Splitter Step
        const auto &outL3AResultReflsFile = compositeSplitter.GetFilePath("L3AResult_refls.tif");
        const auto &outL3AResultWeightsFile = compositeSplitter.GetFilePath("L3AResult_weights.tif");
        const auto &outL3AResultFlagsFile = compositeSplitter.GetFilePath("L3AResult_flags.tif");
        const auto &outL3AResultDatesFile = compositeSplitter.GetFilePath("L3AResult_dates.tif");
        const auto &outL3AResultRgbFile = compositeSplitter.GetFilePath("L3AResult_rgb.tif");
        bool isLastProduct = (i == (listProducts.size() - 1));
        QStringList compositeSplitterArgs = { "CompositeSplitter2",
                                              "-in", outL3AResultFile, "-xml", inputProduct, "-bmap", bandsMapping,
                                              "-outweights", (isLastProduct ? ("\"" + outL3AResultWeightsFile+"?gdal:co:COMPRESS=DEFLATE\"") : outL3AResultWeightsFile),
                                              "-outdates", (isLastProduct ? ("\"" + outL3AResultDatesFile+"?gdal:co:COMPRESS=DEFLATE\"") : outL3AResultDatesFile),
                                              "-outrefls", (isLastProduct ? ("\"" + outL3AResultReflsFile+"?gdal:co:COMPRESS=DEFLATE\"") : outL3AResultReflsFile),
                                              "-outflags", (isLastProduct ? ("\"" + outL3AResultFlagsFile+"?gdal:co:COMPRESS=DEFLATE\"") : outL3AResultFlagsFile),
                                              "-isfinal", (isLastProduct ? "1" : "0")
                                            };
        // we need to create the rgb file only if the last product
        if(isLastProduct) {
            compositeSplitterArgs.append("-outrgb");
            compositeSplitterArgs.append("\"" + outL3AResultRgbFile+"?gdal:co:COMPRESS=DEFLATE\"");
        }
        steps.append(compositeSplitter.CreateStep("CompositeSplitter", compositeSplitterArgs));

        // save the created L3A product file for the next product creation
        prevL3AProdRefls = outL3AResultReflsFile;
        prevL3AProdWeights = outL3AResultWeightsFile;
        prevL3AProdFlags = outL3AResultFlagsFile;
        prevL3AProdDates = outL3AResultDatesFile;
        prevL3ARgbFile = outL3AResultRgbFile;

        if(!cfg.keepJobFiles) {
            TaskToSubmit &cleanupTemporaryFiles = allTasksList[nCurTaskIdx++];
            SubmitTasks(ctx, cfg.jobId, {cleanupTemporaryFiles});
            steps.append(cleanupTemporaryFiles.CreateStep("CleanupTemporaryFiles", cleanupTemporaryFilesList));
        }
    }

    CompositeProductFormatterParams &productFormatterParams = globalExecInfos.prodFormatParams;
    productFormatterParams.prevL3AProdRefls = prevL3AProdRefls;
    productFormatterParams.prevL3AProdWeights = prevL3AProdWeights;
    productFormatterParams.prevL3AProdFlags = prevL3AProdFlags;
    productFormatterParams.prevL3AProdDates = prevL3AProdDates;
    productFormatterParams.prevL3ARgbFile = prevL3ARgbFile;
}

QStringList CompositeHandler::GetGdalWarpArgs(const QString &inImg, const QString &outImg, const QString &dtsNoData,
                                             const QString &gdalwarpMem, const QString &shape, const QString &resolutionStr) {
    QStringList retList = { "-dstnodata", dtsNoData, "-overwrite", "-cutline", shape,
             "-multi", "-wm", gdalwarpMem, "-crop_to_cutline", inImg, outImg };
    if(!resolutionStr.isEmpty()) {
        retList.append("-tr");
        retList.append(resolutionStr);
        retList.append(resolutionStr);
    }

    return retList;
}


void CompositeHandler::WriteExecutionInfosFile(const QString &executionInfosPath,
                                               const CompositeJobConfig &cfg,
                                               const QStringList &listProducts)
{
    std::ofstream executionInfosFile;
    try {
        executionInfosFile.open(executionInfosPath.toStdString().c_str(), std::ofstream::out);
        executionInfosFile << "<?xml version=\"1.0\" ?>" << std::endl;
        executionInfosFile << "<metadata>" << std::endl;
        executionInfosFile << "  <General>" << std::endl;
        executionInfosFile << "    <bands_mapping_file>" << cfg.bandsMapping.toStdString()
                           << "</bands_mapping_file>" << std::endl;
        executionInfosFile << "    <scattering_coefficients_10M_file>" << cfg.scatCoeffs10M.toStdString()
                           << "</scattering_coefficients_10M_file>" << std::endl;
        executionInfosFile << "    <scattering_coefficients_20M_file>" << cfg.scatCoeffs20M.toStdString()
                           << "</scattering_coefficients_20M_file>" << std::endl;
        executionInfosFile << "  </General>" << std::endl;

        executionInfosFile << "  <Weight_AOT>" << std::endl;
        executionInfosFile << "    <weight_aot_min>" << cfg.weightAOTMin.toStdString()
                           << "</weight_aot_min>" << std::endl;
        executionInfosFile << "    <weight_aot_max>" << cfg.weightAOTMax.toStdString()
                           << "</weight_aot_max>" << std::endl;
        executionInfosFile << "    <aot_max>" << cfg.AOTMax.toStdString() << "</aot_max>" << std::endl;
        executionInfosFile << "  </Weight_AOT>" << std::endl;

        executionInfosFile << "  <Weight_On_Clouds>" << std::endl;
        executionInfosFile << "    <coarse_res>" << cfg.coarseRes.toStdString() << "</coarse_res>"
                           << std::endl;
        executionInfosFile << "    <sigma_small_cloud>" << cfg.sigmaSmallCloud.toStdString()
                           << "</sigma_small_cloud>" << std::endl;
        executionInfosFile << "    <sigma_large_cloud>" << cfg.sigmaLargeCloud.toStdString()
                           << "</sigma_large_cloud>" << std::endl;
        executionInfosFile << "  </Weight_On_Clouds>" << std::endl;

        executionInfosFile << "  <Weight_On_Date>" << std::endl;
        executionInfosFile << "    <weight_date_min>" << cfg.weightDateMin.toStdString()
                           << "</weight_date_min>" << std::endl;
        executionInfosFile << "    <l3a_product_date>" << cfg.l3aSynthesisDate.toStdString()
                           << "</l3a_product_date>" << std::endl;
        executionInfosFile << "    <half_synthesis>" << cfg.synthalf.toStdString()
                           << "</half_synthesis>" << std::endl;
        executionInfosFile << "  </Weight_On_Date>" << std::endl;

        executionInfosFile << "  <Dates_information>" << std::endl;
        // TODO: We should get these infos somehow but without parsing here anything
        // executionInfosFile << "    <start_date>" << 2013031 << "</start_date>" << std::endl;
        // executionInfosFile << "    <end_date>" << 20130422 << "</end_date>" << std::endl;
        executionInfosFile << "    <synthesis_date>" << cfg.l3aSynthesisDate.toStdString()
                           << "</synthesis_date>" << std::endl;
        executionInfosFile << "    <synthesis_half>" << cfg.synthalf.toStdString()
                           << "</synthesis_half>" << std::endl;
        executionInfosFile << "  </Dates_information>" << std::endl;

        executionInfosFile << "  <XML_files>" << std::endl;
        for (int i = 0; i < listProducts.size(); i++) {
            executionInfosFile << "    <XML_" << std::to_string(i) << ">"
                               << listProducts[i].toStdString() << "</XML_" << std::to_string(i)
                               << ">" << std::endl;
        }
        executionInfosFile << "  </XML_files>" << std::endl;
        executionInfosFile << "</metadata>" << std::endl;
        executionInfosFile.close();
    } catch (...) {
    }
}

// This function removes the files from the list that are outside the synthesis interval
// and that should not be used in the composition
void CompositeHandler::FilterInputProducts(QStringList &listFiles,
                                           int productDate,
                                           int halfSynthesis)
{
    // TODO: we should extract here the date of the product to compare it with tha synthesis
    // interval
    Q_UNUSED(listFiles);
    Q_UNUSED(productDate);
    Q_UNUSED(halfSynthesis);
}

void CompositeHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                              const JobSubmittedEvent &event)
{
    QStringList listProducts = GetL2AInputProductsTiles(ctx, event);
    if(listProducts.size() == 0) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No products provided at input or no products available in the specified interval").
                    toStdString());
    }

    CompositeJobConfig cfg;
    GetJobConfig(ctx, event, cfg);

    int resolution = cfg.resolution;
    QMap<QString, TileTemporalFilesInfo> mapTiles = GroupTiles(ctx, event.siteId, event.jobId, listProducts,
                                                               ProductType::L2AProductTypeId);
    //ProcessorHandlerHelper::TrimLeftSecondarySatellite(listProducts, mapTiles);

    QList<CompositeProductFormatterParams> listParams;

    TaskToSubmit productFormatterTask{"product-formatter", {}};
    NewStepList allSteps;
    QList<CompositeGlobalExecutionInfos> listCompositeInfos;
    for(auto tileId : mapTiles.keys())
    {
        const TileTemporalFilesInfo &listTemporalTiles = mapTiles.value(tileId);
        int curRes = resolution;
        bool bHasS2 = listTemporalTiles.uniqueSatteliteIds.contains(ProcessorHandlerHelper::SATELLITE_ID_TYPE_S2);
        // if we have S2 maybe we want to create products only for 20m resolution

        for(int i = 0; i<2; i++)  {
            if(i == 0) {
                if(bHasS2 && resolution != 20) { curRes = 10;}  // if S2 and resolution not 20m, force it to 10m
            } else {
                // if we are at the second iteration, check if we have S2, if we should generate for 20m and if we had previously 10m
                if(cfg.bGenerate20MS2Res && bHasS2 && resolution != 20) { curRes = 20;}
                else { break;}  // exit the loop and do not execute anymore the second step
            }
            listCompositeInfos.append(CompositeGlobalExecutionInfos());
            CompositeGlobalExecutionInfos &infos = listCompositeInfos[listCompositeInfos.size()-1];
            infos.prodFormatParams.tileId = GetProductFormatterTile(tileId);
            HandleNewTilesList(ctx, cfg, listTemporalTiles, infos, curRes);
            listParams.append(infos.prodFormatParams);
            productFormatterTask.parentTasks += infos.prodFormatParams.parentsTasksRef;
            allSteps.append(infos.allStepsList);
        }
    }

    SubmitTasks(ctx, event.jobId, {productFormatterTask});

    // finally format the product
    QStringList productFormatterArgs = GetProductFormatterArgs(productFormatterTask, ctx, cfg, listProducts, listParams);

    // add these steps to the steps list to be submitted
    allSteps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
    ctx.SubmitSteps(allSteps);
}

void CompositeHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                              const TaskFinishedEvent &event)
{
    if (event.module == "product-formatter") {
        QString prodName = GetProductFormatterProductName(ctx, event);
        QString productFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId) + "/" + prodName;
        if(prodName != "" && ProcessorHandlerHelper::IsValidHighLevelProduct(productFolder)) {
            // mark the job as finished
            ctx.MarkJobFinished(event.jobId);

            QString quicklook = GetProductFormatterQuicklook(ctx, event);
            QString footPrint = GetProductFormatterFootprint(ctx, event);
            // Insert the product into the database
            QDateTime minDate, maxDate;
            const QStringList &prodTiles = ProcessorHandlerHelper::GetTileIdsFromHighLevelProduct(productFolder);
            ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prodName, minDate, maxDate);
            ctx.InsertProduct({ ProductType::L3AProductTypeId, event.processorId, event.siteId,
                                event.jobId, productFolder, maxDate, prodName, quicklook,
                                footPrint, std::experimental::nullopt, prodTiles });
        } else {
            // mark the job as failed
            ctx.MarkJobFailed(event.jobId);
            Logger::error(QStringLiteral("Cannot insert into database the product with name %1 and folder %2").arg(prodName).arg(productFolder));
        }
        // Now remove the job folder containing temporary files
        RemoveJobFolder(ctx, event.jobId, "l3a");
    }
}

void CompositeHandler::GetJobConfig(EventProcessingContext &ctx,const JobSubmittedEvent &event,CompositeJobConfig &cfg) {
    auto configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3a.");
    std::map<QString, QString> executorConfigParameters = ctx.GetJobConfigurationParameters(event.jobId, "executor.shapes_dir");
    auto execProcConfigParameters = ctx.GetJobConfigurationParameters(event.jobId, "executor.processor.l3a.keep_job_folders");
    //auto resourceParameters = ctx.GetJobConfigurationParameters(event.jobId, "resources.working-mem");
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    cfg.jobId = event.jobId;
    cfg.siteId = event.siteId;
    cfg.resolution = 0;
    if(!GetParameterValueAsInt(parameters, "resolution", cfg.resolution) ||
            cfg.resolution == 0) {
        cfg.resolution = 10;
    }

    const QString &generate20MS2ResStr = configParameters["processor.l3a.generate_20m_s2_resolution"];
    cfg.bGenerate20MS2Res = (generate20MS2ResStr.toInt() != 0);


    cfg.l3aSynthesisDate = parameters["synthesis_date"].toString();
    cfg.synthalf = parameters["half_synthesis"].toString();

    // Get the parameters from the configuration
    // Get the Half Synthesis interval value if it was not specified by the user
    if(cfg.synthalf.length() == 0) {
        cfg.synthalf = configParameters["processor.l3a.half_synthesis"];
        if(cfg.synthalf.length() == 0) {
            cfg.synthalf = "15";
        }
    }
    cfg.lutPath = configParameters["processor.l3a.lut_path"];

    cfg.bandsMapping = configParameters["processor.l3a.bandsmapping"];
    cfg.scatCoeffs10M = configParameters["processor.l3a.preproc.scatcoeffs_10m"];
    cfg.scatCoeffs20M = configParameters["processor.l3a.preproc.scatcoeffs_20m"];
    cfg.weightAOTMin = configParameters["processor.l3a.weight.aot.minweight"];
    cfg.weightAOTMax = configParameters["processor.l3a.weight.aot.maxweight"];
    cfg.AOTMax = configParameters["processor.l3a.weight.aot.maxaot"];
    cfg.coarseRes = configParameters["processor.l3a.weight.cloud.coarseresolution"];
    cfg.sigmaSmallCloud = configParameters["processor.l3a.weight.cloud.sigmasmall"];
    cfg.sigmaLargeCloud = configParameters["processor.l3a.weight.cloud.sigmalarge"];
    cfg.weightDateMin = configParameters["processor.l3a.weight.total.weightdatemin"];

    cfg.shapeFilesFolder = executorConfigParameters["executor.shapes_dir"];

    // by default, do not keep job files
    cfg.keepJobFiles = false;
    auto keepStr = execProcConfigParameters["executor.processor.l3a.keep_job_folders"];
    if(keepStr == "1") cfg.keepJobFiles = true;
}

QStringList CompositeHandler::GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const CompositeJobConfig &cfg,
                                    const QStringList &listProducts, const QList<CompositeProductFormatterParams> &productParams) {

    const auto &targetFolder = GetFinalProductFolder(ctx, cfg.jobId, cfg.siteId);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.xml");
    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);

    WriteExecutionInfosFile(executionInfosPath, cfg, listProducts);

    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot", targetFolder,
                                         "-fileclass", "SVT1",
                                         "-level", "L3A",
                                         "-timeperiod", cfg.l3aSynthesisDate,
                                         "-baseline", "01.00",
                                         "-siteid", QString::number(cfg.siteId),
                                         "-processor", "composite",
                                         "-gipp", executionInfosPath,
                                         "-outprops", outPropsPath};
    productFormatterArgs += "-il";
    productFormatterArgs += listProducts[listProducts.size() - 1];

    if(cfg.lutPath.size() > 0) {
        productFormatterArgs += "-lut";
        productFormatterArgs += cfg.lutPath;
    }

    productFormatterArgs += "-processor.composite.refls";
    for(const CompositeProductFormatterParams &params: productParams) {
        productFormatterArgs += GetProductFormatterTile(params.tileId);
        productFormatterArgs += params.prevL3AProdRefls;
    }
    productFormatterArgs += "-processor.composite.weights";
    for(const CompositeProductFormatterParams &params: productParams) {
        productFormatterArgs += GetProductFormatterTile(params.tileId);
        productFormatterArgs += params.prevL3AProdWeights;
    }
    productFormatterArgs += "-processor.composite.flags";
    for(const CompositeProductFormatterParams &params: productParams) {
        productFormatterArgs += GetProductFormatterTile(params.tileId);
        productFormatterArgs += params.prevL3AProdFlags;
    }
    productFormatterArgs += "-processor.composite.dates";
    for(const CompositeProductFormatterParams &params: productParams) {
        productFormatterArgs += GetProductFormatterTile(params.tileId);
        productFormatterArgs += params.prevL3AProdDates;
    }
    productFormatterArgs += "-processor.composite.rgb";
    for(const CompositeProductFormatterParams &params: productParams) {
        productFormatterArgs += GetProductFormatterTile(params.tileId);
        productFormatterArgs += params.prevL3ARgbFile;
    }

    return productFormatterArgs;
}


bool CompositeHandler::IsProductAcceptableForJob(int jobId, const ProductAvailableEvent &event)
{
    Q_UNUSED(jobId);
    Q_UNUSED(event);

    return false;
}

QStringList CompositeHandler::GetMissionsFromBandsMapping(const QString &bandsMappingFile) {
    // Normally, this is a small file
    QStringList listLines = ProcessorHandlerHelper::GetTextFileLines(bandsMappingFile);
    if(listLines.size() > 0) {
        // we get the first line only
        QString firstLine = listLines[0];
        return firstLine.split(",");
    }
    return QStringList();
}

QString CompositeHandler::DeductBandsMappingFile(const QStringList &listProducts,
                                                 const QString &bandsMappingFile, int &resolution) {
    QFileInfo fileInfo(bandsMappingFile);

    // by default, we consider this is a dir
    QString curBandsMappingPath = bandsMappingFile;
    if(!fileInfo.isDir())
        curBandsMappingPath = fileInfo.dir().absolutePath();
    QList<ProcessorHandlerHelper::L2ProductType> listUniqueProductTypes;
    for (int i = 0; i < listProducts.size(); i++) {
        ProcessorHandlerHelper::L2ProductType productType = ProcessorHandlerHelper::GetL2AProductTypeFromTile(listProducts[i]);
        if(!listUniqueProductTypes.contains(productType)) {
            listUniqueProductTypes.append(productType);
        }
    }
    int cntUniqueProdTypes = listUniqueProductTypes.size();
    if(cntUniqueProdTypes < 1 || cntUniqueProdTypes > 2 ) {
        return bandsMappingFile;
    }
    if(cntUniqueProdTypes == 1) {
        if(listUniqueProductTypes[0] == ProcessorHandlerHelper::L2_PRODUCT_TYPE_S2) {
            return (curBandsMappingPath + "/bands_mapping_s2.txt");
        }
        if(listUniqueProductTypes[0] == ProcessorHandlerHelper::L2_PRODUCT_TYPE_L8) {
            resolution = 30;
            return (curBandsMappingPath + "/bands_mapping_L8.txt");
        }
        if(listUniqueProductTypes[0] == ProcessorHandlerHelper::L2_PRODUCT_TYPE_SPOT4) {
            resolution = 20;
            return (curBandsMappingPath + "/bands_mapping_spot4.txt");
        }
        if(listUniqueProductTypes[0] == ProcessorHandlerHelper::L2_PRODUCT_TYPE_SPOT5) {
            resolution = 10;
            return (curBandsMappingPath + "/bands_mapping_spot5.txt");
        }
    } else {
        if(listUniqueProductTypes.contains(ProcessorHandlerHelper::L2_PRODUCT_TYPE_L8)) {
            if(listUniqueProductTypes.contains(ProcessorHandlerHelper::L2_PRODUCT_TYPE_S2)) {
                if((resolution != 10) && (resolution != 20))
                    resolution = 10;
                return (curBandsMappingPath + "/bands_mapping_s2_L8.txt");
            } else if(listUniqueProductTypes.contains(ProcessorHandlerHelper::L2_PRODUCT_TYPE_SPOT4)) {
                resolution = 10;
                return (curBandsMappingPath + "/bands_mapping_Spot4_L8.txt");
            } else if(listUniqueProductTypes.contains(ProcessorHandlerHelper::L2_PRODUCT_TYPE_SPOT5)) {
                resolution = 10;
                return (curBandsMappingPath + "/bands_mapping_Spot5_L8.txt");
            }
        }
    }
    return bandsMappingFile;
}


ProcessorJobDefinitionParams CompositeHandler::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
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
        Logger::debug(QStringLiteral("Scheduler L3A: Error getting season start dates for site %1 for scheduled date %2!")
                      .arg(siteId)
                      .arg(qScheduledDate.toString()));
        return params;
    }
    QDateTime limitDate = seasonEndDate.addMonths(2);
    if(qScheduledDate > limitDate) {
        Logger::debug(QStringLiteral("Scheduler L3A: Error scheduled date %1 greater than the limit date %2 for site %3!")
                      .arg(qScheduledDate.toString())
                      .arg(limitDate.toString())
                      .arg(siteId));
        return params;
    }

    ConfigurationParameterValueMap mapCfg = ctx.GetConfigurationParameters(QString("processor.l3a."), siteId, requestOverrideCfgValues);

    // we might have an offset in days from starting the downloading products to start the L3A production
    int startSeasonOffset = mapCfg["processor.l3a.start_season_offset"].value.toInt();
    seasonStartDate = seasonStartDate.addDays(startSeasonOffset);

    int halfSynthesis = mapCfg["processor.l3a.half_synthesis"].value.toInt();
    if(halfSynthesis == 0)
        halfSynthesis = 15;
    int synthDateOffset = mapCfg["processor.l3a.synth_date_sched_offset"].value.toInt();
    if(synthDateOffset == 0)
        synthDateOffset = 30;

    // compute the half synthesis date
    QDateTime halfSynthesisDate = qScheduledDate.addDays(-synthDateOffset);
    // compute the start and date time
    QDateTime startDate = halfSynthesisDate.addDays(-halfSynthesis);
    if(startDate < seasonStartDate) {
        startDate = seasonStartDate;
    }
    QDateTime endDate = halfSynthesisDate.addDays(halfSynthesis);

    params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    // Normally, we need at least 1 product available in order to be able to create a L3A product
    // but if we do not return here, the schedule block waiting for products (that might never happen)
    bool waitForAvailProcInputs = (mapCfg["processor.l3a.sched_wait_proc_inputs"].value.toInt() != 0);
    if((waitForAvailProcInputs == false) || (params.productList.size() > 0)) {
        params.isValid = true;
        params.jsonParameters = "{ \"synthesis_date\": \"" + halfSynthesisDate.toString("yyyyMMdd") + "\"}";
        Logger::debug(QStringLiteral("Executing scheduled job. Scheduler extracted for L3A a number "
                                     "of %1 products for site ID %2 with start date %3 and end date %4!")
                      .arg(params.productList.size())
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    } else {
        Logger::debug(QStringLiteral("Scheduled job for L3A and site ID %1 with start date %2 and end date %3 will not be executed (no products)!")
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    }

    return params;
}


