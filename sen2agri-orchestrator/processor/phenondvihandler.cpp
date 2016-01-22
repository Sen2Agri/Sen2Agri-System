#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "phenondvihandler.hpp"

void PhenoNdviHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    const auto &inputProducts = parameters["input_products"].toArray();
    const auto &dateStart = parameters["date_start"].toString();
    const auto &dateEnd = parameters["date_end"].toString();

    QStringList listProducts;
    for (const auto &inputProduct : inputProducts) {
        listProducts.append(ctx.findProductFile(inputProduct.toString()));
    }

    TaskToSubmit bandsExtractor{ "bands-extractor", {} };
    TaskToSubmit featureExtraction{ "feature-extraction", { bandsExtractor } };
    TaskToSubmit sigmoFitting{ "phenondvi-sigmo-fitting", { featureExtraction } };
    TaskToSubmit metricsEstimation{ "phenondvi-metrics-estimation", { sigmoFitting } };
    TaskToSubmit productFormatter{ "product-formatter", { metricsEstimation } };

    ctx.SubmitTasks(event.jobId,
                    { bandsExtractor, featureExtraction, sigmoFitting, metricsEstimation, productFormatter});

    const auto &rawReflBands = bandsExtractor.GetFilePath("reflectances.tif");
    const auto &allMasksImg = bandsExtractor.GetFilePath("mask_summary.tif");
    const auto &dates = bandsExtractor.GetFilePath("dates.txt");
    const auto &ndviImg= featureExtraction.GetFilePath("ndvi.tif");
    const auto &sigmoImg= sigmoFitting.GetFilePath("sigmo.tif");
    const auto &metricsEstimationImg= metricsEstimation.GetFilePath("metric_estimation.tif");
    const auto &targetFolder = productFormatter.GetFilePath("");

    QStringList bandsExtractorArgs = { "BandsExtractor",
                                       "-merge", "true",
                                       "-ndh", "true",
                                       "-out", rawReflBands,
                                       "-allmasks", allMasksImg,
                                       "-outdate", dates,
                                       "-il"};
    bandsExtractorArgs += listProducts;

    QStringList featureExtractionArgs = { "FeatureExtraction",
                                          "-rtocr", allMasksImg,
                                          "-ndvi", ndviImg };
    QStringList sigmoFittingArgs = { "SigmoFitting",
                                     "-in", ndviImg,
                                     "-mask", allMasksImg,
                                     "-dates", dates,
                                     "-out", sigmoImg };

    QStringList metricsEstimationArgs = { "MetricsEstimation",
                                          "-ipf", sigmoImg,
                                          "-indates", dates,
                                          "-opf", metricsEstimationImg};
    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot", targetFolder,
                                         "-fileclass", "SVT1",
                                         "-level", "L3B",
                                         "-timeperiod", dateStart+"_"+dateEnd,
                                         "-baseline", "-01.00",
                                         "-processor", "vegetation",
                                         "-processor.vegetation.pheno", "TILE_T0000", metricsEstimationImg,
                                         "-il", listProducts[0]};

    NewStepList steps = {
        bandsExtractor.CreateStep("BandsExtractor", bandsExtractorArgs),
        bandsExtractor.CreateStep("FeatureExtraction", featureExtractionArgs),
        bandsExtractor.CreateStep("SigmoFitting", sigmoFittingArgs),
        bandsExtractor.CreateStep("MetricsEstimation", metricsEstimationArgs),
        productFormatter.CreateStep("ProductFormatter", productFormatterArgs)
    };

    ctx.SubmitSteps(steps);

}

void PhenoNdviHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
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
                ctx.GetOutputPath(event.jobId, event.taskId, event.module) + "statistics.txt";
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

        ctx.MarkJobFinished(event.jobId);
    }

    //    ctx.InsertProduct({ ProductType::TestProduct,
    //                        event.processorId,
    //                        event.taskId,
    //                        ctx.GetOutputPath(event.jobId, event.taskId),
    //                        QDateTime::currentDateTimeUtc() });
}

