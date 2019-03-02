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

#define L4C_AP_CFG_PREFIX   "processor.s4c_l3c."

void AgricPracticesHandler::CreateTasks(const AgricPracticesSiteCfg &siteCfg, QList<TaskToSubmit> &outAllTasksList,
                                                      const QStringList &ndviPrds, const QStringList &ampPrds, const QStringList &cohePrds)
{
    int curTaskIdx = 0;
    int idsFilterExtrIdx = curTaskIdx++;
    outAllTasksList.append(TaskToSubmit{ "ids-filter-file-extraction", {} });

    // if the products per group is 0 or negative, then create a group with all products
    int ndviPrdsPerGroup = siteCfg.prdsPerGroup > 0 ? siteCfg.prdsPerGroup : ndviPrds.size();
    int ampPrdsPerGroup = siteCfg.prdsPerGroup > 0 ? siteCfg.prdsPerGroup : ampPrds.size();
    int cohePrdsPerGroup = siteCfg.prdsPerGroup > 0 ? siteCfg.prdsPerGroup : cohePrds.size();

    int ndviGroups = (ndviPrds.size()/ndviPrdsPerGroup) + ((ndviPrds.size()%ndviPrdsPerGroup) == 0 ? 0 : 1);
    int ampGroups = (ampPrds.size()/ampPrdsPerGroup) + ((ampPrds.size()%ampPrdsPerGroup) == 0 ? 0 : 1);
    int coheGroups = (cohePrds.size()/cohePrdsPerGroup) + ((cohePrds.size()%cohePrdsPerGroup) == 0 ? 0 : 1);

    // create the tasks for the NDVI - they depend only on the ids extraction
    int minNdviDataExtrIndex = curTaskIdx;
    int maxNdviDataExtrIndex = curTaskIdx + ndviGroups - 1;
    for(int i  = 0; i<ndviGroups; i++) {
        outAllTasksList.append(TaskToSubmit{ "ndvi-data-extraction", {outAllTasksList[idsFilterExtrIdx]} });
        curTaskIdx++;
    }

    // create the tasks for the AMP - they depend only on the ids extraction
    int minAmpDataExtrIndex = curTaskIdx;
    int maxAmpDataExtrIndex = curTaskIdx + ampGroups - 1;
    for(int i  = 0; i<ampGroups; i++) {
        outAllTasksList.append(TaskToSubmit{ "amp-data-extraction", {outAllTasksList[idsFilterExtrIdx]} });
        curTaskIdx++;
    }

    // create the tasks for the COHE - they depend only on the ids extraction
    int minCoheDataExtrIndex = curTaskIdx;
    int maxCoheDataExtrIndex = curTaskIdx + coheGroups - 1;
    for(int i  = 0; i<coheGroups; i++) {
        outAllTasksList.append(TaskToSubmit{ "cohe-data-extraction", {outAllTasksList[idsFilterExtrIdx]} });
        curTaskIdx++;
    }

    outAllTasksList.append(TaskToSubmit{ "ndvi-data-extraction-merge", {} });
    int ndviMergeTaskIdx = curTaskIdx++;
    // update the parents for this task
    for (int i = minNdviDataExtrIndex; i <= maxNdviDataExtrIndex; i++) {
        outAllTasksList[ndviMergeTaskIdx].parentTasks.append(outAllTasksList[i]);
    }

    outAllTasksList.append(TaskToSubmit{ "amp-data-extraction-merge", {} });
    int ampMergeTaskIdx = curTaskIdx++;
    // update the parents for this task
    for (int i = minAmpDataExtrIndex; i <= maxAmpDataExtrIndex; i++) {
        outAllTasksList[ampMergeTaskIdx].parentTasks.append(outAllTasksList[i]);
    }

    outAllTasksList.append(TaskToSubmit{ "cohe-data-extraction-merge", {} });
    int coheMergeTaskIdx = curTaskIdx++;
    // update the parents for this task
    for (int i = minCoheDataExtrIndex; i <= maxCoheDataExtrIndex; i++) {
        outAllTasksList[coheMergeTaskIdx].parentTasks.append(outAllTasksList[i]);
    }


    int ccTsaIdx = -1;
    if (siteCfg.practices.contains("CC")) {
        // this task is independent and can be executed before any others
        outAllTasksList.append(TaskToSubmit{ "cc-practices-extraction", {} });
        int ccPractExtrIdx = curTaskIdx++;
        outAllTasksList.append(TaskToSubmit{ "cc-time-series-analysis", {outAllTasksList[ccPractExtrIdx],
                                                                         outAllTasksList[ndviMergeTaskIdx], outAllTasksList[ampMergeTaskIdx],
                                                                         outAllTasksList[coheMergeTaskIdx]} });
        ccTsaIdx = curTaskIdx++;
    }
    int flTsaIdx = -1;
    if (siteCfg.practices.contains("FL")) {
        // this task is independent and can be executed before any others
        outAllTasksList.append(TaskToSubmit{ "fl-practices-extraction", {} });
        int flPractExtrIdx = curTaskIdx++;
        outAllTasksList.append(TaskToSubmit{ "fl-time-series-analysis", {outAllTasksList[flPractExtrIdx],
                                                                         outAllTasksList[ndviMergeTaskIdx], outAllTasksList[ampMergeTaskIdx],
                                                                         outAllTasksList[coheMergeTaskIdx]} });
        flTsaIdx = curTaskIdx++;
    }
    int nfcTsaIdx = -1;
    if (siteCfg.practices.contains("NFC")) {
        // this task is independent and can be executed before any others
        outAllTasksList.append(TaskToSubmit{ "nfc-practices-extraction", {} });
        int nfcPractExtrIdx = curTaskIdx++;
        outAllTasksList.append(TaskToSubmit{ "nfc-time-series-analysis", {outAllTasksList[nfcPractExtrIdx],
                                                                          outAllTasksList[ndviMergeTaskIdx], outAllTasksList[ampMergeTaskIdx],
                                                                          outAllTasksList[coheMergeTaskIdx]} });
        nfcTsaIdx = curTaskIdx++;

    }
    int naTsaIdx = -1;
    if (siteCfg.practices.contains("NA")) {
        // this task is independent and can be executed before any others
        outAllTasksList.append(TaskToSubmit{ "na-practices-extraction", {} });
        int naPractExtrIdx = curTaskIdx++;
        outAllTasksList.append(TaskToSubmit{ "na-time-series-analysis", {outAllTasksList[naPractExtrIdx],
                                             outAllTasksList[ndviMergeTaskIdx], outAllTasksList[ampMergeTaskIdx],
                                             outAllTasksList[coheMergeTaskIdx]} });
        naTsaIdx = curTaskIdx++;
    }

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

void AgricPracticesHandler::CreateSteps(EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event, QList<TaskToSubmit> &allTasksList,
                                        const AgricPracticesSiteCfg &siteCfg,
                                        const QStringList &ndviPrds, const QStringList &ampPrds, const QStringList &cohePrds,
                                        NewStepList &steps, const QDateTime &minDate, const QDateTime &maxDate)
{
    int curTaskIdx = 0;
    TaskToSubmit &idsExtractorTask = allTasksList[curTaskIdx++];
    const QString &idsFileName = idsExtractorTask.GetFilePath("SEN4CAP_L4C_FilterIds.csv");
    const QStringList &idsExtractorArgs = GetIdsExtractorArgs(siteCfg, idsFileName);
    steps.append(idsExtractorTask.CreateStep("LPISDataSelection", idsExtractorArgs));

    // create the tasks for the NDVI, AMP and COHE - they depend only on the ids extraction
    const QStringList &ndviDataExtrDirs = CreateStepsForDataExtraction(siteCfg, "NDVI", ndviPrds, idsFileName, allTasksList, steps, curTaskIdx);
    const QStringList &ampDataExtrDirs = CreateStepsForDataExtraction(siteCfg, "AMP", ampPrds, idsFileName, allTasksList, steps, curTaskIdx);
    const QStringList &coheDataExtrDirs = CreateStepsForDataExtraction(siteCfg, "COHE", cohePrds, idsFileName, allTasksList, steps, curTaskIdx);

    const QString &ndviMergedFile = CreateStepsForFilesMerge(siteCfg, "NDVI", ndviDataExtrDirs, steps, allTasksList, curTaskIdx);
    const QString &ampMergedFile = CreateStepsForFilesMerge(siteCfg, "AMP", ampDataExtrDirs, steps, allTasksList, curTaskIdx);
    const QString &coheMergedFile = CreateStepsForFilesMerge(siteCfg, "COHE", coheDataExtrDirs, steps, allTasksList, curTaskIdx);

    QStringList productFormatterFiles;
    productFormatterFiles += CreateTimeSeriesAnalysisSteps(siteCfg, "CC", ndviMergedFile, ampMergedFile, coheMergedFile, steps, allTasksList, curTaskIdx);
    productFormatterFiles += CreateTimeSeriesAnalysisSteps(siteCfg, "FL", ndviMergedFile, ampMergedFile, coheMergedFile, steps, allTasksList, curTaskIdx);
    productFormatterFiles += CreateTimeSeriesAnalysisSteps(siteCfg, "NFC", ndviMergedFile, ampMergedFile, coheMergedFile, steps, allTasksList, curTaskIdx);
    productFormatterFiles += CreateTimeSeriesAnalysisSteps(siteCfg, "NA", ndviMergedFile, ampMergedFile, coheMergedFile, steps, allTasksList, curTaskIdx);

    TaskToSubmit &productFormatterTask = allTasksList[curTaskIdx++];
    const QStringList &productFormatterArgs = GetProductFormatterArgs(productFormatterTask, ctx, event,
                                                                      productFormatterFiles, minDate, maxDate);
    steps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
}

void AgricPracticesHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                              const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.s4c_l4c.");
    const QString &siteName = ctx.GetSiteShortName(event.siteId);
    const QString &siteCfgFilePath = GetSiteConfigFilePath(siteName, parameters, configParameters);
    if (siteCfgFilePath == "") {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("Cannot find L4C configuration file for site %1\n").arg(siteName).toStdString());
    }

    QDateTime minDate, maxDate;
    const AgricPracticesSiteCfg &siteCfg = LoadSiteConfigFile(siteCfgFilePath, parameters, configParameters);
    const QStringList &ndviFiles = ExtractNdviFiles(ctx, event, minDate, maxDate);
    if (ndviFiles.size() == 0) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("Cannot find L4C configuration file for site %1. No NDVI files available!\n").arg(siteName).toStdString());
    }
    const QStringList &ampFiles = ExtractAmpFiles(ctx, event, minDate, maxDate);
    if (ampFiles.size() == 0) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("Cannot find L4C configuration file for site %1. No Amplitude files available!\n").arg(siteName).toStdString());
    }
    const QStringList &coheFiles = ExtractCoheFiles(ctx, event, minDate, maxDate);
    if (coheFiles.size() == 0) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("Cannot find L4C configuration file for site %1. No Coherence files available!\n").arg(siteName).toStdString());
    }

    QList<TaskToSubmit> allTasksList;
    CreateTasks(siteCfg, allTasksList, ndviFiles, ampFiles, coheFiles);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    SubmitTasks(ctx, event.jobId, allTasksListRef);
    NewStepList allSteps;
    CreateSteps(ctx, event, allTasksList, siteCfg, ndviFiles, ampFiles, coheFiles, allSteps, minDate, maxDate);
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

    cfg.prdsPerGroup = GetBoolConfigValue(parameters, configParameters, "prds_per_group", L4C_AP_CFG_PREFIX);
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


