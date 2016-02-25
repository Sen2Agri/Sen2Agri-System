#pragma once

#include "model.hpp"
#include "eventprocessingcontext.hpp"
#include "schedulingcontext.h"

#define PRODUCTS_LOCATION_CFG_KEY "archiver.archive_path"

#define SEASON_CFG_KEY_PREFIX "general."
#define START_OF_SEASON_CFG_KEY "general.start_of_season"
#define END_OF_SEASON_CFG_KEY "general.end_of_season"

class ProcessorHandler
{
public:
    virtual ~ProcessorHandler();
    void SetProcessorDescription(const ProcessorDescription &procDescr) {processorDescr = procDescr;}

    void HandleProductAvailable(EventProcessingContext &ctx, const ProductAvailableEvent &event);
    void HandleJobSubmitted(EventProcessingContext &ctx, const JobSubmittedEvent &event);
    void HandleTaskFinished(EventProcessingContext &ctx, const TaskFinishedEvent &event);

    ProcessorJobDefinitionParams GetProcessingDefinition(SchedulingContext &ctx, int siteId, int scheduledDate,
                                          const ConfigurationParameterValueMap &requestOverrideCfgValues);

protected:
    QString GetFinalProductFolder(EventProcessingContext &ctx, int jobId, int siteId);
    QString GetFinalProductFolder(const std::map<QString, QString> &cfgKeys, const QString &siteName, const QString &processorName);
    bool RemoveJobFolder(EventProcessingContext &ctx, int jobId);

private:
    virtual void HandleProductAvailableImpl(EventProcessingContext &ctx,
                                            const ProductAvailableEvent &event);
    virtual void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event) = 0;
    virtual void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                        const TaskFinishedEvent &event) = 0;
    virtual ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) = 0;

protected:
    ProcessorDescription processorDescr;
};
