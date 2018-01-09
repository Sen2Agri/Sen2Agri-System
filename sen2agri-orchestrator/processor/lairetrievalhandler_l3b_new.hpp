#ifndef LAIRETRIEVALHANDLERL3BNEW_HPP
#define LAIRETRIEVALHANDLERL3BNEW_HPP

#include "processorhandler.hpp"

class LaiRetrievalHandlerL3BNew : public ProcessorHandler
{
public:
    typedef struct {
        typedef struct {
            QString model;
            QString errModel;
            bool NeedsModelGeneration() const { return (model == "" || errModel == ""); }
        } ModelInfos;
        ModelInfos modelInfos;
        QString tileFile;
    } TileInfos;

private:

    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    void CreateTasksForNewProduct(EventProcessingContext &ctx, const JobSubmittedEvent &event, QList<TaskToSubmit> &outAllTasksList,
                                   const QList<TileInfos> &tileInfosList, bool bRemoveTempFiles);

    void GetModelFileList(const QString &folderName, const QString &modelPrefix, QStringList &outModelsList);
    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                const QList<TileInfos> &tilesInfosList);

    // Arguments getters
    QStringList GetCompressImgArgs(const QString &inFile, const QString &outFile);
    QStringList GetNdviRviExtractionNewArgs(const QString &inputProduct, const QString &msksFlagsFile,
                                            const QString &ndviFile, const QString &resolution, const QString &laiBandsCfg);
    QStringList GetLaiProcessorArgs(const QString &xmlFile, const QString &resolution, const QString &laiBandsCfg, const QString &monoDateLaiFileName, const QString &indexName);
    QStringList GetQuantifyImageArgs(const QString &inFileName, const QString &outFileName);
    QStringList GetMonoDateMskFlagsArgs(const QString &inputProduct, const QString &monoDateMskFlgsFileName, const QString &monoDateMskFlgsResFileName, const QString &resStr);
    QStringList GetLaiMonoProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                               const QList<TileInfos> &products, const QStringList &ndviList,
                                               const QStringList &laiList, const QStringList &laiFlgsList, const QStringList &faparList, const QStringList &fcoverList);
    NewStepList GetStepsForMonodateLai(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                       const QList<TileInfos> &prdTilesList, QList<TaskToSubmit> &allTasksList, bool bRemoveTempFiles, int tasksStartIdx);
    const QString& GetDefaultCfgVal(std::map<QString, QString> &configParameters, const QString &key, const QString &defVal);

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;
    bool IsParamOrConfigKeySet(const QJsonObject &parameters, std::map<QString, QString> &configParameters,
                                                    const QString &cmdLineParamName, const QString &cfgParamKey, bool defVal = true);
    QSet<QString> GetTilesFilter(const QJsonObject &parameters, std::map<QString, QString> &configParameters);
    bool FilterTile(const QSet<QString> &tilesSet, const QString &prdTileFile);
    void HandleProduct(EventProcessingContext &ctx, const JobSubmittedEvent &event, const QList<TileInfos> &prdTilesList,
                       QList<TaskToSubmit> &allTasksList);
    void SubmitEndOfLaiTask(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                            const QList<TaskToSubmit> &allTasksList);
    void SubmitL3CJobForL3BProduct(EventProcessingContext &ctx, const TaskFinishedEvent &event,
                                   const ProcessorHandlerHelper::SatelliteIdType &satId,
                                   const QString &l3bProdName);
    QMap<QDate, QStringList> GroupProductTilesByDate(const QMap<QString, QStringList> &inputProductToTilesMap);

    bool InRange(double middle, double distance, double value);
    bool ParseModelFileName(const QString &qtModelFileName, double &solarZenith, double &sensorZenith, double &relAzimuth);
    QString GetExistingModelForTile(const QStringList &modelsList, const QString &tileFile);
private:

};

#endif // LAIRETRIEVALHANDLERL3BNEW_HPP