QStringList AgricPracticesHandler::GetIdsExtractorArgs(const AgricPracticesSiteCfg &siteCfg, const QString &outFile)
{
    QStringList retArgs = { "LPISDataSelection",
                            "-inshp", siteCfg.fullShapePath,
                            "-country", siteCfg.country,
                            "-seqidsonly", "1",
                            "-out", outFile
                      };
    if (siteCfg.naPracticeParams.additionalFiles.size() > 0) {
        retArgs += "-addfiles";
        retArgs += siteCfg.naPracticeParams.additionalFiles;
    }
    return retArgs;

}

QStringList AgricPracticesHandler::GetPracticesExtractionArgs(const AgricPracticesSiteCfg &siteCfg, const QString &outFile,
                                                              const QString &practice)
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

QString AgricPracticesHandler::BuildPracticesTableResultFileName(const AgricPracticesSiteCfg &siteCfg, const QString &practice)
{
    return QString("Sen4CAP_L4C_").append(practice).append("_").append(siteCfg.country).
            append("_").append(siteCfg.year).append(".csv");
}

QStringList AgricPracticesHandler::CreateStepsForDataExtraction(const AgricPracticesSiteCfg &siteCfg, const QString &prdType,
                                                                const QStringList &prds, const QString &idsFileName, QList<TaskToSubmit> &allTasksList,
                                                                NewStepList &steps, int &curTaskIdx)
{
    // if the products per group is 0 or negative, then create a group with all products
    int prdsPerGroup = siteCfg.prdsPerGroup > 0 ? siteCfg.prdsPerGroup : prds.size();
    int groups = (prds.size()/prdsPerGroup) + ((prds.size()%prdsPerGroup) == 0 ? 0 : 1);

    // create the tasks for the NDVI, AMP and COHE - they depend only on the ids extraction
    QStringList dataExtrDirs;
    for(int i  = 0; i<groups; i++) {
        TaskToSubmit &dataExtractionTask = allTasksList[curTaskIdx++];
        const QString &taskDataExtrDirName = dataExtractionTask.GetFilePath("");
        dataExtrDirs.append(taskDataExtrDirName);

        // Extract the list of products from the current group
        int grpStartIdx = i * prdsPerGroup;
        int nextGrpStartIdx = (i+1) * prdsPerGroup;
        QStringList sublist;
        for (int j = grpStartIdx; j < nextGrpStartIdx && j < prds.size(); j++) {
            sublist.append(prds.at(j));
        }

        const QStringList &dataExtractionArgs = GetDataExtractionArgs(siteCfg, idsFileName, prdType, "NewID", prds, taskDataExtrDirName);
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

QStringList AgricPracticesHandler::CreateTimeSeriesAnalysisSteps(const AgricPracticesSiteCfg &siteCfg,
                                                                 const QString &practice, const QString &ndviMergedFile,
                                                                 const QString &ampMergedFile, const QString &coheMergedFile,
                                                                 NewStepList &steps,
                                                                 QList<TaskToSubmit> &allTasksList, int &curTaskIdx)
{
    QStringList retList;
    if (siteCfg.practices.contains(practice)) {
        TaskToSubmit &practicesExtractionTask = allTasksList[curTaskIdx++];
        const QString &practicesFile = practicesExtractionTask.GetFilePath(BuildPracticesTableResultFileName(siteCfg, practice));
        const QStringList &ccPracticesExtractionArgs = GetPracticesExtractionArgs(siteCfg, practicesFile, practice);
        steps.append(practicesExtractionTask.CreateStep("LPISDataSelection", ccPracticesExtractionArgs));

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
                const QStringList &tiffFiles = FindNdviProductTiffFile(ctx, event, inputProduct.toString());
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

QStringList AgricPracticesHandler::FindNdviProductTiffFile(EventProcessingContext &ctx, const JobSubmittedEvent &event, const QString &path)
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
        absPath = ctx.GetProductAbsolutePath(event.siteId, path);
    }
    const QString &tilesPath = QDir(absPath).filePath("TILES");
    QStringList retList;
    for (const auto &subDir : QDir(tilesPath).entryList(QDir::NoDotAndDotDot | QDir::Dirs)) {
        const QString &tileDir = QDir(tilesPath).filePath(subDir);
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

QDateTime AgricPracticesHandler::GetNdviProductTime(const QString &prdPath)
{
    static QRegularExpression rex(QStringLiteral(NDVI_PRD_NAME_REGEX));
    QRegularExpression re(rex);
    const QString &fileName = QFileInfo(prdPath).fileName();
    const QRegularExpressionMatch &match = re.match(fileName);
    if (match.hasMatch()) {
        const QString &timestamp = match.captured(1);
        return QDateTime::fromString(timestamp, "yyyyMMddTHHmmss");
    }
    return QDateTime();
}

QDateTime AgricPracticesHandler::GetS1L2AProductTime(const QString &prdPath)
{
    const QString &fileName = QFileInfo(prdPath).fileName();
    static QRegularExpression rex(QStringLiteral(S1_L2A_PRD_NAME_REGEX));
    QRegularExpression re(rex);
    const QRegularExpressionMatch &match = re.match(fileName);
    if (match.hasMatch()) {
        const QString &timestamp1 = match.captured(1);
        const QString &timestamp2 = match.captured(2);
        const QDateTime &qDateTime1 = QDateTime::fromString(timestamp1, "yyyyMMddTHHmmss");
        const QDateTime &qDateTime2 = QDateTime::fromString(timestamp2, "yyyyMMddTHHmmss");
        return qDateTime1 < qDateTime2 ? qDateTime1 : qDateTime2;
    }
    return QDateTime();
}

void AgricPracticesHandler::UpdateMinMaxTimes(const QDateTime &newTime, QDateTime &minTime, QDateTime &maxTime)
{
    if (newTime.isValid()) {
        if (!minTime.isValid() && !maxTime.isValid()) {
            minTime = newTime;
            maxTime = newTime;
        } else if (newTime < minTime) {
            minTime = newTime;
        } else if (newTime > maxTime) {
            maxTime = newTime;
        }
    }
}
