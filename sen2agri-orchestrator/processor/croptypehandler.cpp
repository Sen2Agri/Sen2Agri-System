#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include "croptypehandler.hpp"
#include "processorhandlerhelper.h"

void CropTypeHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    auto configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l4b.");
    auto resourceParameters = ctx.GetJobConfigurationParameters(event.jobId, "resources.");
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    QStringList listProducts;

    // TODO: Handle crop_mask_folder in order to get the crop mask files according to tiles
    const auto &inputProducts = parameters["input_products"].toArray();
    for (const auto &inputProduct : inputProducts) {
        listProducts.append(ctx.findProductFiles(inputProduct.toString()));
    }

    const auto &gdalwarpMem = resourceParameters["resources.gdalwarp.working-mem"];
    const auto &referencePolygons = parameters["reference_polygons"].toString();

    const auto &cropMask = parameters["crop_mask"].toString();
    auto mission = parameters["processor.l4b.mission"].toString();
    if(mission.length() == 0) mission = "SPOT";

    const auto &resolution = parameters["resolution"].toInt();
    const auto &resolutionStr = QString::number(resolution);

    auto randomSeed = parameters["processor.l4b.random_seed"].toString();
    if(randomSeed.isEmpty())  randomSeed = "0";

    auto temporalResamplingMode = parameters["processor.l4b.temporal_resampling_mode"].toString();
    if(temporalResamplingMode != "resample") temporalResamplingMode = "gapfill";

    auto sampleRatio = configParameters["processor.l4b.sample-ratio"];
    if(sampleRatio.length() == 0) sampleRatio = "0.75";

    auto &classifier = configParameters["processor.l4b.classifier"];
    if(classifier.length() == 0) classifier = "rf";
    auto fieldName = configParameters["processor.l4b.classifier.field"];
    if(fieldName.length() == 0) fieldName = "CODE";

    auto classifierRfNbTrees = configParameters["processor.l4b.classifier.rf.nbtrees"];
    if(classifierRfNbTrees.length() == 0) classifierRfNbTrees = "100";
    auto classifierRfMinSamples = configParameters["processor.l4b.classifier.rf.min"];
    if(classifierRfMinSamples.length() == 0) classifierRfMinSamples = "25";
    auto classifierRfMaxDepth = configParameters["processor.l4b.classifier.rf.max"];
    if(classifierRfMaxDepth.length() == 0) classifierRfMaxDepth = "25";

    const auto classifierSvmKernel = configParameters["processor.l4b.classifier.svm.kernel"];
    const auto classifierSvmOptimize = configParameters["processor.l4b.classifier.svm.optimize"];

    TaskToSubmit bandsExtractor{ "bands-extractor", {} };
    TaskToSubmit reprojectPolys{ "ogr2ogr", { bandsExtractor } };
    TaskToSubmit clipPolys{ "ogr2ogr", { reprojectPolys } };
    TaskToSubmit clipRaster{ "gdalwarp", { clipPolys } };
    TaskToSubmit sampleSelection{ "sample-selection", { clipRaster } };
    TaskToSubmit temporalResampling{ "temporal-resampling", { sampleSelection } };
    TaskToSubmit featureExtraction{ "feature-extraction", { temporalResampling } };
    TaskToSubmit computeImagesStatistics{ "compute-images-statistics", { featureExtraction } };
    TaskToSubmit trainImagesClassifier{ "train-images-classifier",
                                        { sampleSelection, computeImagesStatistics } };
    TaskToSubmit reprojectCropMask = TaskToSubmit("gdalwarp", { trainImagesClassifier });
    TaskToSubmit cropCropMask = TaskToSubmit("gdalwarp", { reprojectCropMask });
    TaskToSubmit imageClassifier = (!cropMask.isEmpty()) ?
                TaskToSubmit("image-classifier", { cropCropMask }):
                TaskToSubmit("image-classifier", { trainImagesClassifier });

    TaskToSubmit clipCropType{ "gdalwarp", { imageClassifier } };
    TaskToSubmit computeConfusionMatrix{ "compute-confusion-matrix", { clipCropType } };
    TaskToSubmit colorMapping{ "color-mapping", { computeConfusionMatrix } };
    TaskToSubmit compression{ "compression", { colorMapping } };
    TaskToSubmit xmlMetrics{ "xml-statistics", { compression } };
    TaskToSubmit productFormatter{ "product-formatter", { xmlMetrics} };

    if (!cropMask.isEmpty()) {
        ctx.SubmitTasks(event.jobId,
                        { bandsExtractor, reprojectPolys, clipPolys, clipRaster, sampleSelection, temporalResampling,
                          featureExtraction, computeImagesStatistics, trainImagesClassifier,
                          reprojectCropMask, cropCropMask, imageClassifier, clipCropType, computeConfusionMatrix, colorMapping,
                          compression, xmlMetrics, productFormatter });
    } else {
        ctx.SubmitTasks(event.jobId,
                        { bandsExtractor, reprojectPolys, clipPolys, clipRaster, sampleSelection, temporalResampling,
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
    const auto &cropTypeMapUncompressed = clipCropType.GetFilePath("crop_type_map_uncompressed.tif");
    const auto &confusionMatrixValidation = computeConfusionMatrix.GetFilePath("confusion-matrix-validation.csv");
    const auto &qualityMetrics = computeConfusionMatrix.GetFilePath("quality_metrics.txt");
    const auto &colorCropTypeMap = colorMapping.GetFilePath("color_crop_type_map.tif");
    const auto &cropTypeMap = compression.GetFilePath("crop_type_map.tif");
    const auto &xmlValidationMetrics = xmlMetrics.GetFilePath("validation-metrics.xml");
    //const auto &targetFolder = productFormatter.GetFilePath("");
    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    QString tileId = ProcessorHandlerHelper::GetTileId(listProducts);

    QStringList bandsExtractorArgs = { "BandsExtractor", "-out", rawtocr,  "-mask", rawmask,
                                       "-statusflags", statusFlags, "-outdate", dates,  "-shape", shape };
    if(mission.length() > 0) {
        bandsExtractorArgs.append("-mission");
        bandsExtractorArgs.append(mission);
    }
    if (resolution) {
        bandsExtractorArgs.append("-pixsize");
        bandsExtractorArgs.append(resolutionStr);
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
            "ReprojectPolys", { "-t_srs", shapeEsriPrj, "-overwrite", refPolysReprojected, referencePolygons }),
        clipPolys.CreateStep("ClipPolys",
                             { "-clipsrc", shape, refPolysClipped, refPolysReprojected }),
        clipRaster.CreateStep("ClipRasterImage",
                              { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", gdalwarpMem, rawtocr, tocr }),
        clipRaster.CreateStep("ClipRasterMask",
                              { "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape,
                                "-crop_to_cutline", "-multi", "-wm", gdalwarpMem, rawmask, mask }),
        sampleSelection.CreateStep("SampleSelection", { "SampleSelection", "-ref", refPolysClipped,
                                                        "-ratio", sampleRatio, "-lut", lut, "-tp",
                                                        trainingPolys, "-vp", validationPolys,
                                                        "-seed", randomSeed}),
        temporalResampling.CreateStep("TemporalResampling",
                                      { "TemporalResampling", "-tocr", tocr, "-mask", mask, "-ind",
                                        dates, "-sp", "SENTINEL", "5", "SPOT", "5", "LANDSAT", "16",
                                        "-rtocr", rtocr, "-mode", temporalResamplingMode}),
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
                                 "\"" + cropTypeMap + "?gdal:co:COMPRESS=DEFLATE\"", "int16" }),
        xmlMetrics.CreateStep("XMLMetrics", { "XMLStatistics", "-confmat",
                                              confusionMatrixValidation, "-quality", qualityMetrics,
                                              "-root", "CropType", "-out", xmlValidationMetrics })
    };
    if (!cropMask.isEmpty()) {
        steps.append(reprojectCropMask.CreateStep("ReprojectCropMask", {"-multi", "-wm", gdalwarpMem,  "-dstnodata", "0", "-overwrite",
                                                  "-t_srs", shapeEsriPrj, cropMask, reprojectedCropMaskFile}));
        steps.append(cropCropMask.CreateStep("CroppedCropMask", {"-multi", "-wm", gdalwarpMem,  "-dstnodata", "0", "-overwrite",
                                                                      "-tr", resolutionStr, resolutionStr,"-cutline", shape,
                                                                      "-crop_to_cutline",reprojectedCropMaskFile, croppedCropMaskFile}));
    }
    QStringList productFormatterArgs = { "ProductFormatter", "-destroot", targetFolder, "-fileclass", "SVT1", "-level", "L4B",
                                         "-baseline", "-01.00", "-processor",
                                         "croptype", "-processor.croptype.file", tileId, cropTypeMap,
                                         "-processor.croptype.flags", tileId, statusFlags,
                                         "-processor.croptype.quality", tileId, xmlValidationMetrics,
                                         "-il"};
    productFormatterArgs += listProducts;
    steps.append(productFormatter.CreateStep("ProductFormatter", productFormatterArgs));

    ctx.SubmitSteps(steps);
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

        QString productFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);

        // Insert the product into the database
        ctx.InsertProduct({ ProductType::L4BProductTypeId,
            event.processorId,
            event.taskId,
            productFolder,
            QDateTime::currentDateTimeUtc() });

        // Now remove the job folder containing temporary files
        // TODO: Reinsert this line - commented only for debug purposes
        //RemoveJobFolder(ctx, event.jobId);
    }
}

