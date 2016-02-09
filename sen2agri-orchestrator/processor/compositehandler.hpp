#ifndef COMPOSITEHANDLER_HPP
#define COMPOSITEHANDLER_HPP

#include "processorhandler.hpp"

class CompositeHandler : public ProcessorHandler
{
private:
    void HandleProductAvailableImpl(EventProcessingContext &ctx,
                                    const ProductAvailableEvent &event);
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    void HandleNewProductInJob(EventProcessingContext &ctx,
                               int jobId,
                               const QString &jsonParams,
                               const QStringList &listProducts);
    bool IsProductAcceptableForJob(int jobId, const ProductAvailableEvent &event);
    void FilterInputProducts(QStringList &listFiles, int productDate, int halfSynthesis);

    void CreateNewProductInJobTasks(QList<TaskToSubmit> &outAllTasksList, int nbProducts);
    void WriteExecutionInfosFile(const QString &executionInfosPath,
                                 const QJsonObject &parameters,
                                 std::map<QString, QString> &configParameters,
                                 const QStringList &listProducts);
};

#endif // COMPOSITEHANDLER_HPP
