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
    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.processorId, event.siteId);
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

        QString productFolder = GetFinalProductFolder(ctx, event.jobId, event.processorId, event.siteId);

        // Insert the product into the database
        ctx.InsertProduct({ ProductType::L3BPhenoProductTypeId,
            event.processorId,
            event.taskId,
            productFolder,
            QDateTime::currentDateTimeUtc() });

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

QString PhenoNdviHandler::GetProcessingDefinitionJsonImpl(const QJsonObject &procInfoParams,
                                                      const ProductList &listProducts,
                                                      bool &bIsValid)
{
    bIsValid = false;
    if(!procInfoParams.contains("resolution")) {
        return "Cannot execute PhenoNDVI processor. The parameters should contain the resolution!";
    }

    ProductList usedProductList;
    for(const Product &product: listProducts) {
        if(product.productTypeId == ProductType::L2AProductTypeId) {
            usedProductList.append(product);
        }
    }
    // for PhenoNDVI we need at least 4 products available in order to be able to create a L3B product
    if(usedProductList.size() >= 4) {
        QJsonObject mainObj(procInfoParams);
        QJsonArray inputProductsArr;
        for (const auto &p : usedProductList) {
            inputProductsArr.append(p.fullPath);
        }
        mainObj[QStringLiteral("input_products")] = inputProductsArr;
        bIsValid = true;

        return jsonToString(mainObj);
    }
    return QString("Cannot execute PhenoNDVI processor. There should be at least 4 products but we have only %1 L2A products available!").arg(usedProductList.size());
}
