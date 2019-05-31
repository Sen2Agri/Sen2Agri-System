#ifndef AGRICPRACTICESHANDLER_HPP
#define AGRICPRACTICESHANDLER_HPP

#include "processorhandler.hpp"

typedef struct {
    QString vegStart;
    QString hstart;
    QString hend;
    QString hstartw;
    QString pstart;
    QString pend;
    QString pstartw;
    QString pendw;
    QStringList additionalFiles;
} PracticesTableExtractionParams;

typedef struct {
    QString optthrvegcycle;
    QString ndvidw;
    QString ndviup;
    QString ndvistep;
    QString optthrmin;
    QString cohthrbase;
    QString cohthrhigh;
    QString cohthrabs;
    QString ampthrmin;
    QString efandvithr;
    QString efandviup;
    QString efandvidw;
    QString efacohchange;
    QString efacohvalue;
    QString efandvimin;
    QString efaampthr;
    QString stddevinampthr;
    QString optthrbufden;
    QString ampthrbreakden;
    QString ampthrvalueden;

    QString catchmain;
    QString catchperiod;
    QString catchperiodstart;
    QString catchcropismain;
    QString catchproportion;

    QString flmarkstartdate;
    QString flmarkstenddate;
} TsaPracticeParams;

typedef struct AgricPracticesSiteCfg {
    AgricPracticesSiteCfg() {
        prdsPerGroup = 0;
    }
    // Common parameters
    QString country;
    QString year;
    QString fullShapePath;
    QString ndviIdsGeomShapePath;
    QString ampCoheIdsGeomShapePath;

    // Parameters used for practices tables extraction
    PracticesTableExtractionParams ccPracticeParams;
    PracticesTableExtractionParams flPracticeParams;
    PracticesTableExtractionParams nfcPracticeParams;
    PracticesTableExtractionParams naPracticeParams;

    TsaPracticeParams ccTsaParams;
    TsaPracticeParams flTsaParams;
    TsaPracticeParams nfcTsaParams;
    TsaPracticeParams naTsaParams;

    // parameters used for data extraction step
    int prdsPerGroup;
    QStringList practices;

} AgricPracticesSiteCfg;



enum AgricPractOperation {none = 0x00,
                          dataExtraction = 0x01,
                          catchCrop = 0x02,
                          fallow = 0x04,
                          nfc = 0x08,
                          harvestOnly = 0x10,
                          all = 0xFF};

class AgricPracticesHandler : public ProcessorHandler
{
private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;
    void HandleProductAvailableImpl(EventProcessingContext &ctx,
                                    const ProductAvailableEvent &event) override;

