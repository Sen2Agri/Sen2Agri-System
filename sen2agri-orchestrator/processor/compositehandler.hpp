#ifndef COMPOSITEHANDLER_HPP
#define COMPOSITEHANDLER_HPP

#include "processorhandler.hpp"

typedef struct {

    NewStepList steps;
    QList<std::reference_wrapper<const TaskToSubmit>> parentsTasksRef;
    // args
    QString prevL3AProdRefls;
    QString prevL3AProdWeights;
    QString prevL3AProdFlags;
    QString prevL3AProdDates;
    QString prevL3ARgbFile;
    QString tileId;
} CompositeProductFormatterParams;

typedef struct {
    QList<TaskToSubmit> allTasksList;
    NewStepList allStepsList;
    CompositeProductFormatterParams prodFormatParams;
} CompositeGlobalExecutionInfos;

class CompositeHandler : public ProcessorHandler
{
private:
    void HandleProductAvailableImpl(EventProcessingContext &ctx,
                                    const ProductAvailableEvent &event) override;
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    CompositeGlobalExecutionInfos HandleNewTilesList(EventProcessingContext &ctx,
                               const JobSubmittedEvent &event,
                               const QStringList &listProducts);
    bool IsProductAcceptableForJob(int jobId, const ProductAvailableEvent &event);
    void FilterInputProducts(QStringList &listFiles, int productDate, int halfSynthesis);

    void CreateTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList, QList<std::reference_wrapper<const TaskToSubmit> > &outProdFormatterParentsList, int nbProducts);
    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                 const QJsonObject &parameters,
                                 std::map<QString, QString> &configParameters,
                                 const QStringList &listProducts);
    QStringList GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                        const QStringList &listProducts, const QList<CompositeProductFormatterParams> &productParams);

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;

};

#endif // COMPOSITEHANDLER_HPP
