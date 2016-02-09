#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include "croptypehandler.hpp"

void CropTypeHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    auto configParameters = ctx.GetJobConfigurationParameters(event.jobId, "crop-type.");
    auto resourceParameters = ctx.GetJobConfigurationParameters(event.jobId, "resources.");

    const auto &gdalwarpMem = resourceParameters["resources.gdalwarp.working-mem"];

    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    const auto &inputProducts = parameters["input_products"].toArray();

    const auto &referencePolygons = parameters["reference_polygons"].toString();

    const auto &dateStart = parameters["date_start"].toString();
    const auto &dateEnd = parameters["date_end"].toString();

    const auto &cropMask = parameters["crop_mask"].toString();

    const auto &resolution = parameters["resolution"].toInt();

    const auto &samplingRate = configParameters["crop-type.sampling-rate"];
    const auto &sampleRatio = configParameters["crop-type.sample-ratio"];

    const auto &classifier = configParameters["crop-type.classifier"];
    const auto &fieldName = configParameters["crop-type.classifier.field"];

    const auto &classifierRfNbTrees = configParameters["crop-type.classifier.rf.nbtrees"];
    const auto &classifierRfMinSamples = configParameters["crop-type.classifier.rf.min"];
    const auto &classifierRfMaxDepth = configParameters["crop-type.classifier.rf.max"];

    const auto &classifierSvmKernel = configParameters["crop-type.classifier.svm.k"];
    const auto &classifierSvmOptimize = configParameters["crop-type.classifier.svm.opt"];

    TaskToSubmit bandsExtractor{ "bands-extractor", {} };
    // TaskToSubmit reprojectPolys{ "ogr2ogr", { bandsExtractor } };
    TaskToSubmit clipPolys{ "ogr2ogr", { bandsExtractor } };
    TaskToSubmit clipRaster{ "gdalwarp", { bandsExtractor } };
    TaskToSubmit sampleSelection{ "sample-selection", { clipPolys } };
    TaskToSubmit temporalResampling{ "temporal-resampling", { clipRaster } };
    TaskToSubmit featureExtraction{ "feature-extraction", { temporalResampling } };
    TaskToSubmit computeImagesStatistics{ "compute-images-statistics", { featureExtraction } };
    TaskToSubmit trainImagesClassifier{ "train-images-classifier",
                                        { sampleSelection, computeImagesStatistics } };
    TaskToSubmit imageClassifier{ "image-classifier", { trainImagesClassifier } };
    TaskToSubmit clipCropType{ "gdalwarp", { imageClassifier } };
    TaskToSubmit computeConfusionMatrix{ "compute-confusion-matrix", { clipCropType } };
    TaskToSubmit colorMapping{ "color-mapping", { clipCropType } };
    TaskToSubmit compression{ "compression", { clipCropType } };
    TaskToSubmit xmlMetrics{ "xml-statistics", { computeConfusionMatrix } };
    TaskToSubmit productFormatter{ "product-formatter", { compression, xmlMetrics } };

    ctx.SubmitTasks(event.jobId,
                    { bandsExtractor, clipPolys, clipRaster, sampleSelection, temporalResampling,
                      featureExtraction, computeImagesStatistics, trainImagesClassifier,
                      imageClassifier, clipCropType, computeConfusionMatrix, colorMapping,
                      compression, xmlMetrics, productFormatter });

    const auto &trainingPolys = sampleSelection.GetFilePath("training_polygons.shp");
    const auto &validationPolys = sampleSelection.GetFilePath("validation_polygons.shp");
    const auto &lut = sampleSelection.GetFilePath("lut.txt");

    const auto &rawtocr = bandsExtractor.GetFilePath("rawtocr.tif");
    const auto &rawmask = bandsExtractor.GetFilePath("rawmask.tif");
    const auto &dates = bandsExtractor.GetFilePath("dates.txt");
    const auto &shape = bandsExtractor.GetFilePath("shape.shp");
    const auto &shapePrj = bandsExtractor.GetFilePath("shape.prj");

    // const auto &refPolysReprojected =
    // reprojectPolys.GetFilePath("reference_polygons_reproject.shp");

    const auto &refPolysClipped = clipPolys.GetFilePath("reference_clip.shp");

    const auto &tocr = clipRaster.GetFilePath("tocr.tif");
    const auto &mask = clipRaster.GetFilePath("mask.tif");

    const auto &rtocr = temporalResampling.GetFilePath("rtocr.tif");

    const auto &feFts = featureExtraction.GetFilePath("fts.tif");

    const auto &statistics = computeImagesStatistics.GetFilePath("statistics.xml");

    const auto &model = trainImagesClassifier.GetFilePath("model.txt");
    const auto &confusionMatrix = trainImagesClassifier.GetFilePath("confusion-matrix.csv");

    const auto &cropTypeMapUncut = imageClassifier.GetFilePath("crop_type_map_uncut.tif");

    const auto &cropTypeMapUncompressed =
        clipCropType.GetFilePath("crop_type_map_uncompressed.tif");

    const auto &confusionMatrixValidation =
        computeConfusionMatrix.GetFilePath("confusion-matrix-validation.csv");
    const auto &qualityMetrics = computeConfusionMatrix.GetFilePath("quality_metrics.txt");

    const auto &colorCropTypeMap = colorMapping.GetFilePath("color_crop_type_map.tif");

    const auto &cropTypeMap = compression.GetFilePath("crop_type_map.tif");

    const auto &xmlValidationMetrics = xmlMetrics.GetFilePath("validation-metrics.xml");

    const auto &targetFolder = productFormatter.GetFilePath("");

    QStringList bandsExtractorArgs = { "BandsExtractor", "-out", rawtocr,  "-mask", rawmask,
                                       "-outdate",       dates,  "-shape", shape };
    if (resolution) {
        bandsExtractorArgs.append("-pixsize");
        bandsExtractorArgs.append(QString::number(resolution));
    }

    bandsExtractorArgs.append("-il");
    for (const auto &inputProduct : inputProducts) {
        bandsExtractorArgs.append(ctx.findProductFile(inputProduct.toString()));
    }

    QStringList trainImagesClassifierArgs = {
        "-io.il",      feFts,       "-io.vd",      trainingPolys, "-io.imstat",     statistics,
        "-rand",       "42",        "-sample.bm",  "0",           "-io.confmatout", confusionMatrix,
        "-io.out",     model,       "-sample.mt",  "-1",          "-sample.mv",     "-1",
        "-sample.vtr", sampleRatio, "-sample.vfn", fieldName,     "-classifier",    classifier
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

    QStringList imageClassifierArgs = { "-in",    feFts, "-imstat", statistics,
                                        "-model", model, "-out",    cropTypeMapUncut };
    if (!cropMask.isEmpty()) {
        imageClassifierArgs.append("-mask");
        imageClassifierArgs.append(cropMask);
    }

    NewStepList steps = {
        bandsExtractor.CreateStep("BandsExtractor", bandsExtractorArgs),
        // reprojectPolys.CreateStep(
        //    "ReprojectPolys", { "-t_srs", shapePrj, refPolysReprojected, referencePolygons }),
        clipPolys.CreateStep("ClipPolys",
                             { "-clipsrc", shape, refPolysClipped, referencePolygons }),
        clipRaster.CreateStep("ClipRasterImage",
                              { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", gdalwarpMem, rawtocr, tocr }),
        clipRaster.CreateStep("ClipRasterMask",
                              { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", gdalwarpMem, rawmask, mask }),
        sampleSelection.CreateStep("SampleSelection", { "SampleSelection", "-ref", refPolysClipped,
                                                        "-ratio", sampleRatio, "-lut", lut, "-tp",
                                                        trainingPolys, "-vp", validationPolys }),
        temporalResampling.CreateStep("TemporalResampling",
                                      { "TemporalResampling", "-tocr", tocr, "-mask", mask, "-ind",
                                        dates, "-sp", samplingRate, "-t0", dateStart, "-tend",
                                        dateEnd, "-rtocr", rtocr }),
        featureExtraction.CreateStep("FeatureExtraction",
                                     { "FeatureExtraction", "-rtocr", rtocr, "-fts", feFts }),
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
                                              "-root", "CropType", "-out", xmlValidationMetrics }),
        productFormatter.CreateStep(
            "ProductFormatter",
            { "ProductFormatter", "-destroot", targetFolder, "-fileclass", "SVT1", "-level", "L4B",
              "-timeperiod", dateStart + "_" + dateEnd, "-baseline", "-01.00", "-processor",
              "croptype", "-processor.croptype.file", "TILE_T0000", cropTypeMap,
              "-processor.croptype.quality", xmlValidationMetrics })

    };

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

        //        for (const auto &output : outputs) {
        //            QRegularExpression reK("Kappa index: (\\d+(?:\\.\\d*)?)");
        //            QRegularExpression reAcc("Overall accuracy index: (\\d+(?:\\.\\d*)?)");

        //            const auto &mK = reK.match(qualityMetrics);
        //            const auto &mAcc = reAcc.match(output.stdOutText);

        //            const auto &statisticsPath =
        //                ctx.GetOutputPath(event.jobId, event.taskId, event.module) +
        //                "statistics.txt";
        //            QFile file(statisticsPath);
        //            if (!file.open(QIODevice::WriteOnly)) {
        //                throw std::runtime_error(QStringLiteral("Unable to open %1: %2")
        //                                             .arg(statisticsPath)
        //                                             .arg(file.errorString())
        //                                             .toStdString());
        //            }

        //            QTextStream s(&file);
        //            s << mK.captured(1) << ' ' << mAcc.captured(1);
        //        }

        ctx.MarkJobFinished(event.jobId);
    }

    //    ctx.InsertProduct({ ProductType::TestProduct,
    //                        event.processorId,
    //                        event.taskId,
    //                        ctx.GetOutputPath(event.jobId, event.taskId),
    //                        QDateTime::currentDateTimeUtc() });
}
