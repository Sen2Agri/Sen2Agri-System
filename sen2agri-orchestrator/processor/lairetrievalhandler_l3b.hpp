#ifndef LAIRETRIEVALHANDLERL3B_HPP
#define LAIRETRIEVALHANDLERL3B_HPP

#include "processorhandler.hpp"

class LaiRetrievalHandlerL3B : public ProcessorHandler
{
private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    void CreateTasksForNewProduct(QList<TaskToSubmit> &outAllTasksList,
                                   const QStringList &prdTilesList,
                                   bool bGenModels, bool bRemoveTempFiles);

    void GetModelFileList(QStringList &outListModels, const QString &strPattern, std::map<QString, QString> &configParameters);
    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                const QStringList &listProducts);

    // Arguments getters
    QStringList GetCutImgArgs(const QString &shapePath, const QString &inFile, const QString &outFile);
    QStringList GetCompressImgArgs(const QString &inFile, const QString &outFile);
    QStringList GetNdviRviExtractionArgs(const QString &inputProduct, const QString &msksFlagsFile, const QString &ftsFile, const QString &ndviFile, const QString &resolution);
    QStringList GetBvImageInvArgs(const QString &ftsFile, const QString &msksFlagsFile, const QString &xmlFile, const QString &modelsFolder, const QString &monoDateLaiFileName);
    QStringList GetBvErrImageInvArgs(const QString &ftsFile, const QString &msksFlagsFile, const QString &xmlFile, const QString &modelsFolder, const QString &monoDateErrFileName);
    QStringList GetQuantifyImageArgs(const QString &inFileName, const QString &outFileName);
    QStringList GetMonoDateMskFlagsArgs(const QString &inputProduct, const QString &monoDateMskFlgsFileName, const QString &monoDateMskFlgsResFileName, const QString &resStr);
    QStringList GetLaiMonoProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                               const QStringList &products, const QStringList &ndviList,
                                               const QStringList &laiList, const QStringList &laiErrList, const QStringList &laiFlgsList);
    NewStepList GetStepsForMonodateLai(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                       const QStringList &prdTilesList, QList<TaskToSubmit> &allTasksList, bool bRemoveTempFiles, int tasksStartIdx);
    NewStepList GetStepsToGenModel(std::map<QString, QString> &configParameters,
                                   const QStringList &listProducts, QList<TaskToSubmit> &allTasksList, int tasksStartIdx);
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
    void HandleProduct(EventProcessingContext &ctx, const JobSubmittedEvent &event, const QStringList &prdTilesList,
                       QList<TaskToSubmit> &allTasksList);
    void SubmitEndOfLaiTask(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                            const QList<TaskToSubmit> &allTasksList);
private:

};

#endif // LAIRETRIEVALHANDLERL3B_HPP

