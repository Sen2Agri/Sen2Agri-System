#pragma once

#include "processorhandler.hpp"
#include "optional.hpp"

class S4CCropTypeHandler : public ProcessorHandler
{
    typedef struct {
        int jobId;
        int siteId;
        QDateTime startDate;
        QDateTime endDate;
        QStringList tileIds;

        QString training_ratio;
        QString num_trees;
        QString sample_size;
        QString count_threshold;
        QString count_min;
        QString smote_target;
        QString smote_k;

    } CropTypeJobConfig;

private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;
    QList<std::reference_wrapper<TaskToSubmit>> CreateTasks(QList<TaskToSubmit> &outAllTasksList);
    NewStepList CreateSteps(EventProcessingContext &ctx, const JobSubmittedEvent &event, QList<TaskToSubmit> &allTasksList,
                            const CropTypeJobConfig &cfg);
    QStringList GetCropTypeTaskArgs(const CropTypeJobConfig &cfg, const QString &prdTargetDir,  const QString &workingPath);
    QStringList GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event, const QString &tmpPrdDir,
                                        const QDateTime &minDate, const QDateTime &maxDate);

    bool GetL2AProductsInterval(const QMap<QString, QStringList> &mapTilesMeta,
                                QDateTime &startDate, QDateTime &endDate);
    bool GetStartEndDatesFromProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                      QDateTime &startDate, QDateTime &endDate, QStringList &listTilesMetaFiles);
    QStringList GetTileIdsFromProducts(EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event, const QStringList &listProducts);
};
