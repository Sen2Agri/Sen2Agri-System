#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include "cropmaskhandler.hpp"

void CropMaskHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    auto configParameters = ctx.GetJobConfigurationParameters(event.jobId, "crop-map.");

    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    const auto &referencePolygons = parameters["reference_polygons"].toString();

    const auto &inputProducts = parameters["input_products"].toArray();

    const auto &dateStart = parameters["date_start"].toString();
    const auto &dateEnd = parameters["date_end"].toString();

    const auto &samplingRate = configParameters["crop-mask.sampling-rate"];
    const auto &sampleRatio = configParameters["crop-mask.sample-ratio"];

    const auto &trainingSamplesNumber = configParameters["crop-mask.training-samples-number"];

    const auto &classifier = configParameters["crop-mask.classifier"];
    const auto &fieldName = configParameters["crop-mask.classifier.field"];

    const auto &classifierRfNbTrees = configParameters["crop-mask.classifier.rf.nbtrees"];
    const auto &classifierRfMinSamples = configParameters["crop-mask.classifier.rf.min"];
    const auto &classifierRfMaxDepth = configParameters["crop-mask.classifier.rf.max"];

    const auto &classifierSvmKernel = configParameters["crop-mask.classifier.svm.k"];
    const auto &classifierSvmOptimize = configParameters["crop-mask.classifier.svm.opt"];

    TaskToSubmit bandsExtractor{ "bands-extractor", {} };
    TaskToSubmit clipRaster{ "gdalwarp", { bandsExtractor } };
    TaskToSubmit clipPolys{ "ogr2ogr", { bandsExtractor } };
    TaskToSubmit sampleSelection{ "sample-selection", { clipPolys } };
    TaskToSubmit randomSelection{ "random-selection", { sampleSelection } };
    TaskToSubmit temporalResampling{ "temporal-resampling", { clipRaster } };
    TaskToSubmit featureExtraction{ "feature-extraction", { temporalResampling } };
    TaskToSubmit temporalFeatures{ "temporal-features", { featureExtraction } };
    TaskToSubmit statisticFeatures{ "statistic-features", { featureExtraction } };
    TaskToSubmit concatenateImages{ "concatenate-images", { temporalFeatures, statisticFeatures } };
    TaskToSubmit computeImagesStatistics{ "compute-images-statistics", { concatenateImages } };
    TaskToSubmit trainImagesClassifier{ "train-images-classifier",
                                        { randomSelection, computeImagesStatistics } };
    TaskToSubmit imageClassifier{ "image-classifier", { trainImagesClassifier } };
    TaskToSubmit computeConfusionMatrix{ "compute-confusion-matrix", { imageClassifier } };

    ctx.SubmitTasks(event.jobId, { bandsExtractor,
                                   clipPolys,
                                   clipRaster,
                                   sampleSelection,
                                   temporalResampling,
                                   featureExtraction,
                                   computeImagesStatistics,
                                   trainImagesClassifier,
                                   imageClassifier,
                                   computeConfusionMatrix });

    const auto &trainingPolys = sampleSelection.GetFilePath("training_polygons.shp");
    const auto &validationPolys = sampleSelection.GetFilePath("validation_polygons.shp");

    const auto &randomTrainingPolys = randomSelection.GetFilePath("random_training_polygons.shp");
    // not used
    const auto &randomTestingPolys = randomSelection.GetFilePath("random_testing_polygons.shp");

    const auto &rawtocr = bandsExtractor.GetFilePath("rawtocr.tif");
    const auto &rawmask = bandsExtractor.GetFilePath("rawmask.tif");
    const auto &dates = bandsExtractor.GetFilePath("dates.txt");
    const auto &shape = bandsExtractor.GetFilePath("shape.shp");

    const auto &refPolysClipped = clipPolys.GetFilePath("reference_clip.shp");

    const auto &tocr = clipRaster.GetFilePath("tocr.tif");
    const auto &mask = clipRaster.GetFilePath("mask.tif");

    const auto &rtocr = temporalResampling.GetFilePath("rtocr.tif");
    const auto &days = temporalResampling.GetFilePath("days.txt");

    const auto &ndvi = featureExtraction.GetFilePath("ndvi.tif");
    const auto &ndwi = featureExtraction.GetFilePath("ndwi.tif");
    const auto &brightness = featureExtraction.GetFilePath("brightness.tif");

    const auto &tf = temporalFeatures.GetFilePath("tf.tif");

    const auto &sf = statisticFeatures.GetFilePath("sf.tif");

    const auto &features = concatenateImages.GetFilePath("features.tif");

    const auto &statistics = computeImagesStatistics.GetFilePath("statistics.xml");

    const auto &model = trainImagesClassifier.GetFilePath("model.txt");
    const auto &confusionMatrix = trainImagesClassifier.GetFilePath("confusion-matrix.csv");

    const auto &cropMask = imageClassifier.GetFilePath("crop_mask.tif");

    const auto &confusionMatrixValidation =
        computeConfusionMatrix.GetFilePath("confusion-matrix-validation.csv");

    QStringList bandsExtractorArgs = { "BandsExtractor",
                                       "-out",
                                       rawtocr,
                                       "-mask",
                                       rawmask,
                                       "-outdate",
                                       dates,
                                       "-shape",
                                       shape,
                                       "-il" };

    for (const auto &inputProduct : inputProducts) {
        bandsExtractorArgs.append(ctx.findProductFile(inputProduct.toString()));
    }

    QStringList trainImagesClassifierArgs = { "-io.il",
                                              features,
                                              "-io.vd",
                                              randomTrainingPolys,
                                              "-io.imstat",
                                              statistics,
                                              "-rand",
                                              "42",
                                              "-sample.bm",
                                              "0",
                                              "-io.confmatout",
                                              confusionMatrix,
                                              "-io.out",
                                              model,
                                              "-sample.mt",
                                              "-1",
                                              "-sample.mv",
                                              "-1",
                                              "-sample.vtr",
                                              sampleRatio,
                                              "-sample.vfn",
                                              fieldName,
                                              "-classifier",
                                              classifier };

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
                                        "-model", model,    "-out",    cropMask };
    if (!cropMask.isEmpty()) {
        imageClassifierArgs.append("-mask");
        imageClassifierArgs.append(cropMask);
    }

    NewStepList steps = {
        bandsExtractor.CreateStep("BandsExtractor", bandsExtractorArgs),
        clipPolys.CreateStep(
            "ClipPolys", { "-clipsrc", shape, "-overwrite", refPolysClipped, referencePolygons }),
        clipRaster.CreateStep("ClipRasterImage", { "-dstnodata",
                                                   "\"-10000\"",
                                                   "-overwrite",
                                                   "-cutline",
                                                   shape,
                                                   "-crop_to_cutline",
                                                   rawtocr,
                                                   tocr }),
        clipRaster.CreateStep("ClipRasterMask", { "-dstnodata",
                                                  "1",
                                                  "-overwrite",
                                                  "-cutline",
                                                  shape,
                                                  "-crop_to_cutline",
                                                  rawmask,
                                                  mask }),
        sampleSelection.CreateStep("SampleSelection", { "SampleSelection",
                                                        "-ref",
                                                        refPolysClipped,
                                                        "-ratio",
                                                        sampleRatio,
                                                        "-tp",
                                                        trainingPolys,
                                                        "-vp",
                                                        validationPolys,
                                                        "-nofilter",
                                                        "true" }),
        randomSelection.CreateStep("RandomSelection", { "RandomSelection",
                                                        "-ref",
                                                        trainingPolys,
                                                        "-nbtrsample",
                                                        trainingSamplesNumber,
                                                        "-seed",
                                                        "0",
                                                        "-trp",
                                                        randomTrainingPolys,
                                                        "-tsp",
                                                        randomTestingPolys }),
        temporalResampling.CreateStep("TemporalResampling", { "TemporalResampling",
                                                              "-tocr",
                                                              tocr,
                                                              "-mask",
                                                              mask,
                                                              "-ind",
                                                              dates,
                                                              "-sp",
                                                              samplingRate,
                                                              "-t0",
                                                              dateStart,
                                                              "-tend",
                                                              dateEnd,
                                                              "-rtocr",
                                                              rtocr,
                                                              "-outdays",
                                                              days }),
        featureExtraction.CreateStep("FeatureExtraction", { "FeatureExtraction",
                                                            "-rtocr",
                                                            rtocr,
                                                            "-ndvi",
                                                            ndvi,
                                                            "-ndwi",
                                                            ndwi,
                                                            "-brightness",
                                                            brightness }),
        temporalFeatures.CreateStep(
            "TemporalFeatures", { "TemporalFeatures", "-ndvi", ndvi, "-dates", days, "-tf", tf }),
        statisticFeatures.CreateStep(
            "StatisticFeatures",
            { "StatisticFeatures", "-ndwi", ndwi, "-brightness", brightness, "-sf", sf }),
        concatenateImages.CreateStep("ConcatenateImages",
                                     { "ConcatenateImages", "-il", tf, sf, "-out", features }),
        computeImagesStatistics.CreateStep("ComputeImagesStatistics",
                                           { "-il", features, "-out", statistics }),
        trainImagesClassifier.CreateStep("TrainImagesClassifier", trainImagesClassifierArgs),
        imageClassifier.CreateStep("ImageClassifier", imageClassifierArgs),
        computeConfusionMatrix.CreateStep("ComputeConfusionMatrix", { "-in",
                                                                      cropMask,
                                                                      "-out",
                                                                      confusionMatrixValidation,
                                                                      "-ref",
                                                                      "vector",
                                                                      "-ref.vector.in",
                                                                      validationPolys,
                                                                      "-ref.vector.field",
                                                                      fieldName })
    };

    ctx.SubmitSteps(steps);
}

void CropMaskHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    if (event.module == "compute-confusion-matrix") {
        const auto &outputs = ctx.GetTaskConsoleOutputs(event.taskId);
        for (const auto &output : outputs) {
            QRegularExpression reK("Kappa index: (\\d+(?:\\.\\d*)?)");
            QRegularExpression reAcc("Overall accuracy index: (\\d+(?:\\.\\d*)?)");

            const auto &mK = reK.match(output.stdOutText);
            const auto &mAcc = reAcc.match(output.stdOutText);

            const auto &statisticsPath =
                ctx.GetOutputPath(event.jobId, event.taskId, event.module) + "/statistics.txt";
            QFile file(statisticsPath);
            if (!file.open(QIODevice::WriteOnly)) {
                throw std::runtime_error(QStringLiteral("Unable to open %1")
                                             .arg(statisticsPath)
                                             .arg(file.errorString())
                                             .toStdString());
            }

            QTextStream s(&file);
            s << mK.captured(1) << ' ' << mAcc.captured(1);
        }

        ctx.MarkJobFinished(event.jobId);
    }

    //    ctx.InsertProduct({ ProductType::TestProduct,
    //                        event.processorId,
    //                        event.taskId,
    //                        ctx.GetOutputPath(event.jobId, event.taskId),
    //                        QDateTime::currentDateTimeUtc() });
}
