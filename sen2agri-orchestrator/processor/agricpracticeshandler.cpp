#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "agricpracticeshandler.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"
#include "logger.hpp"
#include "s4c_utils.hpp"

#define L4C_AP_CFG_PREFIX   "processor.s4c_l4c."
#define L4C_AP_GEN_CFG_PREFIX   "processor.s4c_l4c.cfg.gen."

#define L4C_AP_GEN_CC_CFG_PREFIX   "processor.s4c_l4c.cfg.gen.cc."
#define L4C_AP_GEN_FL_CFG_PREFIX   "processor.s4c_l4c.cfg.gen.fl."
#define L4C_AP_GEN_NFC_CFG_PREFIX   "processor.s4c_l4c.cfg.gen.nfc."
#define L4C_AP_GEN_NA_CFG_PREFIX   "processor.s4c_l4c.cfg.gen.na."

#define L4C_AP_TSA_CFG_PREFIX   "processor.s4c_l4c.cfg.tsa."
#define L4C_AP_TSA_CC_CFG_PREFIX   "processor.s4c_l4c.cfg.tsa.cc."
#define L4C_AP_TSA_FL_CFG_PREFIX   "processor.s4c_l4c.cfg.tsa.fl."
#define L4C_AP_TSA_NFC_CFG_PREFIX   "processor.s4c_l4c.cfg.tsa.nfc."
#define L4C_AP_TSA_NA_CFG_PREFIX   "processor.s4c_l4c.cfg.tsa.na."

void AgricPracticesHandler::CreateTasks(const AgricPracticesJobCfg &siteCfg, QList<TaskToSubmit> &outAllTasksList,
                                        const QStringList &ndviPrds, const QStringList &ampPrds, const QStringList &cohePrds,
                                        AgricPractOperation operation)
{
    int curTaskIdx = 0;

    int minNdviDataExtrIndex = -1;
    int maxNdviDataExtrIndex = -1;
    int minAmpDataExtrIndex = -1;
    int maxAmpDataExtrIndex = -1;
    int minCoheDataExtrIndex = -1;
    int maxCoheDataExtrIndex = -1;

    if (IsOperationEnabled(operation, dataExtraction)) {
        int idsFilterExtrIdx = curTaskIdx++;
        outAllTasksList.append(TaskToSubmit{ "ids-filter-file-extraction", {} });

        // create the data extraction tasks
        CreatePrdDataExtrTasks(siteCfg, outAllTasksList, "ndvi-data-extraction", ndviPrds, {outAllTasksList[idsFilterExtrIdx]},
                               operation, minNdviDataExtrIndex, maxNdviDataExtrIndex, curTaskIdx);
        CreatePrdDataExtrTasks(siteCfg, outAllTasksList, "amp-data-extraction", ampPrds, {outAllTasksList[idsFilterExtrIdx]},
                               operation, minAmpDataExtrIndex, maxAmpDataExtrIndex, curTaskIdx);
        CreatePrdDataExtrTasks(siteCfg, outAllTasksList, "cohe-data-extraction", cohePrds, {outAllTasksList[idsFilterExtrIdx]},
                               operation, minCoheDataExtrIndex, maxCoheDataExtrIndex, curTaskIdx);
    }
    if (IsOperationEnabled(operation, catchCrop) || IsOperationEnabled(operation, fallow) ||
        IsOperationEnabled(operation, nfc) || IsOperationEnabled(operation, harvestOnly)) {
        // create the merging tasks
        int ndviMergeTaskIdx = CreateMergeTasks(outAllTasksList, "ndvi-data-extraction-merge", minNdviDataExtrIndex, maxNdviDataExtrIndex, curTaskIdx);
        int ampMergeTaskIdx = CreateMergeTasks(outAllTasksList, "amp-data-extraction-merge", minAmpDataExtrIndex, maxAmpDataExtrIndex, curTaskIdx);
        int coheMergeTaskIdx = CreateMergeTasks(outAllTasksList, "cohe-data-extraction-merge", minCoheDataExtrIndex, maxCoheDataExtrIndex, curTaskIdx);

        int ccTsaIdx = CreateTSATasks(siteCfg, outAllTasksList, "CC", operation, ndviMergeTaskIdx, ampMergeTaskIdx, coheMergeTaskIdx, curTaskIdx);
        int flTsaIdx = CreateTSATasks(siteCfg, outAllTasksList, "FL", operation, ndviMergeTaskIdx, ampMergeTaskIdx, coheMergeTaskIdx, curTaskIdx);
        int nfcTsaIdx = CreateTSATasks(siteCfg, outAllTasksList, "NFC", operation, ndviMergeTaskIdx, ampMergeTaskIdx, coheMergeTaskIdx, curTaskIdx);
        int naTsaIdx = CreateTSATasks(siteCfg, outAllTasksList, "NA", operation, ndviMergeTaskIdx, ampMergeTaskIdx, coheMergeTaskIdx, curTaskIdx);

        int productFormatterIdx = curTaskIdx++;
        outAllTasksList.append(TaskToSubmit{ "product-formatter", {} });
        // product formatter needs completion of time-series-analisys tasks
        if (ccTsaIdx != -1) {
            outAllTasksList[productFormatterIdx].parentTasks.append(outAllTasksList[ccTsaIdx]);
        }
        if (flTsaIdx != -1) {
            outAllTasksList[productFormatterIdx].parentTasks.append(outAllTasksList[flTsaIdx]);
        }
        if (nfcTsaIdx != -1) {
            outAllTasksList[productFormatterIdx].parentTasks.append(outAllTasksList[nfcTsaIdx]);
        }
        if (naTsaIdx != -1) {
            outAllTasksList[productFormatterIdx].parentTasks.append(outAllTasksList[naTsaIdx]);
        }

        outAllTasksList.append(TaskToSubmit{ "export-product-launcher", {outAllTasksList[productFormatterIdx]} });
    }
}

