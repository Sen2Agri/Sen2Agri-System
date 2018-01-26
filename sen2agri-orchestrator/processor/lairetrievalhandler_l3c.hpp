#ifndef LAIRETRIEVALHANDLERL3C_HPP
#define LAIRETRIEVALHANDLERL3C_HPP

#include "processorhandler.hpp"

class LaiRetrievalHandlerL3C : public ProcessorHandler
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

private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    void CreateTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                   LAIProductFormatterParams &outProdFormatterParams, bool bNDayReproc, bool bFittedReproc, bool bRemoveTempFiles);

    void HandleNewTilesList(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                            const TileTemporalFilesInfo &tileTemporalFilesInfo, LAIGlobalExecutionInfos &outGlobalExecInfos, bool bRemoveTempFiles);

    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                std::map<QString, QString> &configParameters, const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles,
                                const QStringList &listProducts, bool bIsReproc);

    // Arguments getters
    QStringList GetTimeSeriesBuilderArgs(const std::map<QString, QString> &configParameters, const QStringList &monoDateLaiFileNames,
                                         const QString &allLaiTimeSeriesFileName, const QString &mainImg, bool bIsFlg = false);
    QStringList GetProfileReprocessingArgs(const std::map<QString, QString> &configParameters, const QString &allLaiTimeSeriesFileName,
                                           const QString &allErrTimeSeriesFileName, const QString &allMsksTimeSeriesFileName,
                                           const QString &reprocTimeSeriesFileName, const QStringList &listDates);
    QStringList GetReprocProfileSplitterArgs(const std::map<QString, QString> &configParameters,
                                             const QString &reprocTimeSeriesFileName, const QString &reprocFileListFileName,
                                             const QString &reprocFlagsFileListFileName, const QStringList &listDates);
    QStringList GetFittedProfileReprocArgs(const QString &allLaiTimeSeriesFileName, const QString &allErrTimeSeriesFileName,
                                           const QString &allMsksTimeSeriesFileName, const QString &fittedTimeSeriesFileName,
                                           const QStringList &ildates);
    QStringList GetFittedProfileReprocSplitterArgs(const std::map<QString, QString> &configParameters, const QString &fittedTimeSeriesFileName, const QString &fittedFileListFileName,
                                                   const QString &fittedFlagsFileListFileName, const QStringList &allXmlsFileName);

    QStringList GetLaiMonoProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                               const QStringList &products, const QStringList &tileIdsList, const QStringList &ndviList,
                                               const QStringList &laiList, const QStringList &laiErrList, const QStringList &laiFlgsList);
    QStringList GetReprocProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event, const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles,
                                        const QStringList &listProducts, const QList<LAIProductFormatterParams> &productParams, bool isFitted);

    NewStepList GetStepsForMultiDateReprocessing(const std::map<QString, QString> &configParameters, const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                                 QList<TaskToSubmit> &allTasksList, bool bNDayReproc, bool bFittedReproc,
                                                 LAIProductFormatterParams &productFormatterParams, int tasksStartIdx, bool bRemoveTempFiles);

    QDate GetSiteFirstSeasonStartDate(EventProcessingContext &ctx,int siteId);
    QDateTime GetL3BLastAcqDate(const QStringList &listL3bPrds);
    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;
    bool IsNDayReproc(const QJsonObject &parameters, std::map<QString, QString> &configParameters);
    bool IsFittedReproc(const QJsonObject &parameters, std::map<QString, QString> &configParameters);
    bool IsReprocessingCompact(const QJsonObject &parameters, std::map<QString, QString> &configParameters);

    bool GetL2AProductsInterval(const QMap<QString, QStringList> &mapTilesMeta, QDateTime &startDate, QDateTime &endDate);
    QStringList GetL3BProductsSinceStartOfSeason(EventProcessingContext &ctx, int siteId, const QStringList &listExistingPrds);
    QStringList GetL3BProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event);
    QMap<QString, TileTemporalFilesInfo> GetL3BMapTiles(EventProcessingContext &ctx,
                                                                                const QStringList &l3bProducts,
                                                                                const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles);
    QMap<QString, TileTemporalFilesInfo> GetL3BMapTiles(EventProcessingContext &ctx, const QString &newestL3BProd,
                                                        const QStringList &l3bProducts,
                                                        const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles,
                                                        int limitL3BPrdsPerTile);
    int GetIntParameterValue(const QJsonObject &parameters, const QString &key, int defVal);
    bool AddTileFileInfo(TileTemporalFilesInfo &temporalTileInfo, const QString &l3bProdDir,
                         const QString &l3bTileDir, ProcessorHandlerHelper::SatelliteIdType satId, const QDateTime &curPrdMinDate,
                         const Tile *pIntersectingTile = 0);
    bool AddTileFileInfo(EventProcessingContext &ctx, TileTemporalFilesInfo &temporalTileInfo, const QString &l3bPrd,
                         const QString &tileId, const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles,
                         ProcessorHandlerHelper::SatelliteIdType satId, const QDateTime &curPrdMinDate);
    bool HasSufficientProducts(const TileTemporalFilesInfo &tileInfo, const ProcessorHandlerHelper::SatelliteIdType &tileSatId, int limitL3BPrdsPerTile);
    void SubmitL3BMapTiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                           const QMap<QString, TileTemporalFilesInfo> &l3bMapTiles, bool bRemoveTempFiles, bool bNDayReproc, QList<TaskToSubmit> &allTasksList);
    void SubmitEndOfLaiTask(EventProcessingContext &ctx, const JobSubmittedEvent &event, const QList<TaskToSubmit> &allTasksList);

    void CreateTasksForNewProducts_New(QList<TaskToSubmit> &outAllTasksList,
                                   LAIProductFormatterParams &outProdFormatterParams,
                                   bool bNDayReproc, bool bRemoveTempFiles);
    NewStepList GetStepsForMultiDateReprocessing_New(std::map<QString, QString> &configParameters, const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                                 QList<TaskToSubmit> &allTasksList, bool bNDayReproc,
                                                 LAIProductFormatterParams &productFormatterParams, int tasksStartIdx, bool bRemoveTempFiles);

    QStringList GetProfileReprocessingArgs_New(const std::map<QString, QString> &configParameters, QStringList &monoDateLaiFileNames, QStringList &errFileNames, QStringList &flgsFileNames,
                                           const QString &mainImg, const QString &reprocTimeSeriesFileName, const QStringList &listDates);
    QStringList GetFittedProfileReprocArgs_New(const std::map<QString, QString> &configParameters,
                                               QStringList &monoDateLaiFileNames, QStringList &errFileNames,
                                           QStringList &flgsFileNames, const QString &mainImg, const QString &reprocTimeSeriesFileName, const QStringList &listDates);
    QMap<QString, TileTemporalFilesInfo> FilterSecondaryProductTiles(const QMap<QString, TileTemporalFilesInfo> &mapTiles,
                                 const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles);
    QStringList GetL3BProductRasterFiles(const TileTemporalFilesInfo &tileTemporalFilesInfo, LAI_RASTER_ADDITIONAL_INFO_IDX idx);
};

#endif // LAIRETRIEVALHANDLERNEW_HPP

