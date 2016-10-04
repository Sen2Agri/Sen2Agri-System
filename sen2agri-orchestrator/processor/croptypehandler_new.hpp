#pragma once

#include "processorhandler.hpp"

class CropTypeHandlerNew : public ProcessorHandler
{
    typedef struct {
        int jobId;
        int siteId;
        int resolution;

        QString referencePolygons;
        QString cropMask;

        QString appsMem;
        QString lutPath;

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

        QString strataShp;

    } CropTypeJobConfig;

private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;
    void GetJobConfig(EventProcessingContext &ctx,const JobSubmittedEvent &event,CropTypeJobConfig &cfg);
    QList<std::reference_wrapper<TaskToSubmit>> CreateTasks(QList<TaskToSubmit> &outAllTasksList);
    NewStepList CreateSteps(EventProcessingContext &ctx, QList<TaskToSubmit> &allTasksList, const CropTypeJobConfig &cfg,
                            const QStringList &listProducts);
    QStringList GetCropTypeTaskArgs(EventProcessingContext &ctx, const CropTypeJobConfig &cfg,
                                        const QStringList &listProducts, TaskToSubmit &cropTypeTask);
};
