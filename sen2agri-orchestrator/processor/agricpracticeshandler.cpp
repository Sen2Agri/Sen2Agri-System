#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "agricpracticeshandler.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"
#include "logger.hpp"

void AgricPracticesHandler::CreateTasks(const AgricPracticesSiteCfg &siteCfg, QList<TaskToSubmit> &outAllTasksList,
                                                      const QStringList &ndviPrds, const QStringList &ampPrds, const QStringList &cohePrds)
{
    int curTaskIdx = 0;
    int idsFilterExtrIdx = curTaskIdx++;
    outAllTasksList.append(TaskToSubmit{ "ids-filter-file-extraction", {} });

    int ndviGroups = (ndviPrds.size()/siteCfg.prdsPerGroup) + ((ndviPrds.size()%siteCfg.prdsPerGroup) == 0 ? 0 : 1);
    int ampGroups = (ampPrds.size()/siteCfg.prdsPerGroup) + ((ampPrds.size()%siteCfg.prdsPerGroup) == 0 ? 0 : 1);
    int coheGroups = (cohePrds.size()/siteCfg.prdsPerGroup) + ((cohePrds.size()%siteCfg.prdsPerGroup) == 0 ? 0 : 1);

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
                                        NewStepList &steps)
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
    productFormatterFiles += CreateTimeSeriesAnalysisSteps(siteCfg, "CatchCrop", ndviMergedFile, ampMergedFile, coheMergedFile, steps, allTasksList, curTaskIdx);
    productFormatterFiles += CreateTimeSeriesAnalysisSteps(siteCfg, "Fallow", ndviMergedFile, ampMergedFile, coheMergedFile, steps, allTasksList, curTaskIdx);
    productFormatterFiles += CreateTimeSeriesAnalysisSteps(siteCfg, "NFC", ndviMergedFile, ampMergedFile, coheMergedFile, steps, allTasksList, curTaskIdx);
    productFormatterFiles += CreateTimeSeriesAnalysisSteps(siteCfg, "NA", ndviMergedFile, ampMergedFile, coheMergedFile, steps, allTasksList, curTaskIdx);

    TaskToSubmit &productFormatterTask = allTasksList[curTaskIdx++];
    const QStringList &productFormatterArgs = GetProductFormatterArgs(productFormatterTask, ctx, event, productFormatterFiles);
    steps.append(productFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));
}

void AgricPracticesHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                              const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l4c.");
    const QString &siteName = ctx.GetSiteShortName(event.siteId);
    const QString &siteCfgFilePath = GetSiteConfigFilePath(siteName, parameters, configParameters);
    if (siteCfgFilePath == "") {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("Cannot find L4C configuration file for site %1\n").arg(siteName).toStdString());
    }

    const AgricPracticesSiteCfg &siteCfg = LoadSiteConfigFile(siteCfgFilePath);
    const QStringList &ndviFiles = ExtractNdviFiles(ctx, event);
    const QStringList &ampFiles = ExtractAmpFiles(ctx, event);
    const QStringList &coheFiles = ExtractCoheFiles(ctx, event);

    QList<TaskToSubmit> allTasksList;
    CreateTasks(siteCfg, allTasksList, ndviFiles, ampFiles, coheFiles);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: allTasksList) {
        allTasksListRef.append(task);
    }
    SubmitTasks(ctx, event.jobId, allTasksListRef);
    NewStepList allSteps;
    CreateSteps(ctx, event, allTasksList, siteCfg, ndviFiles, ampFiles, coheFiles, allSteps);
    ctx.SubmitSteps(allSteps);
}


void AgricPracticesHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
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
                                prodName, quicklook, footPrint, std::experimental::nullopt, TileIdList() });

            // Now remove the job folder containing temporary files
            RemoveJobFolder(ctx, event.jobId, "l3e");
        } else {
            Logger::error(QStringLiteral("Cannot insert into database the product with name %1 and folder %2").arg(prodName).arg(productFolder));
        }
    }
}

