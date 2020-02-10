#ifndef GRASSLANDMOWINGHANDLER_HPP
#define GRASSLANDMOWINGHANDLER_HPP

#include "processorhandler.hpp"
#include "s4c_utils.hpp"

#define L4B_GM_CFG_PREFIX "processor.s4c_l4b."

namespace grassland_mowing {
    enum InputProductsType {none = 0x00, L2_S1 = 0x01, L3B = 0x02, all = 0xFF};


    typedef struct GrasslandMowingExecConfig {
        class GrasslandMowingHandler;
        GrasslandMowingExecConfig(EventProcessingContext *pContext, const JobSubmittedEvent &evt) : event(evt) {
            pCtx = pContext;
            parameters = QJsonDocument::fromJson(evt.parametersJson.toUtf8()).object();
            configParameters = pCtx->GetJobConfigurationParameters(evt.jobId, L4B_GM_CFG_PREFIX);
            inputPrdsType = GetInputProductsType(parameters, configParameters);
            siteShortName = pContext->GetSiteShortName(event.siteId);
            year = ProcessorHandlerHelper::GetStringConfigValue(parameters, configParameters,
                                                                "year", L4B_GM_CFG_PREFIX);
            if (year.size() == 0) {
                year = S4CUtils::GetSiteYearFromDisk(parameters, configParameters, siteShortName,
                                                     "config", L4B_GM_CFG_PREFIX, "cfg_dir");
            }
        }
        static InputProductsType GetInputProductsType(const QJsonObject &parameters, const std::map<QString, QString> &configParameters);
        static InputProductsType GetInputProductsType(const QString &str);

        EventProcessingContext *pCtx;
        const JobSubmittedEvent &event;
        QJsonObject parameters;
        std::map<QString, QString> configParameters;

        bool isScheduled;
        InputProductsType inputPrdsType;
        QDateTime startDate;
        QDateTime endDate;

        QDateTime seasonStartDate;
        QDateTime seasonEndDate;

        QStringList l3bPrds;
        QStringList s1Prds;
        QString siteShortName;
        QString year;
        QString ctNumFilter;
        QString l4bCfgFile;

    } GrasslandMowingExecConfig;
}

class GrasslandMowingHandler : public ProcessorHandler
{
private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    void CreateTasks(grassland_mowing::GrasslandMowingExecConfig &cfg,
                     QList<TaskToSubmit> &outAllTasksList);
    void CreateSteps(grassland_mowing::GrasslandMowingExecConfig &cfg, QList<TaskToSubmit> &allTasksList,
                     NewStepList &steps);

    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                 const QStringList &listProducts);
    QStringList GetProductFormatterArgs(TaskToSubmit &productFormatterTask, grassland_mowing::GrasslandMowingExecConfig &cfg,
                                        const QStringList &listFiles);

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;

private:
    QStringList ExtractL3BProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event, QDateTime &minDate, QDateTime &maxDate);
    QStringList ExtractAmpProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event, QDateTime &minDate, QDateTime &maxDate);
    QStringList ExtractCoheProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event, QDateTime &minDate, QDateTime &maxDate);

    QStringList GetInputShpGeneratorArgs(grassland_mowing::GrasslandMowingExecConfig &cfg, const QString &outShpFile);
    QStringList GetMowingDetectionArgs(grassland_mowing::GrasslandMowingExecConfig &cfg,
                                       const grassland_mowing::InputProductsType &prdType, const QString &inputShpLocation,
                                       const QString &outDataDir, const QString &outFile);

    bool IsScheduledJobRequest(const QJsonObject &parameters);
    bool CheckInputParameters(grassland_mowing::GrasslandMowingExecConfig &cfg, QString &err);
    void UpdatePrdInfos(grassland_mowing::GrasslandMowingExecConfig &cfg, const QJsonArray &arrPrds, QStringList &whereToAdd, QDateTime &startDate, QDateTime &endDate);
    QString GetProcessorDirValue(grassland_mowing::GrasslandMowingExecConfig &cfg, const QString &key, const QString &defVal);
    QString GetL4BConfigFilePath(grassland_mowing::GrasslandMowingExecConfig &jobCfg);
    int GuessYear(const QDateTime &startDateTime, const QDateTime &endDateTime);
    bool LoadConfigFileAdditionalValues(grassland_mowing::GrasslandMowingExecConfig &cfg, QString &err);
    QString GetStringValue(const QSettings &settings, const QString &key);
    QString GetOutputDataDir(grassland_mowing::GrasslandMowingExecConfig &cfg, TaskToSubmit&task, const QString &outDataDirName);
};

#endif // GRASSLANDMOWINGHANDLER_HPP
