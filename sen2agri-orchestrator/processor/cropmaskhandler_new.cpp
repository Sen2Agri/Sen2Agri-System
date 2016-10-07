#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "cropmaskhandler_new.hpp"
#include "processorhandlerhelper.h"
#include "logger.hpp"

void CropMaskHandlerNew::SetProcessorDescription(const ProcessorDescription &procDescr) {
    this->processorDescr = procDescr;
    m_oldL4BHandler.SetProcessorDescription(procDescr);
}


void CropMaskHandlerNew::GetJobConfig(EventProcessingContext &ctx,const JobSubmittedEvent &event,CropMaskJobConfig &cfg) {
    auto configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l4a.");
    auto resourceParameters = ctx.GetJobConfigurationParameters(event.jobId, "resources.working-mem");
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    cfg.jobId = event.jobId;
    cfg.siteId = event.siteId;
    cfg.resolution = 0;
    if(!GetParameterValueAsInt(parameters, "resolution", cfg.resolution) ||
            cfg.resolution == 0) {
        cfg.resolution = 10;
    }

    cfg.referencePolygons = parameters["reference_polygons"].toString();
    cfg.strataShp = parameters["strata_shape"].toString();
    // if the strata is not set by the user, try to take it from the database
    if(cfg.strataShp.size() == 0) {
        QString siteName = ctx.GetSiteShortName(event.siteId);
        // Get the reference dir
        QString refDir = configParameters["processor.l4a.reference_data_dir"];
        refDir = refDir.replace("{site}", siteName);
        QString strataFile;
        if(ProcessorHandlerHelper::GetStrataFile(refDir, strataFile) &&
                QFile::exists(strataFile)) {
            cfg.strataShp = strataFile;
        }
    }

    cfg.referenceRaster = parameters["reference_raster"].toString();
    // if the reference raster is not set, then initialize it with the reference map
    if(cfg.referenceRaster.size() == 0) {
        cfg.referenceRaster = configParameters["processor.l4a.reference-map"];
    }

    cfg.lutPath = configParameters["processor.l4a.lut_path"];
    cfg.appsMem = resourceParameters["resources.working-mem"];

    cfg.randomSeed = configParameters["processor.l4a.random_seed"];
    if(cfg.randomSeed.isEmpty())  cfg.randomSeed = "0";

    cfg.sampleRatio = configParameters["processor.l4a.sample-ratio"];
    if(cfg.sampleRatio.length() == 0) cfg.sampleRatio = "0.75";

    cfg.temporalResamplingMode = configParameters["processor.l4a.temporal_resampling_mode"];
    if(cfg.temporalResamplingMode != "resample") cfg.temporalResamplingMode = "gapfill";

    cfg.window = configParameters["processor.l4a.window"];
    if(cfg.window.length() == 0) cfg.window = "6";

    cfg.nbcomp = configParameters["processor.l4a.nbcomp"];
    if(cfg.nbcomp.length() == 0) cfg.nbcomp = "6";

    cfg.spatialr = configParameters["processor.l4a.segmentation-spatial-radius"];
    if(cfg.spatialr.length() == 0) cfg.spatialr = "10";

    cfg.ranger = configParameters["processor.l4a.range-radius"];
    if(cfg.ranger.length() == 0) cfg.ranger = "0.65";

    cfg.minsize = configParameters["processor.l4a.segmentation-minsize"];
    if(cfg.minsize.length() == 0) cfg.minsize = "10";

    cfg.minarea = configParameters["processor.l4a.min-area"];
    if(cfg.minarea.length() == 0) cfg.minarea = "20";

    cfg.classifier = configParameters["processor.l4a.classifier"];
    if(cfg.classifier.length() == 0) cfg.classifier = "rf";

    cfg.fieldName = configParameters["processor.l4a.classifier.field"];
    if(cfg.fieldName.length() == 0) cfg.fieldName = "CROP";

    cfg.classifierRfNbTrees = configParameters["processor.l4a.classifier.rf.nbtrees"];
    if(cfg.classifierRfNbTrees.length() == 0) cfg.classifierRfNbTrees = "100";

    cfg.classifierRfMinSamples = configParameters["processor.l4a.classifier.rf.min"];
    if(cfg.classifierRfMinSamples.length() == 0) cfg.classifierRfMinSamples = "25";

    cfg.classifierRfMaxDepth = configParameters["processor.l4a.classifier.rf.max"];
    if(cfg.classifierRfMaxDepth.length() == 0) cfg.classifierRfMaxDepth = "25";

    cfg.classifierSvmKernel = configParameters["processor.l4a.classifier.svm.k"];
    cfg.classifierSvmOptimize = configParameters["processor.l4a.classifier.svm.opt"];

    cfg.nbtrsample = configParameters["processor.l4a.training-samples-number"];
    if(cfg.nbtrsample.length() == 0) cfg.nbtrsample = "4000";

    cfg.lmbd = configParameters["processor.l4a.smoothing-lambda"];
    if(cfg.lmbd.length() == 0) cfg.lmbd = "2";

    cfg.erode_radius = configParameters["processor.l4a.erode-radius"];
    if(cfg.erode_radius.length() == 0) cfg.erode_radius = "1";

    cfg.alpha = configParameters["processor.l4a.mahalanobis-alpha"];
    if(cfg.alpha.length() == 0) cfg.alpha = "0.01";
}

