#ifndef LAIRETRIEVALHANDLER_HPP
#define LAIRETRIEVALHANDLER_HPP

#include "processorhandler.hpp"

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

    QList<std::reference_wrapper<const TaskToSubmit>> parentsTasksRef;
    // args
     QStringList listNdvi;
     QStringList listLaiMonoDate;
     QStringList listLaiMonoDateErr;
     QStringList listLaiMonoDateFlgs;
     QString fileLaiReproc;
     QString fileLaiReprocFlgs;
     QString fileLaiFit;
     QString fileLaiFitFlgs;
     QString tileId;
} LAIProductFormatterParams2;

typedef struct {
    QList<TaskToSubmit> allTasksList;
    NewStepList allStepsList;
    LAIProductFormatterParams prodFormatParams;
} LAIGlobalExecutionInfos;

class LaiRetrievalHandler : public ProcessorHandler
{
private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    void CreateTasksForNewProducts(QList<TaskToSubmit> &outAllTasksList,
                                   LAIProductFormatterParams &outProdFormatterParams,
                                   int nbProducts, bool bGenModels, bool bMonoDateLai, bool bNDayReproc, bool bFittedReproc);

    LAIGlobalExecutionInfos HandleNewTilesList(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                               const QStringList &listProducts, const QStringList &listL3BTiles, const QStringList &missingL3BInputs);

    void GetModelFileList(QStringList &outListModels, const QString &strPattern, std::map<QString, QString> &configParameters);
    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                std::map<QString, QString> &configParameters,
                                const QStringList &listProducts, bool bIsReproc);

    // Arguments getters
    QStringList GetNdviRviExtractionArgs(const QString &inputProduct, const QString &msksFlagsFile, const QString &ftsFile, const QString &ndviFile, const QString &resolution);
    QStringList GetBvImageInvArgs(const QString &ftsFile, const QString &xmlFile, const QString &modelsFolder, const QString &monoDateLaiFileName);
    QStringList GetBvErrImageInvArgs(const QString &ftsFile, const QString &xmlFile, const QString &modelsFolder, const QString &monoDateErrFileName);
    QStringList GetQuantifyImageArgs(const QString &inFileName, const QString &outFileName);
    QStringList GetMonoDateMskFlagsArgs(const QString &inputProduct, const QString &monoDateMskFlgsFileName);
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

    QStringList GetLaiMonoProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                               const QString &product, const QStringList &tileIdsList, const QStringList &ndviList,
                                               const QStringList &laiList, const QStringList &laiErrList, const QStringList &laiFlgsList);
    QStringList GetReprocProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event,
                                        const QStringList &listProducts, const QList<LAIProductFormatterParams> &productParams, bool isFitted);

    void GetStepsToGenModel(std::map<QString, QString> &configParameters, const QStringList &listProducts,
                            QList<TaskToSubmit> &allTasksList, NewStepList &steps);
    QStringList GetBVInputVariableGenerationArgs(std::map<QString, QString> &configParameters, const QString &strGenSampleFile);
    QStringList GetProSailSimulatorArgs(const QString &product, const QString &bvFileName, const QString &rsrCfgFileName,
                                       const QString &outSimuReflsFile, const QString &outAngles, std::map<QString, QString> &configParameters);
    QStringList GetTrainingDataGeneratorArgs(const QString &product, const QString &biovarsFile,
                                             const QString &simuReflsFile, const QString &outTrainingFile);
    QStringList GetInverseModelLearningArgs(const QString &trainingFile, const QString &product, const QString &anglesFile,
                                            const QString &errEstFile, const QString &modelsFolder, std::map<QString, QString> &configParameters);
    const QString& GetDefaultCfgVal(std::map<QString, QString> &configParameters, const QString &key, const QString &defVal);

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;
    bool IsGenModels(const QJsonObject &parameters, std::map<QString, QString> &configParameters);
    bool IsGenMonoDate(const QJsonObject &parameters, std::map<QString, QString> &configParameters);
    bool IsNDayReproc(const QJsonObject &parameters, std::map<QString, QString> &configParameters);
    bool IsFittedReproc(const QJsonObject &parameters, std::map<QString, QString> &configParameters);
private:
    int m_nTimeSeriesBuilderIdx;
    int m_nErrTimeSeriesBuilderIdx;
    int m_nLaiMskFlgsTimeSeriesBuilderIdx;
    int m_nProfileReprocessingIdx;
    int m_nReprocessedProfileSplitterIdx;
    int m_nFittedProfileReprocessingIdx;
    int m_nFittedProfileReprocessingSplitterIdx;
    //int m_nProductFormatterIdx;
};

#endif // LAIRETRIEVALHANDLER_HPP

