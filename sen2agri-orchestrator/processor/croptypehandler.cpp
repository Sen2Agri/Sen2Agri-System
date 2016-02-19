#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include "croptypehandler.hpp"
#include "processorhandlerhelper.h"

void CropTypeHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    auto configParameters = ctx.GetJobConfigurationParameters(event.jobId, "crop-type.");
    auto resourceParameters = ctx.GetJobConfigurationParameters(event.jobId, "resources.");
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    QStringList listProducts;
    const auto &inputProducts = parameters["input_products"].toArray();
    for (const auto &inputProduct : inputProducts) {
        listProducts.append(ctx.findProductFile(inputProduct.toString()));
    }

    const auto &gdalwarpMem = resourceParameters["resources.gdalwarp.working-mem"];
    const auto &referencePolygons = parameters["reference_polygons"].toString();

    //const auto &dateStart = parameters["date_start"].toString();
    //const auto &dateEnd = parameters["date_end"].toString();

    const auto &cropMask = parameters["crop_mask"].toString();
    auto mission = parameters["mission"].toString();
    if(mission.length() == 0)
        mission = "SPOT";

    const auto &resolution = parameters["resolution"].toInt();
    const auto &resolutionStr = QString::number(resolution);
    const auto &randomSeed = parameters["random_seed"].toString();
    auto temporalResamplingMode = parameters["temporal_resampling_mode"].toString();
    if(temporalResamplingMode != "resample")
        temporalResamplingMode = "gapfill";

    //const auto &samplingRate = configParameters["crop-type.sampling-rate"];
    const auto &sampleRatio = configParameters["crop-type.sample-ratio"];
    const auto &classifier = configParameters["crop-type.classifier"];
    auto fieldName = configParameters["crop-type.classifier.field"];
    if(fieldName.length() == 0)
        fieldName = "CODE";
    const auto &classifierRfNbTrees = configParameters["crop-type.classifier.rf.nbtrees"];
    const auto &classifierRfMinSamples = configParameters["crop-type.classifier.rf.min"];
    const auto &classifierRfMaxDepth = configParameters["crop-type.classifier.rf.max"];
    const auto &classifierSvmKernel = configParameters["crop-type.classifier.svm.k"];
    const auto &classifierSvmOptimize = configParameters["crop-type.classifier.svm.opt"];

    TaskToSubmit bandsExtractor{ "bands-extractor", {} };
    TaskToSubmit reprojectPolys{ "ogr2ogr", { bandsExtractor } };
    TaskToSubmit clipPolys{ "ogr2ogr", { reprojectPolys } };
    TaskToSubmit clipRaster{ "gdalwarp", { bandsExtractor } };
    TaskToSubmit sampleSelection{ "sample-selection", { clipPolys } };
    TaskToSubmit temporalResampling{ "temporal-resampling", { clipRaster } };
    TaskToSubmit featureExtraction{ "feature-extraction", { temporalResampling } };
    TaskToSubmit computeImagesStatistics{ "compute-images-statistics", { featureExtraction } };
    TaskToSubmit trainImagesClassifier{ "train-images-classifier",
                                        { sampleSelection, computeImagesStatistics } };
    TaskToSubmit reprojectCropMask = TaskToSubmit("gdalwarp", { trainImagesClassifier });
    TaskToSubmit imageClassifier = (!cropMask.isEmpty()) ?
                TaskToSubmit("image-classifier", { reprojectCropMask }):
                TaskToSubmit("image-classifier", { trainImagesClassifier });

    TaskToSubmit clipCropType{ "gdalwarp", { imageClassifier } };
    TaskToSubmit computeConfusionMatrix{ "compute-confusion-matrix", { clipCropType } };
    TaskToSubmit colorMapping{ "color-mapping", { clipCropType } };
    TaskToSubmit compression{ "compression", { clipCropType } };
    TaskToSubmit xmlMetrics{ "xml-statistics", { computeConfusionMatrix } };
    TaskToSubmit productFormatter{ "product-formatter", { compression, xmlMetrics } };

    if (!cropMask.isEmpty()) {
        ctx.SubmitTasks(event.jobId,
                        { bandsExtractor, clipPolys, clipRaster, sampleSelection, temporalResampling,
                          featureExtraction, computeImagesStatistics, trainImagesClassifier,
                          reprojectCropMask, imageClassifier, clipCropType, computeConfusionMatrix, colorMapping,
                          compression, xmlMetrics, productFormatter });
    } else {
        ctx.SubmitTasks(event.jobId,
                        { bandsExtractor, clipPolys, clipRaster, sampleSelection, temporalResampling,
                          featureExtraction, computeImagesStatistics, trainImagesClassifier,
                          imageClassifier, clipCropType, computeConfusionMatrix, colorMapping,
                          compression, xmlMetrics, productFormatter });
    }

    const auto &trainingPolys = sampleSelection.GetFilePath("training_polygons.shp");
    const auto &validationPolys = sampleSelection.GetFilePath("validation_polygons.shp");
    const auto &lut = sampleSelection.GetFilePath("lut.txt");
    const auto &rawtocr = bandsExtractor.GetFilePath("rawtocr.tif");
    const auto &rawmask = bandsExtractor.GetFilePath("rawmask.tif");
    const auto &dates = bandsExtractor.GetFilePath("dates.txt");
    const auto &shape = bandsExtractor.GetFilePath("shape.shp");
    const auto &statusFlags = bandsExtractor.GetFilePath("statusFlags.tif");
    const auto &refPolysReprojected = reprojectPolys.GetFilePath("reference_polygons_reproject.shp");
    const auto &refPolysClipped = clipPolys.GetFilePath("reference_clip.shp");
    const auto &shapePrj = clipPolys.GetFilePath("shape.prj");
    const auto &tocr = clipRaster.GetFilePath("tocr.tif");
    const auto &mask = clipRaster.GetFilePath("mask.tif");
    const auto &rtocr = temporalResampling.GetFilePath("rtocr.tif");
    const auto &feFts = featureExtraction.GetFilePath("fts.tif");
    const auto &statistics = computeImagesStatistics.GetFilePath("statistics.xml");
    const auto &model = trainImagesClassifier.GetFilePath("model.txt");
    const auto &confusionMatrix = trainImagesClassifier.GetFilePath("confusion-matrix.csv");

    const auto &reprojectedCropMaskFile = reprojectCropMask.GetFilePath("reprojected_crop_mask.tif");
    const auto &croppedCropMaskFile = reprojectCropMask.GetFilePath("cropped_crop_mask.tif");

    const auto &cropTypeMapUncut = imageClassifier.GetFilePath("crop_type_map_uncut.tif");
    const auto &cropTypeMapUncompressed = clipCropType.GetFilePath("crop_type_map_uncompressed.tif");
    const auto &confusionMatrixValidation = computeConfusionMatrix.GetFilePath("confusion-matrix-validation.csv");
    const auto &qualityMetrics = computeConfusionMatrix.GetFilePath("quality_metrics.txt");
    const auto &colorCropTypeMap = colorMapping.GetFilePath("color_crop_type_map.tif");
    const auto &cropTypeMap = compression.GetFilePath("crop_type_map.tif");
    const auto &xmlValidationMetrics = xmlMetrics.GetFilePath("validation-metrics.xml");
    //const auto &targetFolder = productFormatter.GetFilePath("");
    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.parametersJson);
    QString tileId = ProcessorHandlerHelper::GetTileId(listProducts);

    QStringList bandsExtractorArgs = { "BandsExtractor", "-out", rawtocr,  "-mask", rawmask,
                                       "-statusflags", statusFlags, "-outdate", dates,  "-shape", shape };
    if(mission.length() > 0) {
        bandsExtractorArgs.append("-mission");
        bandsExtractorArgs.append(mission);
    }
    if (resolution) {
        bandsExtractorArgs.append("-pixsize");
        bandsExtractorArgs.append(QString::number(resolution));
    }
    bandsExtractorArgs.append("-il");
    bandsExtractorArgs += listProducts;

    QStringList trainImagesClassifierArgs = {
        "-io.il",      feFts,       "-io.vd",      trainingPolys, "-io.imstat",     statistics,
        "-rand",       randomSeed,  "-sample.bm",  "0",           "-io.confmatout", confusionMatrix,
        "-io.out",     model,       "-sample.mt",  "-1",          "-sample.mv",     "-1",
        "-sample.vtr", "0.1",       "-sample.vfn", fieldName,        "-classifier",    classifier };

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

    QStringList imageClassifierArgs = { "-in",    feFts, "-imstat", statistics,
                                        "-model", model, "-out",    cropTypeMapUncut };
    if (!cropMask.isEmpty()) {
        imageClassifierArgs.append("-mask");
        imageClassifierArgs.append(croppedCropMaskFile);
    }

    NewStepList steps = {
        bandsExtractor.CreateStep("BandsExtractor", bandsExtractorArgs),

        reprojectPolys.CreateStep(
            "ReprojectPolys", { "-t_srs", "-overwrite", shapePrj, refPolysReprojected, referencePolygons }),
        clipPolys.CreateStep("ClipPolys",
                             { "-clipsrc", shape, refPolysClipped, refPolysReprojected }),
        clipRaster.CreateStep("ClipRasterImage",
                              { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", gdalwarpMem, rawtocr, tocr }),
        clipRaster.CreateStep("ClipRasterMask",
                              { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", gdalwarpMem, rawmask, mask }),
        temporalResampling.CreateStep("TemporalResampling",
                                      { "TemporalResampling", "-tocr", tocr, "-mask", mask, "-ind",
                                        dates, "-sp", "-sp", "SENTINEL", "5", "SPOT", "5", "LANDSAT", "16",
                                        "-rtocr", rtocr, "-mode", temporalResamplingMode}),
        featureExtraction.CreateStep("FeatureExtraction",
                                     { "FeatureExtraction", "-rtocr", rtocr, "-fts", feFts }),

        sampleSelection.CreateStep("SampleSelection", { "SampleSelection", "-ref", refPolysClipped,
                                                        "-ratio", sampleRatio, "-lut", lut, "-tp",
                                                        trainingPolys, "-vp", validationPolys,
                                                        "-seed", randomSeed}),
        computeImagesStatistics.CreateStep("ComputeImagesStatistics",
                                           { "-il", feFts, "-out", statistics }),
        trainImagesClassifier.CreateStep("TrainImagesClassifier", trainImagesClassifierArgs),

        imageClassifier.CreateStep("ImageClassifier", imageClassifierArgs),
        clipCropType.CreateStep("ClipCropTypeMap",
                                { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                  "-crop_to_cutline", "-multi", "-wm", gdalwarpMem,
                                  cropTypeMapUncut, cropTypeMapUncompressed }),
        computeConfusionMatrix.CreateStep("ComputeConfusionMatrix",
                                          { "-in", cropTypeMapUncompressed, "-out",
                                            confusionMatrixValidation, "-ref", "vector",
                                            "-ref.vector.in", validationPolys, "-ref.vector.field",
                                            fieldName, "-nodatalabel", "-10000" }),
        colorMapping.CreateStep("ColorMapping",
                                { "-in", cropTypeMapUncompressed, "-method", "custom",
                                  "-method.custom.lut", lut, "-out", colorCropTypeMap, "int32" }),
        compression.CreateStep("Compression",
                               { "-in", cropTypeMapUncompressed, "-out",
                                 cropTypeMap + "?gdal:co:COMPRESS=DEFLATE", "int16" }),
        xmlMetrics.CreateStep("XMLMetrics", { "XMLStatistics", "-confmat",
                                              confusionMatrixValidation, "-quality", qualityMetrics,
                                              "-root", "CropType", "-out", xmlValidationMetrics })
    };
    if (!cropMask.isEmpty()) {
        steps.append(reprojectCropMask.CreateStep("ReprojectCropMask", {"-multi", "-wm", gdalwarpMem,  "-dstnodata", "0", "-overwrite",
                                                  "-t_srs", "", cropMask, reprojectedCropMaskFile}));
        steps.append(reprojectCropMask.CreateStep("CroppedCropMask", {"-multi", "-wm", gdalwarpMem,  "-dstnodata", "0", "-overwrite",
                                                                      "-t_r", resolutionStr, resolutionStr,"-cutline", shape,
                                                                      "-crop_to_cutline",reprojectedCropMaskFile, croppedCropMaskFile}));
    }
    QStringList productFormatterArgs = { "ProductFormatter", "-destroot", targetFolder, "-fileclass", "SVT1", "-level", "L4B",
                                         "-baseline", "-01.00", "-processor",
                                         "croptype", "-processor.croptype.file", tileId, cropTypeMap,
                                         "-processor.croptype.flags", tileId, statusFlags,
                                         "-processor.croptype.quality", tileId, xmlValidationMetrics,
                                         "-il"};
    productFormatterArgs += listProducts;
    productFormatter.CreateStep("ProductFormatter", productFormatterArgs);

    ctx.SubmitSteps(steps);
}

void CropTypeHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    if (event.module == "compute-confusion-matrix") {
        const auto &outputs = ctx.GetTaskConsoleOutputs(event.taskId);

        const auto &qualityMetrics =
            ctx.GetOutputPath(event.jobId, event.taskId, event.module) + "quality_metrics.txt";

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

        auto archiveConfigParameters = ctx.GetJobConfigurationParameters(
                    event.jobId, "archiver.archive_path");
        // Insert the product into the database
        ctx.InsertProduct({ ProductType::L4BProductTypeId,
            event.processorId,
            event.taskId,
            ctx.GetOutputPath(event.jobId, event.taskId, "product-formatter"),
            QDateTime::currentDateTimeUtc() });

        // Now remove the job folder containing temporary files
        RemoveJobFolder(ctx, event.jobId);
    }


    // Insert the product into the database
//    ctx.InsertProduct({ ProductType::L3BProductTypeId,
//        event.processorId,
//        event.taskId,
//        ctx.GetOutputPath(event.jobId, event.taskId, "product-formatter"),
//        QDateTime::currentDateTimeUtc() });
}

QString CropTypeHandler::GetProcessingDefinitionJsonImpl(const QJsonObject &procInfoParams,
                                                      const ProductList &listProducts,
                                                      bool &bIsValid)
{
    bIsValid = false;
    return QString("Cannot execute CropTypeHandler processor.!");
}
