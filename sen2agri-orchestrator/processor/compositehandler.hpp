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

typedef struct {
    int jobId;
    int siteId;
    int resolution;

    QString lutPath;

    bool bGenerate20MS2Res;

    QString l3aSynthesisDate;
    QString synthalf;
    QString bandsMapping;
    QString scatCoeffs10M;
    QString scatCoeffs20M;
    QString weightAOTMin;
    QString weightAOTMax;
    QString AOTMax;
    QString coarseRes;
    QString sigmaSmallCloud;
    QString sigmaLargeCloud;
    QString weightDateMin;

    QString shapeFilesFolder;

    bool keepJobFiles;

    std::map<QString, QString> allCfgMap;

} CompositeJobConfig;

class CompositeHandler : public ProcessorHandler
{
private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    void HandleNewTilesList(EventProcessingContext &ctx,
                            const CompositeJobConfig &cfg,
                            const TileTemporalFilesInfo &tileTemporalFilesInfo, CompositeGlobalExecutionInfos &globalExecInfos, int resolution);
    bool IsProductAcceptableForJob(int jobId, const ProductAvailableEvent &event);
    void FilterInputProducts(QStringList &listFiles, int productDate, int halfSynthesis);

    void CreateTasksForNewProducts(const CompositeJobConfig &cfg, QList<TaskToSubmit> &outAllTasksList, QList<std::reference_wrapper<const TaskToSubmit> > &outProdFormatterParentsList, const TileTemporalFilesInfo &tileTemporalFilesInfo);
    void WriteExecutionInfosFile(const QString &executionInfosPath, const CompositeJobConfig &cfg,
                                 const QStringList &listProducts);
    QStringList GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const CompositeJobConfig &cfg,
                                        const QStringList &listProducts, const QList<CompositeProductFormatterParams> &productParams);

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;

    QStringList GetMissionsFromBandsMapping(const QString &bandsMappingFile);
    QString DeductBandsMappingFile(const QStringList &listProducts, const QString &bandsMappingFile, int &resolution);

    void GetJobConfig(EventProcessingContext &ctx,const JobSubmittedEvent &event,CompositeJobConfig &cfg);
};

#endif // COMPOSITEHANDLER_HPP
