#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "compositehandler.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"

#define TasksNoPerProduct 7

void CompositeHandler::HandleProductAvailableImpl(EventProcessingContext &ctx,
                                                  const ProductAvailableEvent &event)
{
    Q_UNUSED(ctx);
    // in this moment, we do not handle the products as they appear but maybe later it can be useful
    // for some other processors. The code below can be used as an example.
    return;
    // TODO: Here we must get all job IDs for the current processor id
    std::vector<int> jobIds;

    // for each job ID, check if the job accepts the current product
    for (unsigned int i = 0; i < jobIds.size(); i++) {
        // Check if the product can be processed by the current job
        if (IsProductAcceptableForJob(jobIds[i], event)) {
            // create a list with products that will have in this case only one element
            QStringList listProducts;
            // Get the product path from the productId
            QString strProductPath; // TODO = ctx.GetProductPathFromId(event.productId);
            listProducts.append(strProductPath);
            // process the received L2A product in the current job
            //HandleNewProductInJob(ctx, jobIds[i], "", listProducts);
        } else {
            // if the product is not acceptable for the current job, it might be outside the
            // synthesis interval

            // TODO: If the product date is after the L3A date + HalfSynthesis interval, then we can
            // mark the job finished
            //      ==> Is this enough as condition to finish the composition?
            //      ==> If we can take into account also the current time + offset (days), how much
            //      should be that offset?
        }
    }
    // TODO: we must get the last created L3A product that fits the half synthesis (is inside the
    // synthesi interval) and job's L3A product date
    //      ==> The intermediate L3A products should be saved in DB or they should be kept in the
    //      job's temporary repository?
    //      ==> If not kept in the DB, then we should get the last created L3A product
    //      ==> Anyway, the L3A product should be specific to a job as an L3A intermediate file
    //      could be created with other parameters

    // TODO: After inserting a new product, we should delete the old L3A products
    // TODO: Maybe some internal status params should be added in DB!!!
    //      Additionally, keep the original event parameters that generated job creation!!!

    // PROBLEM: How to handle the case when a processing is in progress for one job and we receive a
    // new accepted
    // product with a date later than the current processing one? In this case, the processing
    // should be delayed until
    // the finishing of the current one.
    //      ==> Does the context handles the submitted steps sequentially???
    //      ==> What happens if we receive products with an older date than the current processed
    //      ones???
}