void AgricPracticesHandler::CreateSteps(EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event, QList<TaskToSubmit> &allTasksList,
                                        const AgricPracticesJobCfg &jobCfg,
                                        const QStringList &ndviPrds, const QStringList &ampPrds, const QStringList &cohePrds,
                                        NewStepList &steps, const QDateTime &minDate, const QDateTime &maxDate,
                                        AgricPractOperation operation)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.s4c_l4c.");

    int curTaskIdx = 0;

    QString idsFileName;

    // if only data extraction is needed, then we create the filter ids step into the general configured directory
    QStringList ndviDataExtrDirs;
    QStringList ampDataExtrDirs;
    QStringList coheDataExtrDirs;
    if (IsOperationEnabled(operation, dataExtraction)) {
        idsFileName = CreateStepForLPISSelection(parameters, configParameters, operation, "", jobCfg,
                                                        allTasksList, steps, curTaskIdx);

        // create the tasks for the NDVI, AMP and COHE - they depend only on the ids extraction
        ndviDataExtrDirs = CreateStepsForDataExtraction(parameters, configParameters, operation, jobCfg, ProductType::L3BProductTypeId,
                                                        ndviPrds, idsFileName, allTasksList, steps, curTaskIdx);
        ampDataExtrDirs = CreateStepsForDataExtraction(parameters, configParameters, operation, jobCfg, ProductType::S4CS1L2AmpProductTypeId,
                                                       ampPrds, idsFileName, allTasksList, steps, curTaskIdx);
        coheDataExtrDirs = CreateStepsForDataExtraction(parameters, configParameters, operation, jobCfg, ProductType::S4CS1L2CoheProductTypeId,
                                                       cohePrds, idsFileName, allTasksList, steps, curTaskIdx);
    } else {
        // no DataExtraction is performed, we must use the directories configures in the database
        ndviDataExtrDirs.append(GetDataExtractionDir(parameters, configParameters, ProductType::L3BProductTypeId, jobCfg.siteShortName));
        ampDataExtrDirs.append(GetDataExtractionDir(parameters, configParameters, ProductType::S4CS1L2AmpProductTypeId, jobCfg.siteShortName));
        coheDataExtrDirs.append(GetDataExtractionDir(parameters, configParameters, ProductType::S4CS1L2CoheProductTypeId, jobCfg.siteShortName));
    }

    if (IsOperationEnabled(operation, catchCrop) || IsOperationEnabled(operation, fallow) ||
        IsOperationEnabled(operation, nfc) || IsOperationEnabled(operation, harvestOnly)) {
        const QString &ndviMergedFile = CreateStepsForFilesMerge(jobCfg, ProductType::L3BProductTypeId, ndviDataExtrDirs, steps,
                                                                 allTasksList, curTaskIdx);
        const QString &ampMergedFile = CreateStepsForFilesMerge(jobCfg, ProductType::S4CS1L2AmpProductTypeId, ampDataExtrDirs, steps,
                                                                allTasksList, curTaskIdx);
        const QString &coheMergedFile = CreateStepsForFilesMerge(jobCfg, ProductType::S4CS1L2CoheProductTypeId, coheDataExtrDirs, steps,
                                                                 allTasksList, curTaskIdx);

        QStringList productFormatterFiles;
        productFormatterFiles += CreateTimeSeriesAnalysisSteps(parameters, configParameters, operation, jobCfg, "CC",
                                                               ndviMergedFile, ampMergedFile, coheMergedFile, steps,
                                                               allTasksList, curTaskIdx, operation);
        productFormatterFiles += CreateTimeSeriesAnalysisSteps(parameters, configParameters, operation, jobCfg, "FL",
                                                               ndviMergedFile, ampMergedFile, coheMergedFile, steps,
                                                               allTasksList, curTaskIdx, operation);
        productFormatterFiles += CreateTimeSeriesAnalysisSteps(parameters, configParameters, operation, jobCfg, "NFC",
                                                               ndviMergedFile, ampMergedFile, coheMergedFile, steps,
                                                               allTasksList, curTaskIdx, operation);
        productFormatterFiles += CreateTimeSeriesAnalysisSteps(parameters, configParameters, operation, jobCfg, "NA",
                                                               ndviMergedFile, ampMergedFile, coheMergedFile, steps,
                                                               allTasksList, curTaskIdx, operation);

        TaskToSubmit &productFormatterTask = allTasksList[curTaskIdx++];
        const QStringList &productFormatterArgs = GetProductFormatterArgs(productFormatterTask, ctx, event,
                                                                          productFormatterFiles, minDate, maxDate);
        steps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));

        const auto & productFormatterPrdFileIdFile = productFormatterTask.GetFilePath("prd_infos.txt");
        TaskToSubmit &exportCsvToShpProductTask = allTasksList[curTaskIdx++];
        const QStringList &exportCsvToShpProductArgs = { "-f", productFormatterPrdFileIdFile,
                                                          "-o", "Sen4CAP_L4C_PRACTICE_" + jobCfg.siteCfg.country + "_" + jobCfg.siteCfg.year + ".gpkg"
                                                    };
        steps.append(exportCsvToShpProductTask.CreateStep("export-product-launcher", exportCsvToShpProductArgs));
    }
}

void AgricPracticesHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                              const JobSubmittedEvent &evt)
{
    auto parameters = QJsonDocument::fromJson(evt.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(evt.jobId, "processor.s4c_l4c.");
    const QString &siteName = ctx.GetSiteShortName(evt.siteId);
    const AgricPractOperation &execOper = GetExecutionOperation(parameters, configParameters);

    // Moved this from the GetProcessingDefinitionImpl function as it might be time consuming and scheduler will
    // throw exception if timeout exceeded
    JobSubmittedEvent event;
    int ret = UpdateJobSubmittedParamsFromSchedReq(ctx, evt, configParameters, parameters, siteName, event);
    // no products available from the scheduling ... just mark job as done
    if (ret == 0 && execOper == dataExtraction) {
        ctx.MarkJobFinished(event.jobId);
        Logger::info(QStringLiteral("Scheduled job with id %1 for site %2 marked as done as no products are availabel for now to process").
                     arg(event.jobId).arg(event.siteId));
        return;
    }

    AgricPracticesJobCfg jobCfg;
    if (!GetSiteConfigForSiteId(ctx, event.siteId, parameters, configParameters, jobCfg.siteCfg)) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("Cannot find L4C configuration file for site %1\n").arg(siteName).toStdString());
    }
    jobCfg.prdsPerGroup = ProcessorHandlerHelper::GetIntConfigValue(parameters, configParameters, "prds_per_group", L4C_AP_CFG_PREFIX);
    jobCfg.siteId = event.siteId;
    jobCfg.siteShortName = siteName;
    jobCfg.isScheduledJob = (ret > 0);
    jobCfg.siteCfg.additionalFilesRootDir = QDir("/mnt/archive/insitu").filePath(siteName);

    // TODO: Here we should check if there are files that do not have the data extraction performed within the
    //      given interval if the requested operation is TimeSeriesAnalysis only
    //      In this case we should force a DataExtraction for the missing files
    QDateTime minDate, maxDate;
    QStringList ndviFiles, ampFiles, coheFiles;
    ExtractProductFiles(ctx, event, ndviFiles, ampFiles, coheFiles, minDate, maxDate, execOper);

    QList<TaskToSubmit> allTasksList;
    CreateTasks(jobCfg, allTasksList, ndviFiles, ampFiles, coheFiles, execOper);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    SubmitTasks(ctx, event.jobId, allTasksListRef);
    NewStepList allSteps;
    CreateSteps(ctx, event, allTasksList, jobCfg, ndviFiles, ampFiles, coheFiles, allSteps, minDate, maxDate,
                execOper);
    ctx.SubmitSteps(allSteps);
}

void AgricPracticesHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                              const TaskFinishedEvent &event)
{
    if (event.module == "product-formatter") {

        QString prodName = GetProductFormatterProductName(ctx, event);
        QString productFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId) + "/" + prodName;
        if(prodName != "") {
            QString quicklook = GetProductFormatterQuicklook(ctx, event);
            QString footPrint = GetProductFormatterFootprint(ctx, event);
            // Insert the product into the database
            QDateTime minDate, maxDate;
            ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prodName, minDate, maxDate);
            int prdId = ctx.InsertProduct({ ProductType::S4CL4CProductTypeId, event.processorId, event.siteId,
                                event.jobId, productFolder, maxDate,
                                prodName, quicklook, footPrint, std::experimental::nullopt, TileIdList() });

            const QString &prodFolderOutPath = ctx.GetOutputPath(event.jobId, event.taskId, event.module, processorDescr.shortName) +
                    "/" + "prd_infos.txt";

            QFile file( prodFolderOutPath );
            if ( file.open(QIODevice::ReadWrite) )
            {
                QTextStream stream( &file );
                stream << prdId << ";" << productFolder << endl;
            }
        } else {
            Logger::error(QStringLiteral("Cannot insert into database the product with name %1 and folder %2").arg(prodName).arg(productFolder));
        }
    } else if (event.module == "export-product-launcher") {
        ctx.MarkJobFinished(event.jobId);
        // Now remove the job folder containing temporary files
        RemoveJobFolder(ctx, event.jobId, processorDescr.shortName);
    }
}

QStringList AgricPracticesHandler::GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx,
                                                           const JobSubmittedEvent &event, const QStringList &listFiles,
                                                           const QDateTime &minDate, const QDateTime &maxDate) {
    // ProductFormatter /home/cudroiu/sen2agri-processors-build
    //    -vectprd 1 -destroot /mnt/archive_new/test/Sen4CAP_L4C_Tests/NLD_Validation_TSA/OutPrdFormatter
    //    -fileclass OPER -level S4C_L4C -baseline 01.00 -siteid 4 -timeperiod 20180101_20181231 -processor generic
    //    -processor.generic.files <files_list>

    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.txt");
    QString strTimePeriod = minDate.toString("yyyyMMddTHHmmss").append("_").append(maxDate.toString("yyyyMMddTHHmmss"));
    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot", targetFolder,
                                         "-fileclass", "OPER",
                                         "-level", "S4C_L4C",
                                         "-vectprd", "1",
                                         "-baseline", "01.00",
                                         "-siteid", QString::number(event.siteId),
                                         "-timeperiod", strTimePeriod,
                                         "-processor", "generic",
                                         "-outprops", outPropsPath,
                                         "-gipp", executionInfosPath };
    productFormatterArgs += "-processor.generic.files";
    productFormatterArgs += listFiles;

    return productFormatterArgs;
}

