#pragma once

#include "processorhandler.hpp"

class CropTypeHandler : public ProcessorHandler
{
    typedef struct {

        NewStepList steps;
        QList<std::reference_wrapper<const TaskToSubmit>> parentsTasksRef;
        // args
        QString cropTypeMap;
        QString rawCropTypeMap;
        QString xmlValidationMetrics;
        QString statusFlags;
        QString tileId;
    } CropTypeProductFormatterParams;

    typedef struct {
        QList<TaskToSubmit> allTasksList;
        NewStepList allStepsList;
        CropTypeProductFormatterParams prodFormatParams;
    } CropTypeGlobalExecutionInfos;

    typedef struct {
        int jobId;
        int siteId;
        int resolution;

        QString referencePolygons;
        QString cropMask;

        QString lutPath;
        QString appsMem;

        QString randomSeed;
        QString temporalResamplingMode;
        QString sampleRatio;

        QString classifier;
        QString fieldName;
        QString classifierRfNbTrees;
        QString classifierRfMinSamples;
        QString classifierRfMaxDepth;
        QString classifierSvmKernel;
        QString classifierSvmOptimize;

    } CropTypeJobConfig;

private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    QList<std::reference_wrapper<TaskToSubmit>> CreateMaskTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                                    QList<std::reference_wrapper<const TaskToSubmit>> &outProdFormatterParentsList);
    QList<std::reference_wrapper<TaskToSubmit>> CreateNoMaskTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                                    QList<std::reference_wrapper<const TaskToSubmit>> &outProdFormatterParentsList);

    void HandleNewTilesList(EventProcessingContext &ctx,
                            const CropTypeJobConfig &cfg, const TileTemporalFilesInfo &tileTemporalFilesInfo,
                            const QString &cropMask, CropTypeGlobalExecutionInfos &globalExecInfos);
    void HandleMaskTilesList(EventProcessingContext &ctx,
                            const CropTypeJobConfig &cfg, const TileTemporalFilesInfo &tileTemporalFilesInfo,
                            const QString &cropMask, CropTypeGlobalExecutionInfos &globalExecInfos);
    void HandleNoMaskTilesList(EventProcessingContext &ctx,
                            const CropTypeJobConfig &cfg, const TileTemporalFilesInfo &tileTemporalFilesInfo,
                            CropTypeGlobalExecutionInfos &globalExecInfos);

    QStringList GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const CropTypeJobConfig &cfg,
                                        const QStringList &listProducts, const QList<CropTypeProductFormatterParams> &productParams);

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;
    void GetJobConfig(EventProcessingContext &ctx,const JobSubmittedEvent &event,CropTypeJobConfig &cfg);
    void WriteExecutionInfosFile(const QString &executionInfosPath, const CropTypeJobConfig &cfg, const QStringList &listProducts);
};
