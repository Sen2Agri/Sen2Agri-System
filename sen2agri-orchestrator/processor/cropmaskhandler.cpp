#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include "cropmaskhandler.hpp"
#include "processorhandlerhelper.h"

void CropMaskHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    const auto &referencePolygons = parameters["reference_polygons"].toString();
    if(referencePolygons.size() > 0) {
        HandleInsituJob(ctx, event);
    } else {
        HandleNoInsituJob(ctx, event);
    }
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

        QString productFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);

        // Insert the product into the database
        ctx.InsertProduct({ ProductType::L4AProductTypeId,
                            event.processorId,
                            event.jobId,
                            event.siteId,
                            productFolder,
                            QDateTime::currentDateTimeUtc(),
                            "name",
                            "quicklook",
                            "POLYGON(())" });

        // Now remove the job folder containing temporary files
        // TODO: Reinsert this line - commented only for debug purposes
        //RemoveJobFolder(ctx, event.jobId);
    }
}

void CropMaskHandler::HandleInsituJob(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)

{
    auto configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l4a.");
    auto resourceParameters = ctx.GetJobConfigurationParameters(event.jobId, "resources.");

    const auto &gdalwarpMem = resourceParameters["resources.gdalwarp.working-mem"];

    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    const auto &referencePolygons = parameters["reference_polygons"].toString();

    QStringList listProducts;
    const auto &inputProducts = parameters["input_products"].toArray();
    for (const auto &inputProduct : inputProducts) {
        listProducts.append(ctx.findProductFiles(inputProduct.toString()));
    }

    auto mission = parameters["processor.l4a.mission"].toString();
    if(mission.length() == 0) mission = "SPOT";

    const auto &resolution = parameters["resolution"].toInt();
    auto randomSeed = parameters["processor.l4a.random_seed"].toString();
    if(randomSeed.isEmpty())  randomSeed = "0";

    auto sampleRatio = configParameters["processor.l4a.sample-ratio"];
    if(sampleRatio.length() == 0) sampleRatio = "0.75";

    auto temporalResamplingMode = parameters["processor.l4a.temporal_resampling_mode"].toString();
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

    /* Tasks:

        BandsExtractor
        gdalwarp for reflectances
        gdalwarp for masks
        TemporalResampling
        FeatureExtraction

        FeaturesWithInsitu
        ComputeImagesStatistics
        ogr2ogr
        ogr2ogr
        SampleSelection
        TrainImagesClassifier
        ImageClassifier
        otbcli_ComputeConfusionMatrix

        PrincipalComponentAnalysis
        MeanShiftSmoothing
        LSMSSegmentation
        LSMSSmallRegionsMerging
        Segmentation
        MajorityVoting
        gdalwarp for crop mask
        otbcli_ComputeConfusionMatrix
        otbcli_Convert
        otbcli_Convert
        XMLStatistics
        ProductFormatter
    */
    TaskToSubmit bandsExtractorTask{ "bands-extractor", {} };
    TaskToSubmit gdalWarpTask{ "gdalwarp", { bandsExtractorTask } };
    TaskToSubmit temporalResamplingTask { "temporal-resampling", { gdalWarpTask } };
    TaskToSubmit featureExtractionTask { "feature-extraction", { temporalResamplingTask } };
    TaskToSubmit featureWithInSituTask { "features-with-insitu", { featureExtractionTask } };
    TaskToSubmit computeImagesStatisticsTask{ "compute-images-statistics", { featureWithInSituTask } };
    TaskToSubmit ogr2ogrTask{ "ogr2ogr", { computeImagesStatisticsTask } };
    TaskToSubmit ogr2ogrTask2{ "ogr2ogr", { ogr2ogrTask } };
    TaskToSubmit sampleSelectionTask{ "sample-selection", { ogr2ogrTask2 } };
    TaskToSubmit trainImagesClassifierTask{ "train-images-classifier",
                                        { sampleSelectionTask } };
    TaskToSubmit imageClassifierTask{ "image-classifier", { trainImagesClassifierTask } };
    TaskToSubmit computeConfusionMatrixTask{ "compute-confusion-matrix", { imageClassifierTask } };

    TaskToSubmit principalComponentAnalysisTask { "principal-component-analysis", { computeConfusionMatrixTask } };
    TaskToSubmit meanShiftSmoothingTask { "mean-shift-smoothing", { principalComponentAnalysisTask } };
    TaskToSubmit lsmsSegmentationTask { "lsms-segmentation", { meanShiftSmoothingTask} };
    TaskToSubmit lsmsSmallRegionsMergingTask { "lsms-small-regions-merging", { lsmsSegmentationTask } };
    TaskToSubmit majorityVotingTask { "majority-voting", { lsmsSmallRegionsMergingTask } };
    TaskToSubmit gdalWarpTask2 { "gdalwarp", { majorityVotingTask} };
    TaskToSubmit computeConfusionMatrixTask2 { "compute-confusion-matrix", { gdalWarpTask2 } };
    TaskToSubmit convertTask { "image-compression", { computeConfusionMatrixTask2 } };
    TaskToSubmit xmlStatisticsTask { "xml-statistics", { convertTask } };
    TaskToSubmit productFormatterTask{ "product-formatter", { xmlStatisticsTask } };

    ctx.SubmitTasks(event.jobId,
                    { bandsExtractorTask, gdalWarpTask, temporalResamplingTask, featureExtractionTask, featureWithInSituTask,
                      computeImagesStatisticsTask, ogr2ogrTask, ogr2ogrTask2, sampleSelectionTask, trainImagesClassifierTask,
                      imageClassifierTask, computeConfusionMatrixTask, principalComponentAnalysisTask, meanShiftSmoothingTask,
                      lsmsSegmentationTask, lsmsSmallRegionsMergingTask, majorityVotingTask, gdalWarpTask2,
                    computeConfusionMatrixTask2, convertTask, xmlStatisticsTask, productFormatterTask});

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

    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    QString tileId = ProcessorHandlerHelper::GetTileId(listProducts);

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

    NewStepList steps = {
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

    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot", targetFolder,
                                         "-fileclass", "SVT1",
                                         "-level", "L4A",
                                         "-baseline", "01.00",
                                         "-processor", "cropmask",
                                         "-processor.cropmask.file", tileId, crop_mask,
                                         "-processor.cropmask.rawfile", tileId, raw_crop_mask,
                                         "-processor.cropmask.quality", tileId, xml_validation_metrics,
                                         "-processor.cropmask.flags", tileId, statusFlags,
                                         "-il"};
    productFormatterArgs += listProducts;
    steps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));

    ctx.SubmitSteps(steps);

}