ProcessorJobDefinitionParams AgricPracticesHandler::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
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

    ConfigurationParameterValueMap mapCfg = ctx.GetConfigurationParameters(QString("processor.s4c_l4c."), siteId, requestOverrideCfgValues);
    std::map<QString, QString> configParams;
    for (const auto &p : mapCfg) {
        configParams.emplace(p.key, p.value);
    }

    // we might have an offset in days from starting the downloading products to start the S4C_L4C production
    // TODO: Is this really needed
    int startSeasonOffset = mapCfg["processor.s4c_l4c.start_season_offset"].value.toInt();
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
        params.jsonParameters.append(", \"execution_operation\": \"" + productType.value + "\"}");
    } else {
        params.jsonParameters.append("}");
    }

    params.isValid = true;


//    const ProductList &ndviPrdsList = ExtractMissingDataExtractionProducts(ctx, siteId, configParams, ProductType::L3BProductTypeId,
//                                                              startDate, endDate);
//    params.productList = FilterAndUpdateAlreadyProcessingPrds(ctx, siteId, ndviPrdsList, ProductType::L3BProductTypeId);

//    const ProductList &ampPrdsList = ExtractMissingDataExtractionProducts(ctx, siteId, configParams, ProductType::S4CS1L2AmpProductTypeId,
//                                                               startDate, endDate);
//    params.productList += FilterAndUpdateAlreadyProcessingPrds(ctx, siteId, ampPrdsList, ProductType::S4CS1L2AmpProductTypeId);

//    const ProductList &cohePrdsList = ExtractMissingDataExtractionProducts(ctx, siteId, configParams, ProductType::S4CS1L2CoheProductTypeId,
//                                                               startDate, endDate);
//    params.productList += FilterAndUpdateAlreadyProcessingPrds(ctx, siteId, cohePrdsList, ProductType::S4CS1L2CoheProductTypeId);

//    // Normally we need at least 3 products available in order to be able to create a S4C_L4C product
//    // but if we do not return here, the schedule block waiting for products (that might never happen)
//    bool waitForAvailProcInputs = (mapCfg["processor.s4c_l4c.sched_wait_proc_inputs"].value.toInt() != 0);
//    if((waitForAvailProcInputs == false) || (params.productList.size() >= 3)) {
//        params.isValid = true;
//        Logger::debug(QStringLiteral("Executing scheduled job. Scheduler extracted for S4C_L4C a number "
//                                     "of %1 products for site ID %2 with start date %3 and end date %4!")
//                      .arg(params.productList.size())
//                      .arg(siteId)
//                      .arg(startDate.toString())
//                      .arg(endDate.toString()));
//    } else {
//        Logger::debug(QStringLiteral("Scheduled job for S4C_L4C and site ID %1 with start date %2 and end date %3 "
//                                     "will not be executed (no products)!")
//                      .arg(siteId)
//                      .arg(startDate.toString())
//                      .arg(endDate.toString()));
//    }

    return params;
}

bool AgricPracticesHandler::GetSiteConfigForSiteId(EventProcessingContext &ctx, int siteId, const QJsonObject &parameters,
                                                                    std::map<QString, QString> &configParameters,
                                                                    AgricPracticesSiteCfg &retCfg)
{
    const QString &siteName = ctx.GetSiteShortName(siteId);
    const QString &siteCfgFilePath = GetSiteConfigFilePath(siteName, parameters, configParameters);
    if (siteCfgFilePath == "") {
        return false;
    }

    retCfg = LoadSiteConfigFile(ctx, siteId, siteCfgFilePath);
    return true;
}

// TODO: This is the version of getting the parameters from the database instead of the file
bool AgricPracticesHandler::GetSiteConfigForSiteId2(EventProcessingContext &ctx, int siteId, const QJsonObject &parameters,
                                                    std::map<QString, QString> &configParameters,
                                                    AgricPracticesSiteCfg &retCfg)
{
    AgricPracticesSiteCfg cfg;
    // TODO: We could try extracting the country and the year from the site name???

    cfg.country = ProcessorHandlerHelper::GetStringConfigValue(parameters, configParameters, "country", L4C_AP_CFG_PREFIX);
    cfg.year = ProcessorHandlerHelper::GetStringConfigValue(parameters, configParameters, "year", L4C_AP_CFG_PREFIX);

    GetLpisProductFiles(ctx, cfg.year, siteId, cfg.ndviIdsGeomShapePath, cfg.ampCoheIdsGeomShapePath, cfg.fullShapePath);

    const QStringList &practices = ProcessorHandlerHelper::GetStringConfigValue(parameters,
                                    configParameters, "practices", L4C_AP_CFG_PREFIX).split(",");
    for (const QString &strPractice: practices) {
        const QString &strTrimmedPractice = strPractice.trimmed();
        if(!strTrimmedPractice.isEmpty()) {
            cfg.practices.append(strTrimmedPractice);
        }
    }
    // validate practices
    TQStrQStrMap *pPracticeParams;
    TQStrQStrMap *pTsaParams;

    TQStrQStrMap practiceCfgParams;
    TQStrQStrMap tsaCfgParams;

    QString practicePrefix;
    QString tsaPrefix;

    for (const auto &practice: cfg.practices) {
        if (practice != "CC" && practice != "FL" && practice != "NFC" && practice != "NA") {
            throw std::runtime_error(QStringLiteral("Unsupported practice called %1.").arg(practice).toStdString());
        }
        if (practice == "CC") {
            pPracticeParams = &cfg.ccPracticeParams;
            pTsaParams = &cfg.ccTsaParams;
            practicePrefix = L4C_AP_GEN_CC_CFG_PREFIX;
            tsaPrefix = L4C_AP_TSA_CC_CFG_PREFIX;
        } else if (practice == "FL") {
            pPracticeParams = &cfg.flPracticeParams;
            pTsaParams = &cfg.flTsaParams;
            practicePrefix = L4C_AP_GEN_FL_CFG_PREFIX;
            tsaPrefix = L4C_AP_TSA_FL_CFG_PREFIX;
        } else if (practice == "NFC") {
            pPracticeParams = &cfg.nfcPracticeParams;
            pTsaParams = &cfg.nfcTsaParams;
            practicePrefix = L4C_AP_GEN_NFC_CFG_PREFIX;
            tsaPrefix = L4C_AP_TSA_NFC_CFG_PREFIX;
        } else if (practice == "NA") {
            pPracticeParams = &cfg.naPracticeParams;
            pTsaParams = &cfg.naTsaParams;
            practicePrefix = L4C_AP_GEN_NA_CFG_PREFIX;
            tsaPrefix = L4C_AP_TSA_NA_CFG_PREFIX;
        }
        practiceCfgParams = ProcessorHandlerHelper::FilterConfigParameters(configParameters, practicePrefix);
        tsaCfgParams = ProcessorHandlerHelper::FilterConfigParameters(configParameters, tsaPrefix);

        UpdatePracticesParams(parameters, configParameters, practiceCfgParams, practicePrefix, pPracticeParams);
        UpdatePracticesParams(parameters, configParameters, tsaCfgParams, tsaPrefix, pTsaParams);
    }

    retCfg = cfg;
    return true;
}

QString AgricPracticesHandler::GetSiteConfigFilePath(const QString &siteName, const QJsonObject &parameters, std::map<QString, QString> &configParameters)
{
    QString strCfgPath;
    // check first if provided in parameters
    if(parameters.contains("config_path")) {
        const auto &value = parameters["config_path"];
        if(value.isString()) {
            strCfgPath = value.toString();
        }
    }
    // if not provided in parameters, check if exists in the db config
    if (strCfgPath.isEmpty()) {
        strCfgPath = configParameters["processor.s4c_l4c.config_path"];
    }
    // otherwise, check if it is in the default location
    if (strCfgPath.isEmpty()) {
        const QString &strCfgRoot = configParameters["processor.s4c_l4c.config_root"];
        QDir dir(strCfgRoot);
        if (!dir.exists()) {
            strCfgPath = dir.filePath("S4C_L4C_Config_" + siteName + ".cfg");
        } else if (QDir("/usr/share/sen2agri/S4C_L4C_Configurations").exists()) {
            strCfgPath = QDir("/usr/share/sen2agri/S4C_L4C_Configurations").filePath("S4C_L4C_Config_" + siteName + ".cfg");
        }
    }

    if (strCfgPath.isEmpty() || !QFileInfo::exists(strCfgPath)) {
        return "";
    }
    return strCfgPath;
}

