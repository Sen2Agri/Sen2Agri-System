#ifndef AGRICPRACTICESHANDLER_HPP
#define AGRICPRACTICESHANDLER_HPP

#include "processorhandler.hpp"

typedef struct AgricPracticesSiteCfg {
    AgricPracticesSiteCfg() {

    }
    QString additionalFilesRootDir;

    // Common parameters
    QString country;
    QString year;
    QString fullShapePath;
    QString ndviIdsGeomShapePath;
    QString ampCoheIdsGeomShapePath;

    TQStrQStrMap ccPracticeParams;
    TQStrQStrMap flPracticeParams;
    TQStrQStrMap nfcPracticeParams;
    TQStrQStrMap naPracticeParams;

    TQStrQStrMap ccTsaParams;
    TQStrQStrMap flTsaParams;
    TQStrQStrMap nfcTsaParams;
    TQStrQStrMap naTsaParams;

    // parameters used for data extraction step
    QStringList practices;

    // TSA minimum acquisitions
    QString tsaMinAcqsNo;

} AgricPracticesSiteCfg;

typedef struct AgricPracticesJobCfg {
    AgricPracticesJobCfg() {
        prdsPerGroup = 0;
        siteId = -1;
        isScheduledJob = false;
    }
    AgricPracticesSiteCfg siteCfg;

    int siteId;
    QString siteShortName;

    // parameters used for data extraction step
    int prdsPerGroup;
    bool isScheduledJob;

} AgricPracticesJobCfg;

enum AgricPractOperation {none = 0x00,
                          dataExtraction = 0x01,
                          catchCrop = 0x02,
                          fallow = 0x04,
                          nfc = 0x08,
                          harvestOnly = 0x10,
                          timeSeriesAnalysis = (catchCrop|fallow|nfc|harvestOnly),
                          all = 0xFF};

class AgricPracticesHandler : public ProcessorHandler
{
private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &evt) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;
    void HandleProductAvailableImpl(EventProcessingContext &ctx,
                                    const ProductAvailableEvent &event) override;

    void CreateTasks(const AgricPracticesJobCfg &siteCfg, QList<TaskToSubmit> &outAllTasksList, const QStringList &ndviPrds,
                     const QStringList &ampPrds, const QStringList &cohePrds, AgricPractOperation operation);
    void CreateSteps(EventProcessingContext &ctx, const JobSubmittedEvent &event, QList<TaskToSubmit> &allTasksList,
                     const AgricPracticesJobCfg &siteCfg, const QStringList &ndviPrds, const QStringList &ampPrds,
                     const QStringList &cohePrds, NewStepList &steps, const QDateTime &minDate, const QDateTime &maxDate,
                     AgricPractOperation operation);
    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                 const QStringList &listProducts);
    QStringList GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                        const QStringList &listFiles, const QDateTime &minDate, const QDateTime &maxDate);

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;