QList<std::reference_wrapper<TaskToSubmit>> CropMaskHandlerNew::CreateTasks(QList<TaskToSubmit> &outAllTasksList) {
    outAllTasksList.append(TaskToSubmit{ "crop-mask-fused", {}} );

    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef;
    for(TaskToSubmit &task: outAllTasksList) {
        allTasksListRef.append(task);
    }
    return allTasksListRef;

}

NewStepList CropMaskHandlerNew::CreateSteps(EventProcessingContext &ctx, QList<TaskToSubmit> &allTasksList, const CropMaskJobConfig &cfg,
                        const QStringList &listProducts) {
    int curTaskIdx = 0;
    NewStepList allSteps;
    TaskToSubmit &cropMaskTask = allTasksList[curTaskIdx++];

    QStringList corpTypeArgs = GetCropTypeTaskArgs(ctx, cfg, listProducts, cropMaskTask);
    allSteps.append(cropMaskTask.CreateStep("CropMaskFused", corpTypeArgs));
    return allSteps;
}

QStringList CropMaskHandlerNew::GetCropTypeTaskArgs(EventProcessingContext &ctx, const CropMaskJobConfig &cfg,
                                    const QStringList &listProducts, TaskToSubmit &cropMaskTask) {

    // perform a first iteration to see the satellites IDs in all tiles
    QList<ProcessorHandlerHelper::SatelliteIdType> satIds;
    // Get the primary satellite id
    ProcessorHandlerHelper::SatelliteIdType primarySatId;
    QMap<QString, TileTemporalFilesInfo>  mapTiles = ProcessorHandlerHelper::GroupTiles(listProducts, satIds, primarySatId);

    const auto &outputDir = cropMaskTask.GetFilePath("");
    const auto &outPropsPath = cropMaskTask.GetFilePath(PRODUCT_FORMATTER_OUT_PROPS_FILE);

    const auto &targetFolder = GetFinalProductFolder(ctx, cfg.jobId, cfg.siteId);
    QStringList cropMaskArgs = { "-ratio", cfg.sampleRatio,
                                 "-nbtrsample", cfg.nbtrsample,
                                 "-classifier", cfg.classifier,
                                 "-rseed", cfg.randomSeed,
                                 "-pixsize", QString::number(cfg.resolution),
                                 "-rfnbtrees", cfg.classifierRfNbTrees,
                                 "-rfmax", cfg.classifierRfMaxDepth,
                                 "-rfmin", cfg.classifierRfMinSamples,
                                 "-window", cfg.window,
                                 "-lmbd", cfg.lmbd,
                                 "-eroderad", cfg.erode_radius,
                                 "-alpha", cfg.alpha,
                                 "-nbcomp", cfg.nbcomp,
                                 "-spatialr", cfg.spatialr,
                                 "-ranger", cfg.ranger,
                                 "-minsize", cfg.minsize,
                                 "-minarea", cfg.minarea,
                                 "-siteid", QString::number(cfg.siteId),
                                 "-outdir", outputDir,
                                 "-targetfolder", targetFolder,
                                 "-outprops", outPropsPath};

    if(cfg.referencePolygons.length() > 0) {
        cropMaskArgs += "-refp";
        cropMaskArgs += cfg.referencePolygons;
    } else if(cfg.referenceRaster.length() > 0) {
        cropMaskArgs += "-refr";
        cropMaskArgs += cfg.referenceRaster;
    }

    QStringList primarySatFiles;
    QStringList secondarySatFiles;
    QStringList primarySatTileNumbers;
    QStringList secondarySatTileNumbers;
    QString mission;
    // first add the products for the primary satellite
    for(auto tileId : mapTiles.keys())
    {
        // We know that we have for each tile only one satellite type
        bool isPrimarySat = false;
        const TileTemporalFilesInfo &listTemporalTiles = mapTiles.value(tileId);
        for(const ProcessorHandlerHelper::InfoTileFile &fileInfo: listTemporalTiles.temporalTilesFileInfos) {
            if(mission.length() == 0) {
                mission = ProcessorHandlerHelper::GetMissionNamePrefixFromSatelliteId(listTemporalTiles.primarySatelliteId);
            }

           if(fileInfo.satId == listTemporalTiles.primarySatelliteId) {
               isPrimarySat = true;
               primarySatFiles.append(fileInfo.file);
           } else {
               secondarySatFiles.append(fileInfo.file);
           }
        }
        if(isPrimarySat) {
            primarySatTileNumbers.append(QString::number(listTemporalTiles.temporalTilesFileInfos.size()));
        } else {
            secondarySatTileNumbers.append(QString::number(listTemporalTiles.temporalTilesFileInfos.size()));
        }
    }
    cropMaskArgs += "-input";
    cropMaskArgs += primarySatFiles;
    cropMaskArgs += secondarySatFiles;

    cropMaskArgs += "-prodspertile";
    cropMaskArgs += primarySatTileNumbers;
    cropMaskArgs += secondarySatTileNumbers;

    cropMaskArgs += "-mission";
    cropMaskArgs += mission;

    if(cfg.strataShp.length() > 0) {
        cropMaskArgs += "-strata";
        cropMaskArgs += cfg.strataShp;
    }

    if(cfg.lutPath.size() > 0) {
        cropMaskArgs += "-lut";
        cropMaskArgs += cfg.lutPath;
    }

    return cropMaskArgs;
}


