#ifndef LAIRETRIEVALHANDLERL3F_HPP
#define LAIRETRIEVALHANDLERL3F_HPP

#include "processorhandler.hpp"

class NdviHandler : public ProcessorHandler
{
public:
    typedef struct {
        QString tileFile;
    } TileInfos;

private:

    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    void CreateTasksForNewProduct(QList<TaskToSubmit> &outAllTasksList,
                                   const QList<TileInfos> &tileInfosList, bool bRemoveTempFiles);

    void GetModelFileList(const QString &folderName, const QString &modelPrefix, QStringList &outModelsList);
    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                const QList<TileInfos> &tilesInfosList);

    // Arguments getters
    QStringList GetCutImgArgs(const QString &shapePath, const QString &inFile, const QString &outFile);
    QStringList GetCompressImgArgs(const QString &inFile, const QString &outFile);
    QStringList GetNdviRviExtractionNewArgs(const QString &inputProduct, const QString &msksFlagsFile,
                                            const QString &ndviFile, const QString &resolution, const QString &laiBandsCfg);
    QStringList GetNdviRviExtractionArgs(const QString &inputProduct, const QString &msksFlagsFile, const QString &ftsFile, const QString &ndviFile, const QString &resolution);
    QStringList GetQuantifyImageArgs(const QString &inFileName, const QString &outFileName);
    QStringList GetMskFlagsArgs(const QString &inputProduct, const QString &monoDateMskFlgsFileName, const QString &monoDateMskFlgsResFileName, const QString &resStr);
    QStringList GetNdviProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                               const QList<TileInfos> &products, const QStringList &ndviList, const QStringList &laiFlgsList);
    NewStepList GetSteps(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                       const QList<TileInfos> &prdTilesList, QList<TaskToSubmit> &allTasksList, bool bRemoveTempFiles, int tasksStartIdx);
    const QString& GetDefaultCfgVal(std::map<QString, QString> &configParameters, const QString &key, const QString &defVal);

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;
    bool IsGenMonoDate(const QJsonObject &parameters, std::map<QString, QString> &configParameters);
    QSet<QString> GetTilesFilter(const QJsonObject &parameters, std::map<QString, QString> &configParameters);
    bool FilterTile(const QSet<QString> &tilesSet, const QString &prdTileFile);
    void HandleProduct(EventProcessingContext &ctx, const JobSubmittedEvent &event, const QList<TileInfos> &prdTilesList,
                       QList<TaskToSubmit> &allTasksList);
    void SubmitEndOfNdviTask(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                            const QList<TaskToSubmit> &allTasksList);
    void SubmitL3CJobForL3BProduct(EventProcessingContext &ctx, const TaskFinishedEvent &event,
                                   const ProcessorHandlerHelper::SatelliteIdType &satId,
                                   const QString &l3bProdName);
    QMap<QDate, QStringList> GroupProductTilesByDate(const QMap<QString, QStringList> &inputProductToTilesMap);

};

#endif // LAIRETRIEVALHANDLERL3F_HPP