QStringList AgricPracticesHandler::GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx,
                                                           const JobSubmittedEvent &event, const QStringList &listFiles) {
    // ProductFormatter /home/cudroiu/sen2agri-processors-build
    //    -vectprd 1 -destroot /mnt/archive_new/test/Sen4CAP_L4C_Tests/NLD_Validation_TSA/OutPrdFormatter
    //    -fileclass OPER -level S4C_L4C -baseline 01.00 -siteid 4 -timeperiod 20180101_20181231 -processor generic
    //    -processor.generic.files <files_list>

    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.txt");
    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot", targetFolder,
                                         "-fileclass", "OPER",
                                         "-level", "S4C_L4C",
                                         "-vectprd", "1",
                                         "-baseline", "01.00",
                                         "-siteid", QString::number(event.siteId),
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
    if(parameters.contains("configs_path")) {
        const auto &value = parameters["configs_path"];
        if(value.isString()) {
            strCfgPath = value.toString();
        }
    }
    if (strCfgPath.isEmpty()) {
        strCfgPath = configParameters["processor.l4c.configs_path"];
    }

    QDir dir(strCfgPath);
    if (!dir.exists()) {
        return "";
    }
    const QString &cfgFilePath = QDir(strCfgPath).filePath("L4C_Config_" + siteName + ".cfg");
    if (!QFileInfo::exists(cfgFilePath)) {
        return "";
    }
    return cfgFilePath;
}

AgricPracticesSiteCfg AgricPracticesHandler::LoadSiteConfigFile(const QString &siteCfgFilePath)
{
    AgricPracticesSiteCfg cfg;

    QSettings settings(siteCfgFilePath, QSettings::IniFormat);
    QString cmnSectionKey = "COMMON/";

    cfg.country = settings.value( cmnSectionKey + "country", "r").toString();
    cfg.year = settings.value( cmnSectionKey + "year", "r").toString();
    cfg.fullShapePath = settings.value( cmnSectionKey + "full_shp", "r").toString();
    cfg.idsGeomShapePath = settings.value( cmnSectionKey + "ids_shp", "r").toString();

    // Parameters used for practices tables extraction
    QStringList practices = {"CC", "FL", "NFC", "NA"};
    for (QString practice: practices) {
        QString practiceParamsSectionName(practice + "_PRACTICES_PARAMS/");
        QString tsaParamsSectionName(practice + "_TIME_SERIES_ANALYSIS_PARAMS/");
        QStringList *pAddFiles;
        PracticesTableExtractionParams *pPracticeParams;
        TsaPracticeParams *pTsaParams;
        if (practice == "CC") {
            pAddFiles = &cfg.ccAdditionalFiles;
            pPracticeParams = &cfg.ccPracticeParams;
            pTsaParams = &cfg.ccTsaParams;
        } else if (practice == "FL") {
            pAddFiles = &cfg.flAdditionalFiles;
            pPracticeParams = &cfg.flPracticeParams;
            pTsaParams = &cfg.flTsaParams;
        } else if (practice == "NFC") {
            pAddFiles = &cfg.nfcAdditionalFiles;
            pPracticeParams = &cfg.nfcPracticeParams;
            pTsaParams = &cfg.nfcTsaParams;
        } else if (practice == "NA") {
            pAddFiles = &cfg.naAdditionalFiles;
            pPracticeParams = &cfg.naPracticeParams;
            pTsaParams = &cfg.naTsaParams;
        }
        (*pAddFiles) = settings.value( practiceParamsSectionName + "additional_files", "r").toString().split(';');
        pPracticeParams->vegStart = settings.value( practiceParamsSectionName + "veg_start", "r").toString();
        pPracticeParams->hstart = settings.value( practiceParamsSectionName + "harvest_start", "r").toString();
        pPracticeParams->hend = settings.value( practiceParamsSectionName + "harvest_end", "r").toString();
        pPracticeParams->hstartw = settings.value( practiceParamsSectionName + "winter_harvest_start", "r").toString();
        pPracticeParams->pstart = settings.value( practiceParamsSectionName + "practice_start", "r").toString();
        pPracticeParams->pend = settings.value( practiceParamsSectionName + "practice_end", "r").toString();
        pPracticeParams->pstartw = settings.value( practiceParamsSectionName + "winter_practice_start", "r").toString();
        pPracticeParams->pendw = settings.value( practiceParamsSectionName + "winter_practice_end", "r").toString();

        pTsaParams->optthrvegcycle = settings.value( tsaParamsSectionName + "optthrvegcycle", "r").toString();
        pTsaParams->ndvidw = settings.value( tsaParamsSectionName + "ndvidw", "r").toString();
        pTsaParams->ndviup = settings.value( tsaParamsSectionName + "ndviup", "r").toString();
        pTsaParams->ndvistep = settings.value( tsaParamsSectionName + "ndvistep", "r").toString();
        pTsaParams->optthrmin = settings.value( tsaParamsSectionName + "optthrmin", "r").toString();
        pTsaParams->cohthrbase = settings.value( tsaParamsSectionName + "cohthrbase", "r").toString();
        pTsaParams->cohthrhigh = settings.value( tsaParamsSectionName + "cohthrhigh", "r").toString();
        pTsaParams->cohthrabs = settings.value( tsaParamsSectionName + "cohthrabs", "r").toString();
        pTsaParams->ampthrmin = settings.value( tsaParamsSectionName + "ampthrmin", "r").toString();
        pTsaParams->efandvithr = settings.value( tsaParamsSectionName + "efandvithr", "r").toString();
        pTsaParams->efandviup = settings.value( tsaParamsSectionName + "efandviup", "r").toString();
        pTsaParams->efandvidw = settings.value( tsaParamsSectionName + "efandvidw", "r").toString();
        pTsaParams->efacohchange = settings.value( tsaParamsSectionName + "efacohchange", "r").toString();
        pTsaParams->efacohvalue = settings.value( tsaParamsSectionName + "efacohvalue", "r").toString();
        pTsaParams->efandvimin = settings.value( tsaParamsSectionName + "efandvimin", "r").toString();
        pTsaParams->efaampthr = settings.value( tsaParamsSectionName + "efaampthr", "r").toString();
        pTsaParams->stddevinampthr = settings.value( tsaParamsSectionName + "stddevinampthr", "r").toString();
        pTsaParams->optthrbufden = settings.value( tsaParamsSectionName + "optthrbufden", "r").toString();

        pTsaParams->catchmain = settings.value( tsaParamsSectionName + "catchmain", "r").toString();
        pTsaParams->catchperiod = settings.value( tsaParamsSectionName + "catchperiod", "r").toString();
        pTsaParams->catchperiodstart = settings.value( tsaParamsSectionName + "catchperiodstart", "r").toString();
        pTsaParams->catchcropismain = settings.value( tsaParamsSectionName + "catchcropismain", "r").toString();
        pTsaParams->catchproportion = settings.value( tsaParamsSectionName + "catchproportion", "r").toString();

        pTsaParams->flmarkstartdate = settings.value( tsaParamsSectionName + "flmarkstartdate", "r").toString();
        pTsaParams->flmarkstenddate = settings.value( tsaParamsSectionName + "flmarkstenddate", "r").toString();
    }

    return cfg;
}