void CompositeHandler::CreateTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
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

    int nbProducts = tileTemporalFilesInfo.temporalTileFiles.size();
    // just create the tasks but with no information so far
    for (int i = 0; i < nbProducts; i++) {
        outAllTasksList.append(TaskToSubmit{ "composite-mask-handler", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-preprocessing", {} });

        // if it is a secondary satellite, then we must cut the masks and the image with
        // gdalwarp according to the primary satellite shape
        if(tileTemporalFilesInfo.satelliteIds[i] != tileTemporalFilesInfo.primarySatelliteId) {
            outAllTasksList.append(TaskToSubmit{ "gdalwarp", {} });
            outAllTasksList.append(TaskToSubmit{ "gdalwarp", {} });
            outAllTasksList.append(TaskToSubmit{ "gdalwarp", {} });
            outAllTasksList.append(TaskToSubmit{ "gdalwarp", {} });
            outAllTasksList.append(TaskToSubmit{ "gdalwarp", {} });
        }

        outAllTasksList.append(TaskToSubmit{ "composite-weight-aot", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-weight-on-clouds", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-total-weight", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-update-synthesis", {} });
        outAllTasksList.append(TaskToSubmit{ "composite-splitter", {} });
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
        if(tileTemporalFilesInfo.satelliteIds[i] != tileTemporalFilesInfo.primarySatelliteId) {
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
            // this is the case when we need to do no cutting (we are either primary satellite or we have only one satellite)
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
        }
        // composite-splitter -> update-synthesis
        outAllTasksList[nCurTaskIdx].parentTasks.append(outAllTasksList[nCurTaskIdx-1]);
        nCurTaskIdx++;
    }
    // product-formatter -> the last composite-splitter
    outProdFormatterParentsList.append(outAllTasksList[outAllTasksList.size() - 1]);
}

void CompositeHandler::HandleNewTilesList(EventProcessingContext &ctx,
                                          const JobSubmittedEvent &event,
                                          const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                          CompositeGlobalExecutionInfos &globalExecInfos)
{
    const QStringList &listProducts = tileTemporalFilesInfo.temporalTileFiles;
    int jobId = event.jobId;
    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters =
        ctx.GetJobConfigurationParameters(jobId, "processor.l3a.");

    // Get L3A Synthesis date
    const auto &l3aSynthesisDate = parameters["synthesis_date"].toString();
    auto synthalf = parameters["half_synthesis"].toString();

    int resolution = 0;
    if(!GetParameterValueAsInt(parameters, "resolution", resolution) ||
            resolution == 0) {
        resolution = 10;    // TODO: We should configure the default resolution in DB
    }

    // Get the parameters from the configuration
    // Get the Half Synthesis interval value if it was not specified by the user
    if(synthalf.length() == 0) {
        synthalf = configParameters["processor.l3a.half_synthesis"];
        if(synthalf.length() == 0) {
            synthalf = "15";
        }
    }
    auto bandsMapping = configParameters["processor.l3a.bandsmapping"];
    const auto &scatCoeffs = configParameters["processor.l3a.preproc.scatcoeffs"];

    const auto &weightAOTMin = configParameters["processor.l3a.weight.aot.minweight"];
    const auto &weightAOTMax = configParameters["processor.l3a.weight.aot.maxweight"];
    const auto &AOTMax = configParameters["processor.l3a.weight.aot.maxaot"];

    const auto &coarseRes = configParameters["processor.l3a.weight.cloud.coarseresolution"];
    const auto &sigmaSmallCloud = configParameters["processor.l3a.weight.cloud.sigmasmall"];
    const auto &sigmaLargeCloud = configParameters["processor.l3a.weight.cloud.sigmalarge"];

    const auto &weightDateMin = configParameters["processor.l3a.weight.total.weightdatemin"];

    bandsMapping = DeductBandsMappingFile(listProducts, bandsMapping, resolution);

    const auto &resolutionStr = QString::number(resolution);

    QList<TaskToSubmit> &allTasksList = globalExecInfos.allTasksList;
    QList<std::reference_wrapper<const TaskToSubmit>> &prodFormParTsksList = globalExecInfos.prodFormatParams.parentsTasksRef;
    CreateTasksForNewProducts(allTasksList, prodFormParTsksList, tileTemporalFilesInfo);

    NewStepList &steps = globalExecInfos.allStepsList;
    int nCurTaskIdx = 0;

    QString shapePath = tileTemporalFilesInfo.shapePath;
    // if we have multiple satellites and we don't have yet the shape file for this tile, then we must
    // create the footprint for the primary satellite ID
    if(tileTemporalFilesInfo.uniqueSatteliteIds.size() > 1 && tileTemporalFilesInfo.shapePath == "") {
        // CreateFootprint should execute on the first primary product metadatafrom the tile
        QString primaryTileMetadata;
        for(int i = 0; i<tileTemporalFilesInfo.temporalTileFiles.size(); i++) {
            if(tileTemporalFilesInfo.satelliteIds[i] == tileTemporalFilesInfo.primarySatelliteId) {
                primaryTileMetadata = tileTemporalFilesInfo.temporalTileFiles[i];
                break;
            }
        }
        TaskToSubmit &createFootprintTaskHandler = allTasksList[nCurTaskIdx++];
        ctx.SubmitTasks(jobId, {createFootprintTaskHandler});
        std::map<QString, QString> executorConfigParameters =
            ctx.GetJobConfigurationParameters(jobId, "executor.shapes_dir");
        QString shapeFilesFolder = executorConfigParameters["executor.shapes_dir"];
        shapePath = ProcessorHandlerHelper::BuildShapeName(shapeFilesFolder, tileTemporalFilesInfo.tileId,
                                                           event.jobId, createFootprintTaskHandler.taskId);
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
        const auto &inputProduct = listProducts[i];
        // Mask Handler Step
        TaskToSubmit &maskHandler = allTasksList[nCurTaskIdx++];
        ctx.SubmitTasks(jobId, {maskHandler});
        const auto &masksFile = maskHandler.GetFilePath("all_masks_file.tif");
        QStringList maskHandlerArgs = { "MaskHandler", "-xml",         inputProduct, "-out",
                                        masksFile,     "-sentinelres", resolutionStr };
        steps.append(maskHandler.CreateStep("MaskHandler", maskHandlerArgs));

        TaskToSubmit &compositePreprocessing = allTasksList[nCurTaskIdx++];
        ctx.SubmitTasks(jobId, {compositePreprocessing});
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
        if(scatCoeffs.length() > 0) {
            compositePreprocessingArgs.append("-scatcoef");
            compositePreprocessingArgs.append(scatCoeffs);
        }
        steps.append(compositePreprocessing.CreateStep("CompositePreprocessing", compositePreprocessingArgs));

        if(tileTemporalFilesInfo.satelliteIds[i] != tileTemporalFilesInfo.primarySatelliteId) {
            TaskToSubmit &cutImgTask = allTasksList[nCurTaskIdx++];
            ctx.SubmitTasks(jobId, {cutImgTask});

            const auto &cutImgFile = cutImgTask.GetFilePath("img_res_bands_clipped.tif");
            QStringList gdalWarpArgs = { "-dstnodata", "-10000", "-overwrite",
                           "-cutline", shapePath, "-crop_to_cutline", outResImgBands, cutImgFile};
            steps.append(cutImgTask.CreateStep("gdalwarp-img", gdalWarpArgs));
            outResImgBands = cutImgFile;

            TaskToSubmit &cutAotTask = allTasksList[nCurTaskIdx++];
            ctx.SubmitTasks(jobId, {cutAotTask});
            const auto &cutAotFile = cutAotTask.GetFilePath("aot_res_clipped.tif");
            QStringList gdalWarpAotArgs = { "-dstnodata", "-10000", "-overwrite",
                            "-cutline", shapePath, "-crop_to_cutline", aotResImg, cutAotFile};
            steps.append(cutAotTask.CreateStep("gdalwarp-aot", gdalWarpAotArgs));
            aotResImg = cutAotFile;

            TaskToSubmit &cutCldTask = allTasksList[nCurTaskIdx++];
            ctx.SubmitTasks(jobId, {cutCldTask});
            const auto &cutCldFile = cutCldTask.GetFilePath("cld_res_clipped.tif");
            QStringList gdalWarpCldArgs = { "-dstnodata", "0", "-overwrite",
                            "-cutline", shapePath, "-crop_to_cutline", cldResImg, cutCldFile};
            steps.append(cutCldTask.CreateStep("gdalwarp-cld", gdalWarpCldArgs));
            cldResImg = cutCldFile;

            TaskToSubmit &cutWatTask = allTasksList[nCurTaskIdx++];
            ctx.SubmitTasks(jobId, {cutWatTask});
            const auto &cutWatFile = cutWatTask.GetFilePath("water_res_clipped.tif");
            QStringList gdalWarpWatArgs = { "-dstnodata", "0", "-overwrite",
                            "-cutline", shapePath, "-crop_to_cutline", waterResImg, cutWatFile};
            steps.append(cutWatTask.CreateStep("gdalwarp-wat", gdalWarpWatArgs));
            waterResImg = cutWatFile;

            TaskToSubmit &cutSnowTask = allTasksList[nCurTaskIdx++];
            ctx.SubmitTasks(jobId, {cutSnowTask});
            const auto &cutSnowFile = cutSnowTask.GetFilePath("snow_res_clipped.tif");
            QStringList gdalWarpSnowArgs = { "-dstnodata", "0", "-overwrite",
                            "-cutline", shapePath, "-crop_to_cutline", snowResImg, cutSnowFile};
            steps.append(cutSnowTask.CreateStep("gdalwarp-snow", gdalWarpSnowArgs));
            snowResImg = cutSnowFile;
        }
        TaskToSubmit &weightAot = allTasksList[nCurTaskIdx++];
        ctx.SubmitTasks(jobId, {weightAot});
        TaskToSubmit &weightOnClouds = allTasksList[nCurTaskIdx++];
        ctx.SubmitTasks(jobId, {weightOnClouds});
        TaskToSubmit &totalWeight = allTasksList[nCurTaskIdx++];
        ctx.SubmitTasks(jobId, {totalWeight});
        TaskToSubmit &updateSynthesis = allTasksList[nCurTaskIdx++];
        ctx.SubmitTasks(jobId, {updateSynthesis});
        TaskToSubmit &compositeSplitter = allTasksList[nCurTaskIdx++];
        ctx.SubmitTasks(jobId, {compositeSplitter});

        // Weight AOT Step
        const auto &outWeightAotFile = weightAot.GetFilePath("weight_aot.tif");
        QStringList weightAotArgs = { "WeightAOT",     "-xml",     inputProduct, "-in",
                                      aotResImg,       "-waotmin", weightAOTMin, "-waotmax",
                                      weightAOTMax,    "-aotmax",  AOTMax,       "-out",
                                      outWeightAotFile };
        steps.append(weightAot.CreateStep("WeightAOT", weightAotArgs));

        // Weight on clouds Step
        const auto &outWeightCldFile = weightOnClouds.GetFilePath("weight_cloud.tif");
        QStringList weightOnCloudArgs = { "WeightOnClouds", "-inxml",         inputProduct,
                                          "-incldmsk",      cldResImg,        "-coarseres",
                                          coarseRes,        "-sigmasmallcld", sigmaSmallCloud,
                                          "-sigmalargecld", sigmaLargeCloud,  "-out", outWeightCldFile };
        steps.append(weightOnClouds.CreateStep("WeightOnClouds", weightOnCloudArgs));

        // Total weight Step
        const auto &outTotalWeighFile = totalWeight.GetFilePath("weight_total.tif");
        QStringList totalWeightArgs = { "TotalWeight",    "-xml",           inputProduct,
                                        "-waotfile",      outWeightAotFile, "-wcldfile",
                                        outWeightCldFile, "-l3adate",       l3aSynthesisDate,
                                        "-halfsynthesis", synthalf,         "-wdatemin",
                                        weightDateMin,    "-out",           outTotalWeighFile };
        steps.append(totalWeight.CreateStep("TotalWeight", totalWeightArgs));

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
        }
        steps.append(updateSynthesis.CreateStep("UpdateSynthesis", updateSynthesisArgs));

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
                                              "-outrgb", (isLastProduct ? ("\"" + outL3AResultRgbFile+"?gdal:co:COMPRESS=DEFLATE\"") : outL3AResultRgbFile)
                                            };
        steps.append(compositeSplitter.CreateStep("CompositeSplitter", compositeSplitterArgs));

        // save the created L3A product file for the next product creation
        prevL3AProdRefls = outL3AResultReflsFile;
        prevL3AProdWeights = outL3AResultWeightsFile;
        prevL3AProdFlags = outL3AResultFlagsFile;
        prevL3AProdDates = outL3AResultDatesFile;
        prevL3ARgbFile = outL3AResultRgbFile;
    }

    CompositeProductFormatterParams &productFormatterParams = globalExecInfos.prodFormatParams;
    productFormatterParams.prevL3AProdRefls = prevL3AProdRefls;
    productFormatterParams.prevL3AProdWeights = prevL3AProdWeights;
    productFormatterParams.prevL3AProdFlags = prevL3AProdFlags;
    productFormatterParams.prevL3AProdDates = prevL3AProdDates;
    productFormatterParams.prevL3ARgbFile = prevL3ARgbFile;
}

void CompositeHandler::WriteExecutionInfosFile(const QString &executionInfosPath,
                                               const QJsonObject &parameters,
                                               std::map<QString, QString> &configParameters,
                                               const QStringList &listProducts)
{
    std::ofstream executionInfosFile;
    try {
        // Get L3A Synthesis date
        const auto &l3aSynthesisDate = parameters["synthesis_date"].toString();
        // Get the Half Synthesis interval value
        auto synthalf = parameters["half_synthesis"].toString();
        if(synthalf.length() == 0) {
            synthalf = configParameters["processor.l3a.half_synthesis"];
            if(synthalf.length() == 0) {
                synthalf = "15";
            }
        }

        // Get the parameters from the configuration
        const auto &bandsMapping = configParameters["processor.l3a.bandsmapping"];
        const auto &scatCoeffs = configParameters["processor.l3a.preproc.scatcoeffs"];

        const auto &weightAOTMin = configParameters["processor.l3a.weight.aot.minweight"];
        const auto &weightAOTMax = configParameters["processor.l3a.weight.aot.maxweight"];
        const auto &AOTMax = configParameters["processor.l3a.weight.aot.maxaot"];

        const auto &coarseRes = configParameters["processor.l3a.weight.cloud.coarseresolution"];
        const auto &sigmaSmallCloud = configParameters["processor.l3a.weight.cloud.sigmasmall"];
        const auto &sigmaLargeCloud = configParameters["processor.l3a.weight.cloud.sigmalarge"];

        const auto &weightDateMin = configParameters["processor.l3a.weight.total.weightdatemin"];

        executionInfosFile.open(executionInfosPath.toStdString().c_str(), std::ofstream::out);
        executionInfosFile << "<?xml version=\"1.0\" ?>" << std::endl;
        executionInfosFile << "<metadata>" << std::endl;
        executionInfosFile << "  <General>" << std::endl;
        executionInfosFile << "    <bands_mapping_file>" << bandsMapping.toStdString()
                           << "</bands_mapping_file>" << std::endl;
        executionInfosFile << "    <scattering_coefficients_file>" << scatCoeffs.toStdString()
                           << "</scattering_coefficients_file>" << std::endl;
        executionInfosFile << "  </General>" << std::endl;

        executionInfosFile << "  <Weight_AOT>" << std::endl;
        executionInfosFile << "    <weight_aot_min>" << weightAOTMin.toStdString()
                           << "</weight_aot_min>" << std::endl;
        executionInfosFile << "    <weight_aot_max>" << weightAOTMax.toStdString()
                           << "</weight_aot_max>" << std::endl;
        executionInfosFile << "    <aot_max>" << AOTMax.toStdString() << "</aot_max>" << std::endl;
        executionInfosFile << "  </Weight_AOT>" << std::endl;

        executionInfosFile << "  <Weight_On_Clouds>" << std::endl;
        executionInfosFile << "    <coarse_res>" << coarseRes.toStdString() << "</coarse_res>"
                           << std::endl;
        executionInfosFile << "    <sigma_small_cloud>" << sigmaSmallCloud.toStdString()
                           << "</sigma_small_cloud>" << std::endl;
        executionInfosFile << "    <sigma_large_cloud>" << sigmaLargeCloud.toStdString()
                           << "</sigma_large_cloud>" << std::endl;
        executionInfosFile << "  </Weight_On_Clouds>" << std::endl;

        executionInfosFile << "  <Weight_On_Date>" << std::endl;
        executionInfosFile << "    <weight_date_min>" << weightDateMin.toStdString()
                           << "</weight_date_min>" << std::endl;
        executionInfosFile << "    <l3a_product_date>" << l3aSynthesisDate.toStdString()
                           << "</l3a_product_date>" << std::endl;
        executionInfosFile << "    <half_synthesis>" << synthalf.toStdString()
                           << "</half_synthesis>" << std::endl;
        executionInfosFile << "  </Weight_On_Date>" << std::endl;

        executionInfosFile << "  <Dates_information>" << std::endl;
        // TODO: We should get these infos somehow but without parsing here anything
        // executionInfosFile << "    <start_date>" << 2013031 << "</start_date>" << std::endl;
        // executionInfosFile << "    <end_date>" << 20130422 << "</end_date>" << std::endl;
        executionInfosFile << "    <synthesis_date>" << l3aSynthesisDate.toStdString()
                           << "</synthesis_date>" << std::endl;
        executionInfosFile << "    <synthesis_half>" << synthalf.toStdString()
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

    QMap<QString, TileTemporalFilesInfo> mapTiles = GroupTiles(ctx, event.jobId, listProducts,
                                                               ProductType::L2AProductTypeId);
    QList<CompositeProductFormatterParams> listParams;

    TaskToSubmit productFormatterTask{"product-formatter", {}};
    NewStepList allSteps;
    QList<CompositeGlobalExecutionInfos> listCompositeInfos;
    for(auto tileId : mapTiles.keys())
    {
       const TileTemporalFilesInfo &listTemporalTiles = mapTiles.value(tileId);
       listCompositeInfos.append(CompositeGlobalExecutionInfos());
       CompositeGlobalExecutionInfos &infos = listCompositeInfos[listCompositeInfos.size()-1];
       infos.prodFormatParams.tileId = GetProductFormatterTile(tileId);
       HandleNewTilesList(ctx, event, listTemporalTiles, infos);
       listParams.append(infos.prodFormatParams);
       productFormatterTask.parentTasks += infos.prodFormatParams.parentsTasksRef;
       allSteps.append(infos.allStepsList);
    }

    ctx.SubmitTasks(event.jobId, {productFormatterTask});

    // finally format the product
    QStringList productFormatterArgs = GetProductFormatterArgs(productFormatterTask, ctx, event, listProducts, listParams);

    // add these steps to the steps list to be submitted
    allSteps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
    ctx.SubmitSteps(allSteps);
}

void CompositeHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                              const TaskFinishedEvent &event)
{
    if (event.module == "product-formatter") {
        ctx.MarkJobFinished(event.jobId);

        QString prodName = GetProductFormatterProductName(ctx, event);
        QString productFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId) + "/" + prodName;
        if(prodName != "" && ProcessorHandlerHelper::IsValidHighLevelProduct(productFolder)) {
            QString quicklook = GetProductFormatterQuicklook(ctx, event);
            QString footPrint = GetProductFormatterFootprint(ctx, event);
            // Insert the product into the database
            QDateTime minDate, maxDate;
            ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prodName, minDate, maxDate);
            ctx.InsertProduct({ ProductType::L3AProductTypeId, event.processorId, event.siteId,
                                event.jobId, productFolder, maxDate, prodName, quicklook,
                                footPrint, std::experimental::nullopt, TileList() });
            // Now remove the job folder containing temporary files
            // TODO: Reinsert this line - commented only for debug purposes
            //RemoveJobFolder(ctx, event.jobId);
        }
    }
}

