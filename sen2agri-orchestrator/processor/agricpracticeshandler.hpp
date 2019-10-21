#ifndef AGRICPRACTICESHANDLER_HPP
#define AGRICPRACTICESHANDLER_HPP

#include "processorhandler.hpp"

#define L4C_AP_CFG_PREFIX   "processor.s4c_l4c."

typedef struct AgricPracticesSiteCfg {
    AgricPracticesSiteCfg() {

    }
    QString additionalFilesRootDir;

    // Common parameters
    QString country;
    QString year;
    // map from practice name to practices file path
    QMap<QString, QString> practices;

    // LPIS informations
    QString fullDeclsFilePath;
    QString ndviIdsGeomShapePath;
    QString ampCoheIdsGeomShapePath;

    // TSA parameters
    TQStrQStrMap ccTsaParams;
    TQStrQStrMap flTsaParams;
    TQStrQStrMap nfcTsaParams;
    TQStrQStrMap naTsaParams;

    // parameters used for data extraction step
    int prdsPerGroup;

    // TSA minimum acquisitions
    QString tsaMinAcqsNo;

} AgricPracticesSiteCfg;

enum AgricPractOperation {none = 0x00,
                          dataExtraction = 0x01,
                          catchCrop = 0x02,
                          fallow = 0x04,
                          nfc = 0x08,
                          harvestOnly = 0x10,
                          timeSeriesAnalysis = (catchCrop|fallow|nfc|harvestOnly),
                          all = 0xFF};

class AgricPracticesHandler;

typedef struct AgricPracticesJobCfg {
    AgricPracticesJobCfg(EventProcessingContext *pContext, const JobSubmittedEvent &evt) : event(evt) {
        pCtx = pContext;
        parameters = QJsonDocument::fromJson(evt.parametersJson.toUtf8()).object();
        configParameters = pCtx->GetJobConfigurationParameters(evt.jobId, L4C_AP_CFG_PREFIX);
        siteId = evt.siteId;
        siteShortName = pContext->GetSiteShortName(evt.siteId);
        siteCfg.prdsPerGroup = ProcessorHandlerHelper::GetIntConfigValue(parameters, configParameters, "prds_per_group", L4C_AP_CFG_PREFIX);
        isScheduledJob = false;
        execOper = GetExecutionOperation(parameters, configParameters);
    }
    static AgricPractOperation GetExecutionOperation(const QJsonObject &parameters,
                                                     const std::map<QString, QString> &configParameters);
    EventProcessingContext *pCtx;
    JobSubmittedEvent event;
    QJsonObject parameters;
    std::map<QString, QString> configParameters;

    AgricPracticesSiteCfg siteCfg;

    int siteId;
    QString siteShortName;

    // parameters used for data extraction step
    bool isScheduledJob;

    // Extracted season start and end dates
    QDateTime seasonStartDate;
    QDateTime seasonEndDate;
    QDateTime prdMinDate;
    QDateTime prdMaxDate;

    AgricPractOperation execOper;

} AgricPracticesJobCfg;

class AgricPracticesHandler : public ProcessorHandler
{
public:
    static AgricPractOperation GetExecutionOperation(const QString &str);


private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &evt) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;
    void HandleProductAvailableImpl(EventProcessingContext &ctx,
                                    const ProductAvailableEvent &event) override;

    void CreateTasks(const AgricPracticesJobCfg &jobCfg, QList<TaskToSubmit> &outAllTasksList, const QStringList &ndviPrds,
                     const QStringList &ampPrds, const QStringList &cohePrds);
    void CreateSteps(QList<TaskToSubmit> &allTasksList,
                     const AgricPracticesJobCfg &siteCfg, const QStringList &ndviPrds, const QStringList &ampPrds,
                     const QStringList &cohePrds, NewStepList &steps);
    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                 const QStringList &listProducts);
    QStringList GetExportProductLauncherArgs(const AgricPracticesJobCfg &jobCfg,
                                            const QString &productFormatterPrdFileIdFile);
    QStringList GetProductFormatterArgs(TaskToSubmit &productFormatterTask, const AgricPracticesJobCfg &jobCfg,
                                        const QStringList &listFiles);

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;

