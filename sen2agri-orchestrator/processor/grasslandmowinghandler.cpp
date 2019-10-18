#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "grasslandmowinghandler.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"
#include "logger.hpp"
#include "s4c_utils.hpp"

using namespace grassland_mowing;

#define DEFAULT_SHP_GEN_PATH "/mnt/archive/grassland_mowing_files/{site}/{year}/InputShp/SEN4CAP_L4B_GeneratedInputShp.shp"

void GrasslandMowingHandler::CreateTasks(GrasslandMowingExecConfig &cfg,
                                         QList<TaskToSubmit> &outAllTasksList)
{
    int curTaskIdx = 0;
    // Create task for creating the input shapefile
    outAllTasksList.append(TaskToSubmit{ "s4c-grassland-gen-input-shp", {} });
    curTaskIdx++;

    QList<int> prdFormatterParentTasks;
    if (cfg.inputPrdsType & L2_S1) {
        prdFormatterParentTasks.append(curTaskIdx++);
        outAllTasksList.append(TaskToSubmit{ "s4c-grassland-mowing", {outAllTasksList[0]} });
    }
    if (cfg.inputPrdsType & L3B) {
        prdFormatterParentTasks.append(curTaskIdx++);
        outAllTasksList.append(TaskToSubmit{ "s4c-grassland-mowing", {outAllTasksList[0]} });
    }

    int productFormatterIdx = curTaskIdx++;
    outAllTasksList.append(TaskToSubmit{ "product-formatter", {} });
    // product formatter needs completion of time-series-analisys tasks
    for (const auto &curIdx : prdFormatterParentTasks) {
        outAllTasksList[productFormatterIdx].parentTasks.append(outAllTasksList[curIdx]);
    }
}

void GrasslandMowingHandler::CreateSteps(GrasslandMowingExecConfig &cfg, QList<TaskToSubmit> &allTasksList,
                                        NewStepList &steps)
{
    int curTaskIdx = 0;
    QStringList productFormatterFiles;

    TaskToSubmit &genInputShpTask = allTasksList[curTaskIdx++];
    QString inputShpLocation;
    if (IsScheduledJobRequest(cfg.parameters)) {
        inputShpLocation = GetProcessorDirValue(cfg, "prepared-input-shp-location", DEFAULT_SHP_GEN_PATH);
    } else {
        inputShpLocation = genInputShpTask.GetFilePath("SEN4CAP_L4B_GeneratedInputShp.shp");
    }
    const QStringList &inShpGenArgs = GetInputShpGeneratorArgs(cfg, inputShpLocation);
    steps.append(genInputShpTask.CreateStep("MowingInputShpGenerator", inShpGenArgs));

    if (cfg.inputPrdsType & L2_S1) {
        TaskToSubmit &s1MowingDetectionTask = allTasksList[curTaskIdx++];
        const QString &s1MowingDetectionOutFile = s1MowingDetectionTask.GetFilePath("SEN4CAP_L4B_S1_MowingDetection.shp");
        const QString &s1OutDir = s1MowingDetectionTask.GetFilePath("SEN4CAP_L4B_S1_OutputData");
        const QStringList &s1MowingDetectionArgs = GetMowingDetectionArgs(cfg, L2_S1, inputShpLocation,
                                                                          s1OutDir, s1MowingDetectionOutFile);
        steps.append(s1MowingDetectionTask.CreateStep("S1MowingDetection", s1MowingDetectionArgs));

        productFormatterFiles += s1MowingDetectionOutFile;
        // add also the dbf, prj and shx files
        productFormatterFiles += s1MowingDetectionTask.GetFilePath("SEN4CAP_L4B_S1_MowingDetection.dbf");
        productFormatterFiles += s1MowingDetectionTask.GetFilePath("SEN4CAP_L4B_S1_MowingDetection.prj");
        productFormatterFiles += s1MowingDetectionTask.GetFilePath("SEN4CAP_L4B_S1_MowingDetection.shx");
    }

    if (cfg.inputPrdsType & L3B) {
        TaskToSubmit &s2MowingDetectionTask = allTasksList[curTaskIdx++];
        const QString &s2MowingDetectionOutFile = s2MowingDetectionTask.GetFilePath("SEN4CAP_L4B_S2_MowingDetection.shp");
        const QString &s2OutDir = s2MowingDetectionTask.GetFilePath("SEN4CAP_L4B_S2_OutputData");
        const QStringList &s2MowingDetectionArgs = GetMowingDetectionArgs(cfg, L3B, inputShpLocation,
                                                                          s2OutDir, s2MowingDetectionOutFile);
        steps.append(s2MowingDetectionTask.CreateStep("S2MowingDetection", s2MowingDetectionArgs));

        productFormatterFiles += s2MowingDetectionOutFile;
        // add also the dbf, prj and shx files
        productFormatterFiles += s2MowingDetectionTask.GetFilePath("SEN4CAP_L4B_S2_MowingDetection.dbf");
        productFormatterFiles += s2MowingDetectionTask.GetFilePath("SEN4CAP_L4B_S2_MowingDetection.prj");
        productFormatterFiles += s2MowingDetectionTask.GetFilePath("SEN4CAP_L4B_S2_MowingDetection.shx");
    }

    TaskToSubmit &productFormatterTask = allTasksList[curTaskIdx++];
    const QStringList &productFormatterArgs = GetProductFormatterArgs(productFormatterTask, cfg, productFormatterFiles);
    steps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
}

