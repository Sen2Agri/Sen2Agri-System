#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "agricpracticeshandler.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"
#include "logger.hpp"
#include "s4c_utils.hpp"

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

#define L4C_AP_DEF_DATA_EXTR_ROOT   "/mnt/archive/agric_practices_files/{site}/{year}/data_extraction/"
#define L4C_AP_DEF_TS_ROOT          "/mnt/archive/agric_practices_files/{site}/{year}/ts_input_tables/"
#define L4C_AP_DEF_CFG_DIR          "/mnt/archive/agric_practices_files/{site}/{year}/config/"

#define SECS_TILL_EOD               86399   // 24 hour x 3600 + 59 minutes x 60 + 59 seconds

bool compareL4CProductDates(const Product& prod1,const Product& prod2)
{
    return (prod1.created < prod2.created);
}

void AgricPracticesHandler::CreateTasks(const AgricPracticesJobCfg &jobCfg, QList<TaskToSubmit> &outAllTasksList,
                                        const QStringList &ndviPrds, const QStringList &ampPrds, const QStringList &cohePrds)
{
    int curTaskIdx = 0;

    int minNdviDataExtrIndex = -1;
    int maxNdviDataExtrIndex = -1;
    int minAmpDataExtrIndex = -1;
    int maxAmpDataExtrIndex = -1;
    int minCoheDataExtrIndex = -1;
    int maxCoheDataExtrIndex = -1;

    if (IsOperationEnabled(jobCfg.execOper, dataExtraction)) {
        // create the data extraction tasks
        CreatePrdDataExtrTasks(jobCfg, outAllTasksList, GetDataExtractionTaskName(jobCfg, "ndvi-data-extraction"),
                               ndviPrds, {},
                               minNdviDataExtrIndex, maxNdviDataExtrIndex, curTaskIdx);
        CreatePrdDataExtrTasks(jobCfg, outAllTasksList, GetDataExtractionTaskName(jobCfg, "amp-data-extraction"),
                               ampPrds, {},
                               minAmpDataExtrIndex, maxAmpDataExtrIndex, curTaskIdx);
        CreatePrdDataExtrTasks(jobCfg, outAllTasksList, GetDataExtractionTaskName(jobCfg, "cohe-data-extraction"),
                               cohePrds, {},
                               minCoheDataExtrIndex, maxCoheDataExtrIndex, curTaskIdx);
    }
    if (IsOperationEnabled(jobCfg.execOper, catchCrop) || IsOperationEnabled(jobCfg.execOper, fallow) ||
        IsOperationEnabled(jobCfg.execOper, nfc) || IsOperationEnabled(jobCfg.execOper, harvestOnly)) {
        // create the merging tasks
        int ndviMergeTaskIdx = CreateMergeTasks(outAllTasksList, "ndvi-data-extraction-merge", minNdviDataExtrIndex, maxNdviDataExtrIndex, curTaskIdx);
        int ampMergeTaskIdx = CreateMergeTasks(outAllTasksList, "amp-data-extraction-merge", minAmpDataExtrIndex, maxAmpDataExtrIndex, curTaskIdx);
        int coheMergeTaskIdx = CreateMergeTasks(outAllTasksList, "cohe-data-extraction-merge", minCoheDataExtrIndex, maxCoheDataExtrIndex, curTaskIdx);

        int ccTsaIdx = CreateTSATasks(jobCfg, outAllTasksList, "CC", ndviMergeTaskIdx, ampMergeTaskIdx, coheMergeTaskIdx, curTaskIdx);
        int flTsaIdx = CreateTSATasks(jobCfg, outAllTasksList, "FL", ndviMergeTaskIdx, ampMergeTaskIdx, coheMergeTaskIdx, curTaskIdx);
        int nfcTsaIdx = CreateTSATasks(jobCfg, outAllTasksList, "NFC", ndviMergeTaskIdx, ampMergeTaskIdx, coheMergeTaskIdx, curTaskIdx);
        int naTsaIdx = CreateTSATasks(jobCfg, outAllTasksList, "NA", ndviMergeTaskIdx, ampMergeTaskIdx, coheMergeTaskIdx, curTaskIdx);

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

void AgricPracticesHandler::CreateSteps(QList<TaskToSubmit> &allTasksList,
                                        const AgricPracticesJobCfg &jobCfg,
                                        const QStringList &ndviPrds, const QStringList &ampPrds, const QStringList &cohePrds,
                                        NewStepList &steps)
{
    int curTaskIdx = 0;

    // if only data extraction is needed, then we create the filter ids step into the general configured directory
    QStringList ndviDataExtrDirs;
    QStringList ampDataExtrDirs;
    QStringList coheDataExtrDirs;
    if (IsOperationEnabled(jobCfg.execOper, dataExtraction)) {
        const QString &idsFileName = GetProcessorDirValue(jobCfg.parameters, jobCfg.configParameters, "filter_ids_path",
                                                          jobCfg.siteShortName, jobCfg.siteCfg.year,
                                                          QString(L4C_AP_DEF_TS_ROOT) + "FilterIds/Sen4CAP_L4C_FilterIds.csv");
        // create the tasks for the NDVI, AMP and COHE - they depend only on the ids extraction
        ndviDataExtrDirs = CreateStepsForDataExtraction(jobCfg, idsFileName, ProductType::L3BProductTypeId,
                                                        ndviPrds, allTasksList, steps, curTaskIdx);
        ampDataExtrDirs = CreateStepsForDataExtraction(jobCfg, idsFileName, ProductType::S4CS1L2AmpProductTypeId,
                                                       ampPrds, allTasksList, steps, curTaskIdx);
        coheDataExtrDirs = CreateStepsForDataExtraction(jobCfg, idsFileName, ProductType::S4CS1L2CoheProductTypeId,
                                                       cohePrds, allTasksList, steps, curTaskIdx);
    }
    if (ndviDataExtrDirs.size() == 0 || ampDataExtrDirs.size() == 0 || coheDataExtrDirs.size() == 0) {
        // if no DataExtraction is performed at all or only some files were data extracted,
        // we must use the directories configures in the database
        ndviDataExtrDirs.append(GetDataExtractionDir(jobCfg, ProductType::L3BProductTypeId));
        ampDataExtrDirs.append(GetDataExtractionDir(jobCfg, ProductType::S4CS1L2AmpProductTypeId));
        coheDataExtrDirs.append(GetDataExtractionDir(jobCfg, ProductType::S4CS1L2CoheProductTypeId));
    }

    if (IsOperationEnabled(jobCfg.execOper, catchCrop) || IsOperationEnabled(jobCfg.execOper, fallow) ||
        IsOperationEnabled(jobCfg.execOper, nfc) || IsOperationEnabled(jobCfg.execOper, harvestOnly)) {
        const QString &ndviMergedFile = CreateStepsForFilesMerge(jobCfg, ProductType::L3BProductTypeId, ndviDataExtrDirs, steps,
                                                                 allTasksList, curTaskIdx);
        const QString &ampMergedFile = CreateStepsForFilesMerge(jobCfg, ProductType::S4CS1L2AmpProductTypeId, ampDataExtrDirs, steps,
                                                                allTasksList, curTaskIdx);
        const QString &coheMergedFile = CreateStepsForFilesMerge(jobCfg, ProductType::S4CS1L2CoheProductTypeId, coheDataExtrDirs, steps,
                                                                 allTasksList, curTaskIdx);

        QStringList productFormatterFiles;
        productFormatterFiles += CreateTimeSeriesAnalysisSteps(jobCfg, "CC", ndviMergedFile, ampMergedFile, coheMergedFile, steps,
                                                               allTasksList, curTaskIdx);
        productFormatterFiles += CreateTimeSeriesAnalysisSteps(jobCfg, "FL", ndviMergedFile, ampMergedFile, coheMergedFile, steps,
                                                               allTasksList, curTaskIdx);
        productFormatterFiles += CreateTimeSeriesAnalysisSteps(jobCfg, "NFC", ndviMergedFile, ampMergedFile, coheMergedFile, steps,
                                                               allTasksList, curTaskIdx);
        productFormatterFiles += CreateTimeSeriesAnalysisSteps(jobCfg, "NA", ndviMergedFile, ampMergedFile, coheMergedFile, steps,
                                                               allTasksList, curTaskIdx);

        TaskToSubmit &productFormatterTask = allTasksList[curTaskIdx++];
        const QStringList &productFormatterArgs = GetProductFormatterArgs(productFormatterTask, jobCfg,
                                                                          productFormatterFiles);
        steps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));

        const auto & productFormatterPrdFileIdFile = productFormatterTask.GetFilePath("prd_infos.txt");
        TaskToSubmit &exportCsvToShpProductTask = allTasksList[curTaskIdx++];

        const QStringList &exportCsvToShpProductArgs = GetExportProductLauncherArgs(jobCfg, productFormatterPrdFileIdFile);
        steps.append(exportCsvToShpProductTask.CreateStep("export-product-launcher", exportCsvToShpProductArgs));
    }
}

void AgricPracticesHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                              const JobSubmittedEvent &evt)
{
    AgricPracticesJobCfg jobCfg(&ctx, evt);

    // Moved this from the GetProcessingDefinitionImpl function as it might be time consuming and scheduler will
    // throw exception if timeout exceeded
    JobSubmittedEvent event;
    // TODO: Here we should check if there are files that do not have the data extraction performed within the
    //      given interval if the requested operation is TimeSeriesAnalysis only
    //      In this case we should force a DataExtraction for the missing files
    UpdateJobSubmittedParamsFromSchedReq(jobCfg, event, jobCfg.isScheduledJob);

    QStringList ndviFiles, ampFiles, coheFiles;
    ExtractProductFiles(jobCfg, ndviFiles, ampFiles, coheFiles);

    if (!ValidateProductsForOperation(jobCfg, ndviFiles, ampFiles, coheFiles)) {
        return;
    }

    // update the season start and date values
    if (!jobCfg.seasonStartDate.isValid()) {
        ConfigurationParameterValueMap map;
        GetSeasonStartEndDates(ctx.GetSiteSeasons(event.siteId), jobCfg.seasonStartDate, jobCfg.seasonEndDate, jobCfg.prdMaxDate, map);
    }

    jobCfg.siteCfg.country = ProcessorHandlerHelper::GetStringConfigValue(jobCfg.parameters, jobCfg.configParameters,
                                                               "country", L4C_AP_CFG_PREFIX);
    jobCfg.siteCfg.tsaMinAcqsNo = ProcessorHandlerHelper::GetStringConfigValue(jobCfg.parameters, jobCfg.configParameters,
                                                                    "tsa_min_acqs_no", L4C_AP_CFG_PREFIX);
    if (jobCfg.siteCfg.year.size() == 0) {
        jobCfg.siteCfg.year = QString::number(ProcessorHandlerHelper::GuessYear(jobCfg.seasonStartDate, jobCfg.seasonEndDate));
    }
    jobCfg.siteCfg.practices = GetPracticeTableFiles(jobCfg.parameters, jobCfg.configParameters, jobCfg.siteShortName, jobCfg.siteCfg.year);

    if (!GetL4CConfigForSiteId(jobCfg)) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("Cannot find L4C configuration file for site %1 and year %2\n")
                    .arg(jobCfg.siteShortName)
                    .arg(jobCfg.siteCfg.year).toStdString());
    }

    QList<TaskToSubmit> allTasksList;
    CreateTasks(jobCfg, allTasksList, ndviFiles, ampFiles, coheFiles);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    SubmitTasks(ctx, event.jobId, allTasksListRef);
    NewStepList allSteps;
    CreateSteps(allTasksList, jobCfg, ndviFiles, ampFiles, coheFiles, allSteps);
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
    } else if ((event.module == "export-product-launcher") || (event.module == "ndvi-data-extraction-only") ||
               (event.module == "amp-data-extraction-only") || (event.module == "cohe-data-extraction-only")) {
        ctx.MarkJobFinished(event.jobId);
        // Now remove the job folder containing temporary files
        RemoveJobFolder(ctx, event.jobId, processorDescr.shortName);
    }
}

QStringList AgricPracticesHandler::GetExportProductLauncherArgs(const AgricPracticesJobCfg &jobCfg,
                                                                const QString &productFormatterPrdFileIdFile) {
    // Get the path for the ogr2ogr
    const auto &paths = jobCfg.pCtx->GetJobConfigurationParameters(jobCfg.event.jobId, QStringLiteral("executor.module.path.ogr2ogr"));
    QString ogr2OgrPath = "ogr2ogr";
    for (const auto &p : paths) {
        ogr2OgrPath = p.second;
        break;
    }

    const QStringList &exportCsvToShpProductArgs = { "-f", productFormatterPrdFileIdFile,
                                                      "-o", "Sen4CAP_L4C_PRACTICE_" + jobCfg.siteCfg.country + "_" +
                                                     jobCfg.siteCfg.year + ".gpkg",
                                                     "-g", ogr2OgrPath
                                                };
    return exportCsvToShpProductArgs;
}