AgricPracticesSiteCfg AgricPracticesHandler::LoadSiteConfigFile(EventProcessingContext &ctx, int siteId,
                                                                const QString &siteCfgFilePath)
{
    AgricPracticesSiteCfg cfg;

    QSettings settings(siteCfgFilePath, QSettings::IniFormat);
    QString cmnSectionKey("COMMON/");

    cfg.country = settings.value( cmnSectionKey + "COUNTRY").toString();
    cfg.year = settings.value( cmnSectionKey + "YEAR").toString();
    cfg.tsaMinAcqsNo = settings.value( cmnSectionKey + "TSA_MIN_ACQ_NO").toString();

    GetLpisProductFiles(ctx, cfg.year, siteId, cfg.ndviIdsGeomShapePath, cfg.ampCoheIdsGeomShapePath, cfg.fullShapePath);
    cfg.practices = GetListValue(settings, cmnSectionKey + "PRACTICES");
    // validate practices
    for (const auto &practice: cfg.practices) {
        if (practice != "CC" && practice != "FL" && practice != "NFC" && practice != "NA") {
            throw std::runtime_error(
                QStringLiteral(
                    "Unsupported practice called %1.")
                    .arg(practice)
                    .toStdString());
        }
    }

    // Load the default practices
    const TQStrQStrMap &defParams = LoadParamsFromFile(settings, "", "DEFAULT_PRACTICES_PARAMS", cfg);
    const TQStrQStrMap &defTsaParams = LoadParamsFromFile(settings, "", "DEFAULT_TIME_SERIES_ANALYSIS_PARAMS", cfg);
    // Parameters used for practices tables extraction
    for (const QString &practice: cfg.practices) {
        TQStrQStrMap *pPracticeParams;
        TQStrQStrMap *pTsaParams;
        if (practice == "CC") {
            pPracticeParams = &cfg.ccPracticeParams;
            pTsaParams = &cfg.ccTsaParams;
        } else if (practice == "FL") {
            pPracticeParams = &cfg.flPracticeParams;
            pTsaParams = &cfg.flTsaParams;
        } else if (practice == "NFC") {
            pPracticeParams = &cfg.nfcPracticeParams;
            pTsaParams = &cfg.nfcTsaParams;
        } else if (practice == "NA") {
            pPracticeParams = &cfg.naPracticeParams;
            pTsaParams = &cfg.naTsaParams;
        }

        (*pPracticeParams) = LoadParamsFromFile(settings, practice, practice + "_PRACTICES_PARAMS", cfg);
        (*pTsaParams) = LoadParamsFromFile(settings, practice, practice + "_TIME_SERIES_ANALYSIS_PARAMS", cfg);
        UpdatePracticesParams(defParams, *pPracticeParams);
        UpdatePracticesParams(defTsaParams, *pTsaParams);
    }

    return cfg;
}

void AgricPracticesHandler::ExtractProductFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                        QStringList &ndviFiles, QStringList &ampFiles, QStringList &coheFiles,
                                                        QDateTime &minDate, QDateTime &maxDate, const AgricPractOperation &execOper)
{
    ndviFiles.append(ExtractNdviFiles(ctx, event, minDate, maxDate));
    if ((execOper == all) && (ndviFiles.size() == 0)) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No NDVI files provided for site %1 for ALL mode!\n").
                    arg(ctx.GetSiteShortName(event.siteId)).toStdString());
    }
    ampFiles.append(ExtractAmpFiles(ctx, event, minDate, maxDate));
    if  ((execOper == all) && (ampFiles.size() == 0)) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No Amplitude files provided for site %1 for ALL mode!\n").
                    arg(ctx.GetSiteShortName(event.siteId)).toStdString());
    }
    coheFiles.append(ExtractCoheFiles(ctx, event, minDate, maxDate));
    if  ((execOper == all) && (coheFiles.size() == 0)) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No Coherence files provided for site %1 for ALL mode!\n").
                    arg(ctx.GetSiteShortName(event.siteId)).toStdString());
    }
    if ((execOper == dataExtraction) && (ndviFiles.size() == 0) && (ampFiles.size() == 0) && (coheFiles.size() == 0)) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No NDVI, Amplitude or Coherence files provided for site %1 for data extraction mode!\n").
                    arg(ctx.GetSiteShortName(event.siteId)).toStdString());
    }
}

QStringList AgricPracticesHandler::ExtractNdviFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                    QDateTime &minDate, QDateTime &maxDate)
{
    return S4CUtils::GetInputProducts(ctx, event, ProductType::L3BProductTypeId,
                                      minDate, maxDate, L4C_AP_CFG_PREFIX);
}

QStringList AgricPracticesHandler::ExtractAmpFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                   QDateTime &minDate, QDateTime &maxDate)
{
    return S4CUtils::GetInputProducts(ctx, event, ProductType::S4CS1L2AmpProductTypeId,
                                      minDate, maxDate, L4C_AP_CFG_PREFIX);
}

QStringList AgricPracticesHandler::ExtractCoheFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                    QDateTime &minDate, QDateTime &maxDate)
{
    return S4CUtils::GetInputProducts(ctx, event, ProductType::S4CS1L2CoheProductTypeId,
                                      minDate, maxDate, L4C_AP_CFG_PREFIX);
}


QStringList AgricPracticesHandler::GetIdsExtractorArgs(const AgricPracticesJobCfg &jobCfg, const QString &outFile,
                                                       const QString &finalTargetDir)
{
    QStringList retArgs = { "LPISDataSelection", "-inshp", jobCfg.siteCfg.fullShapePath, "-country", jobCfg.siteCfg.country,
                            "-seqidsonly", "1", "-out", outFile };
    if (finalTargetDir.size() > 0) {
        retArgs += "-copydir";
        retArgs += finalTargetDir;
    }
    TQStrQStrMap::const_iterator itFiles = jobCfg.siteCfg.naPracticeParams.find("addfiles");
    if (itFiles != jobCfg.siteCfg.naPracticeParams.end() && itFiles->second.trimmed().size() > 0) {
        retArgs += "-addfiles";
        retArgs += GetAdditionalFilesAsList(itFiles->second, jobCfg.siteCfg);
    }
    return retArgs;
}

QStringList AgricPracticesHandler::GetPracticesExtractionArgs(const AgricPracticesJobCfg &jobCfg, const QString &outFile,
                                                              const QString &practice, const QString &finalTargetDir)
{
    const TQStrQStrMap *pPracticeParams = 0;
    QString dataSelPractice = GetTsaExpectedPractice(practice);
    if (practice == "CC") {
        pPracticeParams = &(jobCfg.siteCfg.ccPracticeParams);
    } else if (practice == "FL") {
        pPracticeParams = &(jobCfg.siteCfg.flPracticeParams);
    } else if (practice == "NFC") {
        pPracticeParams = &(jobCfg.siteCfg.nfcPracticeParams);
    } else if (practice == "NA") {
        pPracticeParams = &(jobCfg.siteCfg.naPracticeParams);
    }

    QStringList retArgs = { "LPISDataSelection", "-inshp", jobCfg.siteCfg.fullShapePath, "-country", jobCfg.siteCfg.country,
                            "-practice", dataSelPractice, "-year", jobCfg.siteCfg.year, "-out", outFile };

    if (finalTargetDir.size() > 0) {
        retArgs += "-copydir";
        retArgs += finalTargetDir;
    }

    for (TQStrQStrMap::const_iterator it=pPracticeParams->begin(); it!=pPracticeParams->end(); ++it) {
        if (it->second.size() > 0) {
            retArgs += ("-" + it->first);
            if (it->first == "addfiles") {
                retArgs += GetAdditionalFilesAsList(it->second, jobCfg.siteCfg);
            } else {
                retArgs += it->second;
            }
        }
    }

    return retArgs;

}

