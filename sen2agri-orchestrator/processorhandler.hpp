#pragma once

#include "model.hpp"
#include "eventprocessingcontext.hpp"
#include "schedulingcontext.h"
#include "processorhandlerhelper.h"

#define PRODUCTS_LOCATION_CFG_KEY "archiver.archive_path"
#define PRODUCT_FORMATTER_OUT_PROPS_FILE "product_properties.txt"

typedef ProcessorHandlerHelper::TileTemporalFilesInfo TileTemporalFilesInfo;

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
    QString GetFinalProductFolder(EventProcessingContext &ctx, int jobId, int siteId, const QString &productName);
    QString GetFinalProductFolder(const std::map<QString, QString> &cfgKeys, const QString &siteName, const QString &processorName);
    bool RemoveJobFolder(EventProcessingContext &ctx, int jobId);
    QString GetProductFormatterOutputProductPath(EventProcessingContext &ctx, const TaskFinishedEvent &event);
    QString GetProductFormatterProductName(EventProcessingContext &ctx, const TaskFinishedEvent &event);
    QString GetProductFormatterQuicklook(EventProcessingContext &ctx, const TaskFinishedEvent &event);
    QString GetProductFormatterFootprint(EventProcessingContext &ctx, const TaskFinishedEvent &event);
    bool GetSeasonStartEndDates(SchedulingContext &ctx, int siteId,  QDateTime &startTime, QDateTime &endTime,
                                const ConfigurationParameterValueMap &requestOverrideCfgValues);
    QStringList GetL2AInputProductsTiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                    QMap<QString, QStringList> &mapProductToTilesMetaFiles);
    QStringList GetL2AInputProductsTiles(EventProcessingContext &ctx, const JobSubmittedEvent &event);
    QString GetL2AProductForTileMetaFile(const QMap<QString, QStringList> &mapProductToTilesMetaFiles, const QString &tileMetaFile);
    bool GetParameterValueAsInt(const QJsonObject &parameters, const QString &key, int &outVal);

    QMap<QString, TileTemporalFilesInfo> GroupTiles(EventProcessingContext &ctx, int jobId, const QStringList &listAllProductsTiles, ProductType productType);
    QString GetProductFormatterTile(const QString &tile);

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
