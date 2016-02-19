#pragma once

#include "processorhandler.hpp"

class CropTypeHandler : public ProcessorHandler
{
private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    QString GetProcessingDefinitionJsonImpl(const QJsonObject &procInfoParams, const ProductList &listProducts, bool &bIsValid);

};
