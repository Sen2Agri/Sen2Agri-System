#ifndef LAIRETRIEVALHANDLER_HPP
#define LAIRETRIEVALHANDLER_HPP

#include "processorhandler.hpp"

class LaiRetrievalHandler : public ProcessorHandler
{
private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    void CreateNewProductInJobTasks(QList<TaskToSubmit> &outAllTasksList, int nbProducts, bool bNDayReproc, bool bFittedReproc);
    void HandleNewProductInJob(EventProcessingContext &ctx, int jobId, const QString &jsonParams,
                               const QStringList &listProducts);
    void GetModelFileList(QStringList &outListModels, const QString &strPattern, std::map<QString, QString> &configParameters);
    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                std::map<QString, QString> &configParameters,
                                const QStringList &listProducts);

    // Arguments getters
    QStringList GetNdviRviExtractionArgs(const QString &inputProduct, const QString &ftsFile, const QString &resolution);
    QStringList GetLaiModelExtractorArgs(const QString &inputProduct, const QStringList &modelsList, const QStringList &errModelsList,
                                         const QString &modelFileName, const QString &errModelFileName);
    QStringList GetBvImageInvArgs(const QString &ftsFile, const QString &modelFileName, const QString &monoDateLaiFileName);
    QStringList GetBvErrImageInvArgs(const QString &ftsFile, const QString &errModelFileName, const QString &monoDateErrFileName);
    QStringList GetMonoDateMskFagsArgs(const QString &inputProduct, const QString &monoDateMskFlgsFileName);
    QStringList GetTimeSeriesBuilderArgs(const QStringList &monoDateLaiFileNames, const QString &allLaiTimeSeriesFileName);
    QStringList GetErrTimeSeriesBuilderArgs(const QStringList &monoDateErrLaiFileNames, const QString &allErrTimeSeriesFileName);
    QStringList GetMskFlagsTimeSeriesBuilderArgs(const QStringList &monoDateMskFlagsLaiFileNames, const QString &allMskFlagsTimeSeriesFileName);
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
    QStringList GetProductFormatterArgs(TaskToSubmit &productFormatterTask, std::map<QString, QString> configParameters, const QJsonObject &parameters,
                                        const QStringList &listProducts, const QStringList &listNdvis,
                                        const QStringList &listLaiMonoDate, const QStringList &listLaiMonoDateErr,
                                        const QStringList &listLaiMonoDateFlgs, const QString &fileLaiReproc,
                                        const QString &fileLaiReprocFlgs, const QString &fileLaiFit, const QString &fileLaiFitFlgs,
                                        const QString &tileId);

    bool IsNDaysReprocessingNeeded(const QJsonObject &parameters);
    bool IsFittedReprocessingNeeded(const QJsonObject &parameters);

private:
    int m_nTimeSeriesBuilderIdx;
    int m_nErrTimeSeriesBuilderIdx;
    int m_nLaiMskFlgsTimeSeriesBuilderIdx;
    int m_nProfileReprocessingIdx;
    int m_nReprocessedProfileSplitterIdx;
    int m_nFittedProfileReprocessingIdx;
    int m_nFittedProfileReprocessingSplitterIdx;
    int m_nProductFormatterIdx;
};

#endif // LAIRETRIEVALHANDLER_HPP

