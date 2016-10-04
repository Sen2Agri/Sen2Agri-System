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
         LAIReprocProductFormatterParams laiFitParams;
         QString tileId;
    } LAIProductFormatterParams;

    typedef struct {
        QList<TaskToSubmit> allTasksList;
        NewStepList allStepsList;
        LAIProductFormatterParams prodFormatParams;
    } LAIGlobalExecutionInfos;

private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    void CreateTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                   LAIProductFormatterParams &outProdFormatterParams, bool bNDayReproc, bool bFittedReproc);

    void HandleNewTilesList(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                            const TileTemporalFilesInfo &tileTemporalFilesInfo, LAIGlobalExecutionInfos &outGlobalExecInfos);

    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                std::map<QString, QString> &configParameters,
                                const QStringList &listProducts, bool bIsReproc);

    // Arguments getters
    QStringList GetTimeSeriesBuilderArgs(const QStringList &monoDateLaiFileNames, const QString &allLaiTimeSeriesFileName, const QString &mainImg);
    QStringList GetErrTimeSeriesBuilderArgs(const QStringList &monoDateErrLaiFileNames, const QString &allErrTimeSeriesFileName, const QString &mainImg);
    QStringList GetMskFlagsTimeSeriesBuilderArgs(const QStringList &monoDateMskFlagsLaiFileNames, const QString &allMskFlagsTimeSeriesFileName, const QString &mainImg);
    QStringList GetProfileReprocessingArgs(std::map<QString, QString> configParameters, const QString &allLaiTimeSeriesFileName,
                                           const QString &allErrTimeSeriesFileName, const QString &allMsksTimeSeriesFileName,
                                           const QString &reprocTimeSeriesFileName, const QStringList &listProducts);
    QStringList GetReprocProfileSplitterArgs(const QString &reprocTimeSeriesFileName, const QString &reprocFileListFileName,
                                             const QString &reprocFlagsFileListFileName, const QStringList &allXmlsFileName);
    QStringList GetFittedProfileReprocArgs(const QString &allLaiTimeSeriesFileName, const QString &allErrTimeSeriesFileName,
                                           const QString &allMsksTimeSeriesFileName, const QString &fittedTimeSeriesFileName,
                                           const QStringList &listProducts);
    QStringList GetFittedProfileReprocSplitterArgs(const QString &fittedTimeSeriesFileName, const QString &fittedFileListFileName,
                                                   const QString &fittedFlagsFileListFileName, const QStringList &allXmlsFileName);

    QStringList GetLaiMonoProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                               const QStringList &products, const QStringList &tileIdsList, const QStringList &ndviList,
                                               const QStringList &laiList, const QStringList &laiErrList, const QStringList &laiFlgsList);
    QStringList GetReprocProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event,
                                        const QStringList &listProducts, const QList<LAIProductFormatterParams> &productParams, bool isFitted);

    NewStepList GetStepsForMultiDateReprocessing(std::map<QString, QString> &configParameters, const TileTemporalFilesInfo &tileTemporalFilesInfo,
                                                 QList<TaskToSubmit> &allTasksList, bool bNDayReproc, bool bFittedReproc, LAIProductFormatterParams &productFormatterParams);

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;
    bool IsNDayReproc(const QJsonObject &parameters, std::map<QString, QString> &configParameters);
    bool IsFittedReproc(const QJsonObject &parameters, std::map<QString, QString> &configParameters);

    bool GetL2AProductsInterval(const QMap<QString, QStringList> &mapTilesMeta, QDateTime &startDate, QDateTime &endDate);
    QStringList GetL3BProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                               const QMap<QString, QStringList> &inputProductToTilesMap);
    QMap<QString, TileTemporalFilesInfo> GetL3BMapTiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                        QMap<QString, TileTemporalFilesInfo> mapTiles, QStringList &l3bProductsFiltered);

private:
    int m_nTimeSeriesBuilderIdx;
    int m_nErrTimeSeriesBuilderIdx;
    int m_nLaiMskFlgsTimeSeriesBuilderIdx;
    int m_nProfileReprocessingIdx;
    int m_nReprocessedProfileSplitterIdx;
    int m_nFittedProfileReprocessingIdx;
    int m_nFittedProfileReprocessingSplitterIdx;
};

#endif // LAIRETRIEVALHANDLERNEW_HPP