QStringList AgricPracticesHandler::GetProductFormatterArgs(TaskToSubmit &productFormatterTask, const AgricPracticesJobCfg &jobCfg,
                                                           const QStringList &listFiles) {
    // ProductFormatter /home/cudroiu/sen2agri-processors-build
    //    -vectprd 1 -destroot /mnt/archive_new/test/Sen4CAP_L4C_Tests/NLD_Validation_TSA/OutPrdFormatter
    //    -fileclass OPER -level S4C_L4C -baseline 01.00 -siteid 4 -timeperiod 20180101_20181231 -processor generic
    //    -processor.generic.files <files_list>

    const auto &targetFolder = GetFinalProductFolder(*jobCfg.pCtx, jobCfg.event.jobId, jobCfg.event.siteId);
    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.txt");
    QString strTimePeriod = jobCfg.prdMinDate.toString("yyyyMMddTHHmmss").append("_").append(jobCfg.prdMaxDate.toString("yyyyMMddTHHmmss"));
    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot", targetFolder,
                                         "-fileclass", "OPER",
                                         "-level", "S4C_L4C",
                                         "-vectprd", "1",
                                         "-baseline", "01.00",
                                         "-siteid", QString::number(jobCfg.event.siteId),
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

    ConfigurationParameterValueMap mapCfg = ctx.GetConfigurationParameters(QString(L4C_AP_CFG_PREFIX),
                                                                           siteId, requestOverrideCfgValues);
    std::map<QString, QString> configParams;
    for (const auto &p : mapCfg) {
        configParams.emplace(p.key, p.value);
    }

    // If the execution conditions are not met, then exit
    QJsonObject parameters;
    const QString &siteShortName = ctx.GetSiteShortName(siteId);
    QString errMsg;

    int year = ProcessorHandlerHelper::GuessYear(seasonStartDate, seasonEndDate);
    if (!CheckExecutionPreconditions(parameters, configParams, siteShortName, QString::number(year), errMsg)) {
        Logger::error(errMsg);
        return params;
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

    return params;
}

bool AgricPracticesHandler::GetL4CConfigForSiteId(AgricPracticesJobCfg &jobCfg)
{
    const QString &siteCfgFilePath = GetL4CConfigFilePath(jobCfg);
    if (siteCfgFilePath == "") {
        return false;
    }

    return LoadL4CConfigFile(jobCfg, siteCfgFilePath);
}

QString AgricPracticesHandler::GetL4CConfigFilePath(AgricPracticesJobCfg &jobCfg)
{
    QString strCfgPath;
    const QString &strCfgDir = GetProcessorDirValue(jobCfg.parameters, jobCfg.configParameters,
                                                    "cfg_dir", jobCfg.siteShortName,
                                                    jobCfg.siteCfg.year, L4C_AP_DEF_CFG_DIR);
    QDir directory(strCfgDir);
    QString preferedCfgFileName = "S4C_L4C_Config_" + jobCfg.siteCfg.country + "_" + jobCfg.siteCfg.year + ".cfg";
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
                                                                                   "default_config_path", L4C_AP_CFG_PREFIX);
    }

    if (strCfgPath.isEmpty() || strCfgPath == "N/A" || !QFileInfo::exists(strCfgPath)) {
        return "";
    }
    return strCfgPath;
}

bool AgricPracticesHandler::LoadL4CConfigFile(AgricPracticesJobCfg &jobCfg,
                                               const QString &siteCfgFilePath)
{
    AgricPracticesSiteCfg &cfg = jobCfg.siteCfg;
    QString errMsg;
    if (!CheckExecutionPreconditions(jobCfg.parameters, jobCfg.configParameters, jobCfg.siteShortName, cfg.year, errMsg)) {
        throw std::runtime_error(errMsg.toStdString());
    }

    if (!GetLpisProductFiles(jobCfg)) {
        jobCfg.pCtx->MarkJobFailed(jobCfg.event.jobId);
        throw std::runtime_error(QStringLiteral("Cannot get the LPIS product files for site %1.").
                                 arg(QString::number(jobCfg.siteId)).toStdString());
    }

    // Now load the content of the file
    QSettings settings(siteCfgFilePath, QSettings::IniFormat);

    // Load the default practices
    const TQStrQStrMap &defTsaParams = LoadParamsFromFile(settings, "", "DEFAULT_TIME_SERIES_ANALYSIS_PARAMS", cfg);
    // Parameters used for practices tables extraction
    for (const QString &practice: cfg.practices.keys()) {
        TQStrQStrMap *pTsaParams;
        if (practice == "CC") {
            pTsaParams = &cfg.ccTsaParams;
        } else if (practice == "FL") {
            pTsaParams = &cfg.flTsaParams;
        } else if (practice == "NFC") {
            pTsaParams = &cfg.nfcTsaParams;
        } else if (practice == "NA") {
            pTsaParams = &cfg.naTsaParams;
        }

        (*pTsaParams) = LoadParamsFromFile(settings, practice, practice + "_TIME_SERIES_ANALYSIS_PARAMS", cfg);
        UpdatePracticesParams(defTsaParams, *pTsaParams);
    }

    jobCfg.siteCfg = cfg;
    return true;
}

void AgricPracticesHandler::ExtractProductFiles(AgricPracticesJobCfg &jobCfg,
                                                QStringList &ndviFiles, QStringList &ampFiles, QStringList &coheFiles)
{
    QDateTime prdMinDate;
    QDateTime prdMaxDate;

    ndviFiles.append(ExtractNdviFiles(jobCfg, prdMinDate, prdMaxDate));
    bool bHasCustomDataExtr = (!jobCfg.isScheduledJob && IsOperationEnabled(jobCfg.execOper, dataExtraction));
    if (bHasCustomDataExtr && (ndviFiles.size() == 0)) {
        jobCfg.pCtx->MarkJobFailed(jobCfg.event.jobId);
        throw std::runtime_error(
            QStringLiteral("No NDVI files provided for site %1 for ALL mode!\n").
                    arg(jobCfg.siteShortName).toStdString());
    }
    ampFiles.append(ExtractAmpFiles(jobCfg, prdMinDate, prdMaxDate));
    if  (bHasCustomDataExtr && (ampFiles.size() == 0)) {
        jobCfg.pCtx->MarkJobFailed(jobCfg.event.jobId);
        throw std::runtime_error(
            QStringLiteral("No Amplitude files provided for site %1 for ALL mode!\n").
                    arg(jobCfg.siteShortName).toStdString());
    }
    coheFiles.append(ExtractCoheFiles(jobCfg, prdMinDate, prdMaxDate));
    if  (bHasCustomDataExtr && (coheFiles.size() == 0)) {
        jobCfg.pCtx->MarkJobFailed(jobCfg.event.jobId);
        throw std::runtime_error(
            QStringLiteral("No Coherence files provided for site %1 for ALL mode!\n").
                    arg(jobCfg.siteShortName).toStdString());
    }
    // Update the dates used by the product formatter
    if (!jobCfg.prdMinDate.isValid()) {
        jobCfg.prdMinDate = prdMinDate;
    }
    if (!jobCfg.prdMaxDate.isValid()) {
        jobCfg.prdMaxDate = prdMaxDate;
    }
}

QStringList AgricPracticesHandler::ExtractNdviFiles(AgricPracticesJobCfg &jobCfg,
                                                    QDateTime &minDate, QDateTime &maxDate)
{
    return S4CUtils::GetInputProducts(*(jobCfg.pCtx), jobCfg.event, ProductType::L3BProductTypeId,
                                      minDate, maxDate, L4C_AP_CFG_PREFIX, jobCfg.isScheduledJob);
}

QStringList AgricPracticesHandler::ExtractAmpFiles(AgricPracticesJobCfg &jobCfg,
                                                   QDateTime &minDate, QDateTime &maxDate)
{
    return S4CUtils::GetInputProducts(*(jobCfg.pCtx), jobCfg.event, ProductType::S4CS1L2AmpProductTypeId,
                                      minDate, maxDate, L4C_AP_CFG_PREFIX, jobCfg.isScheduledJob);
}

QStringList AgricPracticesHandler::ExtractCoheFiles(AgricPracticesJobCfg &jobCfg,
                                                    QDateTime &minDate, QDateTime &maxDate)
{
    return S4CUtils::GetInputProducts(*(jobCfg.pCtx), jobCfg.event, ProductType::S4CS1L2CoheProductTypeId,
                                      minDate, maxDate, L4C_AP_CFG_PREFIX, jobCfg.isScheduledJob);
}