    void CreateTasks(const AgricPracticesSiteCfg &siteCfg, QList<TaskToSubmit> &outAllTasksList, const QStringList &ndviPrds,
                     const QStringList &ampPrds, const QStringList &cohePrds, AgricPractOperation operation);
    void CreateSteps(EventProcessingContext &ctx, const JobSubmittedEvent &event, QList<TaskToSubmit> &allTasksList,
                     const AgricPracticesSiteCfg &siteCfg, const QStringList &ndviPrds, const QStringList &ampPrds,
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
    QString GetSiteConfigFilePath(const QString &siteName, const QJsonObject &parameters, std::map<QString, QString> &configParameters);
    AgricPracticesSiteCfg LoadSiteConfigFile(const QString &siteCfgFilePath, const QJsonObject &parameters,
                                             std::map<QString, QString> &configParameters);

    void ExtractProductFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                            QStringList &ndviFiles, QStringList &ampFiles, QStringList &coheFiles,
                            QDateTime &minDate, QDateTime &maxDate, const AgricPractOperation &execOper);
    QStringList ExtractNdviFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event, QDateTime &minDate, QDateTime &maxDate);
    QStringList ExtractAmpFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event, QDateTime &minDate, QDateTime &maxDate);
    QStringList ExtractCoheFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event, QDateTime &minDate, QDateTime &maxDate);
    QStringList GetIdsExtractorArgs(const AgricPracticesSiteCfg &siteCfg, const QString &outFile, const QString &finalTargetDir);
    QStringList GetPracticesExtractionArgs(const AgricPracticesSiteCfg &siteCfg, const QString &outFile, const QString &practice, const QString &finalTargetDir);
    QStringList GetDataExtractionArgs(const AgricPracticesSiteCfg &siteCfg, const QString &filterIdsFile, const QString &prdType, const QString &uidField, const QStringList &inputFiles,
                                      const QString &outDir);
    QStringList GetFilesMergeArgs(const QStringList &listInputPaths, const QString &outFileName);
    QStringList GetTimeSeriesAnalysisArgs(const AgricPracticesSiteCfg &siteCfg, const QString &practice, const QString &practicesFile,
                                          const QString &inNdviFile, const QString &inAmpFile, const QString &inCoheFile,
                                          const QString &outDir);
    QString BuildMergeResultFileName(const AgricPracticesSiteCfg &siteCfg, const QString &prdsType);
    QString BuildPracticesTableResultFileName(const AgricPracticesSiteCfg &siteCfg, const QString &suffix);

    void CreatePrdDataExtrTasks(const AgricPracticesSiteCfg &siteCfg, QList<TaskToSubmit> &outAllTasksList,
                                const QString &taskName,
                                const QStringList &prdsList, const QList<std::reference_wrapper<const TaskToSubmit>> &dataExtParents,
                                AgricPractOperation operation, int &minPrdDataExtrIndex, int &maxPrdDataExtrIndex, int &curTaskIdx);
    int CreateMergeTasks(QList<TaskToSubmit> &outAllTasksList, const QString &taskName,
                          int minPrdDataExtrIndex, int maxPrdDataExtrIndex, int &curTaskIdx);
    int CreateTSATasks(const AgricPracticesSiteCfg &siteCfg, QList<TaskToSubmit> &outAllTasksList,
                       const QString &practiceName, AgricPractOperation operation,
                       int ndviMergeTaskIdx, int ampMergeTaskIdx, int coheMergeTaskIdx, int &curTaskIdx);

    QString CreateStepForLPISSelection(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                              AgricPractOperation operation, const QString &practice, const AgricPracticesSiteCfg &siteCfg,
                                              QList<TaskToSubmit> &allTasksList, NewStepList &steps, int &curTaskIdx);

    QStringList CreateStepsForDataExtraction(const QJsonObject &parameters, const std::map<QString, QString> &configParameters, AgricPractOperation operation,
                                             const AgricPracticesSiteCfg &siteCfg, const QString &prdType,
                                             const QStringList &prds, const QString &idsFileName,
                                             QList<TaskToSubmit> &allTasksList, NewStepList &steps, int &curTaskIdx);

    QString CreateStepsForFilesMerge(const AgricPracticesSiteCfg &siteCfg, const QString &prdType, const QStringList &dataExtrDirs,
                                  NewStepList &steps, QList<TaskToSubmit> &allTasksList, int &curTaskIdx);

    QStringList CreateTimeSeriesAnalysisSteps(const QJsonObject &parameters, const std::map<QString, QString> &configParameters, AgricPractOperation operation, const AgricPracticesSiteCfg &siteCfg, const QString &practice,
                                              const QString &ndviMergedFile, const QString &ampMergedFile, const QString &coheMergedFile,
                                              NewStepList &steps, QList<TaskToSubmit> &allTasksList, int &curTaskIdx, AgricPractOperation activeOper);

    QStringList GetInputProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event, const ProductType &prdType,
                                 QDateTime &minDate, QDateTime &maxDate);
    QStringList FindNdviProductTiffFile(EventProcessingContext &ctx, int siteId, const QString &path, const QStringList &s2L8TilesFilter);

    PracticesTableExtractionParams LoadPracticesParams(const QSettings &settings, const QString &practicePrefix);
    void UpdatePracticesParams(const PracticesTableExtractionParams &defVals, PracticesTableExtractionParams &sectionVals);
    TsaPracticeParams LoadTsaParams(const QSettings &settings, const QString &practicePrefix);
    void UpdateTsaParams(const TsaPracticeParams &defVals, TsaPracticeParams &sectionVals);

    QStringList GetListValue(const QSettings &settings, const QString &key);
    QString GetTsaExpectedPractice(const QString &practice);
    AgricPractOperation GetExecutionOperation(const QJsonObject &parameters, const std::map<QString, QString> &configParameters);
    AgricPractOperation GetExecutionOperation(const QString &str);
    bool IsOperationEnabled(AgricPractOperation oper, AgricPractOperation expected);
};

#endif // AGRICPRACTICESHANDLER_HPP