QStringList AgricPracticesHandler::GetDataExtractionArgs(const AgricPracticesJobCfg &jobCfg, const QString &filterIdsFile,
                                                         const ProductType &prdType, const QString &uidField,
                                                         const QStringList &inputFiles, const QString &outDir)
{
    QStringList retArgs = { "AgricPractDataExtraction", "-oldnf", "0", "-minmax", "0", "-csvcompact", "1", "-field", uidField,
                            "-prdtype", GetShortNameForProductType(prdType), "-outdir", outDir, "-filterids", filterIdsFile, "-il" };
    retArgs += inputFiles;
    const QString *idsGeomShapePath;
    if (prdType == ProductType::L3BProductTypeId) {
        idsGeomShapePath = &(jobCfg.siteCfg.ndviIdsGeomShapePath);
    } else {
        idsGeomShapePath = &(jobCfg.siteCfg.ampCoheIdsGeomShapePath);
    }
    if (idsGeomShapePath->size() > 0) {
        retArgs += "-vec";
        retArgs += *idsGeomShapePath;
    }

    return retArgs;

}

QStringList AgricPracticesHandler::GetFilesMergeArgs(const QStringList &listInputPaths, const QString &outFileName)
{
    QStringList retArgs = { "AgricPractMergeDataExtractionFiles", "-csvcompact", "1", "-outformat", "csv", "-out", outFileName, "-il" };
    retArgs += listInputPaths;
    return retArgs;
}

QStringList AgricPracticesHandler::GetTimeSeriesAnalysisArgs(const AgricPracticesJobCfg &siteJobCfg, const QString &practice, const QString &practicesFile,
                                                             const QString &inNdviFile, const QString &inAmpFile, const QString &inCoheFile,
                                                             const QString &outDir)
{
    const TQStrQStrMap *pTsaPracticeParams = 0;
    QString tsaExpectedPractice = GetTsaExpectedPractice(practice);
    if (practice == "CC") {
        pTsaPracticeParams = &(siteJobCfg.siteCfg.ccTsaParams);
    } else if (practice == "FL") {
        pTsaPracticeParams = &(siteJobCfg.siteCfg.flTsaParams);
    } else if (practice == "NFC") {
        pTsaPracticeParams = &(siteJobCfg.siteCfg.nfcTsaParams);
    } else if (practice == "NA") {
        pTsaPracticeParams = &(siteJobCfg.siteCfg.naTsaParams);
    }
    QStringList retArgs = { "TimeSeriesAnalysis", "-intype", "csv", "-debug", "0", "-allowgaps", "1", "-gapsfill", "0", "-plotgraph", "1",
                            "-rescontprd", "0", "-country", siteJobCfg.siteCfg.country, "-practice", tsaExpectedPractice, "-year", siteJobCfg.siteCfg.year,
                            "-harvestshp", practicesFile, "-diramp", inAmpFile, "-dircohe", inCoheFile, "-dirndvi", inNdviFile,
                            "-outdir", outDir };
    if (siteJobCfg.siteCfg.tsaMinAcqsNo.size() > 0) {
        retArgs += "-minacqs";
        retArgs += siteJobCfg.siteCfg.tsaMinAcqsNo;
    }
    for (TQStrQStrMap::const_iterator it=pTsaPracticeParams->begin(); it!=pTsaPracticeParams->end(); ++it) {
        if (it->second.size() > 0) {
            retArgs += ("-" + it->first);
            retArgs += it->second;
        }
    }
    return retArgs;
}

QString AgricPracticesHandler::BuildMergeResultFileName(const AgricPracticesJobCfg &jobCfg, const ProductType &prdsType)
{
    return QString(jobCfg.siteCfg.country).append("_").append(jobCfg.siteCfg.year).append("_").
            append(GetShortNameForProductType(prdsType)).append("_Extracted_Data.csv");
}

QString AgricPracticesHandler::BuildPracticesTableResultFileName(const AgricPracticesJobCfg &jobCfg, const QString &suffix)
{
    return QString("Sen4CAP_L4C_").append(suffix).append("_").append(jobCfg.siteCfg.country).
            append("_").append(jobCfg.siteCfg.year).append(".csv");
}

void AgricPracticesHandler::CreatePrdDataExtrTasks(const AgricPracticesJobCfg &jobCfg, QList<TaskToSubmit> &outAllTasksList,
                                        const QString &taskName,
                                        const QStringList &prdsList, const QList<std::reference_wrapper<const TaskToSubmit>> &dataExtParents,
                                        AgricPractOperation operation, int &minPrdDataExtrIndex, int &maxPrdDataExtrIndex, int &curTaskIdx) {
    // if the products per group is 0 or negative, then create a group with all products
    int prdsPerGroup = jobCfg.prdsPerGroup > 0 ? jobCfg.prdsPerGroup : prdsList.size();
    // if operation is exactly data extraction, then we the groups will have exactly 1 item
    if (operation == dataExtraction || jobCfg.isScheduledJob) {
        prdsPerGroup = 1;
    }

    int groups = (prdsList.size()/prdsPerGroup) + ((prdsList.size()%prdsPerGroup) == 0 ? 0 : 1);

    // create the tasks for the NDVI - they depend only on the ids extraction
    if (groups > 0) {
        minPrdDataExtrIndex = curTaskIdx;
        maxPrdDataExtrIndex = curTaskIdx + groups - 1;
        for(int i  = 0; i<groups; i++) {
            outAllTasksList.append(TaskToSubmit{ taskName, dataExtParents });
            curTaskIdx++;
        }
    }
}

int AgricPracticesHandler::CreateMergeTasks(QList<TaskToSubmit> &outAllTasksList, const QString &taskName,
                                        int minPrdDataExtrIndex, int maxPrdDataExtrIndex, int &curTaskIdx) {
    outAllTasksList.append(TaskToSubmit{ taskName, {} });
    int mergeTaskIdx = curTaskIdx++;
    // update the parents for this task
    if (minPrdDataExtrIndex != -1) {
        for (int i = minPrdDataExtrIndex; i <= maxPrdDataExtrIndex; i++) {
            outAllTasksList[mergeTaskIdx].parentTasks.append(outAllTasksList[i]);
        }
    }
    return mergeTaskIdx;
}

int AgricPracticesHandler::CreateTSATasks(const AgricPracticesJobCfg &siteCfg, QList<TaskToSubmit> &outAllTasksList,
                                        const QString &practiceName, AgricPractOperation operation,
                                        int ndviMergeTaskIdx, int ampMergeTaskIdx, int coheMergeTaskIdx, int &curTaskIdx) {
    int ccTsaIdx = -1;
    AgricPractOperation expectedOper = GetExecutionOperation(practiceName);
    if (siteCfg.siteCfg.practices.contains(practiceName) && IsOperationEnabled(operation, expectedOper)) {
        // this task is independent and can be executed before any others
        const QString &lowerPracticeName = practiceName.toLower();
        outAllTasksList.append(TaskToSubmit{ lowerPracticeName + "-practices-extraction", {} });
        int ccPractExtrIdx = curTaskIdx++;
        outAllTasksList.append(TaskToSubmit{ lowerPracticeName + "-time-series-analysis", {outAllTasksList[ccPractExtrIdx],
                                                                         outAllTasksList[ndviMergeTaskIdx], outAllTasksList[ampMergeTaskIdx],
                                                                         outAllTasksList[coheMergeTaskIdx]} });
        ccTsaIdx = curTaskIdx++;
    }
    return ccTsaIdx;
}

QString AgricPracticesHandler::CreateStepForLPISSelection(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                                                AgricPractOperation operation, const QString &practice,
                                                                const AgricPracticesJobCfg &jobCfg,
                                                                QList<TaskToSubmit> &allTasksList,
                                                                NewStepList &steps, int &curTaskIdx) {
    QString tsInputTablesDir ;
    TaskToSubmit &lpisSelectionTask = allTasksList[curTaskIdx++];
    QString fileName;
    if (practice.size() == 0) {
        fileName = BuildPracticesTableResultFileName(jobCfg, "FilterIds");
    } else {
        fileName = BuildPracticesTableResultFileName(jobCfg, practice);
    }

    const QString &tmpLpisSelFilePath = lpisSelectionTask.GetFilePath(fileName);
    QString lpisSelFilePath = tmpLpisSelFilePath;
    if (operation == dataExtraction || jobCfg.isScheduledJob) {
        // get the directory where the ids file should be finally moved
        tsInputTablesDir = GetProcessorDirValue(parameters, configParameters, "ts_input_tables_dir", jobCfg.siteShortName,
                             "/mnt/archive/agric_practices_files/{site}/input_files");
        lpisSelFilePath = QDir(tsInputTablesDir).filePath(fileName);
        QDir().mkpath(tsInputTablesDir);
    }
    QStringList lpisSelectionArgs;
    if (practice.size() == 0) {
        lpisSelectionArgs = GetIdsExtractorArgs(jobCfg, tmpLpisSelFilePath, tsInputTablesDir);
    } else {
        lpisSelectionArgs = GetPracticesExtractionArgs(jobCfg, tmpLpisSelFilePath, practice, tsInputTablesDir);
    }

    steps.append(lpisSelectionTask.CreateStep("LPISDataSelection", lpisSelectionArgs));

    return lpisSelFilePath;
}