void CropMaskHandler::HandleNoInsituJob(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    auto configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l4a.");
    auto resourceParameters = ctx.GetJobConfigurationParameters(event.jobId, "resources.");

    const auto &gdalwarpMem = resourceParameters["resources.gdalwarp.working-mem"];

    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    QStringList listProducts;
    const auto &inputProducts = parameters["input_products"].toArray();
    for (const auto &inputProduct : inputProducts) {
        listProducts.append(ctx.findProductFiles(inputProduct.toString()));
    }

    const auto &reference = parameters["reference_raster"].toString();

    auto mission = parameters["processor.l4a.mission"].toString();
    if(mission.length() == 0) mission = "SPOT";

    const auto &resolution = parameters["resolution"].toInt();
    const auto &resolutionStr = QString::number(resolution);
    auto randomSeed = parameters["processor.l4a.random_seed"].toString();
    if(randomSeed.isEmpty())  randomSeed = "0";

    auto sampleRatio = configParameters["processor.l4a.sample-ratio"];
    if(sampleRatio.length() == 0) sampleRatio = "0.75";

    auto temporalResamplingMode = parameters["processor.l4a.temporal_resampling_mode"].toString();
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

    auto nbtrsample = configParameters["processor.l4a.nb-training-samples"];
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

    /* Tasks:

        BandsExtractor
        gdalwarp for reflectances
        gdalwarp for masks
        TemporalResampling
        FeatureExtraction

        DataSmoothing (2 times)
        FeaturesWithoutInsitu
        ComputeImagesStatistics
        gdalwarp (2 times)
        Erosion
        Trimming
        TrainImagesClassifierNew
        ImageClassifier
        otbcli_ComputeConfusionMatrix

        PrincipalComponentAnalysis
        MeanShiftSmoothing
        LSMSSegmentation
        LSMSSmallRegionsMerging
        Segmentation
        MajorityVoting
        gdalwarp for crop mask
        otbcli_ComputeConfusionMatrix
        otbcli_Convert
        otbcli_Convert
        XMLStatistics
        ProductFormatter
    */
    TaskToSubmit bandsExtractorTask{ "bands-extractor", {} };
    TaskToSubmit gdalWarpTask{ "gdalwarp", { bandsExtractorTask } };
    TaskToSubmit temporalResamplingTask { "temporal-resampling", { gdalWarpTask } };
    TaskToSubmit featureExtractionTask { "feature-extraction", { temporalResamplingTask } };

    TaskToSubmit dataSmoothingTask { "data-smoothing", { featureExtractionTask } };
    TaskToSubmit featuresWithoutInsituTask{ "features-without-insitu", { dataSmoothingTask } };
    TaskToSubmit computeImageStatisticsTask{ "compute-image-statistics", { featuresWithoutInsituTask } };
    TaskToSubmit gdalWarpTask2{ "gdalwarp", { computeImageStatisticsTask } };
    TaskToSubmit gdalWarpTask2_1{ "gdalwarp", { gdalWarpTask2 } };
    TaskToSubmit erosionTask{ "erosion", { gdalWarpTask2_1 } };
    TaskToSubmit trimmingTask{ "trimming", { erosionTask } };
    TaskToSubmit trainImagesClassifierNewTask{ "train-images-classifier-new", { trimmingTask } };
    TaskToSubmit imageClassifierTask{ "image-classifier", { trainImagesClassifierNewTask } };
    TaskToSubmit computeConfusionMatrixTask{ "compute-confusion-matrix", { imageClassifierTask } };

    TaskToSubmit principalComponentAnalysisTask { "principal-component-analysis", { computeConfusionMatrixTask } };
    TaskToSubmit meanShiftSmoothingTask { "mean-shift-smoothing", { principalComponentAnalysisTask } };
    TaskToSubmit lsmsSegmentationTask { "lsms-segmentation", { meanShiftSmoothingTask} };
    TaskToSubmit lsmsSmallRegionsMergingTask { "lsms-small-regions-merging", { lsmsSegmentationTask } };
    TaskToSubmit majorityVotingTask { "majority-voting", { lsmsSmallRegionsMergingTask } };
    TaskToSubmit gdalWarpTask3 { "gdalwarp", { majorityVotingTask} };
    TaskToSubmit computeConfusionMatrixTask2 { "compute-confusion-matrix", { gdalWarpTask3 } };
    TaskToSubmit convertTask { "image-compression", { computeConfusionMatrixTask2 } };
    TaskToSubmit xmlStatisticsTask { "xml-statistics", { convertTask } };
    TaskToSubmit productFormatterTask{ "product-formatter", { xmlStatisticsTask } };

    ctx.SubmitTasks(event.jobId,
                    { bandsExtractorTask, gdalWarpTask, temporalResamplingTask, featureExtractionTask, dataSmoothingTask,
                      featuresWithoutInsituTask, computeImageStatisticsTask, gdalWarpTask2, gdalWarpTask2_1, erosionTask, trimmingTask,
                      trainImagesClassifierNewTask, imageClassifierTask, computeConfusionMatrixTask, principalComponentAnalysisTask,
                      meanShiftSmoothingTask, lsmsSegmentationTask, lsmsSmallRegionsMergingTask, majorityVotingTask, gdalWarpTask3,
                    computeConfusionMatrixTask2, convertTask, xmlStatisticsTask, productFormatterTask});

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

    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    QString tileId = ProcessorHandlerHelper::GetTileId(listProducts);

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

    NewStepList steps = {
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

    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot", targetFolder,
                                         "-fileclass", "SVT1",
                                         "-level", "L4A",
                                         "-baseline", "01.00",
                                         "-processor", "cropmask",
                                         "-processor.cropmask.file", tileId, crop_mask,
                                         "-processor.cropmask.rawfile", tileId, raw_crop_mask,
                                         "-processor.cropmask.quality", tileId, xml_validation_metrics,
                                         "-processor.cropmask.flags", tileId, statusFlags,
                                         "-il"};
    productFormatterArgs += listProducts;
    steps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));

    ctx.SubmitSteps(steps);
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

