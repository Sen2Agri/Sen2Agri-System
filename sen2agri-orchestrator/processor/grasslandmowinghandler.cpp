#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "grasslandmowinghandler.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"
#include "logger.hpp"

#define L4B_GM_CFG_PREFIX "processor.s4c_l3b."

void GrasslandMowingHandler::CreateTasks(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                         QList<TaskToSubmit> &outAllTasksList)
{
    int curTaskIdx = 0;
    bool detectS1 = GetBoolConfigValue(parameters, configParameters, "detect_s1", L4B_GM_CFG_PREFIX);
    bool detectS2 = GetBoolConfigValue(parameters, configParameters, "detect_s2", L4B_GM_CFG_PREFIX);
    bool fusion = GetBoolConfigValue(parameters, configParameters, "s1_s2_fusion", L4B_GM_CFG_PREFIX);

    QList<int> prdFormatterParentTasks;
    if (detectS1) {
        prdFormatterParentTasks.append(curTaskIdx++);
        outAllTasksList.append(TaskToSubmit{ "s1-grassland-mowing-detection", {} });
    }
    if (detectS2) {
        prdFormatterParentTasks.append(curTaskIdx++);
        outAllTasksList.append(TaskToSubmit{ "s2-grassland-mowing-detection", {} });
    }
    if (detectS1 && detectS2 && fusion) {
        // TODO : See how should we do the S1+S2 fusion task
    }

    int productFormatterIdx = curTaskIdx++;
    outAllTasksList.append(TaskToSubmit{ "product-formatter", {} });
    // product formatter needs completion of time-series-analisys tasks
    for (const auto &curIdx : prdFormatterParentTasks) {
        outAllTasksList[productFormatterIdx].parentTasks.append(outAllTasksList[curIdx]);
    }
}

void GrasslandMowingHandler::CreateSteps(EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event, QList<TaskToSubmit> &allTasksList,
                                        NewStepList &steps, const QDateTime &minDate, const QDateTime &maxDate)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.s4c_l4b.");
    bool detectS1 = GetBoolConfigValue(parameters, configParameters, "detect_s1", L4B_GM_CFG_PREFIX);
    bool detectS2 = GetBoolConfigValue(parameters, configParameters, "detect_s2", L4B_GM_CFG_PREFIX);
    bool fusion = GetBoolConfigValue(parameters, configParameters, "s1_s2_fusion", L4B_GM_CFG_PREFIX);

    int curTaskIdx = 0;
    QStringList productFormatterFiles;

    if (detectS1) {
        TaskToSubmit &s1MowingDetectionTask = allTasksList[curTaskIdx++];
        const QString &s1MowingDetectionOutFile = s1MowingDetectionTask.GetFilePath("SEN4CAP_L4B_S1_MowingDetection.csv");
        const QString &s1OutDir = s1MowingDetectionTask.GetFilePath("SEN4CAP_L4B_S1_OutputData");
        const QStringList &s1MowingDetectionArgs = GetMowingDetectionArgs(parameters, configParameters, s1OutDir, s1MowingDetectionOutFile,
                                                                             minDate, maxDate);
        steps.append(s1MowingDetectionTask.CreateStep("S1MowingDetection", s1MowingDetectionArgs));

        productFormatterFiles += s1MowingDetectionOutFile;
    }

    if (detectS2) {
        TaskToSubmit &s2MowingDetectionTask = allTasksList[curTaskIdx++];
        const QString &s2MowingDetectionOutFile = s2MowingDetectionTask.GetFilePath("SEN4CAP_L4B_S2_MowingDetection.csv");
        const QString &s2OutDir = s2MowingDetectionTask.GetFilePath("SEN4CAP_L4B_S2_OutputData");
        const QStringList &s2MowingDetectionArgs = GetMowingDetectionArgs(parameters, configParameters, s2OutDir, s2MowingDetectionOutFile,
                                                                             minDate, maxDate);
        steps.append(s2MowingDetectionTask.CreateStep("S2MowingDetection", s2MowingDetectionArgs));

        productFormatterFiles += s2MowingDetectionOutFile;
    }
    if (fusion) {
        // TODO:
    }

    TaskToSubmit &productFormatterTask = allTasksList[curTaskIdx++];
    const QStringList &productFormatterArgs = GetProductFormatterArgs(productFormatterTask, ctx, event,
                                                                      productFormatterFiles, minDate, maxDate);
    steps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
}

void GrasslandMowingHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                              const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.s4c_l4b.");
    const QString &siteName = ctx.GetSiteShortName(event.siteId);

    QDateTime minDate, maxDate;
    const QStringList &ndviFiles = ExtractNdviFiles(ctx, event, minDate, maxDate);
    if (ndviFiles.size() == 0) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("Error producing S4C_L4B product for site %1. No NDVI files available!\n").arg(siteName).toStdString());
    }
    const QStringList &ampFiles = ExtractAmpFiles(ctx, event, minDate, maxDate);
    if (ampFiles.size() == 0) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("Error producing S4C_L4B product for site %1. No Amplitude files available!\n").arg(siteName).toStdString());
    }
    const QStringList &coheFiles = ExtractCoheFiles(ctx, event, minDate, maxDate);
    if (coheFiles.size() == 0) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("Error producing S4C_L4B product for site %1. No Coherence files available!\n").arg(siteName).toStdString());
    }

    QList<TaskToSubmit> allTasksList;
    CreateTasks(parameters, configParameters, allTasksList);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    SubmitTasks(ctx, event.jobId, allTasksListRef);
    NewStepList allSteps;
    CreateSteps(ctx, event, allTasksList, allSteps, minDate, maxDate);
    ctx.SubmitSteps(allSteps);
}


void GrasslandMowingHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                              const TaskFinishedEvent &event)
{
    if (event.module == "product-formatter") {
        ctx.MarkJobFinished(event.jobId);

        QString prodName = GetProductFormatterProductName(ctx, event);
        QString productFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId) + "/" + prodName;
        if(prodName != "") {
            QString quicklook = GetProductFormatterQuicklook(ctx, event);
            QString footPrint = GetProductFormatterFootprint(ctx, event);
            // Insert the product into the database
            QDateTime minDate, maxDate;
            ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prodName, minDate, maxDate);
            ctx.InsertProduct({ ProductType::S4CL4BProductTypeId, event.processorId, event.siteId,
                                event.jobId, productFolder, maxDate,
                                prodName, quicklook, footPrint, std::experimental::nullopt, TileIdList() });

            // Now remove the job folder containing temporary files
            RemoveJobFolder(ctx, event.jobId, "l3e");
        } else {
            Logger::error(QStringLiteral("Cannot insert into database the product with name %1 and folder %2").arg(prodName).arg(productFolder));
        }
    }
}

QStringList GrasslandMowingHandler::GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx,
                                                           const JobSubmittedEvent &event, const QStringList &listFiles,
                                                           const QDateTime &minDate, const QDateTime &maxDate) {
    // ProductFormatter /home/cudroiu/sen2agri-processors-build
    //    -vectprd 1 -destroot /mnt/archive_new/test/Sen4CAP_L4B_Tests/NLD_Validation_TSA/OutPrdFormatter
    //    -fileclass OPER -level S4C_L4B -baseline 01.00 -siteid 4 -timeperiod 20180101_20181231 -processor generic
    //    -processor.generic.files <files_list>

    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.txt");
    QString strTimePeriod = minDate.toString("yyyyMMddTHHmmss").append("_").append(maxDate.toString("yyyyMMddTHHmmss"));
    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot", targetFolder,
                                         "-fileclass", "OPER",
                                         "-level", "S4C_L4B",
                                         "-vectprd", "1",
                                         "-baseline", "01.00",
                                         "-siteid", QString::number(event.siteId),
                                         "-timeperiod", strTimePeriod,
                                         "-processor", "generic",
                                         "-outprops", outPropsPath,
                                         "-gipp", executionInfosPath
                                       };
    productFormatterArgs += "-processor.generic.files";
    productFormatterArgs += listFiles;

    return productFormatterArgs;
}