bool AgricPracticesHandler::ValidateProductsForOperation(const AgricPracticesJobCfg &jobCfg, const QStringList &ndviFiles,
                                                         const QStringList &ampFiles, const QStringList &coheFiles) {
    // Custom job with Data extraction enabled but it is also another operation following it
    if (!jobCfg.isScheduledJob && IsOperationEnabled(jobCfg.execOper, dataExtraction) && (jobCfg.execOper != dataExtraction)) {
        if (! ((ndviFiles.size() == 0) && (ampFiles.size() == 0) && (coheFiles.size() == 0)) &&
                ! ((ndviFiles.size() != 0) && (ampFiles.size() != 0) && (coheFiles.size() != 0)) ) {
            jobCfg.pCtx->MarkJobFailed(jobCfg.event.jobId);
            throw std::runtime_error(
                QStringLiteral("Custom jobs containing not only data extraction %1 must have either all inputs empty or all inputs provided!\n").
                        arg(jobCfg.siteShortName).toStdString());
            return false;
        }
    }
    if ((jobCfg.execOper == dataExtraction) && (ndviFiles.size() == 0) && (ampFiles.size() == 0) && (coheFiles.size() == 0)) {
        // No need to continue in this case as no tasks are needed
        if (jobCfg.isScheduledJob) {
            // no products available from the scheduling ... just mark job as done
            jobCfg.pCtx->MarkJobFinished(jobCfg.event.jobId);
            Logger::info(QStringLiteral("Scheduled job for data extraction with id %1 for site %2 marked as done as "
                                        "no products are available for now to process").
                         arg(jobCfg.event.jobId).arg(jobCfg.event.siteId));
            return false;
        } else {
            jobCfg.pCtx->MarkJobFailed(jobCfg.event.jobId);
            throw std::runtime_error(
                QStringLiteral("No NDVI, Amplitude or Coherence files provided for site %1 for data extraction mode!\n").
                        arg(jobCfg.siteShortName).toStdString());
            return false;
        }
    }
    return true;
}

QStringList AgricPracticesHandler::GetDataExtractionArgs(const AgricPracticesJobCfg &jobCfg, const QString &filterIds,
                                                         const ProductType &prdType, const QString &uidField,
                                                         const QStringList &inputFiles, const QString &outDir)
{
    QStringList retArgs = { "AgricPractDataExtraction", "-oldnf", "0", "-minmax", "0", "-csvcompact", "1", "-field", uidField,
                            "-prdtype", GetShortNameForProductType(prdType), "-filterids", filterIds, "-outdir", outDir, "-il" };
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
    QStringList retArgs = { "AgricPractMergeDataExtractionFiles", "-csvcompact", "1", "-sort", "1",
                            "-outformat", "csv", "-out", outFileName, "-il" };
    retArgs += listInputPaths;
    return retArgs;
}

QStringList AgricPracticesHandler::GetTimeSeriesAnalysisArgs(const AgricPracticesJobCfg &jobCfg, const QString &practice,
                                                             const QString &practicesFile, const QString &inNdviFile,
                                                             const QString &inAmpFile, const QString &inCoheFile,
                                                             const QString &outDir)
{
    const TQStrQStrMap *pTsaPracticeParams = 0;
    QString tsaExpectedPractice = GetTsaExpectedPractice(practice);
    if (practice == "CC") {
        pTsaPracticeParams = &(jobCfg.siteCfg.ccTsaParams);
    } else if (practice == "FL") {
        pTsaPracticeParams = &(jobCfg.siteCfg.flTsaParams);
    } else if (practice == "NFC") {
        pTsaPracticeParams = &(jobCfg.siteCfg.nfcTsaParams);
    } else if (practice == "NA") {
        pTsaPracticeParams = &(jobCfg.siteCfg.naTsaParams);
    }
    QStringList retArgs = { "TimeSeriesAnalysis", "-intype", "csv", "-debug", "0", "-allowgaps", "1", "-gapsfill", "0", "-plotgraph", "1",
                            "-rescontprd", "0", "-country", jobCfg.siteCfg.country, "-practice", tsaExpectedPractice, "-year", jobCfg.siteCfg.year,
                            "-harvestshp", practicesFile, "-diramp", inAmpFile, "-dircohe", inCoheFile, "-dirndvi", inNdviFile,
                            "-outdir", outDir };
    if (jobCfg.siteCfg.tsaMinAcqsNo.size() > 0) {
        retArgs += "-minacqs";
        retArgs += jobCfg.siteCfg.tsaMinAcqsNo;
    }
    for (TQStrQStrMap::const_iterator it=pTsaPracticeParams->begin(); it!=pTsaPracticeParams->end(); ++it) {
        if (it->second.size() > 0) {
            retArgs += ("-" + it->first);
            retArgs += it->second;
        }
    }
    QString prevL4CPrd;
    if (ProcessorHandlerHelper::GetBoolConfigValue(jobCfg.parameters, jobCfg.configParameters, "use_prev_prd", L4C_AP_CFG_PREFIX) &&
            GetPrevL4CProduct(jobCfg, jobCfg.seasonStartDate, jobCfg.prdMaxDate, prevL4CPrd)) {
        retArgs += "-prevprd";
        retArgs += prevL4CPrd;
    }
    return retArgs;
}

QString AgricPracticesHandler::BuildMergeResultFileName(const QString &country, const QString &year, const ProductType &prdsType)
{
    return QString(country).append("_").append(year).append("_").
            append(GetShortNameForProductType(prdsType)).append("_Extracted_Data.csv");
}

QString AgricPracticesHandler::BuildPracticesTableResultFileName(const QString &country, const QString &year, const QString &suffix)
{
    return QString("Sen4CAP_L4C_").append(suffix).append("_").append(country).
            append("_").append(year).append(".csv");
}

