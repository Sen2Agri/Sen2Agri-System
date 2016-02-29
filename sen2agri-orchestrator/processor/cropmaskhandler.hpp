#pragma once

#include "processorhandler.hpp"

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

class CropMaskHandler : public ProcessorHandler
{
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

    CropMaskGlobalExecutionInfos HandleNewTilesList(EventProcessingContext &ctx,
                           const JobSubmittedEvent &event, const QStringList &listProducts);
    void HandleInsituJob(EventProcessingContext &ctx, const JobSubmittedEvent &event, const QStringList &listProducts,
                         CropMaskGlobalExecutionInfos &globalExecInfos);
    void HandleNoInsituJob(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                   const QStringList &listProducts, CropMaskGlobalExecutionInfos &globalExecInfos);

    QStringList GetBandsExtractorArgs(const QString &mission, const QString &outImg, const QString &mask, const QString &statusFlags,
                                        const QString &outDates, const QString &shape, const QStringList &inputProducts, int resolution);
    QStringList GetCompressionArgs(const QString &inImg, const QString &outImg);
    QStringList GetConfusionMatrixArgs(const QString &inRaster, const QString &outCsv, const QString &refIn, const QString &refType="raster",
                                       const QString &refField = "");
    QStringList GetGdalWarpArgs(const QString &inImg, const QString &outImg, const QString &dtsNoData,
                                const QString &gdalwarpMem, const QString &shape, const QString &resolutionStr="");
    QStringList GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                        const QStringList &listProducts, const QList<CropMaskProductFormatterParams> &productParams);
};
