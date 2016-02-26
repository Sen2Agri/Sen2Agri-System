#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "phenondvihandler.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"

void PhenoNdviHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                              const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    const auto &inputProducts = parameters["input_products"].toArray();
    // Get the resolution value
    const auto &resolution = QString::number(parameters["resolution"].toInt());

    QStringList listProducts;
    for (const auto &inputProduct : inputProducts) {
        listProducts.append(ctx.findProductFiles(inputProduct.toString()));
    }

    TaskToSubmit bandsExtractorTask{ "bands-extractor", {} };
    TaskToSubmit featureExtractionTask{ "feature-extraction", { bandsExtractorTask } };
    TaskToSubmit metricsEstimationTask{ "pheno-ndvi-metrics", { featureExtractionTask } };
    TaskToSubmit metricsSplitterTask{ "pheno-ndvi-metrics-splitter", { metricsEstimationTask } };
    TaskToSubmit productFormatterTask{ "product-formatter", { metricsSplitterTask } };

    ctx.SubmitTasks(event.jobId, { bandsExtractorTask, featureExtractionTask, metricsEstimationTask,
                                   metricsSplitterTask, productFormatterTask });

    const auto &rawReflBands = bandsExtractorTask.GetFilePath("reflectances.tif");
    const auto &allMasksImg = bandsExtractorTask.GetFilePath("mask_summary.tif");
    const auto &dates = bandsExtractorTask.GetFilePath("dates.txt");
    const auto &ndviImg = featureExtractionTask.GetFilePath("ndvi.tif");
    const auto &metricsEstimationImg = metricsEstimationTask.GetFilePath("metric_estimation.tif");
    const auto &metricsParamsImg = metricsSplitterTask.GetFilePath("metric_parameters_img.tif");
    const auto &metricsFlagsImg = metricsSplitterTask.GetFilePath("metric_flags_img.tif");
    //const auto &targetFolder = productFormatterTask.GetFilePath("");
    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.xml");

    QStringList bandsExtractorArgs = {
        "BandsExtractor", "-pixsize",   resolution,  "-merge",    "true",     "-ndh", "true",
        "-out",           rawReflBands, "-allmasks", allMasksImg, "-outdate", dates,  "-il"
    };
    bandsExtractorArgs += listProducts;

    QStringList featureExtractionArgs = { "FeatureExtraction", "-rtocr", rawReflBands, "-ndvi",
                                          ndviImg };

    QStringList metricsEstimationArgs = {
        "PhenologicalNDVIMetrics", "-in", ndviImg, "-mask", allMasksImg, "-dates", dates, "-out",
        metricsEstimationImg
    };

    QStringList metricsSplitterArgs = { "PhenoMetricsSplitter",
                                        "-in",
                                        metricsEstimationImg,
                                        "-outparams",
                                        metricsParamsImg,
                                        "-outflags",
                                        metricsFlagsImg,
                                        "-compress",
                                        "1" };

    WriteExecutionInfosFile(executionInfosPath, listProducts);
    QString tileId = ProcessorHandlerHelper::GetTileId(listProducts);
    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot",
                                         targetFolder,
                                         "-fileclass",
                                         "SVT1",
                                         "-level",
                                         "L3B",
                                         "-baseline",
                                         "01.00",
                                         "-processor",
                                         "phenondvi",
                                         "-processor.phenondvi.metrics",
                                         tileId,
                                         metricsParamsImg,
                                         "-processor.phenondvi.flags",
                                         tileId,
                                         metricsFlagsImg,
                                         "-gipp",
                                         executionInfosPath,
                                         "-il" };
    productFormatterArgs += listProducts;

    NewStepList steps = {
        bandsExtractorTask.CreateStep("BandsExtractor", bandsExtractorArgs),
        featureExtractionTask.CreateStep("FeatureExtraction", featureExtractionArgs),
        metricsEstimationTask.CreateStep("PhenologicalNDVIMetrics", metricsEstimationArgs),
        metricsSplitterTask.CreateStep("PhenoMetricsSplitter", metricsSplitterArgs),
        productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs)
    };

    ctx.SubmitSteps(steps);
}

void PhenoNdviHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                              const TaskFinishedEvent &event)
{
    if (event.module == "product-formatter") {
        ctx.MarkJobFinished(event.jobId);

        QString productFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);

        // Insert the product into the database
        ctx.InsertProduct({ ProductType::L3BPhenoProductTypeId,
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

void PhenoNdviHandler::WriteExecutionInfosFile(const QString &executionInfosPath,
                                               const QStringList &listProducts)
{
    std::ofstream executionInfosFile;
    try {
        executionInfosFile.open(executionInfosPath.toStdString().c_str(), std::ofstream::out);
        executionInfosFile << "<?xml version=\"1.0\" ?>" << std::endl;
        executionInfosFile << "<metadata>" << std::endl;
        executionInfosFile << "  <General>" << std::endl;
        executionInfosFile << "  </General>" << std::endl;
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

ProcessorJobDefinitionParams PhenoNdviHandler::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues)
{
    ConfigurationParameterValueMap mapCfg = ctx.GetConfigurationParameters(QString(START_OF_SEASON_CFG_KEY), -1, requestOverrideCfgValues);

    ProcessorJobDefinitionParams params;
    params.isValid = false;

    QDateTime startDate = QDateTime::fromString(mapCfg[START_OF_SEASON_CFG_KEY].value, "yyyymmdd");
    QDateTime endDate = QDateTime::fromTime_t(scheduledDate);

    params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    // for PhenoNDVI we need at least 4 products available in order to be able to create a L3B product
    if(params.productList.size() >= 4) {
        params.isValid = true;
    }
    //return QString("Cannot execute PhenoNDVI processor. There should be at least 4 products but we have only %1 L2A products available!").arg(usedProductList.size());
    return params;
}