QStringList CompositeHandler::GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                    const QStringList &listProducts, const QList<CompositeProductFormatterParams> &productParams) {

    const QJsonObject &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3a.");

    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.xml");
    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &l3aSynthesisDate = parameters["synthesis_date"].toString();

    WriteExecutionInfosFile(executionInfosPath, parameters, configParameters, listProducts);

    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot", targetFolder,
                                         "-fileclass", "SVT1",
                                         "-level", "L3A",
                                         "-timeperiod", l3aSynthesisDate,
                                         "-baseline", "01.00",
                                         "-siteid", QString::number(event.siteId),
                                         "-processor", "composite",
                                         "-gipp", executionInfosPath,
                                         "-outprops", outPropsPath};
    productFormatterArgs += "-il";
    productFormatterArgs += listProducts[listProducts.size() - 1];

    productFormatterArgs += "-processor.composite.refls";
    for(const CompositeProductFormatterParams &params: productParams) {
        productFormatterArgs += params.tileId;
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
    ConfigurationParameterValueMap mapCfg = ctx.GetConfigurationParameters(QString("processor.l3a."), siteId, requestOverrideCfgValues);

    ProcessorJobDefinitionParams params;
    params.isValid = false;

    int halfSynthesis = mapCfg["processor.l3a.half_synthesis"].value.toInt();
    if(halfSynthesis == 0)
        halfSynthesis = 15;
    int synthDateOffset = mapCfg["processor.l3a.synth_date_sched_offset"].value.toInt();
    if(synthDateOffset == 0)
        synthDateOffset = 30;

    // extract the scheduled date
    QDateTime qScheduledDate = QDateTime::fromTime_t(scheduledDate);
    // compute the half synthesis date
    QDateTime halfSynthesisDate = qScheduledDate.addDays(-synthDateOffset);
    // compute the start and date time
    QDateTime startDate = halfSynthesisDate.addDays(-halfSynthesis);
    QDateTime endDate = halfSynthesisDate.addDays(halfSynthesis);

    params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    // we need at least 1 product available in order to be able to create a L3A product
    if(params.productList.size() > 0) {
        params.isValid = true;
        params.jsonParameters = "{ \"synthesis_date\": \"" + halfSynthesisDate.toString("yyyymmdd") + "\"}";
    }
    //return QString("Cannot execute Composite processor. There should be at least 4 products but we have only %1 L2A products available!").arg(usedProductList.size());
    return params;
}


