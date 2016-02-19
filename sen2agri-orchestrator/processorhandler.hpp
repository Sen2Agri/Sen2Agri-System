#pragma once

#include "model.hpp"
#include "eventprocessingcontext.hpp"

class ProcessorHandler
{
public:
    virtual ~ProcessorHandler();

    void HandleProductAvailable(EventProcessingContext &ctx, const ProductAvailableEvent &event);
    void HandleJobSubmitted(EventProcessingContext &ctx, const JobSubmittedEvent &event);
    void HandleTaskFinished(EventProcessingContext &ctx, const TaskFinishedEvent &event);

    QString GetProcessingDefinitionJson(const QJsonObject &procInfoParams, const ProductList &listProducts, bool &bIsValid);

protected:
    QString GetFinalProductFolder(EventProcessingContext &ctx, int jobId, const QString& parametersJson);
    bool RemoveJobFolder(EventProcessingContext &ctx, int jobId);

private:
    virtual void HandleProductAvailableImpl(EventProcessingContext &ctx,
                                            const ProductAvailableEvent &event);
    virtual void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event) = 0;
    virtual void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                        const TaskFinishedEvent &event) = 0;
    virtual QString GetProcessingDefinitionJsonImpl(const QJsonObject &procInfoParams, const ProductList &listProducts, bool &bIsValid) = 0;
};
