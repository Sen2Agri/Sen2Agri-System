#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include "cropmaskhandler.hpp"
#include "processorhandlerhelper.h"
#include "logger.hpp"

QList<std::reference_wrapper<TaskToSubmit>> CropMaskHandler::CreateInSituTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                                QList<std::reference_wrapper<const TaskToSubmit>> &outProdFormatterParentsList)
{
    outAllTasksList.append(TaskToSubmit{ "quality-flags-extractor", {}} );
    outAllTasksList.append(TaskToSubmit{ "bands-extractor", {}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[1]}} );
    outAllTasksList.append(TaskToSubmit{ "temporal-resampling", {outAllTasksList[2]}} );
    outAllTasksList.append(TaskToSubmit{ "feature-extraction", {outAllTasksList[3]}} );
    outAllTasksList.append(TaskToSubmit{ "features-with-insitu", {outAllTasksList[4]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-images-statistics", {outAllTasksList[5]}} );
    outAllTasksList.append(TaskToSubmit{ "ogr2ogr", {outAllTasksList[6]}} );
    outAllTasksList.append(TaskToSubmit{ "ogr2ogr", {outAllTasksList[7]}} );
    outAllTasksList.append(TaskToSubmit{ "sample-selection", {outAllTasksList[8]}} );
    outAllTasksList.append(TaskToSubmit{ "train-images-classifier", {outAllTasksList[9]}} );
    outAllTasksList.append(TaskToSubmit{ "image-classifier", {outAllTasksList[10]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-confusion-matrix", {outAllTasksList[11]}} );
    outAllTasksList.append(TaskToSubmit{ "principal-component-analysis", {outAllTasksList[12]}} );
    outAllTasksList.append(TaskToSubmit{ "mean-shift-smoothing", {outAllTasksList[13]}} );
    outAllTasksList.append(TaskToSubmit{ "lsms-segmentation", {outAllTasksList[14]}} );
    outAllTasksList.append(TaskToSubmit{ "lsms-small-regions-merging", {outAllTasksList[15]}} );
    outAllTasksList.append(TaskToSubmit{ "majority-voting", {outAllTasksList[16]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[17]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-confusion-matrix", {outAllTasksList[18]}} );
    outAllTasksList.append(TaskToSubmit{ "image-compression", {outAllTasksList[19]}} );
    outAllTasksList.append(TaskToSubmit{ "xml-statistics", {outAllTasksList[20]}} );

    // product formatter needs completion of xml-statistics
    outProdFormatterParentsList.append(outAllTasksList[21]);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: outAllTasksList) {
        allTasksListRef.append(task);
    }
    return allTasksListRef;
}

QList<std::reference_wrapper<TaskToSubmit>> CropMaskHandler::CreateNoInSituTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                                QList<std::reference_wrapper<const TaskToSubmit>> &outProdFormatterParentsList)
{
    outAllTasksList.append(TaskToSubmit{ "quality-flags-extractor", {}} );
    outAllTasksList.append(TaskToSubmit{ "bands-extractor", {}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[1]}} );
    outAllTasksList.append(TaskToSubmit{ "temporal-resampling", {outAllTasksList[2]}} );
    outAllTasksList.append(TaskToSubmit{ "feature-extraction", {outAllTasksList[3]}} );
    outAllTasksList.append(TaskToSubmit{ "data-smoothing", {outAllTasksList[4]}} );
    outAllTasksList.append(TaskToSubmit{ "features-without-insitu", {outAllTasksList[5]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-image-statistics", {outAllTasksList[6]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[7]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[8]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[9]}} );
    outAllTasksList.append(TaskToSubmit{ "erosion", {outAllTasksList[10]}} );
    outAllTasksList.append(TaskToSubmit{ "trimming", {outAllTasksList[11]}} );
    outAllTasksList.append(TaskToSubmit{ "train-images-classifier-new", {outAllTasksList[12]}} );
    outAllTasksList.append(TaskToSubmit{ "image-classifier", {outAllTasksList[13]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-confusion-matrix", {outAllTasksList[14]}} );
    outAllTasksList.append(TaskToSubmit{ "principal-component-analysis", {outAllTasksList[15]}} );
    outAllTasksList.append(TaskToSubmit{ "mean-shift-smoothing", {outAllTasksList[16]}} );
    outAllTasksList.append(TaskToSubmit{ "lsms-segmentation", {outAllTasksList[17]}} );
    outAllTasksList.append(TaskToSubmit{ "lsms-small-regions-merging", {outAllTasksList[18]}} );
    outAllTasksList.append(TaskToSubmit{ "majority-voting", {outAllTasksList[19]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[20]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-confusion-matrix", {outAllTasksList[21]}} );
    outAllTasksList.append(TaskToSubmit{ "image-compression", {outAllTasksList[22]}} );
    outAllTasksList.append(TaskToSubmit{ "xml-statistics", {outAllTasksList[23]}} );

    // product formatter needs completion of xml-statistics
    outProdFormatterParentsList.append(outAllTasksList[24]);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: outAllTasksList) {
        allTasksListRef.append(task);
    }
    return allTasksListRef;
}

void CropMaskHandler::HandleNewTilesList(EventProcessingContext &ctx,
                                         const JobSubmittedEvent &event,
                                         const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                         CropMaskGlobalExecutionInfos &globalExecInfos)
{
    QStringList listProducts = ProcessorHandlerHelper::GetTemporalTileFiles(tileTemporalFilesInfo);

    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    const auto &referencePolygons = parameters["reference_polygons"].toString();
    if(referencePolygons.size() > 0) {
        allTasksListRef = CreateInSituTasksForNewProducts(globalExecInfos.allTasksList,
                                                          globalExecInfos.prodFormatParams.parentsTasksRef);
        SubmitTasks(ctx, event.jobId, allTasksListRef);
        HandleInsituJob(ctx, event, tileTemporalFilesInfo, listProducts, globalExecInfos);
    } else {
        const auto &reference_raster = parameters["reference_raster"].toString();
        if(reference_raster.size() == 0) {
            ctx.MarkJobFailed(event.jobId);
            throw std::runtime_error(
                QStringLiteral("Neither of the parameters reference_polygons nor reference_raster were found!").
                        toStdString());
        }
        allTasksListRef = CreateNoInSituTasksForNewProducts(globalExecInfos.allTasksList,
                                                            globalExecInfos.prodFormatParams.parentsTasksRef);
        SubmitTasks(ctx, event.jobId, allTasksListRef);
        HandleNoInsituJob(ctx, event, tileTemporalFilesInfo, listProducts, globalExecInfos);
    }
}

void CropMaskHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
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
    QList<CropMaskProductFormatterParams> listParams;

    TaskToSubmit productFormatterTask{"product-formatter", {}};
    NewStepList allSteps;
    //container for all task
    //QList<TaskToSubmit> allTasksList;
    QList<CropMaskGlobalExecutionInfos> listCropMaskInfos;
    for(auto tileId : mapTiles.keys())
    {
       const TileTemporalFilesInfo &listTemporalTiles = mapTiles.value(tileId);
       listCropMaskInfos.append(CropMaskGlobalExecutionInfos());
       CropMaskGlobalExecutionInfos &infos = listCropMaskInfos[listCropMaskInfos.size()-1];
       infos.prodFormatParams.tileId = GetProductFormatterTile(tileId);
       HandleNewTilesList(ctx, event, listTemporalTiles, infos);
       listParams.append(infos.prodFormatParams);
       productFormatterTask.parentTasks += infos.prodFormatParams.parentsTasksRef;
       //allTasksList.append(infos.allTasksList);
       allSteps.append(infos.allStepsList);
    }

    SubmitTasks(ctx, event.jobId, {productFormatterTask});

    // finally format the product
    QStringList productFormatterArgs = GetProductFormatterArgs(productFormatterTask, ctx, event, listProducts, listParams);

    // add these steps to the steps list to be submitted
    allSteps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
    ctx.SubmitSteps(allSteps);
}

void CropMaskHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    if (event.module == "bands-extractor") {
        // get the value for -t_srs as
        //        # ogr2ogr Reproject insitu data (Step 10)
        //        with open(shape_proj, 'r') as file:
        //                shape_wkt = "ESRI::" + file.read()
        const auto &shapePrjPath =
            ctx.GetOutputPath(event.jobId, event.taskId, event.module, processorDescr.shortName) + "shape.prj";
        const auto &shapePrjEsriPath =
            ctx.GetOutputPath(event.jobId, event.taskId, event.module, processorDescr.shortName) + "shape_esri.prj";

        QFile outFile(shapePrjEsriPath);
        if (!outFile.open(QIODevice::WriteOnly)) {
            throw std::runtime_error(QStringLiteral("Unable to open %1: %2")
                                         .arg(shapePrjEsriPath)
                                         .arg(outFile.errorString())
                                         .toStdString());
        }

        QFile inFile(shapePrjPath);
        if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            throw std::runtime_error(QStringLiteral("Unable to open %1: %2")
                                         .arg(shapePrjPath)
                                         .arg(outFile.errorString())
                                         .toStdString());
        }
        QTextStream inStream(&inFile);
        QTextStream outStream(&outFile);
        outStream << "ESRI::" << inStream.readAll();

    }
    else if (event.module == "compute-confusion-matrix") {
        const auto &outputs = ctx.GetTaskConsoleOutputs(event.taskId);
        for (const auto &output : outputs) {
            QRegularExpression reK("Kappa index: (\\d+(?:\\.\\d*)?)");
            QRegularExpression reAcc("Overall accuracy index: (\\d+(?:\\.\\d*)?)");

            const auto &mK = reK.match(output.stdOutText);
            const auto &mAcc = reAcc.match(output.stdOutText);

            const auto &statisticsPath =
                ctx.GetOutputPath(event.jobId, event.taskId, event.module, processorDescr.shortName) + "crop-mask-quality-metrics.txt";
            QFile file(statisticsPath);
            if (!file.open(QIODevice::WriteOnly)) {
                throw std::runtime_error(QStringLiteral("Unable to open %1: %2")
                                             .arg(statisticsPath)
                                             .arg(file.errorString())
                                             .toStdString());
            }

            QTextStream s(&file);
            s << mK.captured(1) << ' ' << mAcc.captured(1);
        }
    }
    else if (event.module == "product-formatter") {
        ctx.MarkJobFinished(event.jobId);

        QString prodName = GetProductFormatterProductName(ctx, event);
        QString productFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId) + "/" + prodName;
        if(prodName != "" && ProcessorHandlerHelper::IsValidHighLevelProduct(productFolder)) {
            QString quicklook = GetProductFormatterQuicklook(ctx, event);
            QString footPrint = GetProductFormatterFootprint(ctx, event);
            // Insert the product into the database
            QDateTime minDate, maxDate;
            ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prodName, minDate, maxDate);
            ctx.InsertProduct({ ProductType::L4AProductTypeId, event.processorId,
                                event.siteId, event.jobId, productFolder, maxDate,
                                prodName, quicklook, footPrint, std::experimental::nullopt, TileList() });
            // Now remove the job folder containing temporary files
            RemoveJobFolder(ctx, event.jobId, "l4a");
        } else {
            Logger::error(QStringLiteral("Cannot insert into database the product with name %1 and folder %2").arg(prodName).arg(productFolder));
        }
    }
}

void CropMaskHandler::HandleInsituJob(EventProcessingContext &ctx,
                                      const JobSubmittedEvent &event,
                                      const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                      const QStringList &listProducts,
                                      CropMaskGlobalExecutionInfos &globalExecInfos)

{
    auto configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l4a.");
    auto resourceParameters = ctx.GetJobConfigurationParameters(event.jobId, "resources.");

    const auto &appsMem = resourceParameters["resources.working-mem"];

    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    const auto &referencePolygons = parameters["reference_polygons"].toString();

//    auto mission = configParameters["processor.l4a.mission"];
//    if(mission.length() == 0) mission = "SENTINEL";
    auto mission = ProcessorHandlerHelper::GetMissionNamePrefixFromSatelliteId(tileTemporalFilesInfo.primarySatelliteId);

    int resolution = 0;
    if(!GetParameterValueAsInt(parameters, "resolution", resolution) ||
            resolution == 0) {
        resolution = 10;    // TODO: We should configure the default resolution in DB
    }

    auto randomSeed = configParameters["processor.l4a.random_seed"];
    if(randomSeed.isEmpty())  randomSeed = "0";

    auto sampleRatio = configParameters["processor.l4a.sample-ratio"];
    if(sampleRatio.length() == 0) sampleRatio = "0.75";

    auto temporalResamplingMode = configParameters["processor.l4a.temporal_resampling_mode"];
    if(temporalResamplingMode != "resample") temporalResamplingMode = "gapfill";

    auto window = configParameters["processor.l4a.window"];
    if(window.length() == 0) window = "6";

    auto nbcomp = configParameters["processor.l4a.nbcomp"];
    if(nbcomp.length() == 0) nbcomp = "6";

    auto spatialr = configParameters["processor.l4a.segmentation-spatial-radius"];
    if(spatialr.length() == 0) spatialr = "10";

    auto ranger = configParameters["processor.l4a.range-radius"];
    if(ranger.length() == 0) ranger = "0.65";

    auto minsize = configParameters["processor.l4a.segmentation-minsize"];
    if(minsize.length() == 0) minsize = "10";

    auto minarea = configParameters["processor.l4a.min-area"];
    if(minarea.length() == 0) minarea = "20";

    auto &classifier = configParameters["processor.l4a.classifier"];
    if(classifier.length() == 0) classifier = "rf";
    auto fieldName = configParameters["processor.l4a.classifier.field"];
    if(fieldName.length() == 0) fieldName = "CROP";

    auto classifierRfNbTrees = configParameters["processor.l4a.classifier.rf.nbtrees"];
    if(classifierRfNbTrees.length() == 0) classifierRfNbTrees = "100";
    auto classifierRfMinSamples = configParameters["processor.l4a.classifier.rf.min"];
    if(classifierRfMinSamples.length() == 0) classifierRfMinSamples = "25";
    auto classifierRfMaxDepth = configParameters["processor.l4a.classifier.rf.max"];
    if(classifierRfMaxDepth.length() == 0) classifierRfMaxDepth = "25";

    const auto &classifierSvmKernel = configParameters["processor.l4a.classifier.svm.k"];
    const auto &classifierSvmOptimize = configParameters["processor.l4a.classifier.svm.opt"];

    QList<TaskToSubmit> &allTasksList = globalExecInfos.allTasksList;

    int curTaskIdx = 0;
    TaskToSubmit &qualityFlagsExtractorTask = allTasksList[curTaskIdx++];
    TaskToSubmit &bandsExtractorTask = allTasksList[curTaskIdx++];
    TaskToSubmit &gdalWarpTask = allTasksList[curTaskIdx++];
    TaskToSubmit &temporalResamplingTask = allTasksList[curTaskIdx++];
    TaskToSubmit &featureExtractionTask = allTasksList[curTaskIdx++];
    TaskToSubmit &featureWithInSituTask = allTasksList[curTaskIdx++];
    TaskToSubmit &computeImagesStatisticsTask = allTasksList[curTaskIdx++];
    TaskToSubmit &ogr2ogrTask = allTasksList[curTaskIdx++];
    TaskToSubmit &ogr2ogrTask2 = allTasksList[curTaskIdx++];
    TaskToSubmit &sampleSelectionTask = allTasksList[curTaskIdx++];
    TaskToSubmit &trainImagesClassifierTask = allTasksList[curTaskIdx++];
    TaskToSubmit &imageClassifierTask = allTasksList[curTaskIdx++];
    TaskToSubmit &computeConfusionMatrixTask = allTasksList[curTaskIdx++];

    TaskToSubmit &principalComponentAnalysisTask = allTasksList[curTaskIdx++];
    TaskToSubmit &meanShiftSmoothingTask = allTasksList[curTaskIdx++];
    TaskToSubmit &lsmsSegmentationTask = allTasksList[curTaskIdx++];
    TaskToSubmit &lsmsSmallRegionsMergingTask = allTasksList[curTaskIdx++];
    TaskToSubmit &majorityVotingTask = allTasksList[curTaskIdx++];
    TaskToSubmit &gdalWarpTask2 = allTasksList[curTaskIdx++];
    TaskToSubmit &computeConfusionMatrixTask2 = allTasksList[curTaskIdx++];
    TaskToSubmit &convertTask = allTasksList[curTaskIdx++];
    TaskToSubmit &xmlStatisticsTask = allTasksList[curTaskIdx++];
    //TaskToSubmit productFormatterTask{ "product-formatter", { xmlStatisticsTask } };

    const auto &rawtocr = bandsExtractorTask.GetFilePath("rawtocr.tif");
    const auto &rawmask = bandsExtractorTask.GetFilePath("rawmask.tif");
    const auto &dates = bandsExtractorTask.GetFilePath("dates.txt");
    const auto &shape = bandsExtractorTask.GetFilePath("shape.shp");
    const auto &shapeEsriPrj = bandsExtractorTask.GetFilePath("shape_esri.prj");
    const auto &statusFlags = qualityFlagsExtractorTask.GetFilePath("statusFlags.tif");

    const auto &tocr = gdalWarpTask.GetFilePath("tocr.tif");
    const auto &mask = gdalWarpTask.GetFilePath("mask.tif");

    const auto &rtocr = temporalResamplingTask.GetFilePath("rtocr.tif");
    const auto &days = temporalResamplingTask.GetFilePath("days.txt");
    const auto &outdays = temporalResamplingTask.GetFilePath("days.txt");

    const auto &ndvi = featureExtractionTask.GetFilePath("ndvi.tif");
    const auto &ndwi = featureExtractionTask.GetFilePath("ndwi.tif");
    const auto &brightness = featureExtractionTask.GetFilePath("brightness.tif");

    const auto &features = featureWithInSituTask.GetFilePath("features.tif");

    const auto &statistics = computeImagesStatisticsTask.GetFilePath("statistics.xml");

    const auto &refPolysReprojected = ogr2ogrTask.GetFilePath("reference_polygons_reproject.shp");
    const auto &refPolysClipped = ogr2ogrTask2.GetFilePath("reference_clip.shp");

    const auto &trainingPolys = sampleSelectionTask.GetFilePath("training_polygons.shp");
    const auto &validationPolys = sampleSelectionTask.GetFilePath("validation_polygons.shp");

    const auto &model = trainImagesClassifierTask.GetFilePath("model.txt");
    const auto &confusionMatrix = trainImagesClassifierTask.GetFilePath("confusion-matrix.csv");

    const auto &raw_crop_mask_uncompressed = imageClassifierTask.GetFilePath("raw_crop_mask_uncompressed.tif");

    const auto &confusionMatrixValidation = computeConfusionMatrixTask.GetFilePath("confusion-matrix-validation.csv");

    const auto &pca = principalComponentAnalysisTask.GetFilePath("pca.tif");

    const auto &mean_shift_smoothing_spatial = meanShiftSmoothingTask.GetFilePath("mean_shift_smoothing_spatial.tif");
    const auto &mean_shift_smoothing = meanShiftSmoothingTask.GetFilePath("mean_shift_smoothing.tif");

    const auto &tmpfolder = lsmsSegmentationTask.GetFilePath("");
    const auto &segmented = lsmsSegmentationTask.GetFilePath("segmented.tif");

    const auto &segmented_merged = lsmsSmallRegionsMergingTask.GetFilePath("segmented_merged.tif");

    const auto &crop_mask_uncut = majorityVotingTask.GetFilePath("crop_mask_uncut.tif");

    const auto &crop_mask_uncompressed = gdalWarpTask2.GetFilePath("crop_mask_uncomp.tif");
    const auto &crop_mask_cut_uncompressed = gdalWarpTask2.GetFilePath("raw_crop_mask_cut_uncomp.tif");

    const auto &confusion_matrix_validation = computeConfusionMatrixTask2.GetFilePath("crop-mask-confusion-matrix-validation.csv");
    const auto &quality_metrics = computeConfusionMatrixTask2.GetFilePath("crop-mask-quality-metrics.txt");

    const auto &crop_mask = convertTask.GetFilePath("crop_mask.tif");
    const auto &raw_crop_mask = convertTask.GetFilePath("raw_crop_mask.tif");

    const auto &xml_validation_metrics = xmlStatisticsTask.GetFilePath("crop-mask-validation-metrics.xml");

// /////

    QStringList trainImagesClassifierArgs = {
        "-io.il",      features,   "-io.vd",         trainingPolys,
        "-io.imstat",  statistics, "-rand",          randomSeed,
        "-sample.bm",  "0",        "-io.confmatout", confusionMatrix,
        "-io.out",     model,      "-sample.mt",     "4000",
        "-sample.mv",  "-1",       "-sample.vtr",    sampleRatio,
        "-sample.vfn", fieldName,  "-classifier",    classifier
    };

    if (classifier == "rf") {
        trainImagesClassifierArgs.append("-classifier.rf.nbtrees");
        trainImagesClassifierArgs.append(classifierRfNbTrees);
        trainImagesClassifierArgs.append("-classifier.rf.min");
        trainImagesClassifierArgs.append(classifierRfMinSamples);
        trainImagesClassifierArgs.append("-classifier.rf.max");
        trainImagesClassifierArgs.append(classifierRfMaxDepth);
    } else {
        trainImagesClassifierArgs.append("-classifier.svm.k");
        trainImagesClassifierArgs.append(classifierSvmKernel);
        trainImagesClassifierArgs.append("-classifier.svm.opt");
        trainImagesClassifierArgs.append(classifierSvmOptimize);
    }

    QStringList imageClassifierArgs = { "-in",    features, "-imstat", statistics,
                                        "-model", model,    "-out",    raw_crop_mask_uncompressed };

    globalExecInfos.allStepsList = {
        qualityFlagsExtractorTask.CreateStep("QualityFlagsExtractor", GetQualityFlagsExtractorArgs(mission, statusFlags, listProducts, resolution)),
        bandsExtractorTask.CreateStep("BandsExtractor", GetBandsExtractorArgs(mission, rawtocr, rawmask, dates, shape,
                                        listProducts, resolution)),
        gdalWarpTask.CreateStep("ClipRasterImage",
                              { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", appsMem, rawtocr, tocr }),
        gdalWarpTask.CreateStep("ClipRasterMask",
                              { "-dstnodata", "-10000", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", appsMem, rawmask, mask }),
        temporalResamplingTask.CreateStep("TemporalResampling",
                                      { "TemporalResampling", "-tocr", tocr, "-mask", mask, "-ind",
                                        dates, "-sp", "SENTINEL", "5", "SPOT", "5", "LANDSAT", "16",
                                        "-rtocr", rtocr, "-outdays", days, "-mode", temporalResamplingMode, "-merge", "1" }),
        featureExtractionTask.CreateStep("FeatureExtraction",
                                     { "FeatureExtraction", "-rtocr", rtocr, "-ndvi", ndvi, "-ndwi",
                                       ndwi, "-brightness", brightness }),

        featureWithInSituTask.CreateStep("FeaturesWithInsitu",
                                        {"FeaturesWithInsitu", "-ndvi",ndvi,"-ndwi",ndwi,"-brightness",brightness,
                                        "-dates",outdays,"-window", window,"-bm", "true", "-out",features}),

        computeImagesStatisticsTask.CreateStep("ComputeImagesStatistics", {"-il", features,"-out",statistics}),

        ogr2ogrTask.CreateStep(
            "ReprojectPolys", { "-t_srs", shapeEsriPrj, "-overwrite", refPolysReprojected, referencePolygons }),

        ogr2ogrTask2.CreateStep(
            "ClipPolys", { "-clipsrc", shape, "-overwrite", refPolysClipped, refPolysReprojected }),

        sampleSelectionTask.CreateStep("SampleSelection",
                                   { "SampleSelection", "-ref", refPolysClipped, "-ratio",
                                     sampleRatio, "-seed", randomSeed, "-tp", trainingPolys, "-vp", validationPolys,
                                     "-nofilter", "true" }),

        trainImagesClassifierTask.CreateStep("TrainImagesClassifier", trainImagesClassifierArgs),
        imageClassifierTask.CreateStep("ImageClassifier", imageClassifierArgs),
        computeConfusionMatrixTask.CreateStep("ComputeConfusionMatrix",GetConfusionMatrixArgs(raw_crop_mask_uncompressed, confusionMatrixValidation, validationPolys, "vector", fieldName)),

        // The following steps are common for insitu and without insitu data
        principalComponentAnalysisTask.CreateStep("PrincipalComponentAnalysis", { "PrincipalComponentAnalysis", "-ndvi", ndvi, "-nc", nbcomp, "-out", pca }),
        meanShiftSmoothingTask.CreateStep("MeanShiftSmoothing", { "-in", pca,"-modesearch","0", "-spatialr", spatialr, "-ranger", ranger,
                                                              "-maxiter", "20", "-foutpos", mean_shift_smoothing_spatial, "-fout", mean_shift_smoothing }),
        lsmsSegmentationTask.CreateStep("LSMSSegmentation", { "-in", mean_shift_smoothing,"-inpos",
                                                          mean_shift_smoothing_spatial, "-spatialr", spatialr, "-ranger", ranger, "-minsize",
                                                          "0", "-tilesizex", "1024", "-tilesizey", "1024", "-tmpdir", tmpfolder, "-out", segmented, "uint32" }),
        lsmsSmallRegionsMergingTask.CreateStep("LSMSSmallRegionsMerging", { "-in", mean_shift_smoothing,"-inseg", segmented, "-minsize", minsize,
                                                                        "-tilesizex", "1024", "-tilesizey", "1024", "-out", segmented_merged, "uint32", }),

        majorityVotingTask.CreateStep("MajorityVoting", { "MajorityVoting", "-nodatasegvalue", "0", "-nodataclassifvalue", "-10000", "-minarea", minarea,
                                                    "-inclass", raw_crop_mask_uncompressed, "-inseg", segmented_merged, "-rout", crop_mask_uncut }),

        gdalWarpTask2.CreateStep("gdalwarp_raw", { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                              "-crop_to_cutline", raw_crop_mask_uncompressed, crop_mask_cut_uncompressed }),
        gdalWarpTask2.CreateStep("gdalwarp_segmented", { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                              "-crop_to_cutline", crop_mask_uncut, crop_mask_uncompressed }),

        // This is specific only for Insitu data
        computeConfusionMatrixTask2.CreateStep("ComputeConfusionMatrix",GetConfusionMatrixArgs(crop_mask_uncompressed, confusion_matrix_validation, validationPolys, "vector", fieldName)),
        convertTask.CreateStep("CompressionCropMask", GetCompressionArgs(crop_mask_uncompressed, crop_mask)),
        convertTask.CreateStep("CompressionRawCrop", GetCompressionArgs(crop_mask_cut_uncompressed, raw_crop_mask)),

        xmlStatisticsTask.CreateStep("XMLStatistics", { "XMLStatistics", "-confmat", confusion_matrix_validation, "-quality", quality_metrics, "-root", "CropMask", "-out", xml_validation_metrics }),
    };

    CropMaskProductFormatterParams &productFormatterParams = globalExecInfos.prodFormatParams;
    productFormatterParams.crop_mask = crop_mask;
    productFormatterParams.raw_crop_mask = raw_crop_mask;
    productFormatterParams.xml_validation_metrics = xml_validation_metrics;
    productFormatterParams.statusFlags = statusFlags;
}

void CropMaskHandler::HandleNoInsituJob(EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event,
                                        const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                        const QStringList &listProducts,
                                        CropMaskGlobalExecutionInfos &globalExecInfos)
{
    auto configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l4a.");
    auto resourceParameters = ctx.GetJobConfigurationParameters(event.jobId, "resources.");

    const auto &appsMem = resourceParameters["resources.working-mem"];

    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    const auto &reference = parameters["reference_raster"].toString();

    //auto mission = configParameters["processor.l4a.mission"];
    //if(mission.length() == 0) mission = "SPOT";
    auto mission = ProcessorHandlerHelper::GetMissionNamePrefixFromSatelliteId(tileTemporalFilesInfo.primarySatelliteId);

    int resolution = 0;
    if(!GetParameterValueAsInt(parameters, "resolution", resolution) ||
            resolution == 0) {
        resolution = 10;    // TODO: We should configure the default resolution in DB
    }
    const auto &resolutionStr = QString::number(resolution);
    auto randomSeed = configParameters["processor.l4a.random_seed"];
    if(randomSeed.isEmpty())  randomSeed = "0";

    auto sampleRatio = configParameters["processor.l4a.sample-ratio"];
    if(sampleRatio.length() == 0) sampleRatio = "0.75";

    auto temporalResamplingMode = configParameters["processor.l4a.temporal_resampling_mode"];
    if(temporalResamplingMode != "resample") temporalResamplingMode = "gapfill";

    auto window = configParameters["processor.l4a.window"];
    if(window.length() == 0) window = "6";

    auto nbcomp = configParameters["processor.l4a.nbcomp"];
    if(nbcomp.length() == 0) nbcomp = "6";

    auto spatialr = configParameters["processor.l4a.segmentation-spatial-radius"];
    if(spatialr.length() == 0) spatialr = "10";

    auto ranger = configParameters["processor.l4a.range-radius"];
    if(ranger.length() == 0) ranger = "0.65";

    auto minsize = configParameters["processor.l4a.segmentation-minsize"];
    if(minsize.length() == 0) minsize = "10";

    auto nbtrsample = configParameters["processor.l4a.training-samples-number"];
    if(nbtrsample.length() == 0) nbtrsample = "4000";

    auto lmbd = configParameters["processor.l4a.smoothing-lambda"];
    if(lmbd.length() == 0) lmbd = "2";

    auto erode_radius = configParameters["processor.l4a.erode-radius"];
    if(erode_radius.length() == 0) erode_radius = "1";

    auto alpha = configParameters["processor.l4a.mahalanobis-alpha"];
    if(alpha.length() == 0) alpha = "0.01";

    auto minarea = configParameters["processor.l4a.min-area"];
    if(minarea.length() == 0) minarea = "20";

    auto &classifier = configParameters["processor.l4a.classifier"];
    if(classifier.length() == 0) classifier = "rf";
    auto fieldName = configParameters["processor.l4a.classifier.field"];
    if(fieldName.length() == 0) fieldName = "CROP";

    auto classifierRfNbTrees = configParameters["processor.l4a.classifier.rf.nbtrees"];
    if(classifierRfNbTrees.length() == 0) classifierRfNbTrees = "100";
    auto classifierRfMinSamples = configParameters["processor.l4a.classifier.rf.min"];
    if(classifierRfMinSamples.length() == 0) classifierRfMinSamples = "25";
    auto classifierRfMaxDepth = configParameters["processor.l4a.classifier.rf.max"];
    if(classifierRfMaxDepth.length() == 0) classifierRfMaxDepth = "25";

    const auto &classifierSvmKernel = configParameters["processor.l4a.classifier.svm.k"];
    const auto &classifierSvmOptimize = configParameters["processor.l4a.classifier.svm.opt"];

    QList<TaskToSubmit> &allTasksList = globalExecInfos.allTasksList;
    int curTaskIdx = 0;
    TaskToSubmit &qualityFlagsExtractorTask = allTasksList[curTaskIdx++];
    TaskToSubmit &bandsExtractorTask = allTasksList[curTaskIdx++];
    TaskToSubmit &gdalWarpTask = allTasksList[curTaskIdx++];
    TaskToSubmit &temporalResamplingTask = allTasksList[curTaskIdx++];
    TaskToSubmit &featureExtractionTask = allTasksList[curTaskIdx++];
    TaskToSubmit &dataSmoothingTask = allTasksList[curTaskIdx++];
    TaskToSubmit &featuresWithoutInsituTask = allTasksList[curTaskIdx++];
    TaskToSubmit &computeImageStatisticsTask = allTasksList[curTaskIdx++];
    TaskToSubmit &gdalWarpTask2 = allTasksList[curTaskIdx++];
    TaskToSubmit &gdalWarpTask2_1 = allTasksList[curTaskIdx++];
    TaskToSubmit &gdalWarpTask2_2 = allTasksList[curTaskIdx++];
    TaskToSubmit &erosionTask = allTasksList[curTaskIdx++];
    TaskToSubmit &trimmingTask = allTasksList[curTaskIdx++];
    TaskToSubmit &trainImagesClassifierNewTask = allTasksList[curTaskIdx++];
    TaskToSubmit &imageClassifierTask = allTasksList[curTaskIdx++];
    TaskToSubmit &computeConfusionMatrixTask = allTasksList[curTaskIdx++];
    TaskToSubmit &principalComponentAnalysisTask = allTasksList[curTaskIdx++];
    TaskToSubmit &meanShiftSmoothingTask = allTasksList[curTaskIdx++];
    TaskToSubmit &lsmsSegmentationTask = allTasksList[curTaskIdx++];
    TaskToSubmit &lsmsSmallRegionsMergingTask = allTasksList[curTaskIdx++];
    TaskToSubmit &majorityVotingTask = allTasksList[curTaskIdx++];
    TaskToSubmit &gdalWarpTask3 = allTasksList[curTaskIdx++];
    TaskToSubmit &computeConfusionMatrixTask2 = allTasksList[curTaskIdx++];
    TaskToSubmit &convertTask = allTasksList[curTaskIdx++];
    TaskToSubmit &xmlStatisticsTask = allTasksList[curTaskIdx++];

    const auto &rawtocr = bandsExtractorTask.GetFilePath("rawtocr.tif");
    const auto &rawmask = bandsExtractorTask.GetFilePath("rawmask.tif");
    const auto &dates = bandsExtractorTask.GetFilePath("dates.txt");
    const auto &shape = bandsExtractorTask.GetFilePath("shape.shp");
    const auto &shapeEsriPrj = bandsExtractorTask.GetFilePath("shape_esri.prj");
    const auto &statusFlags = qualityFlagsExtractorTask.GetFilePath("statusFlags.tif");

    const auto &tocr = gdalWarpTask.GetFilePath("tocr.tif");
    const auto &mask = gdalWarpTask.GetFilePath("mask.tif");

    const auto &rtocr = temporalResamplingTask.GetFilePath("rtocr.tif");

    const auto &ndvi = featureExtractionTask.GetFilePath("ndvi.tif");
    auto rndvi = featureExtractionTask.GetFilePath("rndvi.tif");

    const auto &spectral_features = featuresWithoutInsituTask.GetFilePath("spectral_features.tif");

    const auto &ndvi_smooth = dataSmoothingTask.GetFilePath("ndvi_smooth.tif");
    const auto &outdays_smooth = dataSmoothingTask.GetFilePath("days_smooth.txt");
    const auto &rtocr_smooth = dataSmoothingTask.GetFilePath("rtocr_smooth.tif");

    const auto &statistics_noinsitu = computeImageStatisticsTask.GetFilePath("statistics_noinsitu.xml");

    const auto &trimmed_reference_raster = trimmingTask.GetFilePath("trimmed_reference_raster.tif");

    const auto &model = trainImagesClassifierNewTask.GetFilePath("model.txt");
    const auto &confusionMatrix = trainImagesClassifierNewTask.GetFilePath("confusion-matrix.csv");

    const auto &raw_crop_mask_uncompressed = imageClassifierTask.GetFilePath("raw_crop_mask_uncompressed.tif");

    const auto &cropped_reference = gdalWarpTask2.GetFilePath("cropped_reference.tif");
    const auto &reprojected_reference = gdalWarpTask2_1.GetFilePath("reprojected_reference.tif");
    const auto &crop_reference = gdalWarpTask2_2.GetFilePath("crop_reference.tif");

    const auto &eroded_reference = erosionTask.GetFilePath("eroded_reference.tif");

    const auto &pca = principalComponentAnalysisTask.GetFilePath("pca.tif");

    const auto &mean_shift_smoothing_spatial = meanShiftSmoothingTask.GetFilePath("mean_shift_smoothing_spatial.tif");
    const auto &mean_shift_smoothing = meanShiftSmoothingTask.GetFilePath("mean_shift_smoothing.tif");

    const auto &tmpfolder = lsmsSegmentationTask.GetFilePath("");
    const auto &segmented = lsmsSegmentationTask.GetFilePath("segmented.tif");

    const auto &segmented_merged = lsmsSmallRegionsMergingTask.GetFilePath("segmented_merged.tif");

    const auto &crop_mask_uncut = majorityVotingTask.GetFilePath("crop_mask_uncut.tif");

    const auto &crop_mask_uncompressed = gdalWarpTask3.GetFilePath("crop_mask_uncomp.tif");
    const auto &crop_mask_cut_uncompressed = gdalWarpTask3.GetFilePath("raw_crop_mask_cut_uncomp.tif");

    const auto &raw_crop_mask_confusion_matrix_validation = computeConfusionMatrixTask.GetFilePath("raw-crop-mask-confusion-matrix-validation.csv");
    const auto &confusion_matrix_validation = computeConfusionMatrixTask2.GetFilePath("crop-mask-confusion-matrix-validation.csv");
    const auto &quality_metrics = computeConfusionMatrixTask2.GetFilePath("crop-mask-quality-metrics.txt");

    const auto &crop_mask = convertTask.GetFilePath("crop_mask.tif");
    const auto &raw_crop_mask = convertTask.GetFilePath("raw_crop_mask.tif");

    const auto &xml_validation_metrics = xmlStatisticsTask.GetFilePath("crop-mask-validation-metrics.xml");

    QStringList trainImagesClassifierArgs = {"TrainImagesClassifierNew",
        "-io.il",           spectral_features,  "-io.rs",       trimmed_reference_raster,
        "-nodatalabel",     "\"-10000\"",           "-io.imstat",   statistics_noinsitu,
        "-rand",            randomSeed,         "-sample.bm",   "0",
        "-io.confmatout",   confusionMatrix,    "-io.out",      model,
        "-sample.mt",       nbtrsample,         "-sample.mv",   "\"-1\"",
        "-sample.vtr",      sampleRatio,        "-classifier",  classifier
    };

    if (classifier == "rf") {
        trainImagesClassifierArgs.append("-classifier.rf.nbtrees");
        trainImagesClassifierArgs.append(classifierRfNbTrees);
        trainImagesClassifierArgs.append("-classifier.rf.min");
        trainImagesClassifierArgs.append(classifierRfMinSamples);
        trainImagesClassifierArgs.append("-classifier.rf.max");
        trainImagesClassifierArgs.append(classifierRfMaxDepth);
    } else {
        trainImagesClassifierArgs.append("-classifier.svm.k");
        trainImagesClassifierArgs.append(classifierSvmKernel);
        trainImagesClassifierArgs.append("-classifier.svm.opt");
        trainImagesClassifierArgs.append(classifierSvmOptimize);
    }

    QStringList imageClassifierArgs = { "-in",    spectral_features, "-imstat", statistics_noinsitu,
                                        "-model", model,    "-out",    raw_crop_mask_uncompressed };

    globalExecInfos.allStepsList = {
        qualityFlagsExtractorTask.CreateStep("QualityFlagsExtractor", GetQualityFlagsExtractorArgs(mission, statusFlags, listProducts, resolution)),
        bandsExtractorTask.CreateStep("BandsExtractor", GetBandsExtractorArgs(mission, rawtocr, rawmask, dates, shape, listProducts, resolution)),
        gdalWarpTask.CreateStep("ClipRasterImage",
                              { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", appsMem, rawtocr, tocr }),
        gdalWarpTask.CreateStep("ClipRasterMask",
                              { "-dstnodata", "-10000", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", appsMem, rawmask, mask }),

        // The following steps are specific to the noinsitu data
        temporalResamplingTask.CreateStep("TemporalResampling",
                                      { "TemporalResampling", "-tocr", tocr, "-mask", mask, "-ind",
                                        dates, "-sp", "SENTINEL", "5", "SPOT", "5", "LANDSAT", "16",
                                        "-rtocr", rtocr, "-mode", temporalResamplingMode, "-merge", "1" }),
        featureExtractionTask.CreateStep("FeatureExtraction",  { "FeatureExtraction", "-rtocr", rtocr, "-ndvi", rndvi }),
        featureExtractionTask.CreateStep("FeatureExtractionNdvi", { "FeatureExtraction", "-rtocr", tocr, "-ndvi", ndvi }),

        dataSmoothingTask.CreateStep("NDVIDataSmoothing",
                                        {"DataSmoothing", "-ts",ndvi,"-mask",mask,"-dates",dates,
                                        "-lambda",lmbd,"-sts", ndvi_smooth,"-outdays", outdays_smooth}),

        dataSmoothingTask.CreateStep("ReflDataSmoothing",
                                        {"DataSmoothing", "-ts",tocr,"-mask",mask,"-dates",dates,
                                        "-lambda",lmbd,"-sts", rtocr_smooth}),
        featuresWithoutInsituTask.CreateStep("FeaturesWithoutInsitu",
                                        {"FeaturesWithoutInsitu", "-ndvi",ndvi_smooth,"-ts",rtocr_smooth,"-dates",outdays_smooth, "-sf", spectral_features}),

        computeImageStatisticsTask.CreateStep("ComputeImagesStatistics", {"-il", spectral_features,"-out",statistics_noinsitu}),


        // The following steps cannot be inserted in one task because they need to be executed one after each other, not parallel
        gdalWarpTask2.CreateStep(
            "RefMap", { "-dstnodata", "0", "-overwrite", "-crop_to_cutline", "-cutline", shape, reference, cropped_reference }),
        gdalWarpTask2_1.CreateStep(
            "ReprojectRefMap", { "-dstnodata", "0", "-overwrite", "-t_srs", shapeEsriPrj, cropped_reference, reprojected_reference }),
        gdalWarpTask2_2.CreateStep(
            "ResampleRefMap", { "-dstnodata", "0", "-overwrite", "-crop_to_cutline", "-cutline", shape, "-tr", resolutionStr, resolutionStr, reprojected_reference, crop_reference }),

        erosionTask.CreateStep("Erosion", { "Erosion", "-in", crop_reference,  "-out", eroded_reference, "-radius", erode_radius }),
        trimmingTask.CreateStep("Trimming", { "Trimming", "-feat", spectral_features,
                                              "-ref", eroded_reference, "-out", trimmed_reference_raster, "-alpha", alpha,
                                              "-nbsamples", "0", "-seed", randomSeed}),

        trainImagesClassifierNewTask.CreateStep("TrainImagesClassifierNew", trainImagesClassifierArgs),
        imageClassifierTask.CreateStep("ImageClassifier", imageClassifierArgs),
        computeConfusionMatrixTask.CreateStep("ComputeConfusionMatrix", GetConfusionMatrixArgs(raw_crop_mask_uncompressed, raw_crop_mask_confusion_matrix_validation, trimmed_reference_raster)),

        // The following steps are common for insitu and without insitu data
        principalComponentAnalysisTask.CreateStep("PrincipalComponentAnalysis", { "PrincipalComponentAnalysis", "-ndvi", ndvi, "-nc", nbcomp, "-out", pca }),
        meanShiftSmoothingTask.CreateStep("MeanShiftSmoothing", { "-in", pca,"-modesearch","0", "-spatialr", spatialr, "-ranger", ranger,
                                                              "-maxiter", "20", "-foutpos", mean_shift_smoothing_spatial, "-fout", mean_shift_smoothing }),
        lsmsSegmentationTask.CreateStep("LSMSSegmentation", { "-in", mean_shift_smoothing,"-inpos",
                                                          mean_shift_smoothing_spatial, "-spatialr", spatialr, "-ranger", ranger, "-minsize",
                                                          "0", "-tilesizex", "1024", "-tilesizey", "1024", "-tmpdir", tmpfolder, "-out", segmented, "uint32" }),
        lsmsSmallRegionsMergingTask.CreateStep("LSMSSmallRegionsMerging", { "-in", mean_shift_smoothing,"-inseg", segmented, "-minsize", minsize,
                                                                        "-tilesizex", "1024", "-tilesizey", "1024", "-out", segmented_merged, "uint32", }),

        majorityVotingTask.CreateStep("MajorityVoting", { "MajorityVoting", "-nodatasegvalue", "0", "-nodataclassifvalue", "-10000", "-minarea", minarea,
                                                    "-inclass", raw_crop_mask_uncompressed, "-inseg", segmented_merged, "-rout", crop_mask_uncut }),

        gdalWarpTask3.CreateStep("gdalwarp_raw", { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                              "-crop_to_cutline", raw_crop_mask_uncompressed, crop_mask_cut_uncompressed }),
        gdalWarpTask3.CreateStep("gdalwarp_segmented", { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                              "-crop_to_cutline", crop_mask_uncut, crop_mask_uncompressed }),

        // This is specific only for NoInsitu data
        computeConfusionMatrixTask2.CreateStep("ComputeConfusionMatrix", GetConfusionMatrixArgs(crop_mask_uncompressed, confusion_matrix_validation, trimmed_reference_raster)),
        convertTask.CreateStep("CompressionCropMask", GetCompressionArgs(crop_mask_uncompressed, crop_mask)),
        convertTask.CreateStep("CompressionRawCrop", GetCompressionArgs(crop_mask_cut_uncompressed, raw_crop_mask)),

        xmlStatisticsTask.CreateStep("XMLStatistics", { "XMLStatistics", "-confmat", confusion_matrix_validation, "-quality", quality_metrics, "-root", "CropMask", "-out", xml_validation_metrics }),
    };

    CropMaskProductFormatterParams &productFormatterParams = globalExecInfos.prodFormatParams;
    productFormatterParams.crop_mask = crop_mask;
    productFormatterParams.raw_crop_mask = raw_crop_mask;
    productFormatterParams.xml_validation_metrics = xml_validation_metrics;
    productFormatterParams.statusFlags = statusFlags;
}

QStringList CropMaskHandler::GetQualityFlagsExtractorArgs(const QString &mission, const QString &statusFlags,
                                                    const QStringList &inputProducts, int resolution) {
    QStringList qualityFlagsExtractorArgs = { "QualityFlagsExtractor", "-mission", mission,
                    "-out", "\"" + statusFlags+"?gdal:co:COMPRESS=DEFLATE\"",
                                       "-il" };

    for (const auto &inputFile : inputProducts) {
        qualityFlagsExtractorArgs.append(inputFile);
    }
    if (resolution) {
        qualityFlagsExtractorArgs.append("-pixsize");
        qualityFlagsExtractorArgs.append(QString::number(resolution));
    }
    return qualityFlagsExtractorArgs;
}

QStringList CropMaskHandler::GetBandsExtractorArgs(const QString &mission, const QString &outImg, const QString &mask,const QString &outDates,
                                                    const QString &shape, const QStringList &inputProducts, int resolution) {
    QStringList bandsExtractorArgs = { "BandsExtractor", "-mission", mission, "-out", outImg,
                                       "-mask", mask, "-outdate", outDates,
                                       "-shape", shape, "-merge", "false", "-il" };

    for (const auto &inputFile : inputProducts) {
        bandsExtractorArgs.append(inputFile);
    }
    if (resolution) {
        bandsExtractorArgs.append("-pixsize");
        bandsExtractorArgs.append(QString::number(resolution));
    }
    return bandsExtractorArgs;
}

QStringList CropMaskHandler::GetCompressionArgs(const QString &inImg, const QString &outImg) {
    return { "-in", inImg, "-out", "\"" + outImg+"?gdal:co:COMPRESS=DEFLATE\"", "int16" };
}

QStringList CropMaskHandler::GetConfusionMatrixArgs(const QString &inRaster, const QString &outCsv, const QString &refIn, const QString &refType,  const QString &refField) {
    QStringList retList = { "-in", inRaster, "-out", outCsv, "-ref", refType,"-ref." + refType + ".in", refIn, "-nodatalabel", "-10000"};
    if(refField.length() > 0) {
        retList.append("-ref." + refType + ".field");
        retList.append(refField);
    }
    return retList;
}

QStringList CropMaskHandler::GetGdalWarpArgs(const QString &inImg, const QString &outImg, const QString &dtsNoData,
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

QStringList CropMaskHandler::GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                    const QStringList &listProducts, const QList<CropMaskProductFormatterParams> &productParams) {

    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot", targetFolder,
                                         "-fileclass", "SVT1",
                                         "-level", "L4A",
                                         "-baseline", "01.00",
                                         "-siteid", QString::number(event.siteId),
                                         "-processor", "cropmask",
                                         "-outprops", outPropsPath};
    productFormatterArgs += "-il";
    productFormatterArgs += listProducts;

    productFormatterArgs += "-processor.cropmask.file";
    for(const CropMaskProductFormatterParams &params: productParams) {
        productFormatterArgs += GetProductFormatterTile(params.tileId);
        productFormatterArgs += params.crop_mask;
    }

    productFormatterArgs += "-processor.cropmask.rawfile";
    for(const CropMaskProductFormatterParams &params: productParams) {
        productFormatterArgs += GetProductFormatterTile(params.tileId);
        productFormatterArgs += params.raw_crop_mask;
    }

    productFormatterArgs += "-processor.cropmask.quality";
    for(const CropMaskProductFormatterParams &params: productParams) {
        productFormatterArgs += GetProductFormatterTile(params.tileId);
        productFormatterArgs += params.xml_validation_metrics;
    }

    productFormatterArgs += "-processor.cropmask.flags";
    for(const CropMaskProductFormatterParams &params: productParams) {
        productFormatterArgs += GetProductFormatterTile(params.tileId);
        productFormatterArgs += params.statusFlags;
    }

    return productFormatterArgs;
}


ProcessorJobDefinitionParams CropMaskHandler::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues)
{
    ProcessorJobDefinitionParams params;
    params.isValid = false;

    QDateTime seasonStartDate;
    QDateTime seasonEndDate;
    GetSeasonStartEndDates(ctx, siteId, seasonStartDate, seasonEndDate, requestOverrideCfgValues);
    QDateTime limitDate = seasonEndDate.addMonths(2);
    // extract the scheduled date
    QDateTime qScheduledDate = QDateTime::fromTime_t(scheduledDate);
    if(qScheduledDate > limitDate) {
        return params;
    }

    ConfigurationParameterValueMap cfgValues = ctx.GetConfigurationParameters("processor.l4a.", siteId, requestOverrideCfgValues);
    QString siteName = ctx.GetSiteShortName(siteId);
    // Get the reference dir
    QString refMap = cfgValues["processor.l4a.reference-map"].value;
    QString refDir = cfgValues["processor.l4a.reference_data_dir"].value;
    refDir = refDir.replace("{site}", siteName);

    // we might have an offset in days from starting the downloading products to start the L4A production
    int startSeasonOffset = cfgValues["processor.l4a.start_season_offset"].value.toInt();
    seasonStartDate = seasonStartDate.addDays(startSeasonOffset);

    QString shapeFile;
    QString referenceRasterFile;
    // if none of the reference files were found, cannot run the CropMask
    if(!ProcessorHandlerHelper::GetCropReferenceFile(refDir, shapeFile, referenceRasterFile) && !QFile::exists(refMap)) {
        return params;
    }
    if (!shapeFile.isEmpty()) {
        params.jsonParameters = "{ \"reference_polygons\": \"" + shapeFile + "\"}";
    } else if (!refMap.isEmpty()) {
        params.jsonParameters = "{ \"reference_raster\": \"" + refMap + "\"}";
    } else {
        params.jsonParameters = "{ \"reference_raster\": \"" + referenceRasterFile + "\"}";
    }

    // Get the start and end date for the production
    QDateTime endDate = qScheduledDate;
    QDateTime startDate = seasonStartDate;

    // get from the database the moving window for 1 year periodicity
    QString movingWindowStr = cfgValues["processor.l4a.moving_window_value"].value;
    int movingWindow = 0;
    if(movingWindowStr.length() > 0) {
        movingWindow = movingWindowStr.toInt();
    } else {
        movingWindow = 12;
    }
    // we have monthly production so we use the season date or the moving window of 12 months (or specified)
    if(endDate.addMonths(-movingWindow) > seasonStartDate) {
        startDate = endDate.addMonths(-movingWindow);
    } else {
        startDate = seasonStartDate;
    }

    params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    // Normally, we need at least 1 product available in order to be able to create a L4A product
    // but if we do not return here, the schedule block waiting for products (that might never happen)
    bool waitForAvailProcInputs = (cfgValues["processor.l4a.sched_wait_proc_inputs"].value.toInt() != 0);
    if((waitForAvailProcInputs == false) || (params.productList.size() > 0)) {
        params.isValid = true;
        Logger::debug(QStringLiteral("Executing scheduled job. Scheduler extracted for L4A a number "
                                     "of %1 products for site ID %2 with start date %3 and end date %4!")
                      .arg(params.productList.size())
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    } else {
        Logger::debug(QStringLiteral("Scheduled job for L4A and site ID %1 with start date %2 and end date %3 will not be executed (no products)!")
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    }

    return params;
}

