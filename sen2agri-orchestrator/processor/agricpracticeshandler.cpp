#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "agricpracticeshandler.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"
#include "logger.hpp"

#define NDVI_PRD_NAME_REGEX R"(S2AGRI_L3B_SNDVI_A(\d{8}T\d{6})_.+\.TIF)"
#define S1_L2A_PRD_NAME_REGEX   R"(SEN4CAP_L2A_.+_V(\d{8}T\d{6})_(\d{8}T\d{6})_.+\.tif)"

#define L4C_AP_CFG_PREFIX   "processor.s4c_l4c."

void AgricPracticesHandler::CreateTasks(const AgricPracticesSiteCfg &siteCfg, QList<TaskToSubmit> &outAllTasksList,
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
                               operation, minNdviDataExtrIndex, maxNdviDataExtrIndex, curTaskIdx);
        CreatePrdDataExtrTasks(siteCfg, outAllTasksList, "cohe-data-extraction", cohePrds, {outAllTasksList[idsFilterExtrIdx]},
                               operation, minNdviDataExtrIndex, maxNdviDataExtrIndex, curTaskIdx);
    }
    if (operation == all || operation == catchCrop || operation == fallow ||
        operation == nfc || operation == harvestOnly) {
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
    }
}

void AgricPracticesHandler::CreateSteps(EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event, QList<TaskToSubmit> &allTasksList,
                                        const AgricPracticesSiteCfg &siteCfg,
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
        idsFileName = CreateStepForLPISSelection(parameters, configParameters, operation, "", siteCfg,
                                                        allTasksList, steps, curTaskIdx);

        // create the tasks for the NDVI, AMP and COHE - they depend only on the ids extraction
        ndviDataExtrDirs = CreateStepsForDataExtraction(parameters, configParameters, operation, siteCfg, "NDVI",
                                                        ndviPrds, idsFileName, allTasksList, steps, curTaskIdx);
        ampDataExtrDirs = CreateStepsForDataExtraction(parameters, configParameters, operation, siteCfg, "AMP",
                                                       ampPrds, idsFileName, allTasksList, steps, curTaskIdx);
        coheDataExtrDirs = CreateStepsForDataExtraction(parameters, configParameters, operation, siteCfg, "COHE",
                                                       cohePrds, idsFileName, allTasksList, steps, curTaskIdx);
    }

    if (operation == all || operation == catchCrop || operation == fallow ||
            operation == nfc || operation == harvestOnly) {
        const QString &ndviMergedFile = CreateStepsForFilesMerge(siteCfg, "NDVI", ndviDataExtrDirs, steps, allTasksList, curTaskIdx);
        const QString &ampMergedFile = CreateStepsForFilesMerge(siteCfg, "AMP", ampDataExtrDirs, steps, allTasksList, curTaskIdx);
        const QString &coheMergedFile = CreateStepsForFilesMerge(siteCfg, "COHE", coheDataExtrDirs, steps, allTasksList, curTaskIdx);

        QStringList productFormatterFiles;
        productFormatterFiles += CreateTimeSeriesAnalysisSteps(parameters, configParameters, operation, siteCfg, "CC",
                                                               ndviMergedFile, ampMergedFile, coheMergedFile, steps,
                                                               allTasksList, curTaskIdx, operation);
        productFormatterFiles += CreateTimeSeriesAnalysisSteps(parameters, configParameters, operation, siteCfg, "FL",
                                                               ndviMergedFile, ampMergedFile, coheMergedFile, steps,
                                                               allTasksList, curTaskIdx, operation);
        productFormatterFiles += CreateTimeSeriesAnalysisSteps(parameters, configParameters, operation, siteCfg, "NFC",
                                                               ndviMergedFile, ampMergedFile, coheMergedFile, steps,
                                                               allTasksList, curTaskIdx, operation);
        productFormatterFiles += CreateTimeSeriesAnalysisSteps(parameters, configParameters, operation, siteCfg, "NA",
                                                               ndviMergedFile, ampMergedFile, coheMergedFile, steps,
                                                               allTasksList, curTaskIdx, operation);

        TaskToSubmit &productFormatterTask = allTasksList[curTaskIdx++];
        const QStringList &productFormatterArgs = GetProductFormatterArgs(productFormatterTask, ctx, event,
                                                                          productFormatterFiles, minDate, maxDate);
        steps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
    }
}

void AgricPracticesHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                              const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.s4c_l4c.");
    const QString &siteName = ctx.GetSiteShortName(event.siteId);
    AgricPracticesSiteCfg siteCfg;
    if (!GetSiteConfigForSiteId(ctx, event.siteId, parameters, configParameters, siteCfg)) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("Cannot find L4C configuration file for site %1\n").arg(siteName).toStdString());
    }

    const AgricPractOperation &execOper = GetExecutionOperation(parameters, configParameters);

    QDateTime minDate, maxDate;
    QStringList ndviFiles, ampFiles, coheFiles;
    ExtractProductFiles(ctx, event, ndviFiles, ampFiles, coheFiles, minDate, maxDate, execOper);

    QList<TaskToSubmit> allTasksList;
    CreateTasks(siteCfg, allTasksList, ndviFiles, ampFiles, coheFiles, execOper);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    SubmitTasks(ctx, event.jobId, allTasksListRef);
    NewStepList allSteps;
    CreateSteps(ctx, event, allTasksList, siteCfg, ndviFiles, ampFiles, coheFiles, allSteps, minDate, maxDate,
                execOper);
    ctx.SubmitSteps(allSteps);
}

void AgricPracticesHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
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
            ctx.InsertProduct({ ProductType::S4CL4CProductTypeId, event.processorId, event.siteId,
                                event.jobId, productFolder, maxDate,
                                prodName, quicklook, footPrint, std::experimental::nullopt, TileIdList() });

            // Now remove the job folder containing temporary files
            RemoveJobFolder(ctx, event.jobId, "l3e");
        } else {
            Logger::error(QStringLiteral("Cannot insert into database the product with name %1 and folder %2").arg(prodName).arg(productFolder));
        }
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
                                         "-gipp", executionInfosPath
                                       };
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
    // we might have an offset in days from starting the downloading products to start the S4C_L4C production
    int startSeasonOffset = mapCfg["processor.s4c_l4c.start_season_offset"].value.toInt();
    seasonStartDate = seasonStartDate.addDays(startSeasonOffset);

    // Get the start and end date for the production
    QDateTime endDate = qScheduledDate;
    QDateTime startDate = seasonStartDate;

    params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    // Normally for PhenoNDVI we need at least 4 products available in order to be able to create a S4C_L4C product
    // but if we do not return here, the schedule block waiting for products (that might never happen)
    bool waitForAvailProcInputs = (mapCfg["processor.s4c_l4c.sched_wait_proc_inputs"].value.toInt() != 0);
    if((waitForAvailProcInputs == false) || (params.productList.size() >= 4)) {
        params.isValid = true;
        Logger::debug(QStringLiteral("Executing scheduled job. Scheduler extracted for S4C_L4C a number "
                                     "of %1 products for site ID %2 with start date %3 and end date %4!")
                      .arg(params.productList.size())
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    } else {
        Logger::debug(QStringLiteral("Scheduled job for S4C_L4C and site ID %1 with start date %2 and end date %3 "
                                     "will not be executed (no products)!")
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    }

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

    retCfg = LoadSiteConfigFile(siteCfgFilePath, parameters, configParameters);
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

AgricPracticesSiteCfg AgricPracticesHandler::LoadSiteConfigFile(const QString &siteCfgFilePath,
                                                                const QJsonObject &parameters,
                                                                std::map<QString, QString> &configParameters)
{
    AgricPracticesSiteCfg cfg;

    QSettings settings(siteCfgFilePath, QSettings::IniFormat);
    QString cmnSectionKey("COMMON/");

    cfg.prdsPerGroup = GetIntConfigValue(parameters, configParameters, "prds_per_group", L4C_AP_CFG_PREFIX);
    cfg.country = settings.value( cmnSectionKey + "COUNTRY").toString();
    cfg.year = settings.value( cmnSectionKey + "YEAR").toString();
    cfg.fullShapePath = settings.value( cmnSectionKey + "FULL_SHP").toString();
    cfg.ndviIdsGeomShapePath = settings.value( cmnSectionKey + "NDVI_IDS_SHP").toString();
    cfg.ampCoheIdsGeomShapePath = settings.value( cmnSectionKey + "AMP_COHE_IDS_SHP").toString();
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
    const PracticesTableExtractionParams &defParams = LoadPracticesParams(settings, "");
    const TsaPracticeParams &defTsaParams = LoadTsaParams(settings, "");
    // Parameters used for practices tables extraction
    for (const QString &practice: cfg.practices) {
        PracticesTableExtractionParams *pPracticeParams;
        TsaPracticeParams *pTsaParams;
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

        (*pPracticeParams) = LoadPracticesParams(settings, practice);
        (*pTsaParams) = LoadTsaParams(settings, practice);
        UpdatePracticesParams(defParams, *pPracticeParams);
        UpdateTsaParams(defTsaParams, *pTsaParams);
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
    return GetInputProducts(ctx, event, ProductType::L3BProductTypeId, minDate, maxDate);
}

QStringList AgricPracticesHandler::ExtractAmpFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                   QDateTime &minDate, QDateTime &maxDate)
{
    return GetInputProducts(ctx, event, ProductType::S4CS1L2AmpProductTypeId, minDate, maxDate);
}

QStringList AgricPracticesHandler::ExtractCoheFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                    QDateTime &minDate, QDateTime &maxDate)
{
    return GetInputProducts(ctx, event, ProductType::S4CS1L2CoheProductTypeId, minDate, maxDate);
}


QStringList AgricPracticesHandler::GetIdsExtractorArgs(const AgricPracticesSiteCfg &siteCfg, const QString &outFile,
                                                       const QString &finalTargetDir)
{
    QStringList retArgs = { "LPISDataSelection",
                            "-inshp", siteCfg.fullShapePath,
                            "-country", siteCfg.country,
                            "-seqidsonly", "1",
                            "-out", outFile
                      };
    if (finalTargetDir.size() > 0) {
        retArgs += "-copydir";
        retArgs += finalTargetDir;
    }
    if (siteCfg.naPracticeParams.additionalFiles.size() > 0) {
        retArgs += "-addfiles";
        retArgs += siteCfg.naPracticeParams.additionalFiles;
    }
    return retArgs;

}

QStringList AgricPracticesHandler::GetPracticesExtractionArgs(const AgricPracticesSiteCfg &siteCfg, const QString &outFile,
                                                              const QString &practice, const QString &finalTargetDir)
{
    const PracticesTableExtractionParams *pPracticeParams = 0;
    QString dataSelPractice = GetTsaExpectedPractice(practice);
    if (practice == "CC") {
        pPracticeParams = &(siteCfg.ccPracticeParams);
    } else if (practice == "FL") {
        pPracticeParams = &(siteCfg.flPracticeParams);
    } else if (practice == "NFC") {
        pPracticeParams = &(siteCfg.nfcPracticeParams);
    } else if (practice == "NA") {
        pPracticeParams = &(siteCfg.naPracticeParams);
    }

    QStringList retArgs = { "LPISDataSelection",
                            "-inshp", siteCfg.fullShapePath,
                            "-country", siteCfg.country,
                            "-practice", dataSelPractice,
                            "-year", siteCfg.year,
                            "-out", outFile
                      };

    if (finalTargetDir.size() > 0) {
        retArgs += "-copydir";
        retArgs += finalTargetDir;
    }

    if (pPracticeParams->vegStart.size() > 0) {
        retArgs += "-vegstart";
        retArgs += pPracticeParams->hstart;
    }
    if (pPracticeParams->hstart.size() > 0) {
        retArgs += "-hstart";
        retArgs += pPracticeParams->hstart;
    }
    if (pPracticeParams->hend.size() > 0) {
        retArgs += "-hend";
        retArgs += pPracticeParams->hend;
    }
    if (pPracticeParams->hstartw.size() > 0) {
        retArgs += "-hwinterstart";
        retArgs += pPracticeParams->hstartw;
    }
    if (pPracticeParams->pstart.size() > 0) {
        retArgs += "-pstart";
        retArgs += pPracticeParams->pstart;
    }
    if (pPracticeParams->pend.size() > 0) {
        retArgs += "-pend";
        retArgs += pPracticeParams->pend;
    }
    if (pPracticeParams->pstartw.size() > 0) {
        retArgs += "-wpstart";
        retArgs += pPracticeParams->pstartw;
    }
    if (pPracticeParams->pendw.size() > 0) {
        retArgs += "-wpend";
        retArgs += pPracticeParams->pendw;
    }

    if (pPracticeParams->additionalFiles.size() > 0) {
        retArgs += "-addfiles";
        retArgs += pPracticeParams->additionalFiles;
    }
    return retArgs;

}

QStringList AgricPracticesHandler::GetDataExtractionArgs(const AgricPracticesSiteCfg &siteCfg, const QString &filterIdsFile,
                                                         const QString &prdType, const QString &uidField,
                                                         const QStringList &inputFiles, const QString &outDir)
{
    QStringList retArgs = { "AgricPractDataExtraction",
                            "-oldnf", "0",
                            "-minmax", "0",
                            "-csvcompact", "1",
                            "-field", uidField,
                            "-prdtype", prdType,
                            "-outdir", outDir,
                            "-filterids", filterIdsFile,
                            "-il"
                      };
    retArgs += inputFiles;
    const QString *idsGeomShapePath;
    if (prdType == "NDVI") {
        idsGeomShapePath = &(siteCfg.ndviIdsGeomShapePath);
    } else {
        idsGeomShapePath = &(siteCfg.ampCoheIdsGeomShapePath);
    }
    if (idsGeomShapePath->size() > 0) {
        retArgs += "-vec";
        retArgs += *idsGeomShapePath;
    }

    return retArgs;

}

QStringList AgricPracticesHandler::GetFilesMergeArgs(const QStringList &listInputPaths, const QString &outFileName)
{
    QStringList retArgs = { "AgricPractMergeDataExtractionFiles",
                            "-csvcompact", "0",
                            "-outformat", "csv",
                            "-out", outFileName,
                            "-il"
                      };
    retArgs += listInputPaths;
    return retArgs;
}

QStringList AgricPracticesHandler::GetTimeSeriesAnalysisArgs(const AgricPracticesSiteCfg &siteCfg, const QString &practice, const QString &practicesFile,
                                                             const QString &inNdviFile, const QString &inAmpFile, const QString &inCoheFile,
                                                             const QString &outDir)
{
    const TsaPracticeParams *pTsaPracticeParams = 0;
    QString tsaExpectedPractice = GetTsaExpectedPractice(practice);
    if (practice == "CC") {
        pTsaPracticeParams = &(siteCfg.ccTsaParams);
    } else if (practice == "FL") {
        pTsaPracticeParams = &(siteCfg.flTsaParams);
    } else if (practice == "NFC") {
        pTsaPracticeParams = &(siteCfg.nfcTsaParams);
    } else if (practice == "NA") {
        pTsaPracticeParams = &(siteCfg.naTsaParams);
    }
    QStringList retArgs = { "TimeSeriesAnalysis",
                            "-intype", "csv",
                            "-debug", "0",
                            "-allowgaps", "1",
                            "-gapsfill", "0",
                            "-plotgraph", "1",
                            "-rescontprd", "0",
                            "-country", siteCfg.country,
                            "-practice", tsaExpectedPractice,
                            "-year", siteCfg.year,
                            "-harvestshp", practicesFile,
                            "-diramp", inAmpFile,
                            "-dircohe", inCoheFile,
                            "-dirndvi", inNdviFile,
                            "-outdir", outDir
                      };
    if (pTsaPracticeParams->optthrvegcycle.size() > 0) {
        retArgs += "-optthrvegcycle";
        retArgs += pTsaPracticeParams->optthrvegcycle;
    }
    if (pTsaPracticeParams->ndvidw.size() > 0) {
        retArgs += "-ndvidw";
        retArgs += pTsaPracticeParams->ndvidw;
    }
    if (pTsaPracticeParams->ndviup.size() > 0) {
        retArgs += "-ndviup";
        retArgs += pTsaPracticeParams->ndviup;
    }
    if (pTsaPracticeParams->ndvistep.size() > 0) {
        retArgs += "-ndvistep";
        retArgs += pTsaPracticeParams->ndvistep;
    }
    if (pTsaPracticeParams->optthrmin.size() > 0) {
        retArgs += "-optthrmin";
        retArgs += pTsaPracticeParams->optthrmin;
    }
    if (pTsaPracticeParams->cohthrbase.size() > 0) {
        retArgs += "-cohthrbase";
        retArgs += pTsaPracticeParams->cohthrbase;
    }
    if (pTsaPracticeParams->cohthrhigh.size() > 0) {
        retArgs += "-cohthrhigh";
        retArgs += pTsaPracticeParams->cohthrhigh;
    }
    if (pTsaPracticeParams->cohthrabs.size() > 0) {
        retArgs += "-cohthrabs";
        retArgs += pTsaPracticeParams->cohthrabs;
    }
    if (pTsaPracticeParams->ampthrmin.size() > 0) {
        retArgs += "-ampthrmin";
        retArgs += pTsaPracticeParams->ampthrmin;
    }
    if (pTsaPracticeParams->efandvithr.size() > 0) {
        retArgs += "-efandvithr";
        retArgs += pTsaPracticeParams->efandvithr;
    }
    if (pTsaPracticeParams->efandviup.size() > 0) {
        retArgs += "-efandviup";
        retArgs += pTsaPracticeParams->efandviup;
    }
    if (pTsaPracticeParams->efandvidw.size() > 0) {
        retArgs += "-efandvidw";
        retArgs += pTsaPracticeParams->efandvidw;
    }
    if (pTsaPracticeParams->efacohchange.size() > 0) {
        retArgs += "-efacohchange";
        retArgs += pTsaPracticeParams->efacohchange;
    }
    if (pTsaPracticeParams->efacohvalue.size() > 0) {
        retArgs += "-efacohvalue";
        retArgs += pTsaPracticeParams->efacohvalue;
    }
    if (pTsaPracticeParams->efandvimin.size() > 0) {
        retArgs += "-efandvimin";
        retArgs += pTsaPracticeParams->efandvimin;
    }
    if (pTsaPracticeParams->efaampthr.size() > 0) {
        retArgs += "-efaampthr";
        retArgs += pTsaPracticeParams->efaampthr;
    }
    if (pTsaPracticeParams->stddevinampthr.size() > 0) {
        retArgs += "-stddevinampthr";
        retArgs += pTsaPracticeParams->stddevinampthr;
    }
    if (pTsaPracticeParams->optthrbufden.size() > 0) {
        retArgs += "-optthrbufden";
        retArgs += pTsaPracticeParams->optthrbufden;
    }

    // CC and FL specific
    if (pTsaPracticeParams->catchmain.size() > 0) {
        retArgs += "-catchmain";
        retArgs += pTsaPracticeParams->catchmain;
    }
    if (pTsaPracticeParams->catchperiod.size() > 0) {
        retArgs += "-catchperiod";
        retArgs += pTsaPracticeParams->catchperiod;
    }
    if (pTsaPracticeParams->catchperiodstart.size() > 0) {
        retArgs += "-catchperiodstart";
        retArgs += pTsaPracticeParams->catchperiodstart;
    }
    if (pTsaPracticeParams->catchcropismain.size() > 0) {
        retArgs += "-catchcropismain";
        retArgs += pTsaPracticeParams->catchcropismain;
    }
    if (pTsaPracticeParams->catchproportion.size() > 0) {
        retArgs += "-catchproportion";
        retArgs += pTsaPracticeParams->catchproportion;
    }

    if (pTsaPracticeParams->flmarkstartdate.size() > 0) {
        retArgs += "-flmarkstartdate";
        retArgs += pTsaPracticeParams->flmarkstartdate;
    }
    if (pTsaPracticeParams->flmarkstenddate.size() > 0) {
        retArgs += "-flmarkstenddate";
        retArgs += pTsaPracticeParams->flmarkstenddate;
    }

    return retArgs;
}

QString AgricPracticesHandler::BuildMergeResultFileName(const AgricPracticesSiteCfg &siteCfg, const QString &prdsType)
{
    return QString(siteCfg.country).append("_").append(siteCfg.year).append("_").append(prdsType).append("_Extracted_Data.csv");
}

QString AgricPracticesHandler::BuildPracticesTableResultFileName(const AgricPracticesSiteCfg &siteCfg, const QString &suffix)
{
    return QString("Sen4CAP_L4C_").append(suffix).append("_").append(siteCfg.country).
            append("_").append(siteCfg.year).append(".csv");
}

void AgricPracticesHandler::CreatePrdDataExtrTasks(const AgricPracticesSiteCfg &siteCfg, QList<TaskToSubmit> &outAllTasksList,
                                        const QString &taskName,
                                        const QStringList &prdsList, const QList<std::reference_wrapper<const TaskToSubmit>> &dataExtParents,
                                        AgricPractOperation operation, int &minPrdDataExtrIndex, int &maxPrdDataExtrIndex, int &curTaskIdx) {
    // if the products per group is 0 or negative, then create a group with all products
    int prdsPerGroup = siteCfg.prdsPerGroup > 0 ? siteCfg.prdsPerGroup : prdsList.size();
    // if operation is exactly data extraction, then we the groups will have exactly 1 item
    if (operation == dataExtraction) {
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

int AgricPracticesHandler::CreateTSATasks(const AgricPracticesSiteCfg &siteCfg, QList<TaskToSubmit> &outAllTasksList,
                                        const QString &practiceName, AgricPractOperation operation,
                                        int ndviMergeTaskIdx, int ampMergeTaskIdx, int coheMergeTaskIdx, int &curTaskIdx) {
    int ccTsaIdx = -1;
    AgricPractOperation expectedOper = GetExecutionOperation(practiceName);
    if (siteCfg.practices.contains(practiceName) && IsOperationEnabled(operation, expectedOper)) {
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
                                                                const AgricPracticesSiteCfg &siteCfg,
                                                                QList<TaskToSubmit> &allTasksList,
                                                                NewStepList &steps, int &curTaskIdx) {
    QString tsInputTablesDir ;
    TaskToSubmit &lpisSelectionTask = allTasksList[curTaskIdx++];
    QString fileName;
    if (practice.size() == 0) {
        fileName = BuildPracticesTableResultFileName(siteCfg, "FilterIds");
    } else {
        fileName = BuildPracticesTableResultFileName(siteCfg, practice);
    }

    const QString &tmpLpisSelFilePath = lpisSelectionTask.GetFilePath(fileName);
    QString lpisSelFilePath = tmpLpisSelFilePath;
    if (operation == dataExtraction) {
        // get the directory where the ids file should be finally moved
        tsInputTablesDir = GetStringConfigValue(parameters, configParameters, "ts_input_tables_dir", L4C_AP_CFG_PREFIX);
        lpisSelFilePath = QDir(tsInputTablesDir).filePath(fileName);
        QDir().mkpath(tsInputTablesDir);
    }
    QStringList lpisSelectionArgs;
    if (practice.size() == 0) {
        lpisSelectionArgs = GetIdsExtractorArgs(siteCfg, tmpLpisSelFilePath, tsInputTablesDir);
    } else {
        lpisSelectionArgs = GetPracticesExtractionArgs(siteCfg, tmpLpisSelFilePath, practice, tsInputTablesDir);
    }

    steps.append(lpisSelectionTask.CreateStep("LPISDataSelection", lpisSelectionArgs));

    return lpisSelFilePath;
}

QStringList AgricPracticesHandler::CreateStepsForDataExtraction(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                                                AgricPractOperation operation,
                                                                const AgricPracticesSiteCfg &siteCfg, const QString &prdType,
                                                                const QStringList &prds, const QString &idsFileName,
                                                                QList<TaskToSubmit> &allTasksList,
                                                                NewStepList &steps, int &curTaskIdx)
{
    // if the products per group is 0 or negative, then create a group with all products
    int prdsPerGroup = siteCfg.prdsPerGroup > 0 ? siteCfg.prdsPerGroup : prds.size();
    if (operation == dataExtraction) {
        prdsPerGroup = 1;
    }
    int groups = (prds.size()/prdsPerGroup) + ((prds.size()%prdsPerGroup) == 0 ? 0 : 1);

    // create the tasks for the NDVI, AMP and COHE - they depend only on the ids extraction
    QStringList dataExtrDirs;
    for(int i  = 0; i<groups; i++) {
        TaskToSubmit &dataExtractionTask = allTasksList[curTaskIdx++];
        QString dataExtrDirName;
        if (operation == all) {
            dataExtrDirName = dataExtractionTask.GetFilePath("");
            dataExtrDirs.append(dataExtrDirName);
        } else {
            QString key(prdType.toLower().append("_data_extr_dir"));
            dataExtrDirName = GetStringConfigValue(parameters, configParameters, key, L4C_AP_CFG_PREFIX);
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

        const QStringList &dataExtractionArgs = GetDataExtractionArgs(siteCfg, idsFileName, prdType, "NewID", prds, dataExtrDirName);
        steps.append(dataExtractionTask.CreateStep("AgricPractDataExtraction", dataExtractionArgs));
    }
    return dataExtrDirs;
}

QString AgricPracticesHandler::CreateStepsForFilesMerge(const AgricPracticesSiteCfg &siteCfg, const QString &prdType,
                              const QStringList &dataExtrDirs, NewStepList &steps,
                              QList<TaskToSubmit> &allTasksList, int &curTaskIdx) {
    TaskToSubmit &mergeTask = allTasksList[curTaskIdx++];
    const QString &mergedFile = mergeTask.GetFilePath(BuildMergeResultFileName(siteCfg, prdType));
    const QStringList &mergeArgs = GetFilesMergeArgs(dataExtrDirs, mergedFile);
    steps.append(mergeTask.CreateStep("AgricPractMergeDataExtractionFiles", mergeArgs));

    return mergedFile;
}

QStringList AgricPracticesHandler::CreateTimeSeriesAnalysisSteps(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                                                 AgricPractOperation operation, const AgricPracticesSiteCfg &siteCfg,
                                                                 const QString &practice, const QString &ndviMergedFile,
                                                                 const QString &ampMergedFile, const QString &coheMergedFile,
                                                                 NewStepList &steps,
                                                                 QList<TaskToSubmit> &allTasksList, int &curTaskIdx,
                                                                 AgricPractOperation activeOper)
{
    QStringList retList;
    AgricPractOperation oper = GetExecutionOperation(practice);
    if (siteCfg.practices.contains(practice) && ((oper & activeOper) != none)) {
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
        const QString &filesPrefix = "Sen4CAP_L4C_" + tsaExpPractice + "_" + siteCfg.country + "_" + siteCfg.year;
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

QStringList AgricPracticesHandler::GetInputProducts(EventProcessingContext &ctx,
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
            Logger::error(QStringLiteral("Unsupported product type %1.").arg((int)prdType));
            return QStringList();
    }
    const auto &inputProducts = parameters[prodsInputKey.c_str()].toArray();
    const std::map<QString, QString> &configParameters = ctx.GetJobConfigurationParameters(event.jobId, L4C_AP_CFG_PREFIX);
    const QString &s2L8TilesStr = GetStringConfigValue(parameters, configParameters, "s2_l8_tiles", L4C_AP_CFG_PREFIX);
    const QStringList &s2L8Tiles = s2L8TilesStr.split(',',  QString::SkipEmptyParts);

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
                if (prdType == ProductType::L3BProductTypeId) {
                    const QStringList &tiffFiles = FindNdviProductTiffFile(ctx, event.siteId, product.fullPath, s2L8Tiles);
                    if (tiffFiles.size() > 0) {
                        listProducts.append(tiffFiles);
                    }
                } else {
                    listProducts.append(product.fullPath);
                }
            }
        }
    } else {
        for (const auto &inputProduct : inputProducts) {
            // if the product is an LAI, we need to extract the TIFF file for the NDVI
            if (prdType == ProductType::L3BProductTypeId) {
                const QStringList &tiffFiles = FindNdviProductTiffFile(ctx, event.siteId, inputProduct.toString(), s2L8Tiles);
                if (tiffFiles.size() > 0) {
                    listProducts.append(tiffFiles);
                    for (const auto &tiffFile: tiffFiles) {
                        const QDateTime &ndviTime = ProcessorHandlerHelper::GetNdviProductTime(tiffFile);
                        ProcessorHandlerHelper::UpdateMinMaxTimes(ndviTime, minDate, maxDate);
                    }
                }
            } else {
                // the S1 AMP and COHE products have directly the path of the tiff in the product table
                const QString &prdPath = ctx.GetProductAbsolutePath(event.siteId, inputProduct.toString());
                if (prdPath.size() > 0) {
                    listProducts.append(prdPath);
                    const QDateTime &s1Time = ProcessorHandlerHelper::GetS1L2AProductTime(prdPath);
                    ProcessorHandlerHelper::UpdateMinMaxTimes(s1Time, minDate, maxDate);
                } else {
                    Logger::error(QStringLiteral("The product path does not exists %1.").arg(inputProduct.toString()));
                    return QStringList();
                }
            }
        }
    }

    return listProducts;
}

QStringList AgricPracticesHandler::FindNdviProductTiffFile(EventProcessingContext &ctx, int siteId,
                                                           const QString &path, const QStringList &s2L8TilesFilter)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.isDir()) {
        const QString &fileName = fileInfo.fileName();
        if (fileName.contains("S2AGRI_L3B_SNDVI_A") && fileName.endsWith(".TIF")) {
            return QStringList(path);
        }
    }
    QString absPath = path;
    if(!fileInfo.isAbsolute()) {
        // if we have the product name, we need to get the product path from the database
        absPath = ctx.GetProductAbsolutePath(siteId, path);
    }
    const QMap<QString, QString> &prdTiles = ProcessorHandlerHelper::GetHighLevelProductTilesDirs(absPath);
    QStringList retList;
    for(const auto &tileId : prdTiles.keys()) {
        if (s2L8TilesFilter.size() > 0 && !s2L8TilesFilter.contains(tileId)) {
            continue;
        }
        const QString &tileDir = prdTiles[tileId];
        const QString &laiFileName = ProcessorHandlerHelper::GetHigLevelProductTileFile(tileDir, "SNDVI");
        if (laiFileName.size() > 0) {
            retList.append(laiFileName);
        }
    }

    if (retList.size() == 0) {
        Logger::error(QStringLiteral("Unable to find any TIFF file for the given input product %1.").arg(absPath));
    }
    return retList;
}

PracticesTableExtractionParams AgricPracticesHandler::LoadPracticesParams(const QSettings &settings, const QString &practicePrefix) {
    PracticesTableExtractionParams ret;
    const QString &sectionName = (practicePrefix.length() > 0 ? practicePrefix : QString("DEFAULT")) + "_PRACTICES_PARAMS/";
    QString keyPrefix;
    if(practicePrefix.length() > 0) {
        keyPrefix = practicePrefix + "_";
    }

    ret.vegStart = settings.value( sectionName + keyPrefix + "VEG_START").toString();
    ret.hstart = settings.value( sectionName + keyPrefix + "HSTART").toString();
    ret.hend = settings.value( sectionName + keyPrefix + "HEND").toString();
    ret.hstartw = settings.value( sectionName + keyPrefix + "HSTARTW").toString();
    ret.pstart = settings.value( sectionName + keyPrefix + "PSTART").toString();
    ret.pend = settings.value( sectionName + keyPrefix + "PEND").toString();
    ret.pstartw = settings.value( sectionName + keyPrefix + "PSTARTW").toString();
    ret.pendw = settings.value( sectionName + keyPrefix + "PENDW").toString();
    ret.additionalFiles = GetListValue(settings, sectionName + keyPrefix + "ADD_FILES");

    return ret;
}

void AgricPracticesHandler::UpdatePracticesParams(const PracticesTableExtractionParams &defVals,
                                                PracticesTableExtractionParams &sectionVals) {
    sectionVals.vegStart = (sectionVals.vegStart.length() == 0 ? defVals.vegStart : sectionVals.vegStart);
    sectionVals.hstart = (sectionVals.hstart.length() == 0 ? defVals.hstart : sectionVals.hstart);
    sectionVals.hend = (sectionVals.hend.length() == 0 ? defVals.hend : sectionVals.hend);
    sectionVals.hstartw = (sectionVals.hstartw.length() == 0 ? defVals.hstartw : sectionVals.hstartw);
    sectionVals.pstart = (sectionVals.pstart.length() == 0 ? defVals.pstart : sectionVals.pstart);
    sectionVals.pend = (sectionVals.pend.length() == 0 ? defVals.pend : sectionVals.pend);
    sectionVals.pstartw = (sectionVals.pstartw.length() == 0 ? defVals.pstartw : sectionVals.pstartw);
    sectionVals.pendw = (sectionVals.pendw.length() == 0 ? defVals.pendw : sectionVals.pendw);
    sectionVals.additionalFiles = (sectionVals.additionalFiles.length() == 0 ? defVals.additionalFiles : sectionVals.additionalFiles);
}

TsaPracticeParams AgricPracticesHandler::LoadTsaParams(const QSettings &settings, const QString &practicePrefix) {
    TsaPracticeParams ret;
    const QString &sectionName = (practicePrefix.length() > 0 ? practicePrefix : QString("DEFAULT")) + "_TIME_SERIES_ANALYSIS_PARAMS/";
    QString keyPrefix;
    if(practicePrefix.length() > 0) {
        keyPrefix = practicePrefix + "_";
    }

    ret.optthrvegcycle = settings.value( sectionName + keyPrefix + "OPTTHRVEGCYCLE").toString();
    ret.ndvidw = settings.value( sectionName + keyPrefix + "NDVIDW").toString();
    ret.ndviup = settings.value( sectionName + keyPrefix + "NDVIUP").toString();
    ret.ndvistep = settings.value( sectionName + keyPrefix + "NDVISTEP").toString();
    ret.optthrmin = settings.value( sectionName + keyPrefix + "OPTTHRMIN").toString();
    ret.cohthrbase = settings.value( sectionName + keyPrefix + "COHTHRBASE").toString();
    ret.cohthrhigh = settings.value( sectionName + keyPrefix + "COHTHRHIGH").toString();
    ret.cohthrabs = settings.value( sectionName + keyPrefix + "COHTHRABS").toString();
    ret.ampthrmin = settings.value( sectionName + keyPrefix + "AMPTHRMIN").toString();
    ret.efandvithr = settings.value( sectionName + keyPrefix + "EFANDVITHR").toString();
    ret.efandviup = settings.value( sectionName + keyPrefix + "EFANDVIUP").toString();
    ret.efandvidw = settings.value( sectionName + keyPrefix + "EFANDVIDW").toString();
    ret.efacohchange = settings.value( sectionName + keyPrefix + "EFACOHCHANGE").toString();
    ret.efacohvalue = settings.value( sectionName + keyPrefix + "EFACOHVALUE").toString();
    ret.efandvimin = settings.value( sectionName + keyPrefix + "EFANDVIMIN").toString();
    ret.efaampthr = settings.value( sectionName + keyPrefix + "EFAAMPTHR").toString();
    ret.stddevinampthr = settings.value( sectionName + keyPrefix + "STDDEVINAMPTHR").toString();
    ret.optthrbufden = settings.value( sectionName + keyPrefix + "OPTTHRBUFDEN").toString();
    ret.ampthrbreakden = settings.value( sectionName + keyPrefix + "AMPTHRBREAKDEN").toString();
    ret.ampthrvalueden = settings.value( sectionName + keyPrefix + "AMPTHRVALUEDEN").toString();


    ret.catchmain = settings.value( sectionName + keyPrefix + "CATCHMAIN").toString();
    ret.catchperiod = settings.value( sectionName + keyPrefix + "CATCHPERIOD").toString();
    ret.catchperiodstart = settings.value( sectionName + keyPrefix + "CATCHPERIODSTART").toString();
    ret.catchcropismain = settings.value( sectionName + keyPrefix + "CATCHCROPISMAIN").toString();
    ret.catchproportion = settings.value( sectionName + keyPrefix + "CATCHPROPORTION").toString();

    ret.flmarkstartdate = settings.value( sectionName + keyPrefix + "FLMARKSTARTDATE").toString();
    ret.flmarkstenddate = settings.value( sectionName + keyPrefix + "FLMARKSTENDDATE").toString();

    return ret;
}

void AgricPracticesHandler::UpdateTsaParams(const TsaPracticeParams &defVals,
                                           TsaPracticeParams &sectionVals) {
    sectionVals.optthrvegcycle = (sectionVals.optthrvegcycle.length() == 0 ? defVals.optthrvegcycle : sectionVals.optthrvegcycle);
    sectionVals.ndvidw = (sectionVals.ndvidw.length() == 0 ? defVals.ndvidw : sectionVals.ndvidw);
    sectionVals.ndviup = (sectionVals.ndviup.length() == 0 ? defVals.ndviup : sectionVals.ndviup);
    sectionVals.ndvistep = (sectionVals.ndvistep.length() == 0 ? defVals.ndvistep : sectionVals.ndvistep);
    sectionVals.optthrmin = (sectionVals.optthrmin.length() == 0 ? defVals.optthrmin : sectionVals.optthrmin);
    sectionVals.cohthrbase = (sectionVals.cohthrbase.length() == 0 ? defVals.cohthrbase : sectionVals.cohthrbase);
    sectionVals.cohthrhigh = (sectionVals.cohthrhigh.length() == 0 ? defVals.cohthrhigh : sectionVals.cohthrhigh);
    sectionVals.cohthrabs = (sectionVals.cohthrabs.length() == 0 ? defVals.cohthrabs : sectionVals.cohthrabs);
    sectionVals.ampthrmin = (sectionVals.ampthrmin.length() == 0 ? defVals.ampthrmin : sectionVals.ampthrmin);
    sectionVals.efandvithr = (sectionVals.efandvithr.length() == 0 ? defVals.efandvithr : sectionVals.efandvithr);
    sectionVals.efandviup = (sectionVals.efandviup.length() == 0 ? defVals.efandviup : sectionVals.efandviup);
    sectionVals.efandvidw = (sectionVals.efandvidw.length() == 0 ? defVals.efandvidw : sectionVals.efandvidw);
    sectionVals.efacohchange = (sectionVals.efacohchange.length() == 0 ? defVals.efacohchange : sectionVals.efacohchange);
    sectionVals.efacohvalue = (sectionVals.efacohvalue.length() == 0 ? defVals.efacohvalue : sectionVals.efacohvalue);
    sectionVals.efandvimin = (sectionVals.efandvimin.length() == 0 ? defVals.efandvimin : sectionVals.efandvimin);
    sectionVals.efaampthr = (sectionVals.efaampthr.length() == 0 ? defVals.efaampthr : sectionVals.efaampthr);
    sectionVals.stddevinampthr = (sectionVals.stddevinampthr.length() == 0 ? defVals.stddevinampthr : sectionVals.stddevinampthr);
    sectionVals.optthrbufden = (sectionVals.optthrbufden.length() == 0 ? defVals.optthrbufden : sectionVals.optthrbufden);
    sectionVals.ampthrbreakden = (sectionVals.ampthrbreakden.length() == 0 ? defVals.ampthrbreakden : sectionVals.ampthrbreakden);
    sectionVals.ampthrvalueden = (sectionVals.ampthrvalueden.length() == 0 ? defVals.ampthrvalueden : sectionVals.ampthrvalueden);

    sectionVals.catchmain = (sectionVals.catchmain.length() == 0 ? defVals.catchmain : sectionVals.catchmain);
    sectionVals.catchperiod = (sectionVals.catchperiod.length() == 0 ? defVals.catchperiod : sectionVals.catchperiod);
    sectionVals.catchperiodstart = (sectionVals.catchperiodstart.length() == 0 ? defVals.catchperiodstart : sectionVals.catchperiodstart);;
    sectionVals.catchcropismain = (sectionVals.catchcropismain.length() == 0 ? defVals.catchcropismain : sectionVals.catchcropismain);
    sectionVals.catchproportion = (sectionVals.catchproportion.length() == 0 ? defVals.catchproportion : sectionVals.catchproportion);

    sectionVals.flmarkstartdate = (sectionVals.flmarkstartdate.length() == 0 ? defVals.flmarkstartdate : sectionVals.flmarkstartdate);
    sectionVals.flmarkstenddate = (sectionVals.flmarkstenddate.length() == 0 ? defVals.flmarkstenddate : sectionVals.flmarkstenddate);
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
    const QString &execOper = GetStringConfigValue(parameters, configParameters, "execution_operation", L4C_AP_CFG_PREFIX);
    return GetExecutionOperation(execOper);
}

AgricPractOperation AgricPracticesHandler::GetExecutionOperation(const QString &str)
{
    if (QString::compare(str, "DataExtraction", Qt::CaseInsensitive) == 0) {
        return dataExtraction;
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
    QString prdKey;
    switch (prd.productTypeId) {
        case ProductType::L3BProductTypeId:
            prdKey = "input_NDVI";
            break;
        case ProductType::S4CS1L2AmpProductTypeId:
            prdKey = "input_AMP";
            break;
        case ProductType::S4CS1L2CoheProductTypeId:
            prdKey = "input_COHE";
            break;
        default:
            return;
    }
    QJsonObject parameters;
    // check if the NRT data extraction is configured for the site
    auto configParameters = ctx.GetConfigurationParameters(L4C_AP_CFG_PREFIX, prd.siteId);
    bool nrtDataExtrEnabled = GetBoolConfigValue(parameters, configParameters, "nrt_data_extr_enabled", L4C_AP_CFG_PREFIX);
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

