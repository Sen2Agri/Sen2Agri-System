#pragma once

#include "processorhandler.hpp"

typedef struct {
    int jobId;
    int siteId;
    int resolution;

    QString referencePolygons;
    QString referenceRaster;
    QString strataShp;

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

class CropMaskHandlerNew : public ProcessorHandler
{
private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;

    void GetJobConfig(EventProcessingContext &ctx,const JobSubmittedEvent &event,CropMaskJobConfig &cfg);
    QList<std::reference_wrapper<TaskToSubmit>> CreateTasks(QList<TaskToSubmit> &outAllTasksList);
    NewStepList CreateSteps(EventProcessingContext &ctx, QList<TaskToSubmit> &allTasksList, const CropMaskJobConfig &cfg,
                            const QStringList &listProducts);
    QStringList GetCropTypeTaskArgs(EventProcessingContext &ctx, const CropMaskJobConfig &cfg,
                                        const QStringList &listProducts, TaskToSubmit &cropMaskTask);
};