QStringList AgricPracticesHandler::CreateStepsForDataExtraction(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                                                AgricPractOperation operation,
                                                                const AgricPracticesJobCfg &jobCfg, const ProductType &prdType,
                                                                const QStringList &prds, const QString &idsFileName,
                                                                QList<TaskToSubmit> &allTasksList,
                                                                NewStepList &steps, int &curTaskIdx)
{
    // if the products per group is 0 or negative, then create a group with all products
    int prdsPerGroup = jobCfg.prdsPerGroup > 0 ? jobCfg.prdsPerGroup : prds.size();
    if (operation == dataExtraction || jobCfg.isScheduledJob) {
        prdsPerGroup = 1;
    }
    int groups = (prds.size()/prdsPerGroup) + ((prds.size()%prdsPerGroup) == 0 ? 0 : 1);

    // create the tasks for the NDVI, AMP and COHE - they depend only on the ids extraction
    QStringList dataExtrDirs;
    for(int i  = 0; i<groups; i++) {
        TaskToSubmit &dataExtractionTask = allTasksList[curTaskIdx++];
        QString dataExtrDirName;
        if (operation == all && !jobCfg.isScheduledJob) {
            dataExtrDirName = dataExtractionTask.GetFilePath("");
        } else {
            dataExtrDirName = GetDataExtractionDir(parameters, configParameters, prdType, jobCfg.siteShortName);
            QDir().mkpath(dataExtrDirName);
        }
        if (!dataExtrDirs.contains(dataExtrDirName)) {
            dataExtrDirs.append(dataExtrDirName);
        }

        // Extract the list of products from the current group
        int grpStartIdx = i * prdsPerGroup;
        int nextGrpStartIdx = (i+1) * prdsPerGroup;
        QStringList sublist;
        for (int j = grpStartIdx; j < nextGrpStartIdx && j < prds.size(); j++) {
            sublist.append(prds.at(j));
        }

        const QStringList &dataExtractionArgs = GetDataExtractionArgs(jobCfg, idsFileName, prdType, "NewID", sublist, dataExtrDirName);
        steps.append(dataExtractionTask.CreateStep("AgricPractDataExtraction", dataExtractionArgs));
    }
    return dataExtrDirs;
}

QString AgricPracticesHandler::CreateStepsForFilesMerge(const AgricPracticesJobCfg &siteCfg, const ProductType &prdType,
                              const QStringList &dataExtrDirs, NewStepList &steps,
                              QList<TaskToSubmit> &allTasksList, int &curTaskIdx) {
    TaskToSubmit &mergeTask = allTasksList[curTaskIdx++];
    const QString &mergedFile = mergeTask.GetFilePath(BuildMergeResultFileName(siteCfg, prdType));
    const QStringList &mergeArgs = GetFilesMergeArgs(dataExtrDirs, mergedFile);
    steps.append(mergeTask.CreateStep("AgricPractMergeDataExtractionFiles", mergeArgs));

    return mergedFile;
}

QStringList AgricPracticesHandler::CreateTimeSeriesAnalysisSteps(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                                                 AgricPractOperation operation, const AgricPracticesJobCfg &siteCfg,
                                                                 const QString &practice, const QString &ndviMergedFile,
                                                                 const QString &ampMergedFile, const QString &coheMergedFile,
                                                                 NewStepList &steps,
                                                                 QList<TaskToSubmit> &allTasksList, int &curTaskIdx,
                                                                 AgricPractOperation activeOper)
{
    QStringList retList;
    AgricPractOperation oper = GetExecutionOperation(practice);
    if (siteCfg.siteCfg.practices.contains(practice) && ((oper & activeOper) != none)) {
        const QString &practicesFile = CreateStepForLPISSelection(parameters, configParameters, operation, practice, siteCfg,
                                                                allTasksList, steps, curTaskIdx);

        TaskToSubmit &timeSeriesAnalysisTask = allTasksList[curTaskIdx++];
        const QString &timeSeriesExtrDir = timeSeriesAnalysisTask.GetFilePath("");
        const QStringList &timeSeriesAnalysisArgs = GetTimeSeriesAnalysisArgs(siteCfg, practice, practicesFile,
                                                                                ndviMergedFile, ampMergedFile, coheMergedFile,
                                                                                timeSeriesExtrDir);
        steps.append(timeSeriesAnalysisTask.CreateStep("TimeSeriesAnalysis", timeSeriesAnalysisArgs));

        // Add the expected files to the productFormatterFiles
        const QString &tsaExpPractice = GetTsaExpectedPractice(practice);
        const QString &filesPrefix = "Sen4CAP_L4C_" + tsaExpPractice + "_" + siteCfg.siteCfg.country + "_" + siteCfg.siteCfg.year;
        const QString &mainFileName = filesPrefix + "_CSV.csv";
        const QString &plotFileName = filesPrefix + "_PLOT.xml";
        const QString &plotIdxFileName = plotFileName + ".idx";
        const QString &contPrdFileName = filesPrefix + "_CSV_ContinousProduct.csv";

        retList.append(QDir(timeSeriesExtrDir).filePath(mainFileName));
        retList.append(QDir(timeSeriesExtrDir).filePath(plotFileName));
        retList.append(QDir(timeSeriesExtrDir).filePath(plotIdxFileName));
        retList.append(QDir(timeSeriesExtrDir).filePath(contPrdFileName));
    }
    return retList;
}

TQStrQStrMap AgricPracticesHandler::LoadParamsFromFile(QSettings &settings, const QString &practicePrefix, const QString &sectionName,
                                                       const AgricPracticesSiteCfg &cfg) {
    TQStrQStrMap params;
    //const QString &sectionName = (practicePrefix.length() > 0 ? practicePrefix : QString("DEFAULT")) + "_PRACTICES_PARAMS/";
    QString keyPrefix;
    if(practicePrefix.length() > 0) {
        keyPrefix = practicePrefix + "_";
    }
    settings.beginGroup(sectionName);
    const QStringList &keys = settings.allKeys();
    foreach (const QString &key, keys) {
        QString keyNoPrefix = key;
        keyNoPrefix.remove(0, keyPrefix.size());
        QString value = settings.value(key).toString();
        value.replace("${YEAR}", cfg.year);
        params.insert(TQStrQStrPair(keyNoPrefix.toLower(), value));
    }
    settings.endGroup();

    return params;
}

void AgricPracticesHandler::UpdatePracticesParams(const QJsonObject &parameters, std::map<QString, QString> &configParameters,
                                                  const TQStrQStrMap &cfgVals, const QString &prefix,
                                                  TQStrQStrMap *params) {
    for (TQStrQStrMap::const_iterator it=cfgVals.begin(); it!=cfgVals.end(); ++it) {
        QString key = it->first;
        // get the last part of the key, without prefix
        key.remove(0, prefix.size());
        // check if the value for this key is somehow provided in the parameters
        // otherwise, take it from the config parameters
        const QString &value = ProcessorHandlerHelper::GetStringConfigValue(parameters, configParameters, key, prefix).trimmed();
        if(value.size() > 0) {
            params->insert(TQStrQStrPair(key, value));
        }
    }
}

void AgricPracticesHandler::UpdatePracticesParams(const TQStrQStrMap &defVals,
                                                TQStrQStrMap &sectionVals) {
    for (TQStrQStrMap::const_iterator it=defVals.begin(); it!=defVals.end(); ++it) {
        if (sectionVals.find(it->first) == sectionVals.end()) {
            // insert the default value
            sectionVals.insert(TQStrQStrPair(it->first, it->second));
        }
    }
}
QStringList AgricPracticesHandler::GetListValue(const QSettings &settings, const QString &key)
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

QString AgricPracticesHandler::GetTsaExpectedPractice(const QString &practice)
{
    QString retPractice = practice;
    if (practice == "CC") {
        retPractice = "CatchCrop";
    } else if (practice == "FL") {
        retPractice = "Fallow";
    }
    return retPractice;
}

