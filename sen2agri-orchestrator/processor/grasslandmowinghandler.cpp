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
#define L4B_GM_DEF_CFG_DIR   "/mnt/archive/grassland_mowing_files/{site}/{year}/config/"
#define DEFAULT_WORKING_DIR_PATH "/mnt/archive/grassland_mowing_files/{site}/{year}/working_dir"

void GrasslandMowingHandler::CreateTasks(GrasslandMowingExecConfig &cfg,
                                         QList<TaskToSubmit> &outAllTasksList)
{
    int curTaskIdx = 0;
    // Create task for creating the input shapefile
    outAllTasksList.append(TaskToSubmit{ "s4c-grassland-gen-input-shp", {} });
    curTaskIdx++;

    QList<int> prdFormatterParentTasks;
    if (cfg.inputPrdsType & L3B) {
        prdFormatterParentTasks.append(curTaskIdx++);
        outAllTasksList.append(TaskToSubmit{ "s4c-grassland-mowing", {outAllTasksList[0]} });
    }
    if (cfg.inputPrdsType & L2_S1) {
        // if we have also the S2, then put this task to be executed after the previous one
        prdFormatterParentTasks.append(curTaskIdx++);
        outAllTasksList.append(TaskToSubmit{ "s4c-grassland-mowing",
                         {((cfg.inputPrdsType & L3B) ? outAllTasksList[1] : outAllTasksList[0])} });
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
    if (cfg.isScheduled) {
        inputShpLocation = GetProcessorDirValue(cfg, "gen_input_shp_path", DEFAULT_SHP_GEN_PATH);
        // create folder for the file if it doesn't exist
        QDir().mkpath(QFileInfo(inputShpLocation).absolutePath());
    } else {
        inputShpLocation = genInputShpTask.GetFilePath("SEN4CAP_L4B_GeneratedInputShp.shp");
    }
    const QStringList &inShpGenArgs = GetInputShpGeneratorArgs(cfg, inputShpLocation);
    steps.append(genInputShpTask.CreateStep("MowingInputShpGenerator", inShpGenArgs));

    // It is assumed that the product formatter task it is the last one in the list
    TaskToSubmit &productFormatterTask = allTasksList[allTasksList.size()-1];

    if (cfg.inputPrdsType & L3B) {
        QString outShpFileName = ((cfg.inputPrdsType & L3B) ?
                                      "SEN4CAP_L4B_S1_S2_MowingDetection" :
                                      "SEN4CAP_L4B_S2_MowingDetection");
        TaskToSubmit &s2MowingDetectionTask = allTasksList[curTaskIdx++];
        const QString &s2MowingDetectionOutFile = productFormatterTask.GetFilePath(outShpFileName + ".shp");
        const QString &s2OutDir = GetOutputDataDir(cfg, s2MowingDetectionTask, "SEN4CAP_L4B_S2_OutputData");
        const QStringList &s2MowingDetectionArgs = GetMowingDetectionArgs(cfg, L3B, inputShpLocation,
                                                                          s2OutDir, s2MowingDetectionOutFile);
        steps.append(s2MowingDetectionTask.CreateStep("S2MowingDetection", s2MowingDetectionArgs));

        productFormatterFiles += s2MowingDetectionOutFile;
        // add also the dbf, prj and shx files
        productFormatterFiles += productFormatterTask.GetFilePath(outShpFileName + ".dbf");
        productFormatterFiles += productFormatterTask.GetFilePath(outShpFileName + ".prj");
        productFormatterFiles += productFormatterTask.GetFilePath(outShpFileName + ".shx");
        // Add also the intermediate files
        // TODO: see if this path should be better configured in database as
        // the script might use it to detect if vrt and other files were already generated
        // by a previous execution
        if (!cfg.isScheduled) {
            productFormatterFiles += s2MowingDetectionTask.GetFilePath("");
        }
    }

    if (cfg.inputPrdsType & L2_S1) {
        QString outShpFileName = ((cfg.inputPrdsType & L3B) ?
                                      "SEN4CAP_L4B_S1_S2_MowingDetection" :
                                      "SEN4CAP_L4B_S1_MowingDetection");
        TaskToSubmit &s1MowingDetectionTask = allTasksList[curTaskIdx++];
        const QString &s1MowingDetectionOutFile = productFormatterTask.GetFilePath(outShpFileName + ".shp");
        const QString &s1OutDir = GetOutputDataDir(cfg, s1MowingDetectionTask, "SEN4CAP_L4B_S1_OutputData");
        const QStringList &s1MowingDetectionArgs = GetMowingDetectionArgs(cfg, L2_S1, inputShpLocation,
                                                                          s1OutDir, s1MowingDetectionOutFile);
        steps.append(s1MowingDetectionTask.CreateStep("S1MowingDetection", s1MowingDetectionArgs));

        productFormatterFiles += s1MowingDetectionOutFile;
        // add also the dbf, prj and shx files
        productFormatterFiles += s1MowingDetectionTask.GetFilePath(outShpFileName + ".dbf");
        productFormatterFiles += s1MowingDetectionTask.GetFilePath(outShpFileName + ".prj");
        productFormatterFiles += s1MowingDetectionTask.GetFilePath(outShpFileName + ".shx");

        // Add also the intermediate files
        // TODO: see if this path should be better configured in database as
        // the script might use it to detect if vrt and other files were already generated
        // by a previous execution
        if (!cfg.isScheduled) {
            productFormatterFiles += s1MowingDetectionTask.GetFilePath("");
        }
    }

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
            cfg.startDate = ProcessorHandlerHelper::GetDateTimeFromString(strStartDate);
            cfg.endDate = ProcessorHandlerHelper::GetDateTimeFromString(strEndDate);

            QString strSeasonStartDate, strSeasonEndDate;
            ProcessorHandlerHelper::GetParameterValueAsString(cfg.parameters, "season_start_date", strSeasonStartDate);
            ProcessorHandlerHelper::GetParameterValueAsString(cfg.parameters, "season_end_date", strSeasonEndDate);
            cfg.seasonStartDate = ProcessorHandlerHelper::GetDateTimeFromString(strSeasonStartDate);
            cfg.seasonEndDate = ProcessorHandlerHelper::GetDateTimeFromString(strSeasonEndDate);

            QString startDateOverride;
            bool found = ProcessorHandlerHelper::GetParameterValueAsString(
                        cfg.parameters, "mowing-start-date", startDateOverride);
            if (found && startDateOverride.size() > 0) {
                cfg.startDate = ProcessorHandlerHelper::GetDateTimeFromString(startDateOverride);
            }
        } else {
            err = "Invalid scheduled request. Start date, end date or request structure are invalid!";
            return false;
        }
    } else {
        cfg.isScheduled = false;
        const QString &startDateStr = ProcessorHandlerHelper::GetStringConfigValue(cfg.parameters, cfg.configParameters,
                                                                                   "start_date", L4B_GM_CFG_PREFIX);
        const QString &endDateStr = ProcessorHandlerHelper::GetStringConfigValue(cfg.parameters, cfg.configParameters,
                                                                                   "end_date", L4B_GM_CFG_PREFIX);
        cfg.startDate = ProcessorHandlerHelper::GetDateTimeFromString(startDateStr);
        cfg.endDate = ProcessorHandlerHelper::GetDateTimeFromString(endDateStr);

        // Custom request
        const QJsonArray &arrPrdsL3B = S4CUtils::GetInputProducts(cfg.parameters, ProductType::L3BProductTypeId);
        const QJsonArray &arrPrdsAmp = S4CUtils::GetInputProducts(cfg.parameters, ProductType::S4CS1L2AmpProductTypeId);
        const QJsonArray &arrPrdsCohe = S4CUtils::GetInputProducts(cfg.parameters, ProductType::S4CS1L2CoheProductTypeId);
        QDateTime startDate, endDate;
        UpdatePrdInfos(cfg, arrPrdsL3B, cfg.l3bPrds, startDate, endDate);
        UpdatePrdInfos(cfg, arrPrdsAmp, cfg.s1Prds, startDate, endDate);
        UpdatePrdInfos(cfg, arrPrdsCohe, cfg.s1Prds, startDate, endDate);
        if (!cfg.startDate.isValid()) {
            cfg.startDate = startDate;
        }
        if (!cfg.endDate.isValid()) {
            cfg.endDate = endDate;
        }

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
    return LoadConfigFileAdditionalValues(cfg, err);
}

void GrasslandMowingHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                              const JobSubmittedEvent &event)
{
    QString err;
    GrasslandMowingExecConfig cfg(&ctx, event);

    if (!CheckInputParameters(cfg, err)) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("Error producing S4C_L4B product for site %1. The error was %2!\n").
                    arg(cfg.siteShortName).arg(err).toStdString());
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
    std::map<QString, QString> configParams;
    for (const auto &p : mapCfg) {
        configParams.emplace(p.key, p.value);
    }

    // we might have an offset in days from starting the downloading products to start the S4C_L4B production
    int startSeasonOffset = mapCfg["processor.s4c_l4b.start_season_offset"].value.toInt();
    seasonStartDate = seasonStartDate.addDays(startSeasonOffset);

    // Get the start and end date for the production
    QDateTime endDate = qScheduledDate;
    QDateTime startDate = seasonStartDate;
    QJsonObject parameters;
    const QString &startDateStr = ProcessorHandlerHelper::GetStringConfigValue(parameters, configParams,
                                                                               "start_date", L4B_GM_CFG_PREFIX);
    QDateTime tempDt = ProcessorHandlerHelper::GetDateTimeFromString(startDateStr);
    if (tempDt.isValid()) {
        startDate = tempDt;
    }

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
                                                                          "default_config_path", L4B_GM_CFG_PREFIX);
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
                            "--start-date", cfg.startDate.toString("yyyy-MM-dd"),
                            "--end-date", cfg.endDate.toString("yyyy-MM-dd"),
                            "--seg-parcel-id-attribute", segParcelIdAttrName,
                            "--output-shapefile", outFile,
                            "--do-cmpl", "True",
                            "--test", "True"
                      };

    if (cfg.isScheduled) {
        retArgs += "--season-start";
        retArgs += cfg.seasonStartDate.toString("yyyy-MM-dd");
        retArgs += "--season-end";
        retArgs += cfg.seasonEndDate.toString("yyyy-MM-dd");
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
    QString dataExtrDirName = ProcessorHandlerHelper::GetStringConfigValue(cfg.parameters, cfg.configParameters,
                                                                           key, L4B_GM_CFG_PREFIX);

    if (dataExtrDirName.size() == 0) {
        dataExtrDirName = defVal;
    }
    dataExtrDirName = dataExtrDirName.replace("{site}", cfg.siteShortName);
    dataExtrDirName = dataExtrDirName.replace("{year}", QString::number(cfg.year));
    dataExtrDirName = dataExtrDirName.replace("{processor}", processorDescr.shortName);

    return dataExtrDirName;
}

