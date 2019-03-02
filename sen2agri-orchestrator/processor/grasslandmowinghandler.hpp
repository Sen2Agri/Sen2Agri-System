#ifndef GRASSLANDMOWINGHANDLER_HPP
#define GRASSLANDMOWINGHANDLER_HPP

#include "processorhandler.hpp"

class GrasslandMowingHandler : public ProcessorHandler
{
private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    void CreateTasks(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                     QList<TaskToSubmit> &outAllTasksList);
    void CreateSteps(EventProcessingContext &ctx, const JobSubmittedEvent &event, QList<TaskToSubmit> &allTasksList,
                     NewStepList &steps, const QDateTime &minDate, const QDateTime &maxDate);

    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                 const QStringList &listProducts);
    QStringList GetProductFormatterArgs(TaskToSubmit &productFormatterTask, EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                        const QStringList &listFiles, const QDateTime &minDate, const QDateTime &maxDate);

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;

private:
    QString GetSiteConfigFilePath(const QString &siteName, const QJsonObject &parameters, std::map<QString, QString> &configParameters);

    QStringList ExtractNdviFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event, QDateTime &minDate, QDateTime &maxDate);
    QStringList ExtractAmpFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event, QDateTime &minDate, QDateTime &maxDate);
    QStringList ExtractCoheFiles(EventProcessingContext &ctx, const JobSubmittedEvent &event, QDateTime &minDate, QDateTime &maxDate);

    QStringList GetMowingDetectionArgs(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                       const QString &outDataDir, const QString &outFile, const QDateTime &minDate, const QDateTime &maxDate);

    QStringList GetInputProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event, const ProductType &prdType,
                                 QDateTime &minDate, QDateTime &maxDate);
    QString FindNdviProductTiffFile(EventProcessingContext &ctx, const JobSubmittedEvent &event, const QString &path);

    QStringList GetListValue(const QSettings &settings, const QString &key);
};

#endif // GRASSLANDMOWINGHANDLER_HPP
