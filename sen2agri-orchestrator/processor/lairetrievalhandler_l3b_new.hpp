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

    typedef struct {
        QString tileId;
        QString tileFile;

        QString ndviFile;
        QString laiFile;
        QString faparFile;
        QString fcoverFile;

        QString statusFlagsFile;
        QString statusFlagsFileResampled;
        QString inDomainFlagsFile;
        QString laiDomainFlagsFile;
        QString faparDomainFlagsFile;
        QString fcoverDomainFlagsFile;

        bool bHasNdvi;
        bool bHasLai;
        bool bHasFapar;
        bool bHasFCover;

        QString resolutionStr;
        QString anglesFile;
    } TileResultFiles;


private:

    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &evt) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    void CreateTasksForNewProduct(EventProcessingContext &ctx, const JobSubmittedEvent &event, QList<TaskToSubmit> &outAllTasksList,
                                   const QList<TileInfos> &tileInfosList, bool bRemoveTempFiles);
    int CreateAnglesTasks(int parentTaskId, QList<TaskToSubmit> &outAllTasksList, int nCurTaskIdx, int & nAnglesTaskId);
    int CreateBiophysicalIndicatorTasks(int parentTaskId, QList<TaskToSubmit> &outAllTasksList,
                                         QList<std::reference_wrapper<const TaskToSubmit>> &productFormatterParentsRefs,
                                         int nCurTaskIdx);

    void GetModelFileList(const QString &folderName, const QString &modelPrefix, QStringList &outModelsList);
    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                const QList<TileResultFiles> &tileResultFilesList);

    QStringList GetCreateAnglesArgs(const QString &inputProduct, const QString &anglesFile);
    QStringList GetGdalTranslateAnglesNoDataArgs(const QString &anglesFile, const QString &resultAnglesFile);
    QStringList GetGdalBuildAnglesVrtArgs(const QString &anglesFile, const QString &resultVrtFile);
    QStringList GetGdalTranslateResampleAnglesArgs(const QString &vrtFile, const QString &resultResampledAnglesFile);
    QStringList GetGenerateInputDomainFlagsArgs(const QString &xmlFile,  const QString &laiBandsCfg,
                                                const QString &outFlagsFileName, const QString &outRes);
    QStringList GetGenerateOutputDomainFlagsArgs(const QString &xmlFile, const QString &laiRasterFile,
                                                const QString &laiBandsCfg, const QString &indexName,
                                                const QString &outFlagsFileName,  const QString &outCorrectedLaiFile, const QString &outRes);

    QStringList GetNdviRviExtractionNewArgs(const QString &inputProduct, const QString &msksFlagsFile,
                                            const QString &ndviFile, const QString &resolution, const QString &laiBandsCfg);
    QStringList GetLaiProcessorArgs(const QString &xmlFile, const QString &anglesFileName, const QString &resolution,
                                    const QString &laiBandsCfg, const QString &monoDateLaiFileName, const QString &indexName);
    QStringList GetQuantifyImageArgs(const QString &inFileName, const QString &outFileName);
    QStringList GetMonoDateMskFlagsArgs(const QString &inputProduct, const QString &monoDateMskFlgsFileName, const QString &monoDateMskFlgsResFileName, const QString &resStr);
    QStringList GetLaiMonoProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event, const QList<TileResultFiles> &tileResultFilesList);
    NewStepList GetStepsForMonodateLai(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                       const QList<TileInfos> &prdTilesList, QList<TaskToSubmit> &allTasksList, bool bRemoveTempFiles, int tasksStartIdx);
    int GetStepsForStatusFlags(QList<TaskToSubmit> &allTasksList, int curTaskIdx,
                                TileResultFiles &tileResultFileInfo, NewStepList &steps, QStringList &cleanupTemporaryFilesList);
    int GetStepsForNdvi(QList<TaskToSubmit> &allTasksList, int curTaskIdx,
                                TileResultFiles &tileResultFileInfo, const QString &laiCfgFile, NewStepList &steps, QStringList &cleanupTemporaryFilesList);
    int GetStepsForAnglesCreation(QList<TaskToSubmit> &allTasksList, int curTaskIdx, TileResultFiles &tileResultFileInfo, NewStepList &steps, QStringList &cleanupTemporaryFilesList);
    int GetStepsForMonoDateBI(QList<TaskToSubmit> &allTasksList,
                               const QString &indexName, int curTaskIdx, const QString &laiCfgFile, TileResultFiles &tileResultFileInfo,
                              NewStepList &steps, QStringList &cleanupTemporaryFilesList);
    int GetStepsForInDomainFlags(QList<TaskToSubmit> &allTasksList, int curTaskIdx,
                                const QString &laiCfgFile, TileResultFiles &tileResultFileInfo, NewStepList &steps,
                                QStringList &cleanupTemporaryFilesList);

    const QString& GetDefaultCfgVal(std::map<QString, QString> &configParameters, const QString &key, const QString &defVal);

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;
    bool IsParamOrConfigKeySet(const QJsonObject &parameters, std::map<QString, QString> &configParameters,
                                                    const QString &cmdLineParamName, const QString &cfgParamKey, bool defVal = true);
    QSet<QString> GetTilesFilter(const QJsonObject &parameters, std::map<QString, QString> &configParameters);
    bool FilterTile(const QSet<QString> &tilesSet, const QString &prdTileFile);
    void InitTileResultFiles(bool bGenNdvi, bool bGenLai, bool bGenFapar, bool bGenFCover, const QString &resolutionStr,
                             const QString tileFileName, TileResultFiles &tileResultFileInfo);

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
    int UpdateJobSubmittedParamsFromSchedReq(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                              QJsonObject &parameters, JobSubmittedEvent &newEvent);
    ProductList GetL2AProductsNotProcessed(EventProcessingContext &ctx,
                                           int siteId, const QDateTime &startDate, const QDateTime &endDate);
    QStringList GetL3BSourceL2APrdsPaths(const QString &prdPath);
    QStringList GetL2ARelPathsFromDB(EventProcessingContext &ctx, int siteId,
                                     const QDateTime &startDate, const QDateTime &endDate, QStringList &retFullPaths, ProductList &prdList);
    QStringList GetL2ARelPathsFromProcessedL3Bs(EventProcessingContext &ctx, int siteId,
                                                const QDateTime &startDate, const QDateTime &endDate, QStringList &retFullPaths);

};

#endif // LAIRETRIEVALHANDLERL3BNEW_HPP