private:
    bool GetSiteConfigForSiteId(EventProcessingContext &ctx, int siteId, const QJsonObject &parameters,
                                                 std::map<QString, QString> &configParameters,
                                                 AgricPracticesSiteCfg &retCfg);
    bool GetSiteConfigForSiteId2(EventProcessingContext &ctx, int siteId, const QJsonObject &parameters,
                                std::map<QString, QString> &configParameters,
                                AgricPracticesSiteCfg &retCfg);

    QString GetSiteConfigFilePath(const QString &siteName, const QJsonObject &parameters, std::map<QString, QString> &configParameters);
    AgricPracticesSiteCfg LoadSiteConfigFile(EventProcessingContext &ctx, int siteId, const QString &siteCfgFilePath);

    void ExtractProductFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                            QStringList &ndviFiles, QStringList &ampFiles, QStringList &coheFiles,
                            QDateTime &minDate, QDateTime &maxDate, const AgricPractOperation &execOper);
    QStringList ExtractNdviFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event, QDateTime &minDate, QDateTime &maxDate);
    QStringList ExtractAmpFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event, QDateTime &minDate, QDateTime &maxDate);
    QStringList ExtractCoheFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event, QDateTime &minDate, QDateTime &maxDate);
    QStringList GetIdsExtractorArgs(const AgricPracticesJobCfg &siteCfg, const QString &outFile, const QString &finalTargetDir);
    QStringList GetPracticesExtractionArgs(const AgricPracticesJobCfg &siteCfg, const QString &outFile, const QString &practice, const QString &finalTargetDir);
    QStringList GetDataExtractionArgs(const AgricPracticesJobCfg &jobCfg, const QString &filterIdsFile, const ProductType &prdType, const QString &uidField, const QStringList &inputFiles,
                                      const QString &outDir);
    QStringList GetFilesMergeArgs(const QStringList &listInputPaths, const QString &outFileName);
    QStringList GetTimeSeriesAnalysisArgs(const AgricPracticesJobCfg &siteCfg, const QString &practice, const QString &practicesFile,
                                          const QString &inNdviFile, const QString &inAmpFile, const QString &inCoheFile,
                                          const QString &outDir);
    QString BuildMergeResultFileName(const AgricPracticesJobCfg &jobCfg, const ProductType &prdsType);
    QString BuildPracticesTableResultFileName(const AgricPracticesJobCfg &jobCfg, const QString &suffix);

    void CreatePrdDataExtrTasks(const AgricPracticesJobCfg &jobCfg, QList<TaskToSubmit> &outAllTasksList,
                                const QString &taskName,
                                const QStringList &prdsList, const QList<std::reference_wrapper<const TaskToSubmit>> &dataExtParents,
                                AgricPractOperation operation, int &minPrdDataExtrIndex, int &maxPrdDataExtrIndex, int &curTaskIdx);
    int CreateMergeTasks(QList<TaskToSubmit> &outAllTasksList, const QString &taskName,
                          int minPrdDataExtrIndex, int maxPrdDataExtrIndex, int &curTaskIdx);
    int CreateTSATasks(const AgricPracticesJobCfg &siteCfg, QList<TaskToSubmit> &outAllTasksList,
                       const QString &practiceName, AgricPractOperation operation,
                       int ndviMergeTaskIdx, int ampMergeTaskIdx, int coheMergeTaskIdx, int &curTaskIdx);

    QString CreateStepForLPISSelection(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                              AgricPractOperation operation, const QString &practice, const AgricPracticesJobCfg &jobCfg,
                                              QList<TaskToSubmit> &allTasksList, NewStepList &steps, int &curTaskIdx);

    QStringList CreateStepsForDataExtraction(const QJsonObject &parameters, const std::map<QString, QString> &configParameters, AgricPractOperation operation,
                                             const AgricPracticesJobCfg &jobCfg, const ProductType &prdType,
                                             const QStringList &prds, const QString &idsFileName,
                                             QList<TaskToSubmit> &allTasksList, NewStepList &steps, int &curTaskIdx);

    QString CreateStepsForFilesMerge(const AgricPracticesJobCfg &siteCfg, const ProductType &prdType, const QStringList &dataExtrDirs,
                                  NewStepList &steps, QList<TaskToSubmit> &allTasksList, int &curTaskIdx);

    QStringList CreateTimeSeriesAnalysisSteps(const QJsonObject &parameters, const std::map<QString, QString> &configParameters, AgricPractOperation operation, const AgricPracticesJobCfg &siteCfg, const QString &practice,
                                              const QString &ndviMergedFile, const QString &ampMergedFile, const QString &coheMergedFile,
                                              NewStepList &steps, QList<TaskToSubmit> &allTasksList, int &curTaskIdx, AgricPractOperation activeOper);

    TQStrQStrMap LoadParamsFromFile(QSettings &settings, const QString &practicePrefix, const QString &sectionName, const AgricPracticesSiteCfg &cfg);
    void UpdatePracticesParams(const TQStrQStrMap &defVals, TQStrQStrMap &sectionVals);

    void UpdatePracticesParams(const QJsonObject &parameters, std::map<QString, QString> &configParameters, const TQStrQStrMap &cfgVals, const QString &prefix, TQStrQStrMap *params);

    QStringList GetListValue(const QSettings &settings, const QString &key);
    QString GetTsaExpectedPractice(const QString &practice);
    AgricPractOperation GetExecutionOperation(const QJsonObject &parameters, const std::map<QString, QString> &configParameters);
    AgricPractOperation GetExecutionOperation(const QString &str);
    bool IsOperationEnabled(AgricPractOperation oper, AgricPractOperation expected);
    bool GetLpisProductFiles(EventProcessingContext &ctx, const QString &yearStr, int siteId,
                            QString &ndviShpFile, QString &ampCoheShpFile, QString &allFieldsFile);
    QStringList GetAdditionalFilesAsList(const QString &files, const AgricPracticesSiteCfg &cfg);

    QString GetShortNameForProductType(const ProductType &prdType);
    int UpdateJobSubmittedParamsFromSchedReq(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                             const std::map<QString, QString> &configParameters, QJsonObject &parameters,
                                             const QString &siteName, JobSubmittedEvent &newEvent);
    QStringList ExtractMissingDataExtractionProducts(EventProcessingContext &ctx, int siteId, const QString &siteName,
                                                     const std::map<QString, QString> &configParameters,
                                                     const ProductType &prdType, const QDateTime &startDate, const QDateTime &endDate, QStringList &alreadyProcessedFiles);
    bool IsDataExtractionPerformed(const QString &dataExtrDirPath, const QString &prdPath);
    QStringList FilterAndUpdateAlreadyProcessingPrds(EventProcessingContext &ctx, int siteId, const QStringList &missingPrdsFiles, const QStringList &processedPrdsFiles, const ProductType &prdType);
    QJsonArray ProductListToJSonArray(const QStringList &prdList);
    bool IsScheduledJobRequest(const QJsonObject &parameters);
    QString GetProcessorDirValue(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                 const QString &key, const QString &siteShortName, const QString &defVal );
    QString GetDataExtractionDir(const QJsonObject &parameters, const std::map<QString, QString> &configParameters, const ProductType &prdType, const QString &siteShortName);
};

#endif // AGRICPRACTICESHANDLER_HPP
