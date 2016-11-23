#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "croptypehandler.hpp"
#include "processorhandlerhelper.h"
#include "logger.hpp"

QList<std::reference_wrapper<TaskToSubmit>> CropTypeHandler::CreateMaskTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                                QList<std::reference_wrapper<const TaskToSubmit>> &outProdFormatterParentsList)
{
    outAllTasksList.append(TaskToSubmit{ "quality-flags-extractor", {}} );
    outAllTasksList.append(TaskToSubmit{ "bands-extractor", {}} );
    outAllTasksList.append(TaskToSubmit{ "ogr2ogr", {outAllTasksList[1]}} );
    outAllTasksList.append(TaskToSubmit{ "ogr2ogr", {outAllTasksList[2]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[3]}} );
    outAllTasksList.append(TaskToSubmit{ "sample-selection", {outAllTasksList[4]}} );
    outAllTasksList.append(TaskToSubmit{ "temporal-resampling", {outAllTasksList[5]}} );
    outAllTasksList.append(TaskToSubmit{ "feature-extraction", {outAllTasksList[6]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-images-statistics", {outAllTasksList[7]}} );
    outAllTasksList.append(TaskToSubmit{ "train-images-classifier", {outAllTasksList[5],
                                                                     outAllTasksList[8]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[9]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[10]}} );
    outAllTasksList.append(TaskToSubmit{ "image-classifier", {outAllTasksList[11]}} );
    outAllTasksList.append(TaskToSubmit{ "otbcli_BandMath", {outAllTasksList[12]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[13]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[14]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-confusion-matrix", {outAllTasksList[15]}} );
    outAllTasksList.append(TaskToSubmit{ "xml-statistics", {outAllTasksList[16]}} );

    // product formatter needs completion of xml-statistics
    outProdFormatterParentsList.append(outAllTasksList[17]);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: outAllTasksList) {
        allTasksListRef.append(task);
    }
    return allTasksListRef;
}

QList<std::reference_wrapper<TaskToSubmit>> CropTypeHandler::CreateNoMaskTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                                QList<std::reference_wrapper<const TaskToSubmit>> &outProdFormatterParentsList)
{
    outAllTasksList.append(TaskToSubmit{ "quality-flags-extractor", {}} );
    outAllTasksList.append(TaskToSubmit{ "bands-extractor", {}} );
    outAllTasksList.append(TaskToSubmit{ "ogr2ogr", {outAllTasksList[1]}} );
    outAllTasksList.append(TaskToSubmit{ "ogr2ogr", {outAllTasksList[2]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[3]}} );
    outAllTasksList.append(TaskToSubmit{ "sample-selection", {outAllTasksList[4]}} );
    outAllTasksList.append(TaskToSubmit{ "temporal-resampling", {outAllTasksList[5]}} );
    outAllTasksList.append(TaskToSubmit{ "feature-extraction", {outAllTasksList[6]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-images-statistics", {outAllTasksList[7]}} );
    outAllTasksList.append(TaskToSubmit{ "train-images-classifier", {outAllTasksList[5],
                                                                     outAllTasksList[8]}} );
    outAllTasksList.append(TaskToSubmit{ "image-classifier", {outAllTasksList[9]}} );
    outAllTasksList.append(TaskToSubmit{ "gdalwarp", {outAllTasksList[10]}} );
    outAllTasksList.append(TaskToSubmit{ "compute-confusion-matrix", {outAllTasksList[11]}} );
    outAllTasksList.append(TaskToSubmit{ "xml-statistics", {outAllTasksList[12]}} );

    // product formatter needs completion of xml-statistics
    outProdFormatterParentsList.append(outAllTasksList[13]);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: outAllTasksList) {
        allTasksListRef.append(task);
    }
    return allTasksListRef;
}


void CropTypeHandler::GetJobConfig(EventProcessingContext &ctx,const JobSubmittedEvent &event,CropTypeJobConfig &cfg) {
    auto configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l4b.");
    auto resourceParameters = ctx.GetJobConfigurationParameters(event.jobId, "resources.working-mem");
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    cfg.jobId = event.jobId;
    cfg.siteId = event.siteId;
    cfg.resolution = 0;
    if(!GetParameterValueAsInt(parameters, "resolution", cfg.resolution) ||
            cfg.resolution == 0) {
        cfg.resolution = 10;
    }

    cfg.referencePolygons = parameters["reference_polygons"].toString();
    // get the crop mask
    cfg.cropMask = parameters["crop_mask"].toString();

    cfg.lutPath = configParameters["processor.l4b.lut_path"];
    cfg.appsMem = resourceParameters["resources.working-mem"];

    cfg.randomSeed = configParameters["processor.l4b.random_seed"];
    if(cfg.randomSeed.isEmpty())  cfg.randomSeed = "0";

    cfg.sampleRatio = configParameters["processor.l4b.sample-ratio"];
    if(cfg.sampleRatio.length() == 0) cfg.sampleRatio = "0.75";

    cfg.temporalResamplingMode = configParameters["processor.l4b.temporal_resampling_mode"];
    if(cfg.temporalResamplingMode != "resample") cfg.temporalResamplingMode = "gapfill";

    cfg.classifier = configParameters["processor.l4b.classifier"];
    if(cfg.classifier.length() == 0) cfg.classifier = "rf";

    cfg.fieldName = configParameters["processor.l4b.classifier.field"];
    if(cfg.fieldName.length() == 0) cfg.fieldName = "CODE";

    cfg.classifierRfNbTrees = configParameters["processor.l4b.classifier.rf.nbtrees"];
    if(cfg.classifierRfNbTrees.length() == 0) cfg.classifierRfNbTrees = "100";

    cfg.classifierRfMinSamples = configParameters["processor.l4b.classifier.rf.min"];
    if(cfg.classifierRfMinSamples.length() == 0) cfg.classifierRfMinSamples = "25";

    cfg.classifierRfMaxDepth = configParameters["processor.l4b.classifier.rf.max"];
    if(cfg.classifierRfMaxDepth.length() == 0) cfg.classifierRfMaxDepth = "25";

    cfg.classifierSvmKernel = configParameters["processor.l4b.classifier.svm.k"];
    cfg.classifierSvmOptimize = configParameters["processor.l4b.classifier.svm.opt"];
}

void CropTypeHandler::HandleNewTilesList(EventProcessingContext &ctx,
                                         const CropTypeJobConfig &cfg, const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                         const QString &cropMask, CropTypeGlobalExecutionInfos &globalExecInfos)
{
    bool bHasCropMask = !cropMask.isEmpty();

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    QList<std::reference_wrapper<const TaskToSubmit>> &prodFormParTsksList = globalExecInfos.prodFormatParams.parentsTasksRef;
    if(bHasCropMask) {
        allTasksListRef = CreateMaskTasksForNewProducts(globalExecInfos.allTasksList, prodFormParTsksList);
        SubmitTasks(ctx, cfg.jobId, allTasksListRef);
        HandleMaskTilesList(ctx, cfg, tileTemporalFilesInfo, cropMask, globalExecInfos);
    } else {
        allTasksListRef = CreateNoMaskTasksForNewProducts(globalExecInfos.allTasksList, prodFormParTsksList);
        SubmitTasks(ctx, cfg.jobId, allTasksListRef);
        HandleNoMaskTilesList(ctx, cfg, tileTemporalFilesInfo, globalExecInfos);
    }
}


void CropTypeHandler::HandleMaskTilesList(EventProcessingContext &ctx,
                                         const CropTypeJobConfig &cfg, const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                         const QString &cropMask, CropTypeGlobalExecutionInfos &globalExecInfos)
{
    QStringList listProducts = ProcessorHandlerHelper::GetTemporalTileFiles(tileTemporalFilesInfo);
    auto mission = ProcessorHandlerHelper::GetMissionNamePrefixFromSatelliteId(tileTemporalFilesInfo.primarySatelliteId);
    const auto &resolutionStr = QString::number(cfg.resolution);

    QList<TaskToSubmit> &allTasksList = globalExecInfos.allTasksList;

    int curTaskIdx = 0;
    TaskToSubmit &qualityFlagsExtractorTask = allTasksList[curTaskIdx++];
    TaskToSubmit &bandsExtractor = allTasksList[curTaskIdx++];
    TaskToSubmit &reprojectPolys = allTasksList[curTaskIdx++];
    TaskToSubmit &clipPolys = allTasksList[curTaskIdx++];
    TaskToSubmit &clipRaster = allTasksList[curTaskIdx++];
    TaskToSubmit &sampleSelection = allTasksList[curTaskIdx++];
    TaskToSubmit &temporalResampling = allTasksList[curTaskIdx++];
    TaskToSubmit &featureExtraction = allTasksList[curTaskIdx++];
    TaskToSubmit &computeImagesStatistics = allTasksList[curTaskIdx++];
    TaskToSubmit &trainImagesClassifier = allTasksList[curTaskIdx++];
    TaskToSubmit &reprojectCropMask = allTasksList[curTaskIdx++];
    TaskToSubmit &cropCropMask = allTasksList[curTaskIdx++];
    TaskToSubmit &imageClassifier = allTasksList[curTaskIdx++];
    TaskToSubmit &bandMathMasking = allTasksList[curTaskIdx++];
    TaskToSubmit &clipCropTypeNoMask = allTasksList[curTaskIdx++];
    TaskToSubmit &clipCropType = allTasksList[curTaskIdx++];
    TaskToSubmit &computeConfusionMatrix = allTasksList[curTaskIdx++];
    TaskToSubmit &xmlMetrics = allTasksList[curTaskIdx++];

    SubmitTasks(ctx, cfg.jobId,
                    { qualityFlagsExtractorTask, bandsExtractor, reprojectPolys, clipPolys, clipRaster, sampleSelection, temporalResampling,
                      featureExtraction, computeImagesStatistics, trainImagesClassifier,
                      reprojectCropMask, cropCropMask, imageClassifier, bandMathMasking, clipCropTypeNoMask, clipCropType, computeConfusionMatrix, xmlMetrics });

    const auto &trainingPolys = sampleSelection.GetFilePath("training_polygons.shp");
    const auto &validationPolys = sampleSelection.GetFilePath("validation_polygons.shp");
    const auto &lut = sampleSelection.GetFilePath("lut.txt");
    const auto &rawtocr = bandsExtractor.GetFilePath("rawtocr.tif");
    const auto &rawmask = bandsExtractor.GetFilePath("rawmask.tif");
    const auto &dates = bandsExtractor.GetFilePath("dates.txt");
    const auto &shape = bandsExtractor.GetFilePath("shape.shp");
    const auto &statusFlags = qualityFlagsExtractorTask.GetFilePath("statusFlags.tif");
    const auto &shapeEsriPrj = bandsExtractor.GetFilePath("shape_esri.prj");

    const auto &refPolysReprojected = reprojectPolys.GetFilePath("reference_polygons_reproject.shp");
    const auto &refPolysClipped = clipPolys.GetFilePath("reference_clip.shp");
    const auto &tocr = clipRaster.GetFilePath("tocr.tif");
    const auto &mask = clipRaster.GetFilePath("mask.tif");
    const auto &rtocr = temporalResampling.GetFilePath("rtocr.tif");
    const auto &feFts = featureExtraction.GetFilePath("fts.tif");
    const auto &statistics = computeImagesStatistics.GetFilePath("statistics.xml");
    const auto &model = trainImagesClassifier.GetFilePath("model.txt");
    const auto &confusionMatrix = trainImagesClassifier.GetFilePath("confusion-matrix.csv");

    const auto &reprojectedCropMaskFile = reprojectCropMask.GetFilePath("reprojected_crop_mask.tif");
    const auto &croppedCropMaskFile = cropCropMask.GetFilePath("cropped_crop_mask.tif");

    const auto &cropTypeMapUncut = imageClassifier.GetFilePath("crop_type_map_uncut.tif");
    const auto &cropTypeMapNoMaskUncut = imageClassifier.GetFilePath("crop_type_map_nomask_uncut.tif");

    const auto &cropTypeMapMaskUncompressed = clipCropType.GetFilePath("crop_type_map_mask_uncompressed.tif");
    const auto &cropTypeMapNoMaskUncompressed = clipCropType.GetFilePath("crop_type_map_nomask_uncompressed.tif");

    const auto &confusionMatrixValidation = computeConfusionMatrix.GetFilePath("confusion-matrix-validation.csv");
    const auto &qualityMetrics = computeConfusionMatrix.GetFilePath("quality_metrics.txt");
    const auto &xmlValidationMetrics = xmlMetrics.GetFilePath("validation-metrics.xml");

    QStringList qualityFlagsExtractorArgs = { "QualityFlagsExtractor",
                                              "-out", "\"" + statusFlags+"?gdal:co:COMPRESS=DEFLATE\"",
                                              "-mission", mission};
    QStringList bandsExtractorArgs = { "BandsExtractor", "-out", rawtocr,  "-mask", rawmask,
                                       "-outdate", dates,  "-shape", shape, "-mission", mission };
    if (cfg.resolution) {
        qualityFlagsExtractorArgs.append("-pixsize");
        qualityFlagsExtractorArgs.append(resolutionStr);
        bandsExtractorArgs.append("-pixsize");
        bandsExtractorArgs.append(resolutionStr);
    }
    qualityFlagsExtractorArgs.append("-il");
    qualityFlagsExtractorArgs += listProducts;
    bandsExtractorArgs.append("-il");
    bandsExtractorArgs += listProducts;

    QStringList trainImagesClassifierArgs = {
        "-io.il",      feFts,       "-io.vd",      trainingPolys, "-io.imstat",     statistics,
        "-rand",       cfg.randomSeed,  "-sample.bm",  "0",           "-io.confmatout", confusionMatrix,
        "-io.out",     model,       "-sample.mt",  "-1",          "-sample.mv",     "-1",
        "-sample.vtr", "0.1",       "-sample.vfn", cfg.fieldName,        "-classifier",    cfg.classifier };

    if (cfg.classifier == "rf") {
        trainImagesClassifierArgs.append("-classifier.rf.nbtrees");
        trainImagesClassifierArgs.append(cfg.classifierRfNbTrees);
        trainImagesClassifierArgs.append("-classifier.rf.min");
        trainImagesClassifierArgs.append(cfg.classifierRfMinSamples);
        trainImagesClassifierArgs.append("-classifier.rf.max");
        trainImagesClassifierArgs.append(cfg.classifierRfMaxDepth);
    } else {
        trainImagesClassifierArgs.append("-classifier.svm.k");
        trainImagesClassifierArgs.append(cfg.classifierSvmKernel);
        trainImagesClassifierArgs.append("-classifier.svm.opt");
        trainImagesClassifierArgs.append(cfg.classifierSvmOptimize);
    }

    globalExecInfos.allStepsList = {
        qualityFlagsExtractorTask.CreateStep("QualityFlagsExtractor", qualityFlagsExtractorArgs),
        bandsExtractor.CreateStep("BandsExtractor", bandsExtractorArgs),
        reprojectPolys.CreateStep(
            "ReprojectPolys", { "-t_srs", shapeEsriPrj, "-overwrite", refPolysReprojected, cfg.referencePolygons }),
        clipPolys.CreateStep("ClipPolys",
                             { "-clipsrc", shape, refPolysClipped, refPolysReprojected }),
        clipRaster.CreateStep("ClipRasterImage",
                              { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", cfg.appsMem, rawtocr, tocr }),
        clipRaster.CreateStep("ClipRasterMask",
                              { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", cfg.appsMem, rawmask, mask }),
        sampleSelection.CreateStep("SampleSelection", { "SampleSelection", "-ref", refPolysClipped,
                                                        "-ratio", cfg.sampleRatio, "-lut", lut, "-tp",
                                                        trainingPolys, "-vp", validationPolys,
                                                        "-seed", cfg.randomSeed}),
        temporalResampling.CreateStep("TemporalResampling",
                                      { "TemporalResampling", "-tocr", tocr, "-mask", mask, "-ind",
                                        dates, "-sp", "SENTINEL", "5", "SPOT", "5", "LANDSAT", "16",
                                        "-rtocr", rtocr, "-mode", cfg.temporalResamplingMode}),
        featureExtraction.CreateStep("FeatureExtraction",
                                     { "FeatureExtraction", "-rtocr", rtocr, "-fts", feFts }),

        computeImagesStatistics.CreateStep("ComputeImagesStatistics",
                                           { "-il", feFts, "-out", statistics }),
        reprojectCropMask.CreateStep("ReprojectCropMask", {"-multi", "-wm", cfg.appsMem,
                                             "-dstnodata", "0", "-overwrite", "-t_srs", shapeEsriPrj, cropMask, reprojectedCropMaskFile}),
        cropCropMask.CreateStep("CroppedCropMask", {"-multi", "-wm", cfg.appsMem,  "-dstnodata", "0",
                                             "-overwrite", "-tr", resolutionStr, resolutionStr,"-cutline", shape,
                                              "-crop_to_cutline",reprojectedCropMaskFile, croppedCropMaskFile}),
        trainImagesClassifier.CreateStep("TrainImagesClassifier", trainImagesClassifierArgs),

        imageClassifier.CreateStep("ImageClassifier", { "-in",    feFts, "-imstat", statistics,
                                                        "-model", model, "-out",    cropTypeMapNoMaskUncut}),
        bandMathMasking.CreateStep("BandMathMasking", {"BandMath", "-il", croppedCropMaskFile, cropTypeMapNoMaskUncut, "-out", cropTypeMapUncut +
                                                       "?gdal:co:COMPRESS=DEFLATE", "-exp", "\"im1b1 == 1 ? im2b1 : 0\""}),
        clipCropTypeNoMask.CreateStep("ClipCropTypeMapNoMask", {"-dstnodata", "\"-10000\"", "-co", "COMPRESS=LZW", "-ot", "int16", "-overwrite", "-cutline", shape,
                                                                "-crop_to_cutline", cropTypeMapNoMaskUncut, cropTypeMapNoMaskUncompressed}),
        clipCropType.CreateStep("ClipCropTypeMap", { "-dstnodata", "\"-10000\"", "-co", "COMPRESS=LZW", "-ot", "int16", "-overwrite", "-cutline", shape,
                                  "-crop_to_cutline", cropTypeMapUncut, cropTypeMapMaskUncompressed }),
        computeConfusionMatrix.CreateStep("ComputeConfusionMatrix",
                                          { "-in", cropTypeMapMaskUncompressed, "-out",
                                            confusionMatrixValidation, "-ref", "vector",
                                            "-ref.vector.in", validationPolys, "-ref.vector.field",
                                            cfg.fieldName, "-nodatalabel", "-10000" }),
        xmlMetrics.CreateStep("XMLMetrics", { "XMLStatistics", "-confmat",
                                              confusionMatrixValidation, "-quality", qualityMetrics,
                                              "-root", "CropType", "-out", xmlValidationMetrics })
    };
    CropTypeProductFormatterParams &productFormatterParams = globalExecInfos.prodFormatParams;
    productFormatterParams.cropTypeMap = cropTypeMapMaskUncompressed;
    productFormatterParams.rawCropTypeMap = cropTypeMapNoMaskUncompressed;
    productFormatterParams.xmlValidationMetrics = xmlValidationMetrics;
    productFormatterParams.statusFlags = statusFlags;
}


void CropTypeHandler::HandleNoMaskTilesList(EventProcessingContext &ctx,
                                         const CropTypeJobConfig &cfg, const TileTemporalFilesInfo &tileTemporalFilesInfo, CropTypeGlobalExecutionInfos &globalExecInfos)
{
    QStringList listProducts = ProcessorHandlerHelper::GetTemporalTileFiles(tileTemporalFilesInfo);
    auto mission = ProcessorHandlerHelper::GetMissionNamePrefixFromSatelliteId(tileTemporalFilesInfo.primarySatelliteId);
    const auto &resolutionStr = QString::number(cfg.resolution);
    QList<TaskToSubmit> &allTasksList = globalExecInfos.allTasksList;

    int curTaskIdx = 0;
    TaskToSubmit &qualityFlagsExtractorTask = allTasksList[curTaskIdx++];
    TaskToSubmit &bandsExtractor = allTasksList[curTaskIdx++];
    TaskToSubmit &reprojectPolys = allTasksList[curTaskIdx++];
    TaskToSubmit &clipPolys = allTasksList[curTaskIdx++];
    TaskToSubmit &clipRaster = allTasksList[curTaskIdx++];
    TaskToSubmit &sampleSelection = allTasksList[curTaskIdx++];
    TaskToSubmit &temporalResampling = allTasksList[curTaskIdx++];
    TaskToSubmit &featureExtraction = allTasksList[curTaskIdx++];
    TaskToSubmit &computeImagesStatistics = allTasksList[curTaskIdx++];
    TaskToSubmit &trainImagesClassifier = allTasksList[curTaskIdx++];
    TaskToSubmit &imageClassifier = allTasksList[curTaskIdx++];
    TaskToSubmit &clipCropType = allTasksList[curTaskIdx++];
    TaskToSubmit &computeConfusionMatrix = allTasksList[curTaskIdx++];
    TaskToSubmit &xmlMetrics = allTasksList[curTaskIdx++];

    SubmitTasks(ctx, cfg.jobId,
                    { qualityFlagsExtractorTask, bandsExtractor, reprojectPolys, clipPolys, clipRaster, sampleSelection, temporalResampling,
                      featureExtraction, computeImagesStatistics, trainImagesClassifier,
                      imageClassifier, clipCropType, computeConfusionMatrix, xmlMetrics });

    const auto &trainingPolys = sampleSelection.GetFilePath("training_polygons.shp");
    const auto &validationPolys = sampleSelection.GetFilePath("validation_polygons.shp");
    const auto &lut = sampleSelection.GetFilePath("lut.txt");
    const auto &rawtocr = bandsExtractor.GetFilePath("rawtocr.tif");
    const auto &rawmask = bandsExtractor.GetFilePath("rawmask.tif");
    const auto &dates = bandsExtractor.GetFilePath("dates.txt");
    const auto &shape = bandsExtractor.GetFilePath("shape.shp");
    const auto &statusFlags = qualityFlagsExtractorTask.GetFilePath("statusFlags.tif");
    const auto &shapeEsriPrj = bandsExtractor.GetFilePath("shape_esri.prj");

    const auto &refPolysReprojected = reprojectPolys.GetFilePath("reference_polygons_reproject.shp");
    const auto &refPolysClipped = clipPolys.GetFilePath("reference_clip.shp");
    const auto &tocr = clipRaster.GetFilePath("tocr.tif");
    const auto &mask = clipRaster.GetFilePath("mask.tif");
    const auto &rtocr = temporalResampling.GetFilePath("rtocr.tif");
    const auto &feFts = featureExtraction.GetFilePath("fts.tif");
    const auto &statistics = computeImagesStatistics.GetFilePath("statistics.xml");
    const auto &model = trainImagesClassifier.GetFilePath("model.txt");
    const auto &confusionMatrix = trainImagesClassifier.GetFilePath("confusion-matrix.csv");

    const auto &cropTypeMapNoMaskUncut = imageClassifier.GetFilePath("crop_type_map_nomask_uncut.tif");
    const auto &cropTypeMapNoMaskUncompressed = clipCropType.GetFilePath("crop_type_map_nomask_uncompressed.tif");

    const auto &confusionMatrixValidation = computeConfusionMatrix.GetFilePath("confusion-matrix-validation.csv");
    const auto &qualityMetrics = computeConfusionMatrix.GetFilePath("quality_metrics.txt");
    const auto &xmlValidationMetrics = xmlMetrics.GetFilePath("validation-metrics.xml");

    QStringList qualityFlagsExtractorArgs = { "QualityFlagsExtractor",
                                              "-out", "\"" + statusFlags+"?gdal:co:COMPRESS=DEFLATE\"",
                                              "-mission", mission};
    QStringList bandsExtractorArgs = { "BandsExtractor", "-out", rawtocr,  "-mask", rawmask,
                                       "-outdate", dates,  "-shape", shape, "-mission", mission };
    if (cfg.resolution) {
        qualityFlagsExtractorArgs.append("-pixsize");
        qualityFlagsExtractorArgs.append(resolutionStr);
        bandsExtractorArgs.append("-pixsize");
        bandsExtractorArgs.append(resolutionStr);
    }
    qualityFlagsExtractorArgs.append("-il");
    qualityFlagsExtractorArgs += listProducts;
    bandsExtractorArgs.append("-il");
    bandsExtractorArgs += listProducts;

    QStringList trainImagesClassifierArgs = {
        "-io.il",      feFts,       "-io.vd",      trainingPolys, "-io.imstat",     statistics,
        "-rand",       cfg.randomSeed,  "-sample.bm",  "0",           "-io.confmatout", confusionMatrix,
        "-io.out",     model,       "-sample.mt",  "-1",          "-sample.mv",     "-1",
        "-sample.vtr", "0.1",       "-sample.vfn", cfg.fieldName,        "-classifier",    cfg.classifier };

    if (cfg.classifier == "rf") {
        trainImagesClassifierArgs.append("-classifier.rf.nbtrees");
        trainImagesClassifierArgs.append(cfg.classifierRfNbTrees);
        trainImagesClassifierArgs.append("-classifier.rf.min");
        trainImagesClassifierArgs.append(cfg.classifierRfMinSamples);
        trainImagesClassifierArgs.append("-classifier.rf.max");
        trainImagesClassifierArgs.append(cfg.classifierRfMaxDepth);
    } else {
        trainImagesClassifierArgs.append("-classifier.svm.k");
        trainImagesClassifierArgs.append(cfg.classifierSvmKernel);
        trainImagesClassifierArgs.append("-classifier.svm.opt");
        trainImagesClassifierArgs.append(cfg.classifierSvmOptimize);
    }

    globalExecInfos.allStepsList = {
        qualityFlagsExtractorTask.CreateStep("QualityFlagsExtractor", qualityFlagsExtractorArgs),
        bandsExtractor.CreateStep("BandsExtractor", bandsExtractorArgs),
        reprojectPolys.CreateStep(
            "ReprojectPolys", { "-t_srs", shapeEsriPrj, "-overwrite", refPolysReprojected, cfg.referencePolygons }),
        clipPolys.CreateStep("ClipPolys",
                             { "-clipsrc", shape, refPolysClipped, refPolysReprojected }),
        clipRaster.CreateStep("ClipRasterImage",
                              { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", cfg.appsMem, rawtocr, tocr }),
        clipRaster.CreateStep("ClipRasterMask",
                              { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", cfg.appsMem, rawmask, mask }),
        sampleSelection.CreateStep("SampleSelection", { "SampleSelection", "-ref", refPolysClipped,
                                                        "-ratio", cfg.sampleRatio, "-lut", lut, "-tp",
                                                        trainingPolys, "-vp", validationPolys,
                                                        "-seed", cfg.randomSeed}),
        temporalResampling.CreateStep("TemporalResampling",
                                      { "TemporalResampling", "-tocr", tocr, "-mask", mask, "-ind",
                                        dates, "-sp", "SENTINEL", "5", "SPOT", "5", "LANDSAT", "16",
                                        "-rtocr", rtocr, "-mode", cfg.temporalResamplingMode}),
        featureExtraction.CreateStep("FeatureExtraction",
                                     { "FeatureExtraction", "-rtocr", rtocr, "-fts", feFts }),

        computeImagesStatistics.CreateStep("ComputeImagesStatistics",
                                           { "-il", feFts, "-out", statistics }),
        trainImagesClassifier.CreateStep("TrainImagesClassifier", trainImagesClassifierArgs),

        imageClassifier.CreateStep("ImageClassifier", { "-in",    feFts, "-imstat", statistics,
                                                        "-model", model, "-out",    cropTypeMapNoMaskUncut }),
        clipCropType.CreateStep("ClipCropTypeMap",
                                { "-dstnodata", "\"-10000\"", "-co", "COMPRESS=LZW", "-ot", "int16", "-overwrite", "-cutline", shape,
                                  "-crop_to_cutline", cropTypeMapNoMaskUncut, cropTypeMapNoMaskUncompressed }),
        computeConfusionMatrix.CreateStep("ComputeConfusionMatrix",
                                          { "-in", cropTypeMapNoMaskUncompressed, "-out",
                                            confusionMatrixValidation, "-ref", "vector",
                                            "-ref.vector.in", validationPolys, "-ref.vector.field",
                                            cfg.fieldName, "-nodatalabel", "-10000" }),
        xmlMetrics.CreateStep("XMLMetrics", { "XMLStatistics", "-confmat",
                                              confusionMatrixValidation, "-quality", qualityMetrics,
                                              "-root", "CropType", "-out", xmlValidationMetrics })
    };

    CropTypeProductFormatterParams &productFormatterParams = globalExecInfos.prodFormatParams;
    productFormatterParams.cropTypeMap = cropTypeMapNoMaskUncompressed;
    productFormatterParams.xmlValidationMetrics = xmlValidationMetrics;
    productFormatterParams.statusFlags = statusFlags;
}


void CropTypeHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                              const JobSubmittedEvent &event)
{
    QStringList listProducts = GetL2AInputProductsTiles(ctx, event);
    if(listProducts.size() == 0) {
        // try to get the start and end date if they are given
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No products provided at input or no products available in the specified interval").
                    toStdString());
    }

    CropTypeJobConfig cfg;
    GetJobConfig(ctx, event, cfg);

    const auto &referencePolygons = cfg.referencePolygons;
    if(referencePolygons.isEmpty()) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No reference polygons provided!").
                    toStdString());
    }

    // get the crop mask
    if(cfg.cropMask.isEmpty()) {
        // determine the crop mask based on the input products
        QDateTime dtStartDate, dtEndDate;
        if(ProcessorHandlerHelper::GetL2AIntevalFromProducts(listProducts, dtStartDate, dtEndDate)) {
            ProductList l4AProductList = ctx.GetProducts(event.siteId, (int)ProductType::L4AProductTypeId, dtStartDate, dtEndDate);
            if(l4AProductList.size() > 0) {
                // get the last L4A product
                cfg.cropMask = l4AProductList[l4AProductList.size()-1].fullPath;
            }
        }
    }

    QMap<QString, TileTemporalFilesInfo> mapTiles = GroupTiles(ctx, event.jobId, listProducts, ProductType::L2AProductTypeId);
    QString cropMaskAbsolutePath = ctx.GetProductAbsolutePath(cfg.cropMask);
    QMap<QString, QString> mapCropMasks;
    if (QFileInfo(cropMaskAbsolutePath).isDir()) {
        mapCropMasks = ProcessorHandlerHelper::GetHigLevelProductFiles(cropMaskAbsolutePath, "CM", false);
    } else {
        // we have only one file for all tiles - but in this case we should have only one tile
        if(mapTiles.size() != 1){
            ctx.MarkJobFailed(event.jobId);
            throw std::runtime_error(
                QStringLiteral("An input CropMask is needed for each tile").
                        toStdString());
        }
        for(const auto &e : mapTiles.keys()) {
            mapCropMasks[e] = cropMaskAbsolutePath;
        }
    }

    QList<CropTypeProductFormatterParams> listParams;

    TaskToSubmit productFormatterTask{"product-formatter", {}};
    NewStepList allSteps;
    //container for all task
    QList<TaskToSubmit> allTasksList;
    QList<CropTypeGlobalExecutionInfos> listCropTypeInfos;
    for(auto tileId : mapTiles.keys())
    {
       const TileTemporalFilesInfo &listTemporalTiles = mapTiles.value(tileId);
       QString cropMask = mapCropMasks.value(tileId);
       listCropTypeInfos.append(CropTypeGlobalExecutionInfos());
       CropTypeGlobalExecutionInfos &infos = listCropTypeInfos[listCropTypeInfos.size()-1];
       infos.prodFormatParams.tileId = GetProductFormatterTile(tileId);
       HandleNewTilesList(ctx, cfg, listTemporalTiles, cropMask, infos);
       listParams.append(infos.prodFormatParams);
       productFormatterTask.parentTasks += infos.prodFormatParams.parentsTasksRef;
       allTasksList.append(infos.allTasksList);
       allSteps.append(infos.allStepsList);
    }

    SubmitTasks(ctx, event.jobId, {productFormatterTask});

    // finally format the product
    QStringList productFormatterArgs = GetProductFormatterArgs(productFormatterTask, ctx, cfg, listProducts, listParams);

    // add these steps to the steps list to be submitted
    allSteps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
    ctx.SubmitSteps(allSteps);
}


void CropTypeHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
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
    if (event.module == "compute-confusion-matrix") {
        const auto &outputs = ctx.GetTaskConsoleOutputs(event.taskId);

        const auto &qualityMetrics =
            ctx.GetOutputPath(event.jobId, event.taskId, event.module, processorDescr.shortName) + "quality_metrics.txt";

        QFile file(qualityMetrics);
        if (!file.open(QIODevice::WriteOnly)) {
            throw std::runtime_error(QStringLiteral("Unable to open %1: %2")
                                         .arg(qualityMetrics)
                                         .arg(file.errorString())
                                         .toStdString());
        }

        QTextStream s(&file);
        s << outputs[0].stdOutText;
    }
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
            ctx.InsertProduct({ ProductType::L4BProductTypeId, event.processorId, event.siteId,
                                event.jobId, productFolder, maxDate, prodName, quicklook,
                                footPrint, std::experimental::nullopt, TileIdList() });

            // Now remove the job folder containing temporary files
            RemoveJobFolder(ctx, event.jobId, "l4b");
        } else {
            Logger::error(QStringLiteral("Cannot insert into database the product with name %1 and folder %2").arg(prodName).arg(productFolder));
        }
    }
}

void CropTypeHandler::WriteExecutionInfosFile(const QString &executionInfosPath,
                                               const CropTypeJobConfig &cfg,
                                               const QStringList &listProducts) {
    std::ofstream executionInfosFile;
    try
    {
        executionInfosFile.open(executionInfosPath.toStdString().c_str(), std::ofstream::out);
        executionInfosFile << "<?xml version=\"1.0\" ?>" << std::endl;
        executionInfosFile << "<metadata>" << std::endl;
        executionInfosFile << "  <General>" << std::endl;
        executionInfosFile << "  </General>" << std::endl;

        // Get the parameters from the configuration
        executionInfosFile << "  <Parameters>" << std::endl;


        executionInfosFile << "    <reference_polygons>"                << cfg.referencePolygons.toStdString()        <<          "</reference_polygons>" << std::endl;
        executionInfosFile << "    <random_seed>"                       << cfg.randomSeed.toStdString()               <<          "</random_seed>" << std::endl;
        executionInfosFile << "    <sample-ratio>"                      << cfg.sampleRatio.toStdString()              <<          "</sample-ratio>" << std::endl;
        executionInfosFile << "    <temporal_resampling_mode>"          << cfg.temporalResamplingMode.toStdString()   <<          "</temporal_resampling_mode>" << std::endl;
        executionInfosFile << "    <classifier>"                        << cfg.classifier.toStdString()               <<          "</classifier>" << std::endl;
        executionInfosFile << "    <classifier.field>"                  << cfg.fieldName.toStdString()                <<          "</classifier.field>" << std::endl;
        executionInfosFile << "    <classifier.rf.nbtrees>"             << cfg.classifierRfNbTrees.toStdString()      <<          "</classifier.rf.nbtrees>" << std::endl;
        executionInfosFile << "    <classifier.rf.min>"                 << cfg.classifierRfMinSamples.toStdString()   <<          "</classifier.rf.min>" << std::endl;
        executionInfosFile << "    <classifier.rf.max>"                 << cfg.classifierRfMaxDepth.toStdString()     <<          "</classifier.rf.max>" << std::endl;
        executionInfosFile << "    <classifier.svm.k>"                  << cfg.classifierSvmKernel.toStdString()      <<          "</classifier.svm.k>" << std::endl;
        executionInfosFile << "    <classifier.svm.opt>"                << cfg.classifierSvmOptimize.toStdString()    <<          "</classifier.svm.opt>" << std::endl;
        executionInfosFile << "    <lut_map>"                           << cfg.lutPath.toStdString()                  <<          "</lut_map>" << std::endl;

        executionInfosFile << "  </Parameters>" << std::endl;
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

QStringList CropTypeHandler::GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const CropTypeJobConfig &cfg,
                                    const QStringList &listProducts, const QList<CropTypeProductFormatterParams> &productParams) {
    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.xml");
    WriteExecutionInfosFile(executionInfosPath, cfg, listProducts);

    const auto &targetFolder = GetFinalProductFolder(ctx, cfg.jobId, cfg.siteId);
    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot", targetFolder,
                                         "-fileclass", "OPER",
                                         "-level", "L4B",
                                         "-baseline", "01.00",
                                         "-siteid", QString::number(cfg.siteId),
                                         "-processor", "croptype",
                                         "-gipp", executionInfosPath,
                                         "-outprops", outPropsPath};
    productFormatterArgs += "-il";
    productFormatterArgs += listProducts;

    productFormatterArgs += "-processor.croptype.file";
    for(const CropTypeProductFormatterParams &params: productParams) {
        productFormatterArgs += GetProductFormatterTile(params.tileId);
        productFormatterArgs += params.cropTypeMap;
    }

    // if has map, then add also the raw crop type
    if(cfg.cropMask.size() != 0) {
        productFormatterArgs += "-processor.croptype.rawfile";
        for(const CropTypeProductFormatterParams &params: productParams) {
            productFormatterArgs += GetProductFormatterTile(params.tileId);
            productFormatterArgs += params.rawCropTypeMap;
        }
    }

    productFormatterArgs += "-processor.croptype.quality";
    for(const CropTypeProductFormatterParams &params: productParams) {
        productFormatterArgs += GetProductFormatterTile(params.tileId);
        productFormatterArgs += params.xmlValidationMetrics;
    }

    productFormatterArgs += "-processor.croptype.flags";
    for(const CropTypeProductFormatterParams &params: productParams) {
        productFormatterArgs += GetProductFormatterTile(params.tileId);
        productFormatterArgs += params.statusFlags;
    }

    if(cfg.lutPath.size() > 0) {
        productFormatterArgs += "-lut";
        productFormatterArgs += cfg.lutPath;
    }

    return productFormatterArgs;
}


ProcessorJobDefinitionParams CropTypeHandler::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
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

    ConfigurationParameterValueMap l4aCfgValues = ctx.GetConfigurationParameters("processor.l4a.reference_data_dir", siteId, requestOverrideCfgValues);
    QString siteName = ctx.GetSiteShortName(siteId);
    // Get the reference dir
    QString refDir = l4aCfgValues["processor.l4a.reference_data_dir"].value;
    refDir = refDir.replace("{site}", siteName);

    ConfigurationParameterValueMap cfgValues = ctx.GetConfigurationParameters("processor.l4b.", siteId, requestOverrideCfgValues);
    // we might have an offset in days from starting the downloading products to start the L4B production
    int startSeasonOffset = cfgValues["processor.l4b.start_season_offset"].value.toInt();
    seasonStartDate = seasonStartDate.addDays(startSeasonOffset);

    QString shapeFile;
    QString referenceRasterFile;
    // if none of the reference files were found, cannot run the CropMask
    if(!ProcessorHandlerHelper::GetCropReferenceFile(refDir, shapeFile, referenceRasterFile)) {
        return params;
    }
    QString refStr = "\"reference_polygons\": \"\"";
    if(!shapeFile.isEmpty()) {
        refStr = "\"reference_polygons\": \"" + shapeFile + "\"";
    }

    QString cropMaskFolder;
    QDateTime startDate = seasonStartDate;
    QDateTime endDate = qScheduledDate;
    ProductList l4AProductList = ctx.GetProducts(siteId, (int)ProductType::L4AProductTypeId, startDate, endDate);
    // get the last created Crop Mask
    QDateTime maxDate;
    for(int i = 0; i<l4AProductList.size(); i++) {
        if(!maxDate.isValid() || (maxDate < l4AProductList[i].created)) {
            cropMaskFolder = l4AProductList[i].fullPath;
            maxDate = l4AProductList[i].created;
        }
    }
    params.jsonParameters = "{ \"crop_mask\": \"" + cropMaskFolder + "\", " + refStr + "}";
    params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    // Normally, we need at least 1 product available, the crop mask and the shapefile in order to be able to create a L4B product
    // but if we do not return here, the schedule block waiting for products (that might never happen)
    bool waitForAvailProcInputs = (cfgValues["processor.l4b.sched_wait_proc_inputs"].value.toInt() != 0);
    if((waitForAvailProcInputs == false) ||
            ((params.productList.size() > 0) && !cropMaskFolder.isEmpty() && !shapeFile.isEmpty())) {
        params.isValid = true;
        Logger::debug(QStringLiteral("Executing scheduled job. Scheduler extracted for L4B a number "
                                     "of %1 products for site ID %2 with start date %3 and end date %4!")
                      .arg(params.productList.size())
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    } else {
        Logger::debug(QStringLiteral("Scheduled job for L4B and site ID %1 with start date %2 and end date %3 will not be executed "
                                     "(productsNo = %4, cropMask = %5, shapeFile = %6)!")
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString())
                      .arg(params.productList.size())
                      .arg(cropMaskFolder)
                      .arg(shapeFile));
    }

    return params;
}
