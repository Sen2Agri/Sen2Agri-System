#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include "cropmaskhandler.hpp"
#include "processorhandlerhelper.h"
#include "logger.hpp"

QList<std::reference_wrapper<TaskToSubmit>> CropMaskHandler::CreateInSituTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                                QList<std::reference_wrapper<const TaskToSubmit>> &outProdFormatterParentsList)
{
    outAllTasksList.append(TaskToSubmit{ "bands-extractor", {}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[0]}} );
    outAllTasksList.append(TaskToSubmit{ "temporal-resampling", {outAllTasksList[1]}} );
    outAllTasksList.append(TaskToSubmit{ "feature-extraction", {outAllTasksList[2]}} );
    outAllTasksList.append(TaskToSubmit{ "features-with-insitu", {outAllTasksList[3]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-images-statistics", {outAllTasksList[4]}} );
    outAllTasksList.append(TaskToSubmit{ "ogr2ogr", {outAllTasksList[5]}} );
    outAllTasksList.append(TaskToSubmit{ "ogr2ogr", {outAllTasksList[6]}} );
    outAllTasksList.append(TaskToSubmit{ "sample-selection", {outAllTasksList[7]}} );
    outAllTasksList.append(TaskToSubmit{ "train-images-classifier", {outAllTasksList[8]}} );
    outAllTasksList.append(TaskToSubmit{ "image-classifier", {outAllTasksList[9]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-confusion-matrix", {outAllTasksList[10]}} );
    outAllTasksList.append(TaskToSubmit{ "principal-component-analysis", {outAllTasksList[11]}} );
    outAllTasksList.append(TaskToSubmit{ "mean-shift-smoothing", {outAllTasksList[12]}} );
    outAllTasksList.append(TaskToSubmit{ "lsms-segmentation", {outAllTasksList[13]}} );
    outAllTasksList.append(TaskToSubmit{ "lsms-small-regions-merging", {outAllTasksList[14]}} );
    outAllTasksList.append(TaskToSubmit{ "majority-voting", {outAllTasksList[15]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[16]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-confusion-matrix", {outAllTasksList[17]}} );
    outAllTasksList.append(TaskToSubmit{ "image-compression", {outAllTasksList[18]}} );
    outAllTasksList.append(TaskToSubmit{ "xml-statistics", {outAllTasksList[19]}} );

    // product formatter needs completion of xml-statistics
    outProdFormatterParentsList.append(outAllTasksList[20]);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: outAllTasksList) {
        allTasksListRef.append(task);
    }
    return allTasksListRef;
}

QList<std::reference_wrapper<TaskToSubmit>> CropMaskHandler::CreateNoInSituTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                                QList<std::reference_wrapper<const TaskToSubmit>> &outProdFormatterParentsList)
{
    outAllTasksList.append(TaskToSubmit{ "bands-extractor", {}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[0]}} );
    outAllTasksList.append(TaskToSubmit{ "temporal-resampling", {outAllTasksList[1]}} );
    outAllTasksList.append(TaskToSubmit{ "feature-extraction", {outAllTasksList[2]}} );
    outAllTasksList.append(TaskToSubmit{ "data-smoothing", {outAllTasksList[3]}} );
    outAllTasksList.append(TaskToSubmit{ "features-without-insitu", {outAllTasksList[4]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-image-statistics", {outAllTasksList[5]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[6]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[7]}} );
    outAllTasksList.append(TaskToSubmit{ "erosion", {outAllTasksList[8]}} );
    outAllTasksList.append(TaskToSubmit{ "trimming", {outAllTasksList[9]}} );
    outAllTasksList.append(TaskToSubmit{ "train-images-classifier-new", {outAllTasksList[10]}} );
    outAllTasksList.append(TaskToSubmit{ "image-classifier", {outAllTasksList[11]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-confusion-matrix", {outAllTasksList[12]}} );
    outAllTasksList.append(TaskToSubmit{ "principal-component-analysis", {outAllTasksList[13]}} );
    outAllTasksList.append(TaskToSubmit{ "mean-shift-smoothing", {outAllTasksList[14]}} );
    outAllTasksList.append(TaskToSubmit{ "lsms-segmentation", {outAllTasksList[15]}} );
    outAllTasksList.append(TaskToSubmit{ "lsms-small-regions-merging", {outAllTasksList[16]}} );
    outAllTasksList.append(TaskToSubmit{ "majority-voting", {outAllTasksList[17]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[18]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-confusion-matrix", {outAllTasksList[19]}} );
    outAllTasksList.append(TaskToSubmit{ "image-compression", {outAllTasksList[20]}} );
    outAllTasksList.append(TaskToSubmit{ "xml-statistics", {outAllTasksList[21]}} );

    // product formatter needs completion of xml-statistics
    outProdFormatterParentsList.append(outAllTasksList[22]);

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
        ctx.SubmitTasks(event.jobId, allTasksListRef);
        HandleInsituJob(ctx, event, listProducts, globalExecInfos);
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
        ctx.SubmitTasks(event.jobId, allTasksListRef);
        HandleNoInsituJob(ctx, event, listProducts, globalExecInfos);
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

    ctx.SubmitTasks(event.jobId, {productFormatterTask});

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
            ctx.GetOutputPath(event.jobId, event.taskId, event.module) + "shape.prj";
        const auto &shapePrjEsriPath =
            ctx.GetOutputPath(event.jobId, event.taskId, event.module) + "shape_esri.prj";

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
                ctx.GetOutputPath(event.jobId, event.taskId, event.module) + "crop-mask-quality-metrics.txt";
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
                                      const QStringList &listProducts,
                                      CropMaskGlobalExecutionInfos &globalExecInfos)

{
    auto configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l4a.");
    auto resourceParameters = ctx.GetJobConfigurationParameters(event.jobId, "resources.");

    const auto &gdalwarpMem = resourceParameters["resources.gdalwarp.working-mem"];

    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    const auto &referencePolygons = parameters["reference_polygons"].toString();

    auto mission = configParameters["processor.l4a.mission"];
    if(mission.length() == 0) mission = "SENTINEL";

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
    if(minsize.length() == 0) minsize = "200";

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

    TaskToSubmit &bandsExtractorTask = allTasksList[0];
    TaskToSubmit &gdalWarpTask = allTasksList[1];
    TaskToSubmit &temporalResamplingTask = allTasksList[2];
    TaskToSubmit &featureExtractionTask = allTasksList[3];
    TaskToSubmit &featureWithInSituTask = allTasksList[4];
    TaskToSubmit &computeImagesStatisticsTask = allTasksList[5];
    TaskToSubmit &ogr2ogrTask = allTasksList[6];
    TaskToSubmit &ogr2ogrTask2 = allTasksList[7];
    TaskToSubmit &sampleSelectionTask = allTasksList[8];
    TaskToSubmit &trainImagesClassifierTask = allTasksList[9];
    TaskToSubmit &imageClassifierTask = allTasksList[10];
    TaskToSubmit &computeConfusionMatrixTask = allTasksList[11];

    TaskToSubmit &principalComponentAnalysisTask = allTasksList[12];
    TaskToSubmit &meanShiftSmoothingTask = allTasksList[13];
    TaskToSubmit &lsmsSegmentationTask = allTasksList[14];
    TaskToSubmit &lsmsSmallRegionsMergingTask = allTasksList[15];
    TaskToSubmit &majorityVotingTask = allTasksList[16];
    TaskToSubmit &gdalWarpTask2 = allTasksList[17];
    TaskToSubmit &computeConfusionMatrixTask2 = allTasksList[18];
    TaskToSubmit &convertTask = allTasksList[19];
    TaskToSubmit &xmlStatisticsTask = allTasksList[20];
    //TaskToSubmit productFormatterTask{ "product-formatter", { xmlStatisticsTask } };

    const auto &rawtocr = bandsExtractorTask.GetFilePath("rawtocr.tif");
    const auto &rawmask = bandsExtractorTask.GetFilePath("rawmask.tif");
    const auto &dates = bandsExtractorTask.GetFilePath("dates.txt");
    const auto &shape = bandsExtractorTask.GetFilePath("shape.shp");
    const auto &shapeEsriPrj = bandsExtractorTask.GetFilePath("shape_esri.prj");
    const auto &statusFlags = bandsExtractorTask.GetFilePath("statusFlags.tif");

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

    const auto &crop_mask_uncompressed = gdalWarpTask2.GetFilePath("crop_mask_uncompressed.tif");

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
        bandsExtractorTask.CreateStep("BandsExtractor", GetBandsExtractorArgs(mission, rawtocr, rawmask, statusFlags, dates, shape,
                                        listProducts, resolution)),
        gdalWarpTask.CreateStep("ClipRasterImage",
                              { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", gdalwarpMem, rawtocr, tocr }),
        gdalWarpTask.CreateStep("ClipRasterMask",
                              { "-dstnodata", "-10000", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", gdalwarpMem, rawmask, mask }),
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

        gdalWarpTask2.CreateStep("gdalwarp", { "-multi", "-wm", "2048", "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                              "-crop_to_cutline", crop_mask_uncut, crop_mask_uncompressed }),

        // This is specific only for Insitu data
        computeConfusionMatrixTask2.CreateStep("ComputeConfusionMatrix",GetConfusionMatrixArgs(crop_mask_uncompressed, confusion_matrix_validation, validationPolys, "vector", fieldName)),
        convertTask.CreateStep("CompressionCropMask", GetCompressionArgs(crop_mask_uncompressed, crop_mask)),
        convertTask.CreateStep("CompressionRawCrop", GetCompressionArgs(raw_crop_mask_uncompressed, raw_crop_mask)),

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
                                        const QStringList &listProducts,
                                        CropMaskGlobalExecutionInfos &globalExecInfos)
{
    auto configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l4a.");
    auto resourceParameters = ctx.GetJobConfigurationParameters(event.jobId, "resources.");

    const auto &gdalwarpMem = resourceParameters["resources.gdalwarp.working-mem"];

    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    const auto &reference = parameters["reference_raster"].toString();

    auto mission = configParameters["processor.l4a.mission"];
    if(mission.length() == 0) mission = "SPOT";

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
    if(minsize.length() == 0) minsize = "200";

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
    TaskToSubmit &bandsExtractorTask = allTasksList[0];
    TaskToSubmit &gdalWarpTask = allTasksList[1];
    TaskToSubmit &temporalResamplingTask = allTasksList[2];
    TaskToSubmit &featureExtractionTask = allTasksList[3];
    TaskToSubmit &dataSmoothingTask = allTasksList[4];
    TaskToSubmit &featuresWithoutInsituTask = allTasksList[5];
    TaskToSubmit &computeImageStatisticsTask = allTasksList[6];
    TaskToSubmit &gdalWarpTask2 = allTasksList[7];
    TaskToSubmit &gdalWarpTask2_1 = allTasksList[8];
    TaskToSubmit &erosionTask = allTasksList[9];
    TaskToSubmit &trimmingTask = allTasksList[10];
    TaskToSubmit &trainImagesClassifierNewTask = allTasksList[11];
    TaskToSubmit &imageClassifierTask = allTasksList[12];
    TaskToSubmit &computeConfusionMatrixTask = allTasksList[13];
    TaskToSubmit &principalComponentAnalysisTask = allTasksList[14];
    TaskToSubmit &meanShiftSmoothingTask = allTasksList[15];
    TaskToSubmit &lsmsSegmentationTask = allTasksList[16];
    TaskToSubmit &lsmsSmallRegionsMergingTask = allTasksList[17];
    TaskToSubmit &majorityVotingTask = allTasksList[18];
    TaskToSubmit &gdalWarpTask3 = allTasksList[19];
    TaskToSubmit &computeConfusionMatrixTask2 = allTasksList[20];
    TaskToSubmit &convertTask = allTasksList[21];
    TaskToSubmit &xmlStatisticsTask = allTasksList[22];

    const auto &rawtocr = bandsExtractorTask.GetFilePath("rawtocr.tif");
    const auto &rawmask = bandsExtractorTask.GetFilePath("rawmask.tif");
    const auto &dates = bandsExtractorTask.GetFilePath("dates.txt");
    const auto &shape = bandsExtractorTask.GetFilePath("shape.shp");
    const auto &shapeEsriPrj = bandsExtractorTask.GetFilePath("shape_esri.prj");
    const auto &statusFlags = bandsExtractorTask.GetFilePath("statusFlags.tif");

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

    const auto &reprojected_reference = gdalWarpTask2.GetFilePath("reprojected_reference.tif");
    const auto &crop_reference = gdalWarpTask2_1.GetFilePath("crop_reference.tif");

    const auto &eroded_reference = erosionTask.GetFilePath("eroded_reference.tif");

    const auto &pca = principalComponentAnalysisTask.GetFilePath("pca.tif");

    const auto &mean_shift_smoothing_spatial = meanShiftSmoothingTask.GetFilePath("mean_shift_smoothing_spatial.tif");
    const auto &mean_shift_smoothing = meanShiftSmoothingTask.GetFilePath("mean_shift_smoothing.tif");

    const auto &tmpfolder = lsmsSegmentationTask.GetFilePath("");
    const auto &segmented = lsmsSegmentationTask.GetFilePath("segmented.tif");

    const auto &segmented_merged = lsmsSmallRegionsMergingTask.GetFilePath("segmented_merged.tif");

    const auto &crop_mask_uncut = majorityVotingTask.GetFilePath("crop_mask_uncut.tif");

    const auto &crop_mask_uncompressed = gdalWarpTask3.GetFilePath("crop_mask_uncompressed.tif");

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
        bandsExtractorTask.CreateStep("BandsExtractor", GetBandsExtractorArgs(mission, rawtocr, rawmask, statusFlags, dates, shape, listProducts, resolution)),
        gdalWarpTask.CreateStep("ClipRasterImage",
                              { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", gdalwarpMem, rawtocr, tocr }),
        gdalWarpTask.CreateStep("ClipRasterMask",
                              { "-dstnodata", "-10000", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", gdalwarpMem, rawmask, mask }),

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


        // The following 2 steps cannot be inserted in one task because they need to be executed one after each other, not parallel
        gdalWarpTask2.CreateStep(
            "ReprojectRefMap", { "-multi", "-wm", "2048", "-dstnodata", "0", "-overwrite", "-t_srs", shapeEsriPrj, reference, reprojected_reference }),

        gdalWarpTask2_1.CreateStep("ClipRefMap", GetGdalWarpArgs(reprojected_reference, crop_reference, "0", "2048", shape, resolutionStr)),

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

        gdalWarpTask3.CreateStep("gdalwarp", GetGdalWarpArgs(crop_mask_uncut, crop_mask_uncompressed, "\"-10000\"", "2048", shape)),
        // This is specific only for NoInsitu data
        computeConfusionMatrixTask2.CreateStep("ComputeConfusionMatrix", GetConfusionMatrixArgs(crop_mask_uncompressed, confusion_matrix_validation, trimmed_reference_raster)),
        convertTask.CreateStep("CompressionCropMask", GetCompressionArgs(crop_mask_uncompressed, crop_mask)),
        convertTask.CreateStep("CompressionRawCrop", GetCompressionArgs(raw_crop_mask_uncompressed, raw_crop_mask)),

        xmlStatisticsTask.CreateStep("XMLStatistics", { "XMLStatistics", "-confmat", confusion_matrix_validation, "-quality", quality_metrics, "-root", "CropMask", "-out", xml_validation_metrics }),
    };

    CropMaskProductFormatterParams &productFormatterParams = globalExecInfos.prodFormatParams;
    productFormatterParams.crop_mask = crop_mask;
    productFormatterParams.raw_crop_mask = raw_crop_mask;
    productFormatterParams.raw_crop_mask = xml_validation_metrics;
    productFormatterParams.raw_crop_mask = statusFlags;
}

QStringList CropMaskHandler::GetBandsExtractorArgs(const QString &mission, const QString &outImg, const QString &mask, const QString &statusFlags,
                                                    const QString &outDates, const QString &shape, const QStringList &inputProducts, int resolution) {
    QStringList bandsExtractorArgs = { "BandsExtractor", "-mission", mission, "-out", outImg,
                                       "-mask", mask, "-statusflags", statusFlags, "-outdate", outDates,
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

    ConfigurationParameterValueMap cfgValues = ctx.GetConfigurationParameters("processor.l4a.", siteId, requestOverrideCfgValues);
    // Get the reference dir
    QString refDir = cfgValues["processor.l4a.reference_data_dir"].value;
    QString shapeFile;
    QString referenceRasterFile;
    // if none of the reference files were found, cannot run the CropMask
    if(!ProcessorHandlerHelper::GetCropReferenceFile(refDir, shapeFile, referenceRasterFile)) {
        return params;
    }
    if(!shapeFile.isEmpty()) {
        params.jsonParameters = "{ \"reference_polygons\": \"" + shapeFile + "\"}";
    } else {
        params.jsonParameters = "{ \"reference_raster\": \"" + referenceRasterFile + "\"}";
    }

    QDateTime seasonStartDate;
    QDateTime seasonEndDate;
    GetSeasonStartEndDates(ctx, siteId, seasonStartDate, seasonEndDate, requestOverrideCfgValues);
    // Get the start and end date for the production
    QDateTime endDate = QDateTime::fromTime_t(scheduledDate);
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
    if(params.productList.size() > 0) {
        params.isValid = true;
    }
    return params;
}