void CropMaskHandlerNew::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                              const JobSubmittedEvent &event)
{
    QStringList listProducts = GetL2AInputProductsTiles(ctx, event);
    if(listProducts.size() == 0) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("No products provided at input or no products available in the specified interval").
                    toStdString());
    }

    CropMaskJobConfig cfg;
    GetJobConfig(ctx, event, cfg);

    //TODO: This should be removed when the unsupervised mode will be implemented in CropMaskFused.py
    if(cfg.referencePolygons.length() == 0) {
        m_oldL4BHandler.HandleJobSubmitted(ctx, event);
        return;
    }

    QList<TaskToSubmit> allTasksList;
    QList<std::reference_wrapper<TaskToSubmit>> allTasksListRef = CreateTasks(allTasksList);
    SubmitTasks(ctx, cfg.jobId, allTasksListRef);
    NewStepList allSteps = CreateSteps(ctx, allTasksList, cfg, listProducts);
    ctx.SubmitSteps(allSteps);
}

void CropMaskHandlerNew::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    if (event.module == "crop-mask-fused") {
        QString prodName = GetProductFormatterProductName(ctx, event);
        QString productFolder = GetFinalProductFolder(ctx, event.jobId, event.siteId) + "/" + prodName;
        if(prodName != "" && ProcessorHandlerHelper::IsValidHighLevelProduct(productFolder)) {
            ctx.MarkJobFinished(event.jobId);
            QString quicklook = GetProductFormatterQuicklook(ctx, event);
            QString footPrint = GetProductFormatterFootprint(ctx, event);
            // Insert the product into the database
            QDateTime minDate, maxDate;
            ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prodName, minDate, maxDate);
            ctx.InsertProduct({ ProductType::L4AProductTypeId, event.processorId,
                                event.siteId, event.jobId, productFolder, maxDate,
                                prodName, quicklook, footPrint, std::experimental::nullopt, TileList() });
        } else {
            ctx.MarkJobFailed(event.jobId);
            Logger::error(QStringLiteral("Cannot insert into database the product with name %1 and folder %2").arg(prodName).arg(productFolder));
        }
        // Now remove the job folder containing temporary files
        RemoveJobFolder(ctx, event.jobId, "l4a");
    } else {
        // check if the message can be handled by the old handler
        //TODO: This should be removed when the unsupervised mode will be implemented in CropMaskFused.py
        m_oldL4BHandler.HandleTaskFinished(ctx, event);
    }
}

