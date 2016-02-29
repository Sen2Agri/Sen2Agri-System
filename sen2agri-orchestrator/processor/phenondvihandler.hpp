#ifndef PHENONDVIHANDLER_HPP
#define PHENONDVIHANDLER_HPP

#include "processorhandler.hpp"

typedef struct {

    NewStepList steps;
    QList<std::reference_wrapper<const TaskToSubmit>> parentsTasksRef;
    // args
    QString metricsParamsImg;
    QString metricsFlagsImg;
    QString tileId;
} PhenoProductFormatterParams;

typedef struct {
    QList<TaskToSubmit> allTasksList;
    NewStepList allStepsList;
    PhenoProductFormatterParams prodFormatParams;
} PhenoGlobalExecutionInfos;

class PhenoNdviHandler : public ProcessorHandler
{
private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    void CreateTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                            QList<std::reference_wrapper<const TaskToSubmit>> &outProdFormatterParentsList);
    PhenoGlobalExecutionInfos HandleNewTilesList(EventProcessingContext &ctx, const JobSubmittedEvent &event, const QStringList &listProducts);
    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                 const QStringList &listProducts);
    QStringList GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                        const QStringList &listProducts, const QList<PhenoProductFormatterParams> &productParams);

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;
};

#endif // PHENONDVIHANDLER_HPP
