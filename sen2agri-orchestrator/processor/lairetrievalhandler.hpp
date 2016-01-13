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

    void CreateNewProductInJobTasks(QList<TaskToSubmit> &outAllTasksList, int nbProducts);
    void HandleNewProductInJob(EventProcessingContext &ctx, int jobId, const QString &jsonParams,
                               const QStringList &listProducts);
    void GetModelFileList(QStringList &outListModels, const QString &strPattern, std::map<QString, QString> &configParameters);
    void WriteExecutionInfosFile(const QString &executionInfosPath, const QJsonObject &parameters,
                                std::map<QString, QString> &configParameters,
                                const QStringList &listProducts);
};

#endif // LAIRETRIEVALHANDLER_HPP