AgricPractOperation AgricPracticesHandler::GetExecutionOperation(const QJsonObject &parameters, const std::map<QString, QString> &configParameters)
{
    const QString &execOper = ProcessorHandlerHelper::GetStringConfigValue(parameters, configParameters, "execution_operation", L4C_AP_CFG_PREFIX);
    return GetExecutionOperation(execOper);
}

AgricPractOperation AgricPracticesHandler::GetExecutionOperation(const QString &str)
{
    if (QString::compare(str, "DataExtraction", Qt::CaseInsensitive) == 0) {
        return dataExtraction;
    } else if (QString::compare(str, "AllTimeSeriesAnalysis", Qt::CaseInsensitive) == 0) {
        return timeSeriesAnalysis;
    } else if (QString::compare(str, "CatchCrop", Qt::CaseInsensitive) == 0 ||
               QString::compare(str, "CC", Qt::CaseInsensitive) == 0) {
        return catchCrop;
    } else if (QString::compare(str, "Fallow", Qt::CaseInsensitive) == 0 ||
               QString::compare(str, "FL", Qt::CaseInsensitive) == 0) {
        return fallow;
    } else if (QString::compare(str, "NFC", Qt::CaseInsensitive) == 0) {
        return nfc;
    } else if (QString::compare(str, "HarvestOnly", Qt::CaseInsensitive) == 0 ||
               QString::compare(str, "NA", Qt::CaseInsensitive) == 0) {
        return harvestOnly;
    } else if (QString::compare(str, "ALL", Qt::CaseInsensitive) == 0) {
        return all;
    }
    return none;
}

bool AgricPracticesHandler::IsOperationEnabled(AgricPractOperation oper, AgricPractOperation expected) {
    return ((oper & expected) != none);
}

void AgricPracticesHandler::HandleProductAvailableImpl(EventProcessingContext &ctx,
                                const ProductAvailableEvent &event)
{
    // Get the product description from the database
    const Product &prd = ctx.GetProduct(event.productId);
    // Check that the product type is NDVI, AMP or COHE
    const QString &prdTypeShortName = GetShortNameForProductType(prd.productTypeId);
    const QString &prdKey = "input_" + prdTypeShortName;
    if (prdKey == "input_") {
        Logger::error(QStringLiteral("Unsupported product type %1.").arg(QString::number((int)prd.productTypeId)));
        return;
    }
    QJsonObject parameters;
    // check if the NRT data extraction is configured for the site
    auto configParameters = ctx.GetConfigurationParameters(L4C_AP_CFG_PREFIX, prd.siteId);
    bool nrtDataExtrEnabled = ProcessorHandlerHelper::GetBoolConfigValue(parameters, configParameters, "nrt_data_extr_enabled", L4C_AP_CFG_PREFIX);
    if (nrtDataExtrEnabled) {
        // Create a new JOB
        NewJob newJob;
        newJob.processorId = processorDescr.processorId;  //send the job to this processor
        newJob.siteId = prd.siteId;
        newJob.startType = JobStartType::Triggered;

        QJsonObject processorParamsObj;
        QJsonArray prodsJsonArray;
        prodsJsonArray.append(prd.fullPath);
        processorParamsObj[prdKey] = prodsJsonArray;
        processorParamsObj["execution_operation"] = "DataExtraction";
        newJob.parametersJson = jsonToString(processorParamsObj);
        ctx.SubmitJob(newJob);
    }
}

bool AgricPracticesHandler::GetLpisProductFiles(EventProcessingContext &ctx, const QString &yearStr, int siteId,
                                                QString &ndviShpFile, QString &ampCoheShpFile, QString &allFieldsFile) {
    ndviShpFile = "";
    ampCoheShpFile = "";
    allFieldsFile = "";
    // TODO: We should take it the last one for this site.
    QDate  startDate, endDate;
    startDate.setDate(1970, 1, 1);
    QDateTime startDateTime(startDate);
    endDate.setDate(2050, 12, 31);
    QDateTime endDateTime(endDate);
    const ProductList &lpisPrds = ctx.GetProducts(siteId, (int)ProductType::S4CLPISProductTypeId, startDateTime, endDateTime);
    if (lpisPrds.size() == 0) {
        throw std::runtime_error("No LPIS product found in database for the agricultural practices execution");
    }
    const QString &siteName = ctx.GetSiteShortName(siteId);

    // If the year is >= 2019, then use LAEA for AMP and COHE and no matter which other for NDVI
    const QString &prdLpisPath = lpisPrds[lpisPrds.size()-1].fullPath;
    QDir directory(prdLpisPath);
    QString allFieldsName = "decl_" + siteName + "_" + yearStr + ".csv";

    const QStringList &dirFiles = directory.entryList(QStringList() << "*.shp" << "*.csv",QDir::Files);
    foreach(const QString &fileName, dirFiles) {
        // we don't want for NDVI the LAEA projection
        if (fileName.endsWith("_buf_5m.shp") && (!fileName.endsWith("_3035_buf_5m.shp")) && (ndviShpFile.size() == 0)) {
            ndviShpFile = directory.filePath(fileName);
        }
        // LAEA projection have priority for 10m buffer
        if (fileName.endsWith("_3035_buf_10m.shp") ||
                (fileName.endsWith("_buf_10m.shp") && ampCoheShpFile.size() == 0)) {
            ampCoheShpFile = directory.filePath(fileName);
        }
        // we know its name but we want to be sure it is there
        if (fileName == allFieldsName) {
            allFieldsFile = directory.filePath(fileName);
        }
    }

    return (ndviShpFile.size() != 0 && ampCoheShpFile.size() != 0 && allFieldsFile.size() != 0);
}

QStringList AgricPracticesHandler::GetAdditionalFilesAsList(const QString &files, const AgricPracticesSiteCfg &cfg) {
    QRegExp separators(",| |;");
    const QStringList &listFileNames = files.split(separators,  QString::SkipEmptyParts);
    QStringList retList;
    for(const QString &fileName: listFileNames) {
        if (fileName.trimmed().size() > 0) {
            if (QFileInfo::exists(fileName) && QFileInfo(fileName).isFile()) {
                retList.append(fileName);
            } else {
                // Build the file path
                retList.append(QDir(cfg.additionalFilesRootDir).filePath(fileName));
            }
        }
    }
    return retList;
}

QString AgricPracticesHandler::GetShortNameForProductType(const ProductType &prdType) {
    switch(prdType) {
        case ProductType::L3BProductTypeId:
            return "NDVI";
        case ProductType::S4CS1L2AmpProductTypeId:
            return "AMP";
        case ProductType::S4CS1L2CoheProductTypeId:
            return "COHE";
        default:
        return "";
    }
}

