#pragma once

#include "processorhandler.hpp"

class CropMaskHandler : public ProcessorHandler
{
    typedef struct {

        NewStepList steps;
        QList<std::reference_wrapper<const TaskToSubmit>> parentsTasksRef;
        // args
        QString crop_mask;
        QString raw_crop_mask;
        QString xml_validation_metrics;
        QString statusFlags;

        QString tileId;
    } CropMaskProductFormatterParams;

    typedef struct {
        QList<TaskToSubmit> allTasksList;
        NewStepList allStepsList;
        CropMaskProductFormatterParams prodFormatParams;
    } CropMaskGlobalExecutionInfos;

    typedef struct {
        int jobId;
        int siteId;
        int resolution;

        QString referencePolygons;
        QString referenceRaster;

        QString lutPath;
        QString appsMem;

        QString randomSeed;
        QString sampleRatio;
        QString temporalResamplingMode;
        QString window;
        QString nbcomp;
        QString spatialr;
        QString ranger;
        QString minsize;
        QString minarea;
        QString classifier;
        QString fieldName;
        QString classifierRfNbTrees;
        QString classifierRfMinSamples;
        QString classifierRfMaxDepth;
        QString classifierSvmKernel;
        QString classifierSvmOptimize;

        QString nbtrsample;
        QString lmbd;
        QString erode_radius;
        QString alpha;

    } CropMaskJobConfig;

private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;

    QList<std::reference_wrapper<TaskToSubmit> > CreateInSituTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                        QList<std::reference_wrapper<const TaskToSubmit>> &outProdFormatterParentsList);
    QList<std::reference_wrapper<TaskToSubmit> > CreateNoInSituTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                        QList<std::reference_wrapper<const TaskToSubmit>> &outProdFormatterParentsList);

    void HandleNewTilesList(EventProcessingContext &ctx, const CropMaskJobConfig &cfg, const TileTemporalFilesInfo &tileTemporalFilesInfo, CropMaskGlobalExecutionInfos &globalExecInfos);
    void HandleInsituJob(const CropMaskJobConfig &cfg, const TileTemporalFilesInfo &tileTemporalFilesInfo, const QStringList &listProducts,
                         CropMaskGlobalExecutionInfos &globalExecInfos);
    void HandleNoInsituJob(const CropMaskJobConfig &cfg, const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                                   const QStringList &listProducts, CropMaskGlobalExecutionInfos &globalExecInfos);

    QStringList GetQualityFlagsExtractorArgs(const QString &mission, const QString &statusFlags, const QStringList &inputProducts, int resolution);
    QStringList GetBandsExtractorArgs(const QString &mission, const QString &outImg, const QString &mask,
                                        const QString &outDates, const QString &shape, const QStringList &inputProducts, int resolution);
    QStringList GetCompressionArgs(const QString &inImg, const QString &outImg);
    QStringList GetConfusionMatrixArgs(const QString &inRaster, const QString &outCsv, const QString &refIn, const QString &refType="raster",
                                       const QString &refField = "");
    QStringList GetGdalWarpArgs(const QString &inImg, const QString &outImg, const QString &dtsNoData,
                                const QString &gdalwarpMem, const QString &shape, const QString &resolutionStr="");
    QStringList GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const CropMaskJobConfig &cfg,
                                        const QStringList &listProducts, const QList<CropMaskProductFormatterParams> &productParams);
    void WriteExecutionInfosFile(const QString &executionInfosPath, const CropMaskJobConfig &cfg, const QStringList &listProducts);
    void GetJobConfig(EventProcessingContext &ctx,const JobSubmittedEvent &event,CropMaskJobConfig &cfg);
};
