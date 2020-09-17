#ifndef LAIRETRIEVALHANDLERMULTIDATEBASE_HPP
#define LAIRETRIEVALHANDLERMULTIDATEBASE_HPP

#include "processorhandler.hpp"

class LaiRetrievalHandlerMultiDateBase : public ProcessorHandler
{
    typedef struct {

        QList<std::reference_wrapper<const TaskToSubmit>> parentsTasksRef;
        // args
         QString ndvi;
         QString laiMonoDate;
         QString laiMonoDateErr;
         QString laiMonoDateFlgs;
    } LAIMonoDateProductFormatterParams;

    typedef struct {

        QList<std::reference_wrapper<const TaskToSubmit>> parentsTasksRef;
        // args
        QString fileLaiReproc;
        QString fileLaiReprocFlgs;
    } LAIReprocProductFormatterParams;

    typedef struct {
         QList<LAIMonoDateProductFormatterParams> listLaiMonoParams;
         LAIReprocProductFormatterParams laiReprocParams;
         QString tileId;
    } LAIProductFormatterParams;

    typedef struct {
        QList<TaskToSubmit> allTasksList;
        NewStepList allStepsList;
        LAIProductFormatterParams prodFormatParams;
    } LAIGlobalExecutionInfos;

    typedef enum {LAI_RASTER_ADD_INFO_IDX=0, LAI_ERR_RASTER_ADD_INFO_IDX=1, LAI_FLG_RASTER_ADD_INFO_IDX=2} LAI_RASTER_ADDITIONAL_INFO_IDX;

protected:
    QString GetProcessorDBPrefix();
    bool AddTileFileInfo(EventProcessingContext &ctx, TileTemporalFilesInfo &temporalTileInfo, const QString &l3bPrd,
                         const QString &tileId, const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles,
                         ProcessorHandlerHelper::SatelliteIdType satId, const QDateTime &curPrdMinDate);
    QMap<QString, TileTemporalFilesInfo> FilterSecondaryProductTiles(const QMap<QString, TileTemporalFilesInfo> &mapTiles,
                                 const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles);

private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    void CreateTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                   LAIProductFormatterParams &outProdFormatterParams, bool bCompact, bool bRemoveTempFiles);

    void HandleNewTilesList(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                            const TileTemporalFilesInfo &tileTemporalFilesInfo, LAIGlobalExecutionInfos &outGlobalExecInfos, bool bRemoveTempFiles);

    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                const std::map<QString, QString> &configParameters, const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles,
                                const QStringList &listProducts);

    // Arguments getters
    QStringList GetTimeSeriesBuilderArgs(const QStringList &monoDateLaiFileNames,
                                         const QString &allLaiTimeSeriesFileName, const QString &mainImg, bool bIsFlg = false);

    QStringList GetReprocessingArgs(const std::map<QString, QString> &configParameters, const QString &allLaiTimeSeriesFileName,
                                           const QString &allErrTimeSeriesFileName, const QString &allMsksTimeSeriesFileName,
                                           const QString &reprocTimeSeriesFileName, const QStringList &listDates);
    QStringList GetCompactReprocessingArgs(const std::map<QString, QString> &configParameters, QStringList &monoDateLaiFileNames,
                                           QStringList &errFileNames, QStringList &flgsFileNames, const QString &mainImg,
                                           const QString &reprocTimeSeriesFileName, const QStringList &listDates);
    virtual QStringList GetSpecificReprocessingArgs(const std::map<QString, QString> &configParameters) = 0;
    virtual ProductType GetOutputProductType() = 0;
    virtual QString GetOutputProductShortName() = 0;
    virtual void WriteExecutionSpecificParamsValues(const std::map<QString, QString> &configParameters, std::ofstream &stream) = 0;
    virtual QString GetPrdFormatterRasterFlagName() = 0;
    virtual QString GetPrdFormatterMskFlagName() = 0;
    virtual ProductList GetScheduledJobProductList(SchedulingContext &ctx, int siteId, const QDateTime &seasonStartDate,
                                                   const QDateTime &seasonEndDate, const QDateTime &qScheduledDate,
                                                   const ConfigurationParameterValueMap &requestOverrideCfgValues) = 0;
    virtual bool AcceptSchedJobProduct(const QString &l2aPrdHdrPath, ProcessorHandlerHelper::SatelliteIdType satId) = 0;

    virtual QList<QMap<QString, TileTemporalFilesInfo>> ExtractL3BMapTiles(EventProcessingContext &ctx,
                                                       const JobSubmittedEvent &event, const QStringList &l3bProducts,
                                                       const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles) = 0;

    QStringList GetReprocProfileSplitterArgs(const QString &reprocTimeSeriesFileName, const QString &reprocFileListFileName,
                                             const QString &reprocFlagsFileListFileName, const QStringList &listDates);
    QStringList GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event, const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles,
                                        const QStringList &listProducts, const QList<LAIProductFormatterParams> &productParams);

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;
    bool IsReprocessingCompact(const QJsonObject &parameters, std::map<QString, QString> &configParameters);

    bool GetL2AProductsInterval(const QMap<QString, QStringList> &mapTilesMeta, QDateTime &startDate, QDateTime &endDate);
    QStringList GetL3BProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event);
    int GetIntParameterValue(const QJsonObject &parameters, const QString &key, int defVal);
    bool AddTileFileInfo(TileTemporalFilesInfo &temporalTileInfo, const QString &l3bProdDir,
                         const QString &l3bTileDir, ProcessorHandlerHelper::SatelliteIdType satId, const QDateTime &curPrdMinDate,
                         const Tile *pIntersectingTile = 0);
    void SubmitL3BMapTiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                           const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles, bool bRemoveTempFiles, QList<TaskToSubmit> &allTasksList);
    void SubmitEndOfLaiTask(EventProcessingContext &ctx, const JobSubmittedEvent &event, const QList<TaskToSubmit> &allTasksList);

    void CreateTasksForNewProducts_New(QList<TaskToSubmit> &outAllTasksList,
                                   LAIProductFormatterParams &outProdFormatterParams,
                                   bool bNDayReproc, bool bRemoveTempFiles);
    NewStepList GetStepsForMultiDateReprocessing(const std::map<QString, QString> &configParameters, const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                                 QList<TaskToSubmit> &allTasksList,
                                                 LAIProductFormatterParams &productFormatterParams, int tasksStartIdx, bool bRemoveTempFiles);

    NewStepList GetStepsForCompactMultiDateReprocessing(std::map<QString, QString> &configParameters, const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                                 QList<TaskToSubmit> &allTasksList,
                                                 LAIProductFormatterParams &productFormatterParams, int tasksStartIdx, bool bRemoveTempFiles);

    QStringList GetL3BProductRasterFiles(const TileTemporalFilesInfo &tileTemporalFilesInfo, LAI_RASTER_ADDITIONAL_INFO_IDX idx);

private:
    QString m_procDbPrefix;
    friend class LaiRetrievalHandler;
};

#endif // LAIRETRIEVALHANDLERMULTIDATEBASE_HPP