ProcessorJobDefinitionParams CropTypeHandler::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues)
{
    ProcessorJobDefinitionParams params;
    params.isValid = false;

    // Get the start and end date for the production
    ConfigurationParameterValueMap cfgValues = ctx.GetConfigurationParameters(START_OF_SEASON_CFG_KEY, -1, requestOverrideCfgValues);
    QDateTime startSeasonDate = QDateTime::fromString(cfgValues[START_OF_SEASON_CFG_KEY].value, "yyyymmdd");
    QDateTime endDate = QDateTime::fromTime_t(scheduledDate);

    params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startSeasonDate, endDate);
    if(params.productList.size() > 0) {
        params.isValid = true;
    }
    QString cropMaskProductsFolder = GetFinalProductFolder(ctx.GetConfigurationParameterValues(PRODUCTS_LOCATION_CFG_KEY),
                                                           ctx.GetSiteName(siteId), processorDescr.shortName);

    QString cropMaskFolder;
    QDirIterator it(cropMaskProductsFolder, QStringList() << "*", QDir::Dirs);
    while (it.hasNext()) {
        QString subDir = it.next();
        // check the dates of this folder
        int startDatesIdx = subDir.indexOf("_V");
        if(startDatesIdx != -1) {
            QString dateStr = subDir.right(subDir.length() - startDatesIdx);
            QStringList datesList = dateStr.split( "_" );
            QString startOfSeasonStr = cfgValues[START_OF_SEASON_CFG_KEY].value;
            if(datesList.size() == 2 && datesList[0] == startOfSeasonStr && datesList[1] == endDate.toString("yyyymmdd")) {
                cropMaskFolder = cropMaskProductsFolder + "/" + subDir;
                break;
            }

        }
    }

    params.jsonParameters = "{ \"crop_mask_folder\": \"" + cropMaskFolder + "\"}";

    return params;
}