QString GrasslandMowingHandler::GetL4BConfigFilePath(GrasslandMowingExecConfig &jobCfg)
{
    QString strCfgPath;
    const QString &strCfgDir = GetProcessorDirValue(jobCfg, "cfg_dir", L4B_GM_DEF_CFG_DIR);
    QDir directory(strCfgDir);
    QString preferedCfgFileName = "S4C_L4B_Config_" + QString::number(jobCfg.year) + ".cfg";
    if (directory.exists()) {
        const QStringList &dirFiles = directory.entryList(QStringList() <<"*.cfg",QDir::Files);
        foreach(const QString &fileName, dirFiles) {
            if (strCfgPath.size() == 0) {
                // get the first available file
                strCfgPath = directory.filePath(fileName);
            }
            // check if the filename is a prefered file name
            if (fileName == preferedCfgFileName) {
                strCfgPath = directory.filePath(fileName);
                break;
            }
        }
    }
    // get the default config path
    if (strCfgPath.size() == 0 || strCfgPath == "N/A") {
        strCfgPath = ProcessorHandlerHelper::GetStringConfigValue(jobCfg.parameters, jobCfg.configParameters,
                                                                                   "default_config_path", L4B_GM_CFG_PREFIX);
    }

    if (strCfgPath.isEmpty() || strCfgPath == "N/A" || !QFileInfo::exists(strCfgPath)) {
        return "";
    }
    return strCfgPath;
}

bool GrasslandMowingHandler::LoadConfigFileAdditionalValues(GrasslandMowingExecConfig &cfg, QString &err)
{
    const QString &cfgFile = GetL4BConfigFilePath(cfg);
    if (cfgFile == "") {
        err = "Cannot get L4B configuration file for site with short name " + cfg.siteShortName;
        return false;
    }

    Logger::info(QStringLiteral("S4C_L4B: Loading settings from file %1 ").arg(cfgFile));
    QSettings settings(cfgFile, QSettings::IniFormat);

    QString cmnSectionKey("GENERAL_CONFIG/");
    cfg.ctNumFilter = GetStringValue(settings, cmnSectionKey + "CTNUM_FILTER");

    return true;
}

QString GrasslandMowingHandler::GetStringValue(const QSettings &settings, const QString &key)
{
    QVariant value = settings.value(key);
    QString string;
    if (value.type() == QVariant::StringList) {
      string = value.toStringList().join(",");
    } else {
      string = value.toString();
    }
    return string;
}

QString GrasslandMowingHandler::GetOutputDataDir(GrasslandMowingExecConfig &cfg, TaskToSubmit&task, const QString &outDataDirName) {
    if (cfg.isScheduled) {
        const QString &workingDirStr = GetProcessorDirValue(cfg, "working_dir", DEFAULT_WORKING_DIR_PATH);
        QDir directory(workingDirStr);
        const QString &strOutDataPath = directory.filePath(outDataDirName);
        QDir dir2(strOutDataPath);
        // create folder for the file if it doesn't exist
        dir2.mkpath(".");
        return strOutDataPath;
    } else {
        return task.GetFilePath(outDataDirName);
    }
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