ProcessorJobDefinitionParams GrasslandMowingHandler::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
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

    ConfigurationParameterValueMap mapCfg = ctx.GetConfigurationParameters(QString("processor.s4c_l4b."), siteId, requestOverrideCfgValues);
    // we might have an offset in days from starting the downloading products to start the S4C_L4B production
    int startSeasonOffset = mapCfg["processor.s4c_l4b.start_season_offset"].value.toInt();
    seasonStartDate = seasonStartDate.addDays(startSeasonOffset);

    // Get the start and end date for the production
    QDateTime endDate = qScheduledDate;
    QDateTime startDate = seasonStartDate;

    params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    // Normally for PhenoNDVI we need at least 4 products available in order to be able to create a S4C_L4B product
    // but if we do not return here, the schedule block waiting for products (that might never happen)
    bool waitForAvailProcInputs = (mapCfg["processor.s4c_l4b.sched_wait_proc_inputs"].value.toInt() != 0);
    if((waitForAvailProcInputs == false) || (params.productList.size() >= 4)) {
        params.isValid = true;
        Logger::debug(QStringLiteral("Executing scheduled job. Scheduler extracted for S4C_L4B a number "
                                     "of %1 products for site ID %2 with start date %3 and end date %4!")
                      .arg(params.productList.size())
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    } else {
        Logger::debug(QStringLiteral("Scheduled job for S4C_L4B and site ID %1 with start date %2 and end date %3 "
                                     "will not be executed (no products)!")
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    }

    return params;
}

QStringList GrasslandMowingHandler::ExtractNdviFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                    QDateTime &minDate, QDateTime &maxDate)
{
    return GetInputProducts(ctx, event, ProductType::L3BProductTypeId, minDate, maxDate);
}

QStringList GrasslandMowingHandler::ExtractAmpFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                   QDateTime &minDate, QDateTime &maxDate)
{
    return GetInputProducts(ctx, event, ProductType::S4CS1L2AmpProductTypeId, minDate, maxDate);
}

QStringList GrasslandMowingHandler::ExtractCoheFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                    QDateTime &minDate, QDateTime &maxDate)
{
    return GetInputProducts(ctx, event, ProductType::S4CS1L2CoheProductTypeId, minDate, maxDate);
}


QStringList GrasslandMowingHandler::GetMowingDetectionArgs(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                                           const QString &outDataDir,
                                                           const QString &outFile, const QDateTime &minDate, const QDateTime &maxDate)
{
    // TODO: The arguments should be updated to retrieve:
    //      - A list of files
    //      - A start date and and end date
    //      - the list of tiles ?
    const QString &cfgFile = GetStringConfigValue(parameters, configParameters, "config_file", L4B_GM_CFG_PREFIX);
    const QString &inputShp = GetStringConfigValue(parameters, configParameters, "input_shp", L4B_GM_CFG_PREFIX);

    QStringList retArgs = { "--cfg-file", cfgFile,
                            "--input-shp", inputShp,
                            "--out-data-dir", outDataDir,
                            "--start-date", minDate.toString("yyyyMMdd"),
                            "--end-date", maxDate.toString("yyyyMMdd"),
                            "--seq-parcel-id", "NewID",
                            "--out-shp-file", outFile,
                            "--do-compl", "True"
                      };
    return retArgs;
}

