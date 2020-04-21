#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "logger.hpp"
#include "processorhandlerhelper.h"
#include "s4c_croptypehandler.hpp"

#define L4A_CT_CFG_PREFIX "processor.s4c_l4a."

S4CCropTypeHandler::S4CCropTypeHandler()
{
    m_cfgKeys = QStringList() << "lc"
                              << "min-s2-pix"
                              << "min-s1-pix"
                              << "best-s2-pix"
                              << "pa-min"
                              << "pa-train-h"
                              << "pa-train-l"
                              << "sample-ratio-h"
                              << "sample-ratio-l"
                              << "smote-target"
                              << "smote-k"
                              << "num-trees"
                              << "min-node-size"
                              << "mode";
}

QList<std::reference_wrapper<TaskToSubmit>>
S4CCropTypeHandler::CreateTasks(QList<TaskToSubmit> &outAllTasksList)
{
    outAllTasksList.append(TaskToSubmit{ "s4c-crop-type", {} });
    outAllTasksList.append(TaskToSubmit{ "product-formatter", {} });

    // product formatter to wait for the crop type script
    outAllTasksList[1].parentTasks.append(outAllTasksList[0]);
    outAllTasksList.append(TaskToSubmit{ "export-product-launcher", { outAllTasksList[1] } });

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for (TaskToSubmit &task : outAllTasksList) {
        allTasksListRef.append(task);
    }
    return allTasksListRef;
}

NewStepList S4CCropTypeHandler::CreateSteps(EventProcessingContext &ctx,
                                            const JobSubmittedEvent &event,
                                            QList<TaskToSubmit> &allTasksList,
                                            const CropTypeJobConfig &cfg)
{
    int curTaskIdx = 0;
    NewStepList allSteps;
    TaskToSubmit &cropTypeTask = allTasksList[curTaskIdx++];
    TaskToSubmit &prdFormatterTask = allTasksList[curTaskIdx++];
    const QString &workingPath = cropTypeTask.GetFilePath("");
    const QString &prdFinalFilesDir = prdFormatterTask.GetFilePath("");

    QStringList corpTypeArgs = GetCropTypeTaskArgs(cfg, prdFinalFilesDir, workingPath);
    allSteps.append(cropTypeTask.CreateStep("S4CCropType", corpTypeArgs));

    const QStringList &productFormatterArgs = GetProductFormatterArgs(
        prdFormatterTask, ctx, event, prdFinalFilesDir, cfg.startDate, cfg.endDate);
    allSteps.append(prdFormatterTask.CreateStep("ProductFormatter", productFormatterArgs));

    const auto &productFormatterPrdFileIdFile = prdFormatterTask.GetFilePath("prd_infos.txt");
    TaskToSubmit &exportCsvToShpProductTask = allTasksList[curTaskIdx++];
    const QStringList &exportCsvToShpProductArgs = { "-f", productFormatterPrdFileIdFile, "-o",
                                                     "CropType.gpkg" };
    allSteps.append(
        exportCsvToShpProductTask.CreateStep("export-product-launcher", exportCsvToShpProductArgs));

    return allSteps;
}

QStringList S4CCropTypeHandler::GetCropTypeTaskArgs(const CropTypeJobConfig &cfg,
                                                    const QString &prdTargetDir,
                                                    const QString &workingPath)
{

    QStringList cropTypeArgs = { "-s",
                                 QString::number(cfg.siteId),
                                 "--season-start",
                                 cfg.startDate.toString("yyyy-MM-dd"),
                                 "--season-end",
                                 cfg.endDate.toString("yyyy-MM-dd"),
                                 "--working-path",
                                 workingPath,
                                 "--out-path",
                                 prdTargetDir,
                                 "--tiles" };

    cropTypeArgs += cfg.tileIds;

    for (const QString &cfgKey : m_cfgKeys) {
        auto it = cfg.mapCfgValues.find(cfgKey);
        if (it != cfg.mapCfgValues.end()) {
            const QString &value = it.value();
            if (value.size() > 0) {
                cropTypeArgs += QString("--").append(cfgKey);
                cropTypeArgs += value;
            }
        }
    }

    return cropTypeArgs;
}

