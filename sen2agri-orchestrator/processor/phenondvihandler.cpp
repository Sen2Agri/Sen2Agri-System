#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "phenondvihandler.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"

void PhenoNdviHandler::CreateTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                                QList<std::reference_wrapper<const TaskToSubmit>> &outProdFormatterParentsList)
{
    outAllTasksList.append(TaskToSubmit{ "bands-extractor", {} });
    outAllTasksList.append(TaskToSubmit{ "feature-extraction", {outAllTasksList[0]} });
    outAllTasksList.append(TaskToSubmit{ "pheno-ndvi-metrics", {outAllTasksList[1]} });
    outAllTasksList.append(TaskToSubmit{ "pheno-ndvi-metrics-splitter", {outAllTasksList[2]} });

    // product formatter needs completion of pheno-ndvi-metrics-splitter
    outProdFormatterParentsList.append(outAllTasksList[3]);
}

PhenoGlobalExecutionInfos PhenoNdviHandler::HandleNewTilesList(EventProcessingContext &ctx,
                                          const JobSubmittedEvent &event,
                                          const QStringList &listProducts)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    // Get the resolution value
    int resolution = 0;
    if(!GetParameterValueAsInt(parameters, "resolution", resolution) ||
            resolution == 0) {
        resolution = 10;    // TODO: We should configure the default resolution in DB
    }
    const auto &resolutionStr = QString::number(resolution);

    PhenoGlobalExecutionInfos globalExecInfos;
    QList<TaskToSubmit> &allTasksList = globalExecInfos.allTasksList;
    QList<std::reference_wrapper<const TaskToSubmit>> &prodFormParTsksList = globalExecInfos.prodFormatParams.parentsTasksRef;
    CreateTasksForNewProducts(allTasksList, prodFormParTsksList);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    ctx.SubmitTasks(event.jobId, allTasksListRef);

    TaskToSubmit &bandsExtractorTask = allTasksList[0];
    TaskToSubmit &featureExtractionTask = allTasksList[1];
    TaskToSubmit &metricsEstimationTask = allTasksList[2];
    TaskToSubmit &metricsSplitterTask = allTasksList[3];

    const auto &rawReflBands = bandsExtractorTask.GetFilePath("reflectances.tif");
    const auto &allMasksImg = bandsExtractorTask.GetFilePath("mask_summary.tif");
    const auto &dates = bandsExtractorTask.GetFilePath("dates.txt");
    const auto &ndviImg = featureExtractionTask.GetFilePath("ndvi.tif");
    const auto &metricsEstimationImg = metricsEstimationTask.GetFilePath("metric_estimation.tif");
    const auto &metricsParamsImg = metricsSplitterTask.GetFilePath("metric_parameters_img.tif");
    const auto &metricsFlagsImg = metricsSplitterTask.GetFilePath("metric_flags_img.tif");

    QStringList bandsExtractorArgs = {
        "BandsExtractor", "-pixsize",   resolutionStr,  "-merge",    "true",     "-ndh", "true",
        "-out",           rawReflBands, "-allmasks", allMasksImg, "-outdate", dates,  "-il"
    };
    bandsExtractorArgs += listProducts;

    QStringList featureExtractionArgs = { "FeatureExtraction", "-rtocr", rawReflBands, "-ndvi", ndviImg };

    QStringList metricsEstimationArgs = {"PhenologicalNDVIMetrics",
        "-in", ndviImg, "-mask", allMasksImg, "-dates", dates, "-out", metricsEstimationImg
    };

    QStringList metricsSplitterArgs = { "PhenoMetricsSplitter",
                                        "-in", metricsEstimationImg,
                                        "-outparams", metricsParamsImg,
                                        "-outflags", metricsFlagsImg,
                                        "-compress", "1" };

    globalExecInfos.allStepsList = {
        bandsExtractorTask.CreateStep("BandsExtractor", bandsExtractorArgs),
        featureExtractionTask.CreateStep("FeatureExtraction", featureExtractionArgs),
        metricsEstimationTask.CreateStep("PhenologicalNDVIMetrics", metricsEstimationArgs),
        metricsSplitterTask.CreateStep("PhenoMetricsSplitter", metricsSplitterArgs),
    };

    PhenoProductFormatterParams &productFormatterParams = globalExecInfos.prodFormatParams;
    productFormatterParams.metricsParamsImg = metricsParamsImg;
    productFormatterParams.metricsFlagsImg = metricsFlagsImg;
    // Get the tile ID from the product XML name. We extract it from the first product in the list as all
    // producs should be for the same tile
    productFormatterParams.tileId = ProcessorHandlerHelper::GetTileId(listProducts);

    return globalExecInfos;
}

void PhenoNdviHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                              const JobSubmittedEvent &event)
{
    QStringList listProducts = GetL2AInputProductsTiles(ctx, event);
    if(listProducts.size() == 0) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No products provided at input or no products available in the specified interval").
                    toStdString());
    }

    QMap<QString, QStringList> mapTiles = ProcessorHandlerHelper::GroupTiles(listProducts);
    QList<PhenoProductFormatterParams> listParams;

    TaskToSubmit productFormatterTask{"product-formatter", {}};
    NewStepList allSteps;
    //container for all task
    QList<TaskToSubmit> allTasksList;
    for(auto tile : mapTiles.keys())
    {
       QStringList listTemporalTiles = mapTiles.value(tile);
       PhenoGlobalExecutionInfos infos = HandleNewTilesList(ctx, event, listTemporalTiles);
       listParams.append(infos.prodFormatParams);
       productFormatterTask.parentTasks += infos.prodFormatParams.parentsTasksRef;
       allTasksList.append(infos.allTasksList);
       allSteps.append(infos.allStepsList);
    }

    ctx.SubmitTasks(event.jobId, {productFormatterTask});

    // finally format the product
    QStringList productFormatterArgs = GetProductFormatterArgs(productFormatterTask, ctx, event, listProducts, listParams);

    // add these steps to the steps list to be submitted
    allSteps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
    ctx.SubmitSteps(allSteps);
}


void PhenoNdviHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                              const TaskFinishedEvent &event)
{
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
            ctx.InsertProduct({ ProductType::L3EProductTypeId, event.processorId, event.siteId,
                                event.jobId, productFolder, maxDate,
                                prodName, quicklook, footPrint, TileList() });

            // Now remove the job folder containing temporary files
            // TODO: Reinsert this line - commented only for debug purposes
            //RemoveJobFolder(ctx, event.jobId);
        }
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

QStringList PhenoNdviHandler::GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                    const QStringList &listProducts, const QList<PhenoProductFormatterParams> &productParams) {
    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.txt");
    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot",
                                         targetFolder,
                                         "-fileclass", "OPER",
                                         "-level", "L3E",
                                         "-baseline", "01.00",
                                         "-siteid", QString::number(event.siteId),
                                         "-processor", "phenondvi",
                                         "-gipp", executionInfosPath,
                                         "-outprops", outPropsPath};
    productFormatterArgs += "-il";
    productFormatterArgs += listProducts;

    productFormatterArgs += "-processor.phenondvi.metrics";
    for(const PhenoProductFormatterParams &params: productParams) {
        productFormatterArgs += params.tileId;
        productFormatterArgs += params.metricsParamsImg;
    }

    productFormatterArgs += "-processor.phenondvi.flags";
    for(const PhenoProductFormatterParams &params: productParams) {
        productFormatterArgs += params.tileId;
        productFormatterArgs += params.metricsFlagsImg;
    }
    return productFormatterArgs;
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