QStringList GrasslandMowingHandler::GetInputProducts(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event, const ProductType &prdType, QDateTime &minDate, QDateTime &maxDate) {
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    std::string prodsInputKey;
    switch(prdType) {
        case ProductType::L3BProductTypeId:
            prodsInputKey = "input_NDVI";
            break;
        case ProductType::S4CS1L2AmpProductTypeId:
            prodsInputKey = "input_AMP";
            break;
        case ProductType::S4CS1L2CoheProductTypeId:
            prodsInputKey = "input_COHE";
            break;
        default:
            throw std::runtime_error(
                QStringLiteral(
                    "Unsupported product type %1.")
                    .arg((int)prdType)
                    .toStdString());
    }
    const auto &inputProducts = parameters[prodsInputKey.c_str()].toArray();
    QStringList listProducts;

    // get the products from the input_products or based on start_date or date_end
    if(inputProducts.size() == 0) {
        const auto &startDate = QDateTime::fromString(parameters["start_date"].toString(), "yyyyMMdd");
        const auto &endDateStart = QDateTime::fromString(parameters["end_date"].toString(), "yyyyMMdd");
        if(startDate.isValid() && endDateStart.isValid()) {
            // we consider the end of the end date day
            const auto endDate = endDateStart.addSecs(SECONDS_IN_DAY-1);
            if (!minDate.isValid() || minDate > startDate) {
                minDate = startDate;
            }
            if (!maxDate.isValid() || maxDate < endDate) {
                maxDate = endDate;
            }

            ProductList productsList = ctx.GetProducts(event.siteId, (int)prdType, startDate, endDate);
            for(const auto &product: productsList) {
                listProducts.append(product.fullPath);
            }
        }
    } else {
        for (const auto &inputProduct : inputProducts) {
            // if the product is an LAI, we need to extract the TIFF file for the NDVI
            if (prdType == ProductType::L3BProductTypeId) {
                const QString &tiffFile = FindNdviProductTiffFile(ctx, event, inputProduct.toString());
                if (tiffFile.size() > 0) {
                    listProducts.append(tiffFile);
                    const QDateTime &ndviTime = ProcessorHandlerHelper::GetNdviProductTime(tiffFile);
                    ProcessorHandlerHelper::UpdateMinMaxTimes(ndviTime, minDate, maxDate);
                }
            } else {
                // the S1 AMP and COHE products have directly the path of the tiff in the product table
                const QString &prdPath = ctx.GetProductAbsolutePath(event.siteId, inputProduct.toString());
                if (prdPath.size() > 0) {
                    listProducts.append(prdPath);
                    const QDateTime &s1Time = ProcessorHandlerHelper::GetS1L2AProductTime(prdPath);
                    ProcessorHandlerHelper::UpdateMinMaxTimes(s1Time, minDate, maxDate);
                } else {
                    throw std::runtime_error(
                        QStringLiteral(
                            "The product path does not exists %1.")
                            .arg(inputProduct.toString())
                            .toStdString());
                }
            }
        }
    }

    return listProducts;
}

QString GrasslandMowingHandler::FindNdviProductTiffFile(EventProcessingContext &ctx, const JobSubmittedEvent &event, const QString &path)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.isDir()) {
        const QString &fileName = fileInfo.fileName();
        if (fileName.contains("S2AGRI_L3B_SNDVI_A") && fileName.endsWith(".TIF")) {
            return path;
        }
    }
    QString absPath = path;
    if(!fileInfo.isAbsolute()) {
        // if we have the product name, we need to get the product path from the database
        absPath = ctx.GetProductAbsolutePath(event.siteId, path);
    }
    const QString &laiFileName = ProcessorHandlerHelper::GetHigLevelProductTileFile(absPath, "SNDVI");

    if (laiFileName.isEmpty()) {
        throw std::runtime_error(
            QStringLiteral(
                "Unable to find the TIFF file for the given input product %1.")
                .arg(absPath)
                .toStdString());
    }
    return laiFileName;
}


QStringList GrasslandMowingHandler::GetListValue(const QSettings &settings, const QString &key)
{
    const QVariant &value = settings.value(key);
    QStringList retList;
    if (value.type() == QVariant::StringList) {
      retList = value.toStringList();
    } else {
      retList.append(value.toString());
    }

    // remove the empty values
    retList.removeAll(QString(""));

    return retList;
}

