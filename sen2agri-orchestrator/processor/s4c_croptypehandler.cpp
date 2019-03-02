#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "s4c_croptypehandler.hpp"
#include "processorhandlerhelper.h"
#include "logger.hpp"

#define L4A_CT_CFG_PREFIX   "processor.s4c_l4a."

QList<std::reference_wrapper<TaskToSubmit>> S4CCropTypeHandler::CreateTasks(QList<TaskToSubmit> &outAllTasksList) {
    outAllTasksList.append(TaskToSubmit{ "s4c-crop-type", {}} );
    outAllTasksList.append(TaskToSubmit{ "product-formatter", {} });

    // product formatter to wait for the crop type script
     outAllTasksList[1].parentTasks.append(outAllTasksList[0]);

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: outAllTasksList) {
        allTasksListRef.append(task);
    }
    return allTasksListRef;
}

NewStepList S4CCropTypeHandler::CreateSteps(EventProcessingContext &ctx, const JobSubmittedEvent &event,
            QList<TaskToSubmit> &allTasksList, const CropTypeJobConfig &cfg) {
    int curTaskIdx = 0;
    NewStepList allSteps;
    TaskToSubmit &cropTypeTask = allTasksList[curTaskIdx++];
    TaskToSubmit &prdFormatterTask = allTasksList[curTaskIdx++];
    const QString &workingPath = cropTypeTask.GetFilePath("");
    const QString &prdFinalFilesDir = prdFormatterTask.GetFilePath("");

    QStringList corpTypeArgs = GetCropTypeTaskArgs(cfg, prdFinalFilesDir, workingPath);
    allSteps.append(cropTypeTask.CreateStep("S4CCropType", corpTypeArgs));

    const QStringList &productFormatterArgs = GetProductFormatterArgs(prdFormatterTask, ctx, event,
                                                                      prdFinalFilesDir, cfg.startDate, cfg.endDate);
    allSteps.append(prdFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));

    return allSteps;
}

QStringList S4CCropTypeHandler::GetCropTypeTaskArgs(const CropTypeJobConfig &cfg, const QString &prdTargetDir,  const QString &workingPath) {

    QStringList cropTypeArgs = { "-s", QString::number(cfg.siteId),
                                 "--season-start", cfg.startDate.toString("yyyy-MM-dd"),
                                 "--season-end", cfg.endDate.toString("yyyy-MM-dd"),
                                 "--working-path", workingPath,
                                 "--out-path", prdTargetDir,
                                 "--training-ratio", cfg.training_ratio,
                                 "--num-trees",      cfg.num_trees,
                                 "--sample-size",    cfg.sample_size,
                                 "--count-threshold",cfg.count_threshold,
                                 "--count-min",      cfg.count_min,
                                 "--smote-target",   cfg.smote_target,
                                 "--smote-k",        cfg.smote_k,
                                 "--tiles"
                               };

    cropTypeArgs += cfg.tileIds;

    return cropTypeArgs;
}

QStringList S4CCropTypeHandler::GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx,
                                                           const JobSubmittedEvent &event, const QString &tmpPrdDir,
                                                           const QDateTime &minDate, const QDateTime &maxDate) {
    // ProductFormatter /home/cudroiu/sen2agri-processors-build
    //    -vectprd 1 -destroot /mnt/archive_new/test/Sen4CAP_L4C_Tests/NLD_Validation_TSA/OutPrdFormatter
    //    -fileclass OPER -level S4C_L4C -baseline 01.00 -siteid 4 -timeperiod 20180101_20181231 -processor generic
    //    -processor.generic.files <dir_with_list>

    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.txt");
    QString strTimePeriod = minDate.toString("yyyyMMddTHHmmss").append("_").append(maxDate.toString("yyyyMMddTHHmmss"));
    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot", targetFolder,
                                         "-fileclass", "OPER",
                                         "-level", "S4C_L4A",
                                         "-vectprd", "1",
                                         "-baseline", "01.00",
                                         "-siteid", QString::number(event.siteId),
                                         "-timeperiod", strTimePeriod,
                                         "-processor", "generic",
                                         "-outprops", outPropsPath,
                                         "-gipp", executionInfosPath
                                       };
    productFormatterArgs += "-processor.generic.files";
    productFormatterArgs += tmpPrdDir;

    return productFormatterArgs;
}