ProcessorJobDefinitionParams CropMaskHandlerNew::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
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

    ConfigurationParameterValueMap cfgValues = ctx.GetConfigurationParameters("processor.l4a.", siteId, requestOverrideCfgValues);
    QString siteName = ctx.GetSiteShortName(siteId);
    // Get the reference dir
    QString refMap = cfgValues["processor.l4a.reference-map"].value;
    QString refDir = cfgValues["processor.l4a.reference_data_dir"].value;
    refDir = refDir.replace("{site}", siteName);

    // we might have an offset in days from starting the downloading products to start the L4A production
    int startSeasonOffset = cfgValues["processor.l4a.start_season_offset"].value.toInt();
    seasonStartDate = seasonStartDate.addDays(startSeasonOffset);

    QString shapeFile;
    QString referenceRasterFile;
    QString strataShapeFile;
    // if none of the reference files were found, cannot run the CropMask
    if(!ProcessorHandlerHelper::GetCropReferenceFile(refDir, shapeFile, referenceRasterFile, strataShapeFile) && !QFile::exists(refMap)) {
        return params;
    }
    if (!shapeFile.isEmpty()) {
        params.jsonParameters = "{ \"reference_polygons\": \"" + shapeFile + "\"";
    } else if (!refMap.isEmpty()) {
        params.jsonParameters = "{ \"reference_raster\": \"" + refMap + "\"";
    } else {
        params.jsonParameters = "{ \"reference_raster\": \"" + referenceRasterFile + "\"";
    }
    if(!strataShapeFile.isEmpty()) {
        params.jsonParameters.append(", \"strata_shape\": \"" + strataShapeFile + "\"}");
    } else {
        params.jsonParameters.append("}");
    }

    // Get the start and end date for the production
    QDateTime endDate = qScheduledDate;
    QDateTime startDate = seasonStartDate;

    // get from the database the moving window for 1 year periodicity
    QString movingWindowStr = cfgValues["processor.l4a.moving_window_value"].value;
    int movingWindow = 0;
    if(movingWindowStr.length() > 0) {
        movingWindow = movingWindowStr.toInt();
    } else {
        movingWindow = 12;
    }
    // we have monthly production so we use the season date or the moving window of 12 months (or specified)
    if(endDate.addMonths(-movingWindow) > seasonStartDate) {
        startDate = endDate.addMonths(-movingWindow);
    } else {
        startDate = seasonStartDate;
    }

    params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    // Normally, we need at least 1 product available in order to be able to create a L4A product
    // but if we do not return here, the schedule block waiting for products (that might never happen)
    bool waitForAvailProcInputs = (cfgValues["processor.l4a.sched_wait_proc_inputs"].value.toInt() != 0);
    if((waitForAvailProcInputs == false) || (params.productList.size() > 0)) {
        params.isValid = true;
        Logger::debug(QStringLiteral("Executing scheduled job. Scheduler extracted for L4A a number "
                                     "of %1 products for site ID %2 with start date %3 and end date %4!")
                      .arg(params.productList.size())
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    } else {
        Logger::debug(QStringLiteral("Scheduled job for L4A and site ID %1 with start date %2 and end date %3 will not be executed (no products)!")
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    }

    return params;
}