private:
    bool GetL4CConfigForSiteId(AgricPracticesJobCfg &jobCfg);
    bool GetSiteConfigForSiteId2(AgricPracticesJobCfg &jobCfg);

    QString GetL4CConfigFilePath(AgricPracticesJobCfg &jobCfg);
    bool LoadL4CConfigFile(AgricPracticesJobCfg &jobCfg, const QString &siteCfgFilePath);

    void ExtractProductFiles(AgricPracticesJobCfg &jobCfg, QStringList &ndviFiles, QStringList &ampFiles, QStringList &coheFiles);
    bool ValidateProductsForOperation(const AgricPracticesJobCfg &jobCfg, const QStringList &ndviFiles,
                                      const QStringList &ampFiles, const QStringList &coheFiles);
    QStringList ExtractNdviFiles(AgricPracticesJobCfg &jobCfg, QDateTime &minDate, QDateTime &maxDate);
    QStringList ExtractAmpFiles(AgricPracticesJobCfg &jobCfg, QDateTime &minDate, QDateTime &maxDate);
    QStringList ExtractCoheFiles(AgricPracticesJobCfg &jobCfg, QDateTime &minDate, QDateTime &maxDate);
    // QStringList GetIdsExtractorArgs(const AgricPracticesJobCfg &siteCfg, const QString &outFile, const QString &finalTargetDir);
    // QStringList GetPracticesExtractionArgs(const AgricPracticesJobCfg &siteCfg, const QString &outFile, const QString &practice, const QString &finalTargetDir);
    QStringList GetDataExtractionArgs(const AgricPracticesJobCfg &jobCfg, const QString &filterIds, const ProductType &prdType,
                                      const QString &uidField, const QStringList &inputFiles,
                                      const QString &outDir);
    QStringList GetFilesMergeArgs(const QStringList &listInputPaths, const QString &outFileName);
    QStringList GetTimeSeriesAnalysisArgs(const AgricPracticesJobCfg &jobCfg, const QString &practice, const QString &practicesFile,
                                          const QString &inNdviFile, const QString &inAmpFile, const QString &inCoheFile,
                                          const QString &outDir);
    QString BuildMergeResultFileName(const QString &country, const QString &year, const ProductType &prdsType);
    QString BuildPracticesTableResultFileName(const QString &country, const QString &year, const QString &suffix);

    void CreatePrdDataExtrTasks(const AgricPracticesJobCfg &jobCfg, QList<TaskToSubmit> &outAllTasksList,
                                const QString &taskName,
                                const QStringList &prdsList, const QList<std::reference_wrapper<const TaskToSubmit>> &dataExtParents, int &minPrdDataExtrIndex, int &maxPrdDataExtrIndex, int &curTaskIdx);
    int CreateMergeTasks(QList<TaskToSubmit> &outAllTasksList, const QString &taskName,
                          int minPrdDataExtrIndex, int maxPrdDataExtrIndex, int &curTaskIdx);
    int CreateTSATasks(const AgricPracticesJobCfg &jobCfg, QList<TaskToSubmit> &outAllTasksList,
                       const QString &practiceName,
                       int ndviMergeTaskIdx, int ampMergeTaskIdx, int coheMergeTaskIdx, int &curTaskIdx);

//    QString CreateStepForLPISSelection(const QString &practice, const AgricPracticesJobCfg &jobCfg,
//                                              QList<TaskToSubmit> &allTasksList, NewStepList &steps, int &curTaskIdx);

    QStringList CreateStepsForDataExtraction(const AgricPracticesJobCfg &jobCfg, const QString &filterIds, const ProductType &prdType,
                                             const QStringList &prds,
                                             QList<TaskToSubmit> &allTasksList, NewStepList &steps, int &curTaskIdx);

    QString CreateStepsForFilesMerge(const AgricPracticesJobCfg &jobCfg, const ProductType &prdType, const QStringList &dataExtrDirs,
                                  NewStepList &steps, QList<TaskToSubmit> &allTasksList, int &curTaskIdx);

    QStringList CreateTimeSeriesAnalysisSteps(const AgricPracticesJobCfg &jobCfg, const QString &practice,
                                              const QString &ndviMergedFile, const QString &ampMergedFile, const QString &coheMergedFile,
                                              NewStepList &steps, QList<TaskToSubmit> &allTasksList, int &curTaskIdx);

    TQStrQStrMap LoadParamsFromFile(QSettings &settings, const QString &practicePrefix, const QString &sectionName, const AgricPracticesSiteCfg &cfg);
    void UpdatePracticesParams(const TQStrQStrMap &defVals, TQStrQStrMap &sectionVals);

    void UpdatePracticesParams(const QJsonObject &parameters, std::map<QString, QString> &configParameters, const TQStrQStrMap &cfgVals, const QString &prefix, TQStrQStrMap *params);

    QStringList GetListValue(const QSettings &settings, const QString &key);
    QString GetTsaExpectedPractice(const QString &practice);
    bool IsOperationEnabled(AgricPractOperation oper, AgricPractOperation expected);
    bool GetPrevL4CProduct(const AgricPracticesJobCfg &jobCfg,  const QDateTime &seasonStart, const QDateTime &curDate, QString &prevL4cProd);
    bool GetLpisProductFiles(AgricPracticesJobCfg &jobCfg);
    //QStringList GetAdditionalFilesAsList(const QString &files, const AgricPracticesSiteCfg &cfg);

    QString GetShortNameForProductType(const ProductType &prdType);
    int UpdateJobSubmittedParamsFromSchedReq(AgricPracticesJobCfg &jobCfg, JobSubmittedEvent &newEvent, bool &isSchedJob);
    QStringList ExtractMissingDataExtractionProducts(AgricPracticesJobCfg &jobCfg, const ProductType &prdType, const QDateTime &startDate,
                                                     const QDateTime &endDate, QStringList &alreadyProcessedFiles);
    bool IsDataExtractionPerformed(const QString &dataExtrDirPath, const QString &prdPath);
    QStringList FilterAndUpdateAlreadyProcessingPrds(AgricPracticesJobCfg &jobCfg, const QStringList &missingPrdsFiles, const QStringList &processedPrdsFiles, const ProductType &prdType);
    QJsonArray ProductListToJSonArray(const QStringList &prdList);
    bool IsScheduledJobRequest(const QJsonObject &parameters);
    QString GetProcessorDirValue(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                 const QString &key, const QString &siteShortName, const QString &year, const QString &defVal );
    QString GetDataExtractionDir(const AgricPracticesJobCfg &jobCfg, const ProductType &prdType);
    QString GetTsInputTablesDir(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                const QString &siteShortName, const QString &year, const QString &practice);

    QMap<QString, QString> GetPracticeTableFiles(const QJsonObject &parameters,
                                                 const std::map<QString, QString> &configParameters,
                                                 const QString &siteShortName, const QString &year);
    bool CheckExecutionPreconditions(const QJsonObject &parameters,
                                        const std::map<QString, QString> &configParameters,
                                        const QString &siteShortName, const QString &year, QString &errMsg);
    QString GetDataExtractionTaskName(const AgricPracticesJobCfg &jobCfg, const QString &taskName);
};

#endif // AGRICPRACTICESHANDLER_HPP