bool S4CCropTypeHandler::GetStartEndDatesFromProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                             QDateTime &startDate, QDateTime &endDate, QStringList &listTilesMetaFiles) {
    QMap<QString, QStringList> inputProductToTilesMap;
   listTilesMetaFiles = GetL2AInputProductsTiles(ctx, event, inputProductToTilesMap);

    return GetL2AProductsInterval(inputProductToTilesMap, startDate, endDate);
}

bool S4CCropTypeHandler::GetL2AProductsInterval(const QMap<QString, QStringList> &mapTilesMeta,
                                                    QDateTime &startDate, QDateTime &endDate) {
    bool bDatesInitialized = false;
    for(const QString &prd: mapTilesMeta.keys()) {
        const QStringList &listFiles = mapTilesMeta.value(prd);
        if(listFiles.size() > 0) {
            const QString &tileMeta = listFiles.at(0);
            QDateTime dtPrdDate = ProcessorHandlerHelper::GetL2AProductDateFromPath(tileMeta);
            if(!bDatesInitialized) {
                startDate = dtPrdDate;
                endDate = dtPrdDate;
                bDatesInitialized = true;
            } else {
                if(startDate > dtPrdDate) {
                    startDate = dtPrdDate;
                }
                if(endDate < dtPrdDate) {
                    endDate = dtPrdDate;
                }
            }
        }

    }
    return (bDatesInitialized && startDate.isValid() && endDate.isValid());
}



void S4CCropTypeHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                              const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, L4A_CT_CFG_PREFIX);

    CropTypeJobConfig cfg;
    cfg.jobId = event.jobId;
    cfg.siteId = event.siteId;
    cfg.training_ratio = GetStringConfigValue(parameters, configParameters, "training_ratio", L4A_CT_CFG_PREFIX);
    cfg.num_trees = GetStringConfigValue(parameters, configParameters, "num_trees", L4A_CT_CFG_PREFIX);
    cfg.sample_size = GetStringConfigValue(parameters, configParameters, "sample_size", L4A_CT_CFG_PREFIX);
    cfg.count_threshold = GetStringConfigValue(parameters, configParameters, "count_threshold", L4A_CT_CFG_PREFIX);
    cfg.count_min = GetStringConfigValue(parameters, configParameters, "count_min", L4A_CT_CFG_PREFIX);
    cfg.smote_target = GetStringConfigValue(parameters, configParameters, "smote_target", L4A_CT_CFG_PREFIX);
    cfg.smote_k = GetStringConfigValue(parameters, configParameters, "smote_k", L4A_CT_CFG_PREFIX);


    QStringList listProducts;
    bool ret = GetStartEndDatesFromProducts(ctx, event, cfg.startDate, cfg.endDate, listProducts);
    if(!ret || listProducts.size() == 0) {
        // try to get the start and end date if they are given
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No products provided at input or no products available in the specified interval").
                    toStdString());
    }

    cfg.tileIds = GetTileIdsFromProducts(ctx, event, listProducts);

    QList<TaskToSubmit> allTasksList;
    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef = CreateTasks(allTasksList);
    SubmitTasks(ctx, cfg.jobId, allTasksListRef);
    NewStepList allSteps = CreateSteps(ctx, event, allTasksList, cfg);
    ctx.SubmitSteps(allSteps);
}