bool GrasslandMowingHandler::CheckInputParameters(GrasslandMowingExecConfig &cfg, QString &err) {
    QString strStartDate, strEndDate;
    if(IsScheduledJobRequest(cfg.parameters)) {
        if (ProcessorHandlerHelper::GetParameterValueAsString(cfg.parameters, "start_date", strStartDate) &&
            ProcessorHandlerHelper::GetParameterValueAsString(cfg.parameters, "end_date", strEndDate) &&
            cfg.parameters.contains("input_products") && cfg.parameters["input_products"].toArray().size() == 0) {
            cfg.isScheduled = true;
            cfg.startDate = QDateTime::fromString(strStartDate, "yyyyMMdd");
            cfg.endDate = QDateTime::fromString(strEndDate, "yyyyMMdd");

            QString strSeasonStartDate, strSeasonEndDate;
            ProcessorHandlerHelper::GetParameterValueAsString(cfg.parameters, "season_start_date", strSeasonStartDate);
            ProcessorHandlerHelper::GetParameterValueAsString(cfg.parameters, "season_end_date", strSeasonEndDate);
            cfg.seasonStartDate = QDateTime::fromString(strSeasonStartDate, "yyyyMMdd");
            cfg.seasonEndDate = QDateTime::fromString(strSeasonEndDate, "yyyyMMdd");

            QString startDateOverride;
            bool found = ProcessorHandlerHelper::GetParameterValueAsString(
                        cfg.parameters, "mowing-start-date", startDateOverride);
            if (found && startDateOverride.size() > 0) {
                cfg.startDate = QDateTime::fromString(startDateOverride, "yyyyMMdd");
            }
        } else {
            err = "Invalid scheduled request. Start date, end date or request structure are invalid!";
            return false;
        }
    } else {
        cfg.isScheduled = false;
        // Custom request
        const QJsonArray &arrPrdsL3B = S4CUtils::GetInputProducts(cfg.parameters, ProductType::L3BProductTypeId);
        const QJsonArray &arrPrdsAmp = S4CUtils::GetInputProducts(cfg.parameters, ProductType::S4CS1L2AmpProductTypeId);
        const QJsonArray &arrPrdsCohe = S4CUtils::GetInputProducts(cfg.parameters, ProductType::S4CS1L2CoheProductTypeId);
        UpdatePrdInfos(cfg, arrPrdsL3B, cfg.l3bPrds, cfg.startDate, cfg.endDate);
        UpdatePrdInfos(cfg, arrPrdsAmp, cfg.s1Prds, cfg.startDate, cfg.endDate);
        UpdatePrdInfos(cfg, arrPrdsCohe, cfg.s1Prds, cfg.startDate, cfg.endDate);

        // if we have no l3b products then we set the inputPrdsType in the configuration
        // to S1 type in order to avoid creation of L3B mowing detection task and steps
        if (cfg.l3bPrds.size() == 0) {
            cfg.inputPrdsType = L2_S1;
        }
        // if we have no S1 products then we set the inputPrdsType in the configuration
        // to L3B type in order to avoid creation of S1 mowing detection task and steps
        if (cfg.s1Prds.size() == 0) {
            if (cfg.inputPrdsType == L2_S1) {
                err = "Invalid custom request. No products were provided !";
                return false;
            } else {
                cfg.inputPrdsType = L3B;
            }
        }
    }
    cfg.year = ProcessorHandlerHelper::GuessYear(cfg.startDate, cfg.endDate);
    return LoadConfigFileAdditionalValues(cfg);
}

void GrasslandMowingHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                              const JobSubmittedEvent &event)
{
    const QString &siteName = ctx.GetSiteShortName(event.siteId);
    QString err;
    GrasslandMowingExecConfig cfg(&ctx, event);

    if (!CheckInputParameters(cfg, err)) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("Error producing S4C_L4B product for site %1. The error was %2!\n").
                    arg(siteName).arg(err).toStdString());
    }

    QList<TaskToSubmit> allTasksList;
    CreateTasks(cfg, allTasksList);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    SubmitTasks(ctx, event.jobId, allTasksListRef);
    NewStepList allSteps;
    CreateSteps(cfg, allTasksList, allSteps);
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
            RemoveJobFolder(ctx, event.jobId, "s4c_l4b");
        } else {
            Logger::error(QStringLiteral("Cannot insert into database the product with name %1 and folder %2").arg(prodName).arg(productFolder));
        }
    }
}

QStringList GrasslandMowingHandler::GetProductFormatterArgs(TaskToSubmit &productFormatterTask,
                                                            GrasslandMowingExecConfig &cfg, const QStringList &listFiles) {
    // ProductFormatter /home/cudroiu/sen2agri-processors-build
    //    -vectprd 1 -destroot /mnt/archive_new/test/Sen4CAP_L4B_Tests/NLD_Validation_TSA/OutPrdFormatter
    //    -fileclass OPER -level S4C_L4B -baseline 01.00 -siteid 4 -timeperiod 20180101_20181231 -processor generic
    //    -processor.generic.files <files_list>

    const auto &targetFolder = GetFinalProductFolder(*(cfg.pCtx), cfg.event.jobId, cfg.event.siteId);
    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.txt");
    QString strTimePeriod = cfg.startDate.toString("yyyyMMddTHHmmss").append("_").append(cfg.endDate.toString("yyyyMMddTHHmmss"));
    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot", targetFolder,
                                         "-fileclass", "OPER",
                                         "-level", "S4C_L4B",
                                         "-vectprd", "1",
                                         "-baseline", "01.00",
                                         "-siteid", QString::number(cfg.event.siteId),
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

    ConfigurationParameterValueMap mapCfg = ctx.GetConfigurationParameters(QString(L4B_GM_CFG_PREFIX), siteId, requestOverrideCfgValues);
    // we might have an offset in days from starting the downloading products to start the S4C_L4B production
    int startSeasonOffset = mapCfg["processor.s4c_l4b.start_season_offset"].value.toInt();
    seasonStartDate = seasonStartDate.addDays(startSeasonOffset);

    // Get the start and end date for the production
    QDateTime endDate = qScheduledDate;
    QDateTime startDate = seasonStartDate;

    params.jsonParameters.append("{ \"scheduled_job\": \"1\", \"start_date\": \"" + startDate.toString("yyyyMMdd") + "\", " +
                                 "\"end_date\": \"" + endDate.toString("yyyyMMdd") + "\", " +
                                 "\"season_start_date\": \"" + seasonStartDate.toString("yyyyMMdd") + "\", " +
                                 "\"season_end_date\": \"" + seasonEndDate.toString("yyyyMMdd") + "\"");
    if(requestOverrideCfgValues.contains("product_type")) {
        const ConfigurationParameterValue &productType = requestOverrideCfgValues["product_type"];
        params.jsonParameters.append(", \"input_product_types\": \"" + productType.value + "\"}");
    } else {
        params.jsonParameters.append("}");
    }

    params.isValid = true;

//    params.productList = ctx.GetProducts(siteId, (int)ProductType::L3BProductTypeId, startDate, endDate);
//    params.productList += ctx.GetProducts(siteId, (int)ProductType::S4CS1L2AmpProductTypeId, startDate, endDate);
//    params.productList += ctx.GetProducts(siteId, (int)ProductType::S4CS1L2CoheProductTypeId, startDate, endDate);

//    // Normally for PhenoNDVI we need at least 4 products available in order to be able to create a S4C_L4B product
//    // but if we do not return here, the schedule block waiting for products (that might never happen)
//    bool waitForAvailProcInputs = (mapCfg["processor.s4c_l4b.sched_wait_proc_inputs"].value.toInt() != 0);
//    if((waitForAvailProcInputs == false) || (params.productList.size() >= 4)) {
//        params.isValid = true;
//        Logger::debug(QStringLiteral("Executing scheduled job. Scheduler extracted for S4C_L4B a number "
//                                     "of %1 products for site ID %2 with start date %3 and end date %4!")
//                      .arg(params.productList.size())
//                      .arg(siteId)
//                      .arg(startDate.toString())
//                      .arg(endDate.toString()));
//    } else {
//        Logger::debug(QStringLiteral("Scheduled job for S4C_L4B and site ID %1 with start date %2 and end date %3 "
//                                     "will not be executed (no products)!")
//                      .arg(siteId)
//                      .arg(startDate.toString())
//                      .arg(endDate.toString()));
//    }

    return params;
}