QStringList AgricPracticesHandler::ExtractNdviFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event)
{
    return GetInputProducts(ctx, event, ProductType::L3BProductTypeId);
}

QStringList AgricPracticesHandler::ExtractAmpFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event)
{
    return GetInputProducts(ctx, event, ProductType::S4CS1L2AmpProductTypeId);
}

QStringList AgricPracticesHandler::ExtractCoheFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event)
{
    return GetInputProducts(ctx, event, ProductType::S4CS1L2CoheProductTypeId);
}


QStringList AgricPracticesHandler::GetIdsExtractorArgs(const AgricPracticesSiteCfg &siteCfg, const QString &outFile)
{
    QStringList retArgs = { "LPISDataSelection",
                            "-inshp", siteCfg.fullShapePath,
                            "-country", siteCfg.country,
                            "-seqidsonly", "1",
                            "-out", outFile
                      };
    if (siteCfg.naAdditionalFiles.size() > 0) {
        retArgs += "-addfiles";
        retArgs += siteCfg.naAdditionalFiles;
    }
    return retArgs;

}

QStringList AgricPracticesHandler::GetPracticesExtractionArgs(const AgricPracticesSiteCfg &siteCfg, const QString &outFile,
                                                              const QString &practice)
{
    QStringList retArgs = { "LPISDataSelection",
                            "-inshp", siteCfg.fullShapePath,
                            "-country", siteCfg.country,
                            "-practice", practice,
                            "-year", siteCfg.year,
                            "-out", outFile
                      };

    const QStringList *pPracticeAddFiles = 0;
    const PracticesTableExtractionParams *pPracticeParams = 0;
    if (practice == "CatchCrop") {
        pPracticeParams = &(siteCfg.ccPracticeParams);
        pPracticeAddFiles = &(siteCfg.ccAdditionalFiles);
    } else if (practice == "Fallow") {
        pPracticeParams = &(siteCfg.flPracticeParams);
        pPracticeAddFiles = &(siteCfg.flAdditionalFiles);
    } else if (practice == "NFC") {
        pPracticeParams = &(siteCfg.nfcPracticeParams);
        pPracticeAddFiles = &(siteCfg.nfcAdditionalFiles);
    } else if (practice == "NA") {
        pPracticeParams = &(siteCfg.naPracticeParams);
        pPracticeAddFiles = &(siteCfg.naAdditionalFiles);
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

    if (siteCfg.naAdditionalFiles.size() > 0) {
        retArgs += "-addfiles";
        retArgs += (*pPracticeAddFiles);
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
    if (siteCfg.idsGeomShapePath.size() > 0) {
        retArgs += "-vec";
        retArgs += siteCfg.idsGeomShapePath;
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

QStringList AgricPracticesHandler::GetTimeSeriesAnalysisArgs(const AgricPracticesSiteCfg &siteCfg, const QString &practice,
                                                             const QString &inNdviFile, const QString &inAmpFile, const QString &inCoheFile,
                                                             const QString &outDir)
{
    const TsaPracticeParams *pTsaPracticeParams = 0;
    if (practice == "CatchCrop") {
        pTsaPracticeParams = &(siteCfg.ccTsaParams);
    } else if (practice == "Fallow") {
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
                            "-practice", practice,
                            "-year", siteCfg.year,
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
    return QString("Sen4CAP_L4C_").append(practice).append(siteCfg.country).append("_").append(siteCfg.year).append(".csv");
}

QStringList AgricPracticesHandler::CreateStepsForDataExtraction(const AgricPracticesSiteCfg &siteCfg, const QString &prdType,
                                                                const QStringList &prds, const QString &idsFileName, QList<TaskToSubmit> &allTasksList,
                                                                NewStepList &steps, int &curTaskIdx)
{
    int groups = (prds.size()/siteCfg.prdsPerGroup) + ((prds.size()%siteCfg.prdsPerGroup) == 0 ? 0 : 1);

    // create the tasks for the NDVI, AMP and COHE - they depend only on the ids extraction
    QStringList dataExtrDirs;
    for(int i  = 0; i<groups; i++) {
        TaskToSubmit &dataExtractionTask = allTasksList[curTaskIdx++];
        const QString &taskDataExtrDirName = dataExtractionTask.GetFilePath("");
        dataExtrDirs.append(taskDataExtrDirName);
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
        const QStringList &timeSeriesAnalysisArgs = GetTimeSeriesAnalysisArgs(siteCfg, practice,
                                                                                ndviMergedFile, ampMergedFile, coheMergedFile,
                                                                                timeSeriesExtrDir);
        steps.append(timeSeriesAnalysisTask.CreateStep("TimeSeriesAnalysis", timeSeriesAnalysisArgs));

        // Add the expected files to the productFormatterFiles
        const QString &filesPrefix = "Sen4CAP_L4C_" + practice + "_" + siteCfg.country + "_" + siteCfg.year;
        const QString &mainFileName = filesPrefix + "_CSV.csv";
        const QString &plotFileName = filesPrefix + "_PLOT.csv";
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
                                const JobSubmittedEvent &event, const ProductType &prdType) {
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    std::string prodsInputKey;
    switch(prdType) {
        case ProductType::L3BProductTypeId:
            prodsInputKey = "ndvi_input_products";
            break;
        case ProductType::S4CS1L2AmpProductTypeId:
            prodsInputKey = "amp_input_products";
            break;
        case ProductType::S4CS1L2CoheProductTypeId:
            prodsInputKey = "cohe_input_products";
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

    // get the products from the input_products or based on date_start or date_end
    if(inputProducts.size() == 0) {
        const auto &startDate = QDateTime::fromString(parameters["date_start"].toString(), "yyyyMMdd");
        const auto &endDateStart = QDateTime::fromString(parameters["date_end"].toString(), "yyyyMMdd");
        if(startDate.isValid() && endDateStart.isValid()) {
            // we consider the end of the end date day
            const auto endDate = endDateStart.addSecs(SECONDS_IN_DAY-1);
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
                }
            } else {
                // the S1 AMP and COHE products have directly the path of the tiff in the product table
                listProducts.append(ctx.GetProductAbsolutePath(event.siteId, inputProduct.toString()));
            }
        }
    }

    return listProducts;
}

QString AgricPracticesHandler::FindNdviProductTiffFile(EventProcessingContext &ctx, const JobSubmittedEvent &event, const QString &path)
{
    QFileInfo fileInfo(path);
    QString absPath = path;
    if(!fileInfo.isAbsolute()) {
        // if we have the product name, we need to get the product path from the database
        absPath = ctx.GetProductAbsolutePath(event.siteId, path);
    }
    const QString &laiFileName = ProcessorHandlerHelper::GetHigLevelProductTileFile(absPath, "SNDVI");

    if (laiFileName.isEmpty()) {
        throw std::runtime_error(
            QStringLiteral(
                "Unable to find an HDR or xml file in path %1. Unable to determine the product "
                "metadata file.")
                .arg(absPath)
                .toStdString());
    }
    return laiFileName;
}