void AgricPracticesHandler::CreatePrdDataExtrTasks(const AgricPracticesJobCfg &jobCfg, QList<TaskToSubmit> &outAllTasksList,
                                        const QString &taskName,
                                        const QStringList &prdsList, const QList<std::reference_wrapper<const TaskToSubmit>> &dataExtParents,
                                        int &minPrdDataExtrIndex, int &maxPrdDataExtrIndex, int &curTaskIdx) {
    if (prdsList.size() == 0) {
        return;
    }
    // if the products per group is 0 or negative, then create a group with all products
    int prdsPerGroup = jobCfg.siteCfg.prdsPerGroup > 0 ? jobCfg.siteCfg.prdsPerGroup : prdsList.size();
    // if operation is exactly data extraction, then we the groups will have exactly 1 item
    if (jobCfg.execOper == dataExtraction || jobCfg.isScheduledJob) {
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

int AgricPracticesHandler::CreateTSATasks(const AgricPracticesJobCfg &jobCfg, QList<TaskToSubmit> &outAllTasksList,
                                          const QString &practiceName,  int ndviMergeTaskIdx, int ampMergeTaskIdx,
                                          int coheMergeTaskIdx, int &curTaskIdx) {
    int ccTsaIdx = -1;
    AgricPractOperation expectedOper = GetExecutionOperation(practiceName);
    if (jobCfg.siteCfg.practices.keys().contains(practiceName) && IsOperationEnabled(jobCfg.execOper, expectedOper)) {
        // this task is independent and can be executed before any others
        const QString &lowerPracticeName = practiceName.toLower();
        outAllTasksList.append(TaskToSubmit{ lowerPracticeName + "-time-series-analysis",
                                             {outAllTasksList[ndviMergeTaskIdx],
                                              outAllTasksList[ampMergeTaskIdx],
                                              outAllTasksList[coheMergeTaskIdx]} });
        ccTsaIdx = curTaskIdx++;
    }
    return ccTsaIdx;
}

QStringList AgricPracticesHandler::CreateStepsForDataExtraction(const AgricPracticesJobCfg &jobCfg, const QString &filterIds,
                                                                const ProductType &prdType, const QStringList &prds,
                                                                QList<TaskToSubmit> &allTasksList,
                                                                NewStepList &steps, int &curTaskIdx)
{
    QStringList dataExtrDirs;
    if (prds.size() == 0) {
        return dataExtrDirs;
    }
    // if the products per group is 0 or negative, then create a group with all products
    int prdsPerGroup = jobCfg.siteCfg.prdsPerGroup > 0 ? jobCfg.siteCfg.prdsPerGroup : prds.size();
    if (jobCfg.execOper == dataExtraction || jobCfg.isScheduledJob) {
        prdsPerGroup = 1;
    }
    int groups = (prds.size()/prdsPerGroup) + ((prds.size()%prdsPerGroup) == 0 ? 0 : 1);

    // create the tasks for the NDVI, AMP and COHE - they depend only on the ids extraction
    for(int i  = 0; i<groups; i++) {
        TaskToSubmit &dataExtractionTask = allTasksList[curTaskIdx++];
        QString dataExtrDirName;
        if (jobCfg.execOper == all && !jobCfg.isScheduledJob) {
            dataExtrDirName = dataExtractionTask.GetFilePath("");
        } else {
            dataExtrDirName = GetDataExtractionDir(jobCfg, prdType);
        }
        if (!dataExtrDirs.contains(dataExtrDirName)) {
            QDir().mkpath(dataExtrDirName);
            dataExtrDirs.append(dataExtrDirName);
        }

        // Extract the list of products from the current group
        int grpStartIdx = i * prdsPerGroup;
        int nextGrpStartIdx = (i+1) * prdsPerGroup;
        QStringList sublist;
        for (int j = grpStartIdx; j < nextGrpStartIdx && j < prds.size(); j++) {
            sublist.append(prds.at(j));
        }

        const QStringList &dataExtractionArgs = GetDataExtractionArgs(jobCfg, filterIds, prdType, "NewID", sublist, dataExtrDirName);
        steps.append(dataExtractionTask.CreateStep("AgricPractDataExtraction", dataExtractionArgs));
    }
    return dataExtrDirs;
}

QString AgricPracticesHandler::CreateStepsForFilesMerge(const AgricPracticesJobCfg &jobCfg, const ProductType &prdType,
                              const QStringList &dataExtrDirs, NewStepList &steps,
                              QList<TaskToSubmit> &allTasksList, int &curTaskIdx) {
    TaskToSubmit &mergeTask = allTasksList[curTaskIdx++];
    const QString &mergedFile = mergeTask.GetFilePath(BuildMergeResultFileName(jobCfg.siteCfg.country, jobCfg.siteCfg.year, prdType));
    const QStringList &mergeArgs = GetFilesMergeArgs(dataExtrDirs, mergedFile);
    steps.append(mergeTask.CreateStep("AgricPractMergeDataExtractionFiles", mergeArgs));

    return mergedFile;
}

QStringList AgricPracticesHandler::CreateTimeSeriesAnalysisSteps(const AgricPracticesJobCfg &jobCfg, const QString &practice,
                                                                 const QString &ndviMergedFile, const QString &ampMergedFile,
                                                                 const QString &coheMergedFile, NewStepList &steps,
                                                                 QList<TaskToSubmit> &allTasksList, int &curTaskIdx)
{
    QStringList retList;
    AgricPractOperation oper = GetExecutionOperation(practice);
    if (jobCfg.siteCfg.practices.keys().contains(practice) && ((oper & jobCfg.execOper) != none)) {
        //const QString &practicesFile = CreateStepForLPISSelection(practice, jobCfg, allTasksList, steps, curTaskIdx);

        TaskToSubmit &timeSeriesAnalysisTask = allTasksList[curTaskIdx++];
        const QString &timeSeriesExtrDir = timeSeriesAnalysisTask.GetFilePath("");
        const QStringList &timeSeriesAnalysisArgs = GetTimeSeriesAnalysisArgs(jobCfg, practice,
                                                                              jobCfg.siteCfg.practices.value(practice),
                                                                              ndviMergedFile, ampMergedFile, coheMergedFile,
                                                                              timeSeriesExtrDir);
        steps.append(timeSeriesAnalysisTask.CreateStep("TimeSeriesAnalysis", timeSeriesAnalysisArgs));

        // Add the expected files to the productFormatterFiles
        const QString &tsaExpPractice = GetTsaExpectedPractice(practice);
        const QString &filesPrefix = "Sen4CAP_L4C_" + tsaExpPractice + "_" + jobCfg.siteCfg.country + "_" + jobCfg.siteCfg.year;
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
    QJsonObject parameters;
    auto configParameters = ctx.GetConfigurationParameters(L4C_AP_CFG_PREFIX, prd.siteId);
    const QString &siteShortName = ctx.GetSiteShortName(prd.siteId);
    QString errMsg;
    if (!CheckExecutionPreconditions(parameters, configParameters, siteShortName,
                                     QString::number(prd.created.date().year()),
                                     errMsg)) {
        return;
    }

    // Check that the product type is NDVI, AMP or COHE
    const QString &prdTypeShortName = GetShortNameForProductType(prd.productTypeId);
    const QString &prdKey = "input_" + prdTypeShortName;
    if (prdKey == "input_") {
        Logger::error(QStringLiteral("Unsupported product type %1.").arg(QString::number((int)prd.productTypeId)));
        return;
    }
    // check if the NRT data extraction is configured for the site
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

bool AgricPracticesHandler::GetPrevL4CProduct(const AgricPracticesJobCfg &jobCfg,  const QDateTime &seasonStart,
                                              const QDateTime &curDate, QString &prevL4cProd) {
    ProductList l4cPrds = jobCfg.pCtx->GetProducts(jobCfg.siteId, (int)ProductType::S4CL4CProductTypeId, seasonStart, curDate);
    ProductList l4cPrdsFiltered;
    for (const Product &prd: l4cPrds) {
        if (prd.created.addDays(3) > curDate) {
            continue;
        }
        l4cPrdsFiltered.append(prd);
    }
    if (l4cPrdsFiltered.size() > 0) {
        qSort(l4cPrdsFiltered.begin(), l4cPrdsFiltered.end(), compareL4CProductDates);
        // remove products that have the same day as the current one
        prevL4cProd = l4cPrdsFiltered[l4cPrdsFiltered.size()-1].fullPath;
        return true;
    }
    return false;
}

bool AgricPracticesHandler::GetLpisProductFiles(AgricPracticesJobCfg &jobCfg) {

    jobCfg.siteCfg.ndviIdsGeomShapePath = "";
    jobCfg.siteCfg.ampCoheIdsGeomShapePath = "";
    jobCfg.siteCfg.fullDeclsFilePath = "";
    // We take it the last LPIS product for this site.
    QDate  startDate, endDate;
    startDate.setDate(1970, 1, 1);
    QDateTime startDateTime(startDate);
    endDate.setDate(2050, 12, 31);
    QDateTime endDateTime(endDate);
    const ProductList &lpisPrds = jobCfg.pCtx->GetProducts(jobCfg.siteId, (int)ProductType::S4CLPISProductTypeId, startDateTime, endDateTime);
    if (lpisPrds.size() == 0) {
        jobCfg.pCtx->MarkJobFailed(jobCfg.event.jobId);
        throw std::runtime_error(QStringLiteral("No LPIS product found in database for the agricultural practices execution for site %1.").
                                 arg(QString::number(jobCfg.siteId)).toStdString());
    }

    // If the year is >= 2019, then use LAEA for AMP and COHE and no matter which other for NDVI
    const QString &prdLpisPath = lpisPrds[lpisPrds.size()-1].fullPath;
    QDir directory(prdLpisPath);
    QString allFieldsName = "decl_" + jobCfg.siteShortName + "_" + jobCfg.siteCfg.year + ".csv";

    const QStringList &dirFiles = directory.entryList(QStringList() << "*.shp" << "*.csv",QDir::Files);
    foreach(const QString &fileName, dirFiles) {
        // we don't want for NDVI the LAEA projection
        if (fileName.endsWith("_buf_5m.shp") && (!fileName.endsWith("_3035_buf_5m.shp")) && (jobCfg.siteCfg.ndviIdsGeomShapePath.size() == 0)) {
            jobCfg.siteCfg.ndviIdsGeomShapePath = directory.filePath(fileName);
        }
        // LAEA projection have priority for 10m buffer
        if (fileName.endsWith("_3035_buf_10m.shp") ||
                (fileName.endsWith("_buf_10m.shp") && jobCfg.siteCfg.ampCoheIdsGeomShapePath.size() == 0)) {
            jobCfg.siteCfg.ampCoheIdsGeomShapePath = directory.filePath(fileName);
        }
        // we know its name but we want to be sure it is there
        if (fileName == allFieldsName) {
            jobCfg.siteCfg.fullDeclsFilePath = directory.filePath(fileName);
        }
    }

    Logger::info(QStringLiteral("Agricultural Practices Scheduled using for site %1 LPIS from = %2, with files: NDVI = %3, "
                                "AMP_COHE = %4, DECLS = %5 (expected %6)").
                 arg(jobCfg.event.siteId).arg(prdLpisPath).arg(jobCfg.siteCfg.ndviIdsGeomShapePath)
                 .arg(jobCfg.siteCfg.ampCoheIdsGeomShapePath).arg(jobCfg.siteCfg.fullDeclsFilePath)
                 .arg(allFieldsName));


    return (jobCfg.siteCfg.ndviIdsGeomShapePath.size() != 0 &&
            jobCfg.siteCfg.ampCoheIdsGeomShapePath.size() != 0 &&
            jobCfg.siteCfg.fullDeclsFilePath.size() != 0);
}

QString AgricPracticesHandler::GetShortNameForProductType(const ProductType &prdType) {
    switch(prdType) {
        case ProductType::L3BProductTypeId:         return "NDVI";
        case ProductType::S4CS1L2AmpProductTypeId:  return "AMP";
        case ProductType::S4CS1L2CoheProductTypeId: return "COHE";
        default:                                    return "";
    }
}

int AgricPracticesHandler::UpdateJobSubmittedParamsFromSchedReq(AgricPracticesJobCfg &jobCfg, JobSubmittedEvent &newEvent, bool &isSchedJob) {
    // initialize the new event
    newEvent = jobCfg.event;
    isSchedJob = false;

    QString strStartDate, strEndDate;
    if(IsScheduledJobRequest(jobCfg.parameters) &&
            ProcessorHandlerHelper::GetParameterValueAsString(jobCfg.parameters, "start_date", strStartDate) &&
            ProcessorHandlerHelper::GetParameterValueAsString(jobCfg.parameters, "end_date", strEndDate) &&
            jobCfg.parameters.contains("input_products") && jobCfg.parameters["input_products"].toArray().size() == 0) {

        isSchedJob = true;
        // If is scheduled job and we have only analysis operation, then force to perform the data extraction
        // for the missing products
        if (!IsOperationEnabled(jobCfg.execOper, dataExtraction)) {
            jobCfg.execOper = (AgricPractOperation)(jobCfg.execOper | dataExtraction);
        }

        QString strSeasonStartDate, strSeasonEndDate;
        ProcessorHandlerHelper::GetParameterValueAsString(jobCfg.parameters, "season_start_date", strSeasonStartDate);
        ProcessorHandlerHelper::GetParameterValueAsString(jobCfg.parameters, "season_end_date", strSeasonEndDate);

        // TODO: Should we use here ProcessorHandlerHelper::GetLocalDateTime ???

        jobCfg.seasonStartDate = QDateTime::fromString(strSeasonStartDate, "yyyyMMdd");
        jobCfg.seasonEndDate = QDateTime::fromString(strSeasonEndDate, "yyyyMMdd").addSecs(SECS_TILL_EOD);

        if (jobCfg.siteCfg.year.size() == 0) {
            jobCfg.siteCfg.year = QString::number(ProcessorHandlerHelper::GuessYear(jobCfg.seasonStartDate, jobCfg.seasonEndDate));
        }

        const auto &startDate = QDateTime::fromString(strStartDate, "yyyyMMdd");
        const auto &endDate = QDateTime::fromString(strEndDate, "yyyyMMdd").addSecs(SECS_TILL_EOD);

        // set these values by default
        jobCfg.prdMinDate = startDate;
        jobCfg.prdMaxDate = endDate;

        Logger::info(QStringLiteral("Agricultural Practices Scheduled job received for siteId = %1, startDate=%2, endDate=%3").
                     arg(jobCfg.event.siteId).arg(startDate.toString("yyyyMMdd")).arg(endDate.toString("yyyyMMdd")));

        QStringList ndviProcessedFiles, ampProcessedFiles, coheProcessedFiles;
        const QStringList &ndviMissingPrdsList = ExtractMissingDataExtractionProducts(jobCfg, ProductType::L3BProductTypeId,
                                                                      startDate, endDate, ndviProcessedFiles);
        const QStringList &ndviFilteredPrdsList = FilterAndUpdateAlreadyProcessingPrds(jobCfg, ndviMissingPrdsList,
                                                                                       ndviProcessedFiles, ProductType::L3BProductTypeId);

        const QStringList &ampMissingPrdsList = ExtractMissingDataExtractionProducts(jobCfg, ProductType::S4CS1L2AmpProductTypeId,
                                                                       startDate, endDate, ampProcessedFiles);
        const QStringList &ampFilteredPrdsList = FilterAndUpdateAlreadyProcessingPrds(jobCfg, ampMissingPrdsList,
                                                                                      coheProcessedFiles, ProductType::S4CS1L2AmpProductTypeId);

        const QStringList &coheMissingPrdsList = ExtractMissingDataExtractionProducts(jobCfg, ProductType::S4CS1L2CoheProductTypeId,
                                                                       startDate, endDate, coheProcessedFiles);
        const QStringList &coheFilteredPrdsList = FilterAndUpdateAlreadyProcessingPrds(jobCfg, coheMissingPrdsList,
                                                                                       coheProcessedFiles, ProductType::S4CS1L2CoheProductTypeId);

        const QJsonArray &ndviInputProductsArr = ProductListToJSonArray(ndviFilteredPrdsList);
        const QJsonArray &ampInputProductsArr = ProductListToJSonArray(ampFilteredPrdsList);
        const QJsonArray &coheInputProductsArr = ProductListToJSonArray(coheFilteredPrdsList);
        Logger::info(QStringLiteral("Agricultural Practices Scheduled job : Updating input products for jobId = %1, siteId = %2 with a "
                                    "number of %3 NDVI products, %4 AMP products and %5 COHE products").
                     arg(jobCfg.event.jobId).arg(jobCfg.siteId).arg(ndviInputProductsArr.size()).
                     arg(ampInputProductsArr.size()).arg(coheInputProductsArr.size()));

        if (ndviInputProductsArr.size() > 0) {
            jobCfg.parameters[QStringLiteral("input_NDVI")] = ndviInputProductsArr;
        }
        if (ampInputProductsArr.size() > 0) {
            jobCfg.parameters[QStringLiteral("input_AMP")] = ampInputProductsArr;
        }
        if (coheInputProductsArr.size() > 0) {
            jobCfg.parameters[QStringLiteral("input_COHE")] = coheInputProductsArr;
        }

        newEvent.parametersJson = jsonToString(jobCfg.parameters);
        jobCfg.event = newEvent;
        return ndviInputProductsArr.size() + ampInputProductsArr.size() + coheInputProductsArr.size();
    }
    return -1;
}

QStringList AgricPracticesHandler::ExtractMissingDataExtractionProducts(AgricPracticesJobCfg &jobCfg,
                                                                        const ProductType &prdType, const QDateTime &startDate,
                                                                        const QDateTime &endDate, QStringList &alreadyProcessedFiles) {
    const QString &dataExtrDirName = GetDataExtractionDir(jobCfg, prdType);

    const ProductList &prds = jobCfg.pCtx->GetProducts(jobCfg.siteId, (int)prdType, startDate, endDate);
    QStringList retList;
    for(const Product &prd: prds) {
        QStringList prdFileNames {prd.fullPath};
        if (prdType == ProductType::L3BProductTypeId) {
            prdFileNames = S4CUtils::FindL3BProductTiffFiles(prd.fullPath, {}, "SNDVI");
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
    const QStringList &cvsFiles = directory.entryList(QStringList() << fileNameNoExt + ".csv",QDir::Files);
    if (cvsFiles.size() > 0) {
        return true;
    }

    return false;
}

/**
 * @brief AgricPracticesHandler::FilterAndUpdateAlreadyProcessingPrds
 *  Returns the products that were not already launched for processing in case of overlapping schedulings
 */
QStringList AgricPracticesHandler::FilterAndUpdateAlreadyProcessingPrds(AgricPracticesJobCfg &jobCfg, const QStringList &missingPrdsFiles,
                                                                        const QStringList &processedPrdsFiles, const ProductType &prdType) {
    QStringList filteredPrds;

    QString fileParentPath = GetProcessorDirValue(jobCfg.parameters, jobCfg.configParameters, "data_extr_dir",
                                jobCfg.siteShortName, jobCfg.siteCfg.year, L4C_AP_DEF_DATA_EXTR_ROOT);
    // we can put it in the same folder with the products but is better to put them all in another location,
    // not taking into account the product type (is easier to maintain)
    fileParentPath = fileParentPath.replace("{product_type}", "");
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
                                                    const QString &key, const QString &siteShortName, const QString &year,
                                                    const QString &defVal ) {
    QString dataExtrDirName = ProcessorHandlerHelper::GetStringConfigValue(parameters, configParameters, key, L4C_AP_CFG_PREFIX);

    if (dataExtrDirName.size() == 0) {
        dataExtrDirName = defVal;
    }
    dataExtrDirName = dataExtrDirName.replace("{site}", siteShortName);
    dataExtrDirName = dataExtrDirName.replace("{year}", year);
    dataExtrDirName = dataExtrDirName.replace("{processor}", processorDescr.shortName);

    return dataExtrDirName;

}

QString AgricPracticesHandler::GetDataExtractionDir(const AgricPracticesJobCfg &jobCfg, const ProductType &prdType) {
    const QString &prdTypeShortName = GetShortNameForProductType(prdType).toLower();
    QString val = GetProcessorDirValue(jobCfg.parameters, jobCfg.configParameters, "data_extr_dir",
                                jobCfg.siteShortName, jobCfg.siteCfg.year,
                                L4C_AP_DEF_DATA_EXTR_ROOT + prdTypeShortName);
    if (val.indexOf("{product_type}") == -1) {
        // force the product type to be added at the end of the directory
        return QDir(val).filePath(prdTypeShortName);
    }
    return val.replace("{product_type}", prdTypeShortName);
}

QString AgricPracticesHandler::GetTsInputTablesDir(const QJsonObject &parameters, const std::map<QString,
                                                   QString> &configParameters, const QString &siteShortName,
                                                   const QString &year, const QString &practice) {

    // we expect the value to be something like /mnt/archive/agric_practices_files/{site}/{year}/ts_input_tables/{practice}/
    QString val = GetProcessorDirValue(parameters, configParameters, "ts_input_tables_dir", siteShortName, year,
                                       L4C_AP_DEF_TS_ROOT + practice);
    if (val.indexOf("{practice}") == -1) {
        // force the practice to be added at the end of the directory
        return QDir(val).filePath(practice);
    }
    return val.replace("{practice}", practice);
}

QMap<QString, QString> AgricPracticesHandler::GetPracticeTableFiles(const QJsonObject &parameters,
                                                                 const std::map<QString, QString> &configParameters,
                                                                 const QString &siteShortName, const QString &year) {
    QMap<QString, QString> retMap;
    const QStringList &practices = ProcessorHandlerHelper::GetStringConfigValue(parameters,
                                    configParameters, "practices", L4C_AP_CFG_PREFIX).split(",");
    for (const QString &strPractice: practices) {
        const QString &strTrimmedPractice = strPractice.trimmed();
        if (strTrimmedPractice != "CC" && strTrimmedPractice != "FL" && strTrimmedPractice != "NFC" && strTrimmedPractice != "NA") {
            // ignore unknow practice names
            Logger::warn(QStringLiteral("Unknown practice name %1 configured for site %2. Just ignoring it ...").
                         arg(strTrimmedPractice).arg(siteShortName));
            continue;
        }
        const QString &tsInputTablesDir = GetTsInputTablesDir(parameters, configParameters, siteShortName, year, strTrimmedPractice);
        const QString &country = ProcessorHandlerHelper::GetStringConfigValue(parameters,
                                        configParameters, "country", L4C_AP_CFG_PREFIX);

        const QString &fileName = BuildPracticesTableResultFileName(country, year, strTrimmedPractice);
        const QString &practiceFilePath = QDir(tsInputTablesDir).filePath(fileName);
        if(QFileInfo(practiceFilePath).exists()) {
            retMap.insert(strTrimmedPractice, practiceFilePath);
        } else {
            // Just put an empty string and let the caller decide what to do
            retMap.insert(strTrimmedPractice, "");
        }
    }

    return retMap;
}

bool AgricPracticesHandler::CheckExecutionPreconditions(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                                        const QString &siteShortName, const QString &year, QString &errMsg) {
    errMsg = "";
    const QMap<QString, QString> &retMap = GetPracticeTableFiles(parameters, configParameters, siteShortName, year);
    for(const QString & practice : retMap.keys()) {
        // if it is not NA and a file does not exist, then return false
        // We allow the NA to have no files as this is activated by default in the system
        if (retMap.value(practice).size() == 0 && practice != "NA") {
            errMsg = QStringLiteral("Error checking S4C_L4C preconditions for site %1. "
                                    "The practice %2 does not have a configured practices file for year %3!.")
                    .arg(siteShortName).arg(practice).arg(year);
            return false;
        }
    }
    return true;
}

QString AgricPracticesHandler::GetDataExtractionTaskName(const AgricPracticesJobCfg &jobCfg, const QString &taskName) {
    if (jobCfg.execOper == dataExtraction) {
        return taskName + "-only";
    }
    return taskName;
}

// ######################## THESE FUNCTIONS ARE NOT USED ANYMORE ################################
// TODO: This is the version of getting the parameters from the database instead of the file
bool AgricPracticesHandler::GetSiteConfigForSiteId2(AgricPracticesJobCfg &jobCfg)
{
    // TODO: We could try extracting the country and the year from the site name???

    jobCfg.siteCfg.country = ProcessorHandlerHelper::GetStringConfigValue(jobCfg.parameters, jobCfg.configParameters, "country", L4C_AP_CFG_PREFIX);
    jobCfg.siteCfg.year = ProcessorHandlerHelper::GetStringConfigValue(jobCfg.parameters, jobCfg.configParameters, "year", L4C_AP_CFG_PREFIX);

    if (!GetLpisProductFiles(jobCfg)) {
        Logger::error(QStringLiteral("Cannot get the LPIS product files for site %1.")
                      .arg(QString::number(jobCfg.siteId)));
        return false;
    }

    jobCfg.siteCfg.practices = GetPracticeTableFiles(jobCfg.parameters, jobCfg.configParameters,
                                                     jobCfg.siteShortName, jobCfg.siteCfg.year);

    // validate practices
    TQStrQStrMap *pTsaParams;
    TQStrQStrMap tsaCfgParams;

    QString practicePrefix;
    QString tsaPrefix;

    for (const auto &practice: jobCfg.siteCfg.practices.keys()) {
        if (practice != "CC" && practice != "FL" && practice != "NFC" && practice != "NA") {
            jobCfg.pCtx->MarkJobFailed(jobCfg.event.jobId);
            throw std::runtime_error(QStringLiteral("Unsupported practice called %1.").arg(practice).toStdString());
        }
        if (practice == "CC") {
            pTsaParams = &jobCfg.siteCfg.ccTsaParams;
            practicePrefix = L4C_AP_GEN_CC_CFG_PREFIX;
            tsaPrefix = L4C_AP_TSA_CC_CFG_PREFIX;
        } else if (practice == "FL") {
            pTsaParams = &jobCfg.siteCfg.flTsaParams;
            practicePrefix = L4C_AP_GEN_FL_CFG_PREFIX;
            tsaPrefix = L4C_AP_TSA_FL_CFG_PREFIX;
        } else if (practice == "NFC") {
            pTsaParams = &jobCfg.siteCfg.nfcTsaParams;
            practicePrefix = L4C_AP_GEN_NFC_CFG_PREFIX;
            tsaPrefix = L4C_AP_TSA_NFC_CFG_PREFIX;
        } else if (practice == "NA") {
            pTsaParams = &jobCfg.siteCfg.naTsaParams;
            practicePrefix = L4C_AP_GEN_NA_CFG_PREFIX;
            tsaPrefix = L4C_AP_TSA_NA_CFG_PREFIX;
        }
        tsaCfgParams = ProcessorHandlerHelper::FilterConfigParameters(jobCfg.configParameters, tsaPrefix);

        UpdatePracticesParams(jobCfg.parameters, jobCfg.configParameters, tsaCfgParams, tsaPrefix, pTsaParams);
    }

    return true;
}

/*
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

QStringList AgricPracticesHandler::GetIdsExtractorArgs(const AgricPracticesJobCfg &jobCfg, const QString &outFile,
                                                       const QString &finalTargetDir)
{
    QStringList retArgs = { "LPISDataSelection", "-inshp", jobCfg.siteCfg.fullDeclsFilePath, "-country", jobCfg.siteCfg.country,
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

QString AgricPracticesHandler::CreateStepForLPISSelection(const QString &practice, const AgricPracticesJobCfg &jobCfg,
                                                          QList<TaskToSubmit> &allTasksList, NewStepList &steps, int &curTaskIdx) {
    QString tsInputTablesDir ;
    TaskToSubmit &lpisSelectionTask = allTasksList[curTaskIdx++];
    QString fileName;
    if (practice.size() == 0) {
        fileName = BuildPracticesTableResultFileName(jobCfg.siteCfg.country, jobCfg.siteCfg.year, "FilterIds");
    } else {
        fileName = BuildPracticesTableResultFileName(jobCfg.siteCfg.country, jobCfg.siteCfg.year, practice);
    }

    const QString &tmpLpisSelFilePath = lpisSelectionTask.GetFilePath(fileName);
    QString lpisSelFilePath = tmpLpisSelFilePath;
    if (jobCfg.execOper == dataExtraction || jobCfg.isScheduledJob) {
        tsInputTablesDir = GetTsInputTablesDir(jobCfg.parameters, jobCfg.configParameters, jobCfg.siteShortName, practice);
//        // get the directory where the ids file should be finally moved
//        tsInputTablesDir = GetProcessorDirValue(jobCfg.parameters, jobCfg.configParameters, "ts_input_tables_dir", jobCfg.siteShortName,
//                             "/mnt/archive/agric_practices_files/{site}/input_files");
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

    QStringList retArgs = { "LPISDataSelection", "-inshp", jobCfg.siteCfg.fullDeclsFilePath, "-country", jobCfg.siteCfg.country,
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
*/

// ###################### AgricPracticesJobCfg functions ############################

AgricPractOperation AgricPracticesJobCfg::GetExecutionOperation(const QJsonObject &parameters, const std::map<QString, QString> &configParameters)
{
    const QString &execOper = ProcessorHandlerHelper::GetStringConfigValue(parameters, configParameters, "execution_operation", L4C_AP_CFG_PREFIX);
    return AgricPracticesHandler::GetExecutionOperation(execOper);
}