QStringList GrasslandMowingHandler::ExtractL3BProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                    QDateTime &minDate, QDateTime &maxDate)
{
    return S4CUtils::GetInputProducts(ctx, event, ProductType::L3BProductTypeId, minDate, maxDate, L4B_GM_CFG_PREFIX);
}

QStringList GrasslandMowingHandler::ExtractAmpProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                   QDateTime &minDate, QDateTime &maxDate)
{
    return S4CUtils::GetInputProducts(ctx, event, ProductType::S4CS1L2AmpProductTypeId, minDate, maxDate, L4B_GM_CFG_PREFIX);
}

QStringList GrasslandMowingHandler::ExtractCoheProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                    QDateTime &minDate, QDateTime &maxDate)
{
    return S4CUtils::GetInputProducts(ctx, event, ProductType::S4CS1L2CoheProductTypeId, minDate, maxDate, L4B_GM_CFG_PREFIX);
}

QStringList GrasslandMowingHandler::GetInputShpGeneratorArgs(GrasslandMowingExecConfig &cfg,
                                                             const QString &outShpFile)
{
    const QString &pyScriptPath = ProcessorHandlerHelper::GetStringConfigValue(cfg.parameters, cfg.configParameters,
                                                                          "gen_shp_py_script", L4B_GM_CFG_PREFIX);

    QStringList retArgs =  {"-s", QString::number(cfg.event.siteId),
            "-y", QString::number(cfg.year),
            "-o", outShpFile};
    if (pyScriptPath.size() > 0) {
        retArgs += "-p";
        retArgs += pyScriptPath;
    }
    if (cfg.ctNumFilter.size() > 0) {
        retArgs += "-f";
        retArgs += cfg.ctNumFilter;
    }
    return retArgs;
}

QStringList GrasslandMowingHandler::GetMowingDetectionArgs(GrasslandMowingExecConfig &cfg, const InputProductsType &prdType,
                                                           const QString &inputShpLocation,
                                                           const QString &outDataDir,
                                                           const QString &outFile)
{
    const QString &cfgFile = ProcessorHandlerHelper::GetStringConfigValue(cfg.parameters, cfg.configParameters,
                                                                          "config_path", L4B_GM_CFG_PREFIX);
    const QString keyScript = (prdType == L2_S1) ? "s1_py_script" : "s2_py_script";
    const QString &scriptToInvoke = ProcessorHandlerHelper::GetStringConfigValue(cfg.parameters, cfg.configParameters,
                                                                                 keyScript, L4B_GM_CFG_PREFIX);
    QString segParcelIdAttrName = ProcessorHandlerHelper::GetStringConfigValue(cfg.parameters, cfg.configParameters,
                                                                                 "seg-parcel-id-attribute", L4B_GM_CFG_PREFIX);
    if (segParcelIdAttrName.size() == 0) {
        segParcelIdAttrName = "NewID";
    }

    QStringList retArgs = {
                            "--script-path", scriptToInvoke,
                            "--site-id", QString::number(cfg.event.siteId),
                            "--config-file", cfgFile,
                            "--input-shape-file", inputShpLocation,
                            "--output-data-dir", outDataDir,
                            "--start-date", cfg.startDate.toString("yyyyMMdd"),
                            "--end-date", cfg.endDate.toString("yyyyMMdd"),
                            "--seg-parcel-id-attribute", segParcelIdAttrName,
                            "--output-shapefile", outFile,
                            "--do-cmpl", "True",
                            "--test", "True"
                      };

    if (cfg.isScheduled) {
        retArgs += "--season-start";
        retArgs += cfg.seasonStartDate.toString("yyyyMMdd");
        retArgs += "--season-end";
        retArgs += cfg.seasonEndDate.toString("yyyyMMdd");
    } else {
        retArgs += "--input-products-list";
        retArgs += (prdType == L2_S1) ? cfg.s1Prds : cfg.l3bPrds;
    }
    return retArgs;
}