int AgricPracticesHandler::UpdateJobSubmittedParamsFromSchedReq(EventProcessingContext &ctx,
                                           const JobSubmittedEvent &event, const std::map<QString, QString> &configParameters,
                                           QJsonObject &parameters, const QString &siteName, JobSubmittedEvent &newEvent) {
    // initialize the new event
    newEvent = event;

    QString strStartDate, strEndDate;
    if(IsScheduledJobRequest(parameters) &&
            ProcessorHandlerHelper::GetParameterValueAsString(parameters, "start_date", strStartDate) &&
            ProcessorHandlerHelper::GetParameterValueAsString(parameters, "end_date", strEndDate) &&
            parameters.contains("input_products") && parameters["input_products"].toArray().size() == 0) {

        const auto &startDate = QDateTime::fromString(strStartDate, "yyyyMMdd");
        const auto &endDate = QDateTime::fromString(strEndDate, "yyyyMMdd");
        Logger::info(QStringLiteral("Agricultural Practices Scheduled job received for siteId = %1, startDate=%2, endDate=%3").
                     arg(event.siteId).arg(startDate.toString("yyyyMMdd")).arg(endDate.toString("yyyyMMdd")));

        QStringList ndviProcessedFiles, ampProcessedFiles, coheProcessedFiles;
        const QStringList &ndviMissingPrdsList = ExtractMissingDataExtractionProducts(ctx, event.siteId, siteName,
                                                                      configParameters, ProductType::L3BProductTypeId,
                                                                      startDate, endDate, ndviProcessedFiles);
        const QStringList &ndviFilteredPrdsList = FilterAndUpdateAlreadyProcessingPrds(ctx, event.siteId, ndviMissingPrdsList,
                                                                                       ndviProcessedFiles, ProductType::L3BProductTypeId);

        const QStringList &ampMissingPrdsList = ExtractMissingDataExtractionProducts(ctx, event.siteId, siteName,
                                                                       configParameters, ProductType::S4CS1L2AmpProductTypeId,
                                                                       startDate, endDate, ampProcessedFiles);
        const QStringList &ampFilteredPrdsList = FilterAndUpdateAlreadyProcessingPrds(ctx, event.siteId, ampMissingPrdsList,
                                                                                      coheProcessedFiles, ProductType::S4CS1L2AmpProductTypeId);

        const QStringList &coheMissingPrdsList = ExtractMissingDataExtractionProducts(ctx, event.siteId, siteName,
                                                                       configParameters, ProductType::S4CS1L2CoheProductTypeId,
                                                                       startDate, endDate, coheProcessedFiles);
        const QStringList &coheFilteredPrdsList = FilterAndUpdateAlreadyProcessingPrds(ctx, event.siteId, coheMissingPrdsList,
                                                                                       coheProcessedFiles, ProductType::S4CS1L2CoheProductTypeId);

        const QJsonArray &ndviInputProductsArr = ProductListToJSonArray(ndviFilteredPrdsList);
        const QJsonArray &ampInputProductsArr = ProductListToJSonArray(ampFilteredPrdsList);
        const QJsonArray &coheInputProductsArr = ProductListToJSonArray(coheFilteredPrdsList);
        Logger::info(QStringLiteral("Agricultural Practices Scheduled job : Updating input products for jobId = %1, siteId = %2 with a "
                                    "number of %3 NDVI products, %4 AMP products and %5 COHE products").
                     arg(event.jobId).arg(event.siteId).arg(ndviInputProductsArr.size()).
                     arg(ampInputProductsArr.size()).arg(coheInputProductsArr.size()));

        if (ndviInputProductsArr.size() > 0) {
            parameters[QStringLiteral("input_NDVI")] = ndviInputProductsArr;
        }
        if (ampInputProductsArr.size() > 0) {
            parameters[QStringLiteral("input_AMP")] = ampInputProductsArr;
        }
        if (coheInputProductsArr.size() > 0) {
            parameters[QStringLiteral("input_COHE")] = coheInputProductsArr;
        }

        newEvent.parametersJson = jsonToString(parameters);
        return ndviInputProductsArr.size() + ampInputProductsArr.size() + coheInputProductsArr.size();
    }
    return -1;
}

QStringList AgricPracticesHandler::ExtractMissingDataExtractionProducts(EventProcessingContext &ctx, int siteId, const QString &siteName,
                                                                        const std::map<QString, QString> &configParameters,
                                                                        const ProductType &prdType, const QDateTime &startDate,
                                                                        const QDateTime &endDate, QStringList &alreadyProcessedFiles) {
    QJsonObject parameters;
    const QString &dataExtrDirName = GetDataExtractionDir(parameters, configParameters, prdType, siteName);;

    const ProductList &prds = ctx.GetProducts(siteId, (int)prdType, startDate, endDate);
    QStringList retList;
    for(const Product &prd: prds) {
        QStringList prdFileNames {prd.fullPath};
        if (prdType == ProductType::L3BProductTypeId) {
            prdFileNames = S4CUtils::FindL3BProductTiffFiles(prd.fullPath, {});
        }
        for(const QString &prdFileName: prdFileNames) {
            if (!IsDataExtractionPerformed(dataExtrDirName, prdFileName)) {
                retList.append(prdFileName);
            } else {
                alreadyProcessedFiles.append(prdFileName);
            }
        }
    }
    return retList;
}

bool AgricPracticesHandler::IsDataExtractionPerformed(const QString &dataExtrDirPath, const QString &prdPath) {
    QFileInfo fileInfo(prdPath);
    QDir directory(dataExtrDirPath);
    // empty parameters
    const QString & fileNameNoExt = fileInfo.completeBaseName();
    const QStringList &txtFiles = directory.entryList(QStringList() << fileNameNoExt + ".txt",QDir::Files);
    if (txtFiles.size() > 0) {
        return true;
    }

    return false;
}

/**
 * @brief AgricPracticesHandler::FilterAndUpdateAlreadyProcessingPrds
 *  Returns the products that were not already launched for processing in case of overlapping schedulings
 */
QStringList AgricPracticesHandler::FilterAndUpdateAlreadyProcessingPrds(EventProcessingContext &ctx, int siteId, const QStringList &missingPrdsFiles,
                                                                        const QStringList &processedPrdsFiles, const ProductType &prdType) {
    QStringList filteredPrds;
    const std::map<QString, QString> &mapCfg = ctx.GetConfigurationParameters(PRODUCTS_LOCATION_CFG_KEY);
    std::map<QString, QString>::const_iterator it = mapCfg.find(PRODUCTS_LOCATION_CFG_KEY);
    QString fileParentPath;
    if (it != mapCfg.end()) {
        fileParentPath = it->second;
    } else {
        fileParentPath = "/mnt/archive/{site}/{processor}/";
    }
    fileParentPath = fileParentPath.replace("{site}", ctx.GetSiteShortName(siteId));
    fileParentPath = fileParentPath.replace("{processor}", processorDescr.shortName);
    const QString &filePath = QDir::cleanPath(fileParentPath + QDir::separator() + GetShortNameForProductType(prdType) +
                                              "_current_data_extraction_products.txt");

    QDir().mkpath(QFileInfo(filePath).absolutePath());
    QFile file( filePath );
    // First read all the entries in the file to see what are the products that are currently processing

    QStringList curProcPrds;
    if (file.open(QIODevice::ReadOnly))
    {
       QTextStream in(&file);
       while (!in.atEnd())
       {
          curProcPrds.append(in.readLine());
       }
       file.close();
    }
    if (curProcPrds.size() > 0) {
        // remove already processed L2A products from this file
        for(const QString &prdPath: processedPrdsFiles) {
            curProcPrds.removeAll(prdPath);
        }
    }
    // add the products that will be processed next
    for (int i = 0; i<missingPrdsFiles.size(); i++) {
        const QString &prdFile =  missingPrdsFiles[i];
        if (!curProcPrds.contains(prdFile)) {
            curProcPrds.append(prdFile);
            filteredPrds.append(prdFile);
        }
        // else, if the product was already in this list, then it means it was already scheduled for processing
        // by another schedule operation
    }

    if ( file.open(QIODevice::ReadWrite | QFile::Truncate) )
    {
        QTextStream stream( &file );
        for (const QString &prdPath: curProcPrds) {
            stream << prdPath << endl;
        }
    }

    return filteredPrds;
}

QJsonArray AgricPracticesHandler::ProductListToJSonArray(const QStringList &prdList) {
    QJsonArray retArr;
    // we consider only products in the current season
    for (const QString &prd: prdList) {
        retArr.append(prd);
    }
    return retArr;
}

bool AgricPracticesHandler::IsScheduledJobRequest(const QJsonObject &parameters) {
    int jobVal;
    return ProcessorHandlerHelper::GetParameterValueAsInt(parameters, "scheduled_job", jobVal) && (jobVal == 1);
}

QString AgricPracticesHandler::GetProcessorDirValue(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                                    const QString &key, const QString &siteShortName, const QString &defVal ) {
    QString dataExtrDirName = ProcessorHandlerHelper::GetStringConfigValue(parameters, configParameters, key, L4C_AP_CFG_PREFIX);

    if (dataExtrDirName.size() == 0) {
        dataExtrDirName = defVal;
    }
    dataExtrDirName = dataExtrDirName.replace("{site}", siteShortName);

    return dataExtrDirName;

}

QString AgricPracticesHandler::GetDataExtractionDir(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                                    const ProductType &prdType, const QString &siteShortName ) {
    const QString &prdTypeShortName = GetShortNameForProductType(prdType).toLower();
    return GetProcessorDirValue(parameters, configParameters, QString(prdTypeShortName).append("_data_extr_dir"),
                                siteShortName,
                                "/mnt/archive/agric_practices_intermediate_files/{site}/data_extraction/" + prdTypeShortName);
}
