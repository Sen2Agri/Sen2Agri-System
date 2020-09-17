#ifndef LAIRETRIEVALHANDLERL3B_HPP
#define LAIRETRIEVALHANDLERL3B_HPP

#include "processorhandler.hpp"

class LaiRetrievalHandlerL3B : public ProcessorHandler
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

    void CreateTasksForNewProduct(QList<TaskToSubmit> &outAllTasksList,
                                   const QList<TileInfos> &tileInfosList,
                                   bool bForceGenModels, bool bRemoveTempFiles);

    void GetModelFileList(const QString &folderName, const QString &modelPrefix, QStringList &outModelsList);
    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                const QList<TileInfos> &tilesInfosList);

    // Arguments getters
    QStringList GetNdviRviExtractionNewArgs(const QString &inputProduct, const QString &msksFlagsFile, const QString &ftsFile,
                                            const QString &ndviFile, const QString &resolution, const QString &laiBandsCfg);
    QStringList GetNdviRviExtractionArgs(const QString &inputProduct, const QString &msksFlagsFile, const QString &ftsFile, const QString &ndviFile, const QString &resolution);
    QStringList GetBvImageInvArgs(const QString &ftsFile, const QString &msksFlagsFile, const QString &xmlFile, const QString &modelsFolder, const QString &monoDateLaiFileName);
    QStringList GetBvErrImageInvArgs(const QString &ftsFile, const QString &msksFlagsFile, const QString &xmlFile, const QString &modelsFolder, const QString &monoDateErrFileName);
    QStringList GetQuantifyImageArgs(const std::map<QString, QString> &configParams, const QString &inFileName, const QString &outFileName);
    QStringList GetMonoDateMskFlagsArgs(const QString &inputProduct, const QString &monoDateMskFlgsFileName, const QString &monoDateMskFlgsResFileName, const QString &resStr);
    QStringList GetLaiMonoProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                               const QList<TileInfos> &products, const QStringList &ndviList,
                                               const QStringList &laiList, const QStringList &laiErrList, const QStringList &laiFlgsList);
    NewStepList GetStepsForMonodateLai(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                       const QList<TileInfos> &prdTilesList, QList<TaskToSubmit> &allTasksList, bool bRemoveTempFiles, int tasksStartIdx);
    NewStepList GetStepsToGenModel(const std::map<QString, QString> &configParameters,
                                   const QList<TileInfos> &listPrdTiles, QList<TaskToSubmit> &allTasksList, int tasksStartIdx, bool bForceGenModels);
    QStringList GetBVInputVariableGenerationArgs(const std::map<QString, QString> &configParameters, const QString &strGenSampleFile);
    QStringList GetProSailSimulatorNewArgs(const QString &product, const QString &bvFileName, const QString &rsrCfgFileName,
                                       const QString &outSimuReflsFile, const QString &outAngles,
                                       const QString &laiBandsCfg);
    QStringList GetProSailSimulatorArgs(const QString &product, const QString &bvFileName, const QString &rsrCfgFileName,
                                       const QString &outSimuReflsFile, const QString &outAngles, std::map<QString, QString> &configParameters);
    QStringList GetTrainingDataGeneratorNewArgs(const QString &product, const QString &biovarsFile,
                                                const QString &simuReflsFile, const QString &outTrainingFile,
                                                const QString &laiBandsCfg);
    QStringList GetTrainingDataGeneratorArgs(const QString &product, const QString &biovarsFile,
                                             const QString &simuReflsFile, const QString &outTrainingFile);
    QStringList GetInverseModelLearningArgs(const QString &trainingFile, const QString &product, const QString &anglesFile,
                                            const QString &errEstFile, const QString &modelsFolder, const std::map<QString, QString> &configParameters);
    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;
    bool IsForceGenModels(const QJsonObject &parameters, const std::map<QString, QString> &configParameters);
    void HandleProduct(EventProcessingContext &ctx, const JobSubmittedEvent &event, const QList<TileInfos> &prdTilesList,
                       QList<TaskToSubmit> &allTasksList);
    void SubmitEndOfLaiTask(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                            const QList<TaskToSubmit> &allTasksList);
    QMap<QDate, QStringList> GroupProductTilesByDate(const QMap<QString, QStringList> &inputProductToTilesMap);

    bool InRange(double middle, double distance, double value);
    bool ParseModelFileName(const QString &qtModelFileName, double &solarZenith, double &sensorZenith, double &relAzimuth);
    QString GetExistingModelForTile(const QStringList &modelsList, const QString &tileFile);
private:

    friend class LaiRetrievalHandler;

};

#endif // LAIRETRIEVALHANDLERL3B_HPP