bool GrasslandMowingHandler::IsScheduledJobRequest(const QJsonObject &parameters) {
    int jobVal;
    return ProcessorHandlerHelper::GetParameterValueAsInt(parameters, "scheduled_job", jobVal) && (jobVal == 1);
}

void GrasslandMowingHandler::UpdatePrdInfos(GrasslandMowingExecConfig &cfg,
                                            const QJsonArray &arrPrds, QStringList &whereToAdd,
                                            QDateTime &startDate, QDateTime &endDate)
{
    QDateTime tmpStartDate, tmpEndDate;
    for (const auto &prd: arrPrds) {
        const QString &prdPath = cfg.pCtx->GetProductAbsolutePath(cfg.event.siteId, prd.toString());
        if (ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prd.toString(), tmpStartDate, tmpEndDate)) {
            ProcessorHandlerHelper::UpdateMinMaxTimes(tmpEndDate, startDate, endDate);
        }
        whereToAdd.append(prdPath);
    }
}


QString GrasslandMowingHandler::GetProcessorDirValue(GrasslandMowingExecConfig &cfg,
                                                    const QString &key, const QString &defVal ) {
    const QString &siteShortName = cfg.pCtx->GetSiteShortName(cfg.event.siteId);
    QString dataExtrDirName = ProcessorHandlerHelper::GetStringConfigValue(cfg.parameters, cfg.configParameters,
                                                                           key, L4B_GM_CFG_PREFIX);

    if (dataExtrDirName.size() == 0) {
        dataExtrDirName = defVal;
    }
    dataExtrDirName = dataExtrDirName.replace("{site}", siteShortName);
    dataExtrDirName = dataExtrDirName.replace("{year}", QString::number(cfg.year));

    return dataExtrDirName;
}

bool GrasslandMowingHandler::LoadConfigFileAdditionalValues(GrasslandMowingExecConfig &cfg)
{
    const QString &cfgFile = ProcessorHandlerHelper::GetStringConfigValue(cfg.parameters, cfg.configParameters,
                                                                          "config_path", L4B_GM_CFG_PREFIX);
    if (cfgFile == "") {
        return false;
    }

    QSettings settings(cfgFile, QSettings::IniFormat);
    QString cmnSectionKey("GENERAL/");

    cfg.ctNumFilter = settings.value( cmnSectionKey + "CTNUM_FILTER").toString();

    return true;
}


// ###################### GrasslandMowingExecConfig functions ############################
InputProductsType GrasslandMowingExecConfig::GetInputProductsType(const QString &str)
{
    if (QString::compare(str, "S1", Qt::CaseInsensitive) == 0) {
        return L2_S1;
    } else if (QString::compare(str, "S2", Qt::CaseInsensitive) == 0) {
        return L3B;
    }
    return all;
}

InputProductsType GrasslandMowingExecConfig::GetInputProductsType(const QJsonObject &parameters, const std::map<QString, QString> &configParameters)
{
    const QString &inPrdsType = ProcessorHandlerHelper::GetStringConfigValue(parameters, configParameters,
                                                                             "input_product_types", L4B_GM_CFG_PREFIX);
    return GetInputProductsType(inPrdsType);
}