void S4CCropTypeHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    if (event.module == "product-formatter") {
        QString prodName = GetProductFormatterProductName(ctx, event);
        QString productFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId) + "/" + prodName;
        if(prodName != "") {
            ctx.MarkJobFinished(event.jobId);
            QString quicklook = GetProductFormatterQuicklook(ctx, event);
            QString footPrint = GetProductFormatterFootprint(ctx, event);
            // Insert the product into the database
            QDateTime minDate, maxDate;
            ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prodName, minDate, maxDate);
            ctx.InsertProduct({ ProductType::S4CL4AProductTypeId, event.processorId, event.siteId,
                                event.jobId, productFolder, maxDate, prodName, quicklook,
                                footPrint, std::experimental::nullopt, TileIdList() });

        } else {
            ctx.MarkJobFailed(event.jobId);
            Logger::error(QStringLiteral("Cannot insert into database the product with name %1 and folder %2").arg(prodName).arg(productFolder));
        }
        // Now remove the job folder containing temporary files
        RemoveJobFolder(ctx, event.jobId, "s4c_l4a");
    }
}

ProcessorJobDefinitionParams S4CCropTypeHandler::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues)
{
    ProcessorJobDefinitionParams params;
    params.isValid = false;

    QDateTime seasonStartDate;
    QDateTime seasonEndDate;
    // extract the scheduled date
    QDateTime qScheduledDate = QDateTime::fromTime_t(scheduledDate);
    bool success = GetSeasonStartEndDates(ctx, siteId, seasonStartDate, seasonEndDate, qScheduledDate, requestOverrideCfgValues);
    // if cannot get the season dates
    if(!success) {
        Logger::debug(QStringLiteral("Scheduler CropType: Error getting season start dates for site %1 for scheduled date %2!")
                      .arg(siteId)
                      .arg(qScheduledDate.toString()));
        return params;
    }

    QDateTime limitDate = seasonEndDate.addMonths(2);
    if(qScheduledDate > limitDate) {
        Logger::debug(QStringLiteral("Scheduler CropType: Error scheduled date %1 greater than the limit date %2 for site %3!")
                      .arg(qScheduledDate.toString())
                      .arg(limitDate.toString())
                      .arg(siteId));
        return params;
    }

    ConfigurationParameterValueMap cfgValues = ctx.GetConfigurationParameters("processor.s4c_l4a.", siteId, requestOverrideCfgValues);
    // we might have an offset in days from starting the downloading products to start the S4C L4A production
    int startSeasonOffset = cfgValues["processor.s4c_l4a.start_season_offset"].value.toInt();
    seasonStartDate = seasonStartDate.addDays(startSeasonOffset);

    QDateTime startDate = seasonStartDate;
    QDateTime endDate = qScheduledDate;
    // get the last created Crop Mask
    params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    // Normally, we need at least 1 product available, the crop mask and the shapefile in order to be able to create a S4C L4A product
    // but if we do not return here, the schedule block waiting for products (that might never happen)
    bool waitForAvailProcInputs = (cfgValues["processor.s4c_l4a.sched_wait_proc_inputs"].value.toInt() != 0);
    if((waitForAvailProcInputs == false) || ((params.productList.size() > 0))) {
        params.isValid = true;
        Logger::debug(QStringLiteral("Executing scheduled job. Scheduler extracted for S4C L4A a number "
                                     "of %1 products for site ID %2 with start date %3 and end date %4!")
                      .arg(params.productList.size())
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    } else {
        Logger::debug(QStringLiteral("Scheduled job for S4C L4A and site ID %1 with start date %2 and end date %3 will not be executed "
                                     "(productsNo = %4)!")
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString())
                      .arg(params.productList.size()));
    }

    return params;
}

QStringList S4CCropTypeHandler::GetTileIdsFromProducts(EventProcessingContext &ctx,
                       const JobSubmittedEvent &event, const QStringList &listProducts) {

    const QMap<QString, TileTemporalFilesInfo>  &mapTiles = GroupTiles(ctx, event.siteId, event.jobId, listProducts,
                                   ProductType::L2AProductTypeId);

    // normally, we can use only one list by we want (not necessary) to have the
    // secondary satellite tiles after the main satellite tiles
    QStringList tilesList;
    for(const auto &tileId : mapTiles.keys())
    {
        tilesList.append(tileId);
    }
    return tilesList;
}