QStringList S4CCropTypeHandler::GetProductFormatterArgs(TaskToSubmit &productFormatterTask,
                                                        EventProcessingContext &ctx,
                                                        const JobSubmittedEvent &event,
                                                        const QString &tmpPrdDir,
                                                        const QDateTime &minDate,
                                                        const QDateTime &maxDate)
{
    // ProductFormatter /home/cudroiu/sen2agri-processors-build
    //    -vectprd 1 -destroot
    //    /mnt/archive_new/test/Sen4CAP_L4C_Tests/NLD_Validation_TSA/OutPrdFormatter -fileclass OPER
    //    -level S4C_L4C -baseline 01.00 -siteid 4 -timeperiod 20180101_20181231 -processor generic
    //    -processor.generic.files <dir_with_list>

    const auto &targetFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId);
    const auto &outPropsPath = productFormatterTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);
    const auto &executionInfosPath = productFormatterTask.GetFilePath("executionInfos.txt");
    QString strTimePeriod =
        minDate.toString("yyyyMMddTHHmmss").append("_").append(maxDate.toString("yyyyMMddTHHmmss"));
    QStringList productFormatterArgs = { "ProductFormatter",
                                         "-destroot",
                                         targetFolder,
                                         "-fileclass",
                                         "OPER",
                                         "-level",
                                         "S4C_L4A",
                                         "-vectprd",
                                         "1",
                                         "-baseline",
                                         "01.00",
                                         "-siteid",
                                         QString::number(event.siteId),
                                         "-timeperiod",
                                         strTimePeriod,
                                         "-processor",
                                         "generic",
                                         "-outprops",
                                         outPropsPath,
                                         "-gipp",
                                         executionInfosPath };
    productFormatterArgs += "-processor.generic.files";
    productFormatterArgs += tmpPrdDir;

    return productFormatterArgs;
}

bool S4CCropTypeHandler::GetStartEndDatesFromProducts(EventProcessingContext &ctx,
                                                      const JobSubmittedEvent &event,
                                                      QDateTime &startDate,
                                                      QDateTime &endDate,
                                                      QStringList &listTilesMetaFiles)
{
    QMap<QString, QStringList> inputProductToTilesMap;
    listTilesMetaFiles = GetL2AInputProductsTiles(ctx, event, inputProductToTilesMap);

    return GetL2AProductsInterval(inputProductToTilesMap, startDate, endDate);
}