ProcessorJobDefinitionParams CropMaskHandler::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues)
{
    ProcessorJobDefinitionParams params;
    params.isValid = false;

    // TODO: get from the database the moving window for 1 month periodicity
    ConfigurationParameterValueMap cfgValues = ctx.GetConfigurationParameters("processor.l4a.", siteId, requestOverrideCfgValues);
    // Get the reference dir
    QString refDir = cfgValues["processor.l4a.reference_data_dir"].value;
    // if folder not defined, cannot run the CropMask
    if(refDir.isEmpty()) {
        return params;
    }
    QDirIterator it(refDir, QStringList() << "*.shp", QDir::Files);
    // get the last shape file found
    QString shapeFile;
    QString referenceRasterFile;
    while (it.hasNext()) {
        shapeFile = it.next();
    }
    // if no shape file was found, search for the reference raster for no-insitu case
    if(shapeFile.isEmpty()) {
        QDirIterator it2(refDir, QStringList() << "*.tif", QDir::Files);
        // get the last reference raster file found
        while (it2.hasNext()) {
            referenceRasterFile = it2.next();
        }
    }
    // no insitu shape or reference raster found
    if(shapeFile.isEmpty() && referenceRasterFile.isEmpty()) {
        return params;
    }
    if(!shapeFile.isEmpty()) {
        params.jsonParameters = "{ \"reference_polygons\": \"" + shapeFile + "\"}";
    } else {
        params.jsonParameters = "{ \"reference_raster\": \"" + referenceRasterFile + "\"}";
    }

    // Get the start and end date for the production
    ConfigurationParameterValueMap seasonCfgValues = ctx.GetConfigurationParameters(SEASON_CFG_KEY_PREFIX, -1, requestOverrideCfgValues);
    QDateTime startSeasonDate = QDateTime::fromString(seasonCfgValues[START_OF_SEASON_CFG_KEY].value, "yyyymmdd");
    QDateTime endSeasonDate = QDateTime::fromString(seasonCfgValues[END_OF_SEASON_CFG_KEY].value, "yyyymmdd");
    QDateTime endDate = QDateTime::fromTime_t(scheduledDate);
    QDateTime startDate;

    qint64 daysFromStartOfSeason = startSeasonDate.daysTo(endDate);
    // if we are 1 month after the end of season, we do no processing
    if(endSeasonDate.daysTo(endDate) > 30) {
        return params;
    }

    // if we have less than aprox 6 months from the start of the season, we do no processing
    // as the production starts 6 months after the start of the season
    if(daysFromStartOfSeason < (5 * 30) + 15 ) {
        return params;
    } else {
        // we have the first production
        if(daysFromStartOfSeason < (6 * 30) + 10 ) {
            startDate = startSeasonDate;
        } else {
            QString movingWindowStr = cfgValues["processor.l4a.moving_window_value"].value;
            int movingWindow = 0;
            if(movingWindowStr.length() > 0) {
                movingWindow = movingWindowStr.toInt();
            } else {
                movingWindow = 12;
            }
            // we have monthly production so we use the season date or the moving window of 12 months (or specified)
            if(endDate.addMonths(-movingWindow) > startSeasonDate) {
                startDate = endDate.addMonths(-movingWindow);
            } else {
                startDate = startSeasonDate;
            }
        }
    }
    params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    if(params.productList.size() > 0) {
        params.isValid = true;
    }
    return params;
}

