#pragma once

#include "processorhandler.hpp"

typedef struct {

    NewStepList steps;
    QList<std::reference_wrapper<const TaskToSubmit>> parentsTasksRef;
    // args
    QString cropTypeMap;
    QString xmlValidationMetrics;
    QString statusFlags;
    QString tileId;
} CropTypeProductFormatterParams;

typedef struct {
    QList<TaskToSubmit> allTasksList;
    NewStepList allStepsList;
    CropTypeProductFormatterParams prodFormatParams;
} CropTypeGlobalExecutionInfos;

class CropTypeHandler : public ProcessorHandler
{
private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    QList<std::reference_wrapper<TaskToSubmit>> CreateTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                                    QList<std::reference_wrapper<const TaskToSubmit>> &outProdFormatterParentsList,
                                                    bool bCropMaskEmpty);
    void HandleNewTilesList(EventProcessingContext &ctx,
                            const JobSubmittedEvent &event, const QStringList &listProducts,
                            const QString &cropMask, CropTypeGlobalExecutionInfos &globalExecInfos);
    QStringList GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                        const QStringList &listProducts, const QList<CropTypeProductFormatterParams> &productParams);

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;

};