bool S4CCropTypeHandler::GetL2AProductsInterval(const QMap<QString, QStringList> &mapTilesMeta,
                                                QDateTime &startDate,
                                                QDateTime &endDate)
{
    bool bDatesInitialized = false;
    for (const QString &prd : mapTilesMeta.keys()) {
        const QStringList &listFiles = mapTilesMeta.value(prd);
        if (listFiles.size() > 0) {
            const QString &tileMeta = listFiles.at(0);
            const QDateTime &dtPrdDate =
                ProcessorHandlerHelper::GetL2AProductDateFromPath(tileMeta);
            if (!bDatesInitialized) {
                startDate = dtPrdDate;
                endDate = dtPrdDate;
                bDatesInitialized = true;
            } else {
                if (startDate > dtPrdDate) {
                    startDate = dtPrdDate;
                }
                if (endDate < dtPrdDate) {
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
    std::map<QString, QString> configParameters =
        ctx.GetJobConfigurationParameters(event.jobId, L4A_CT_CFG_PREFIX);

    CropTypeJobConfig cfg;
    cfg.jobId = event.jobId;
    cfg.siteId = event.siteId;

    for (QString cfgKey : m_cfgKeys) {
        cfg.mapCfgValues.insert(
            cfgKey, ProcessorHandlerHelper::GetStringConfigValue(parameters, configParameters,
                                                                 cfgKey, L4A_CT_CFG_PREFIX));
    }

    QStringList listProducts;
    bool ret = GetStartEndDatesFromProducts(ctx, event, cfg.startDate, cfg.endDate, listProducts);
    if (!ret || listProducts.size() == 0) {
        // try to get the start and end date if they are given
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral(
                "No products provided at input or no products available in the specified interval")
                .toStdString());
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
        QString productFolder =
            GetFinalProductFolder(ctx, event.jobId, event.siteId) + "/" + prodName;
        if (prodName != "") {
            QString quicklook = GetProductFormatterQuicklook(ctx, event);
            QString footPrint = GetProductFormatterFootprint(ctx, event);
            // Insert the product into the database
            QDateTime minDate, maxDate;
            ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prodName, minDate, maxDate);
            int prdId = ctx.InsertProduct({ ProductType::S4CL4AProductTypeId, event.processorId,
                                            event.siteId, event.jobId, productFolder, maxDate,
                                            prodName, quicklook, footPrint,
                                            std::experimental::nullopt, TileIdList() });
            const QString &prodFolderOutPath =
                ctx.GetOutputPath(event.jobId, event.taskId, event.module,
                                  processorDescr.shortName) +
                "/" + "prd_infos.txt";

            QFile file(prodFolderOutPath);
            if (file.open(QIODevice::ReadWrite)) {
                QTextStream stream(&file);
                stream << prdId << ";" << productFolder << endl;
            }
        } else {
            ctx.MarkJobFailed(event.jobId);
            Logger::error(
                QStringLiteral("Cannot insert into database the product with name %1 and folder %2")
                    .arg(prodName)
                    .arg(productFolder));
        }
    } else if (event.module == "s4c-crop-type") {
        const auto &tasks =ctx.GetJobTasksByStatus(event.jobId, {ExecutionStatus::Error});
        if (tasks.size() > 0) {
            ctx.MarkJobFailed(event.jobId);
            Logger::error(
                QStringLiteral("Crop type step for job %1 failed. Marking job as failed ...")
                    .arg(event.jobId));
            RemoveJobFolder(ctx, event.jobId, processorDescr.shortName);
        }
    } else if (event.module == "export-product-launcher") {
        ctx.MarkJobFinished(event.jobId);
        // Now remove the job folder containing temporary files
        RemoveJobFolder(ctx, event.jobId, processorDescr.shortName);
    }
}

ProcessorJobDefinitionParams S4CCropTypeHandler::GetProcessingDefinitionImpl(
    SchedulingContext &ctx,
    int siteId,
    int scheduledDate,
    const ConfigurationParameterValueMap &requestOverrideCfgValues)
{
    ProcessorJobDefinitionParams params;
    params.isValid = false;

    QDateTime seasonStartDate;
    QDateTime seasonEndDate;
    // extract the scheduled date
    QDateTime qScheduledDate = QDateTime::fromTime_t(scheduledDate);
    bool success = GetSeasonStartEndDates(ctx, siteId, seasonStartDate, seasonEndDate,
                                          qScheduledDate, requestOverrideCfgValues);
    // if cannot get the season dates
    if (!success) {
        Logger::debug(QStringLiteral("Scheduler CropType: Error getting season start dates for "
                                     "site %1 for scheduled date %2!")
                          .arg(siteId)
                          .arg(qScheduledDate.toString()));
        return params;
    }

    QDateTime limitDate = seasonEndDate.addMonths(2);
    if (qScheduledDate > limitDate) {
        Logger::debug(QStringLiteral("Scheduler CropType: Error scheduled date %1 greater than the "
                                     "limit date %2 for site %3!")
                          .arg(qScheduledDate.toString())
                          .arg(limitDate.toString())
                          .arg(siteId));
        return params;
    }

    ConfigurationParameterValueMap cfgValues =
        ctx.GetConfigurationParameters("processor.s4c_l4a.", siteId, requestOverrideCfgValues);
    // we might have an offset in days from starting the downloading products to start the S4C L4A
    // production
    int startSeasonOffset = cfgValues["processor.s4c_l4a.start_season_offset"].value.toInt();
    seasonStartDate = seasonStartDate.addDays(startSeasonOffset);

    QDateTime startDate = seasonStartDate;
    QDateTime endDate = qScheduledDate;
    // get the last created Crop Mask
    params.productList =
        ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    // Normally, we need at least 1 product available, the crop mask and the shapefile in order to
    // be able to create a S4C L4A product but if we do not return here, the schedule block waiting
    // for products (that might never happen)
    bool waitForAvailProcInputs =
        (cfgValues["processor.s4c_l4a.sched_wait_proc_inputs"].value.toInt() != 0);
    if ((waitForAvailProcInputs == false) || ((params.productList.size() > 0))) {
        params.isValid = true;
        Logger::debug(
            QStringLiteral("Executing scheduled job. Scheduler extracted for S4C L4A a number "
                           "of %1 products for site ID %2 with start date %3 and end date %4!")
                .arg(params.productList.size())
                .arg(siteId)
                .arg(startDate.toString())
                .arg(endDate.toString()));
    } else {
        Logger::debug(QStringLiteral("Scheduled job for S4C L4A and site ID %1 with start date %2 "
                                     "and end date %3 will not be executed "
                                     "(productsNo = %4)!")
                          .arg(siteId)
                          .arg(startDate.toString())
                          .arg(endDate.toString())
                          .arg(params.productList.size()));
    }

    return params;
}

QStringList S4CCropTypeHandler::GetTileIdsFromProducts(EventProcessingContext &ctx,
                                                       const JobSubmittedEvent &event,
                                                       const QStringList &listProducts)
{

    const QMap<QString, TileTemporalFilesInfo> &mapTiles =
        GroupTiles(ctx, event.siteId, event.jobId, listProducts, ProductType::L2AProductTypeId);

    // normally, we can use only one list by we want (not necessary) to have the
    // secondary satellite tiles after the main satellite tiles
    QStringList tilesList;
    for (const auto &tileId : mapTiles.keys()) {
        tilesList.append(tileId);
    }
    return tilesList;
}
