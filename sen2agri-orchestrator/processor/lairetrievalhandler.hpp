#ifndef LAIRETRIEVALHANDLER_HPP
#define LAIRETRIEVALHANDLER_HPP

#include "processorhandler.hpp"

typedef struct {

    NewStepList steps;
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
} LAIProductFormatterParams;

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
                                   QList<std::reference_wrapper<const TaskToSubmit> > &outProdFormatterParentsList,
                                   int nbProducts, bool bGenModels, bool bNDayReproc, bool bFittedReproc);

    LAIGlobalExecutionInfos HandleNewTilesList(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                               const QStringList &listProducts);

    void GetModelFileList(QStringList &outListModels, const QString &strPattern, std::map<QString, QString> &configParameters);
    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                std::map<QString, QString> &configParameters,
                                const QStringList &listProducts);

    // Arguments getters
    QStringList GetNdviRviExtractionArgs(const QString &inputProduct, const QString &ftsFile, const QString &ndviFile, const QString &resolution);
    QStringList GetBvImageInvArgs(const QString &ftsFile, const QString &xmlFile, const QString &modelsFolder, const QString &monoDateLaiFileName);
    QStringList GetBvErrImageInvArgs(const QString &ftsFile, const QString &xmlFile, const QString &modelsFolder, const QString &monoDateErrFileName);
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
    QStringList GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event,
                                        const QStringList &listProducts, const QList<LAIProductFormatterParams> &productParams);

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

