#pragma once

#include "model.hpp"
#include "eventprocessingcontext.hpp"
#include "schedulingcontext.h"
#include "processorhandlerhelper.h"

#define PRODUCTS_LOCATION_CFG_KEY "archiver.archive_path"
#define PRODUCT_FORMATTER_OUT_PROPS_FILE "product_properties.txt"
#define CLOUD_OPTIMIZED_GEOTIFF_ENABLED "processor."

typedef ProcessorHandlerHelper::TileTemporalFilesInfo TileTemporalFilesInfo;

class ProcessorHandler
{
public:
    virtual ~ProcessorHandler();
    virtual void SetProcessorDescription(const ProcessorDescription &procDescr) {processorDescr = procDescr;}

    void HandleProductAvailable(EventProcessingContext &ctx, const ProductAvailableEvent &event);
    void HandleJobSubmitted(EventProcessingContext &ctx, const JobSubmittedEvent &event);
    void HandleTaskFinished(EventProcessingContext &ctx, const TaskFinishedEvent &event);

    ProcessorJobDefinitionParams GetProcessingDefinition(SchedulingContext &ctx, int siteId, int scheduledDate,
                                          const ConfigurationParameterValueMap &requestOverrideCfgValues);

protected:
    QString GetFinalProductFolder(EventProcessingContext &ctx, int jobId, int siteId);
    QString GetFinalProductFolder(EventProcessingContext &ctx, int jobId, int siteId, const QString &productName);
    QString GetFinalProductFolder(const std::map<QString, QString> &cfgKeys, const QString &siteName, const QString &processorName);
    bool NeedRemoveJobFolder(EventProcessingContext &ctx, int jobId, const QString &procName);
    bool RemoveJobFolder(EventProcessingContext &ctx, int jobId, const QString &procName);
    QString GetProductFormatterOutputProductPath(EventProcessingContext &ctx, const TaskFinishedEvent &event);
    QString GetProductFormatterProductName(EventProcessingContext &ctx, const TaskFinishedEvent &event);
    QString GetProductFormatterQuicklook(EventProcessingContext &ctx, const TaskFinishedEvent &event);
    QString GetProductFormatterFootprint(EventProcessingContext &ctx, const TaskFinishedEvent &event);
    //QDate GetSeasonDate(const QString &seasonDateStr, const QDateTime &executionDate, bool &bDateHasYear);
    void EnsureStartSeasonLessThanEndSeasonDate(QDate &startSeasonDate, QDate &endSeasonDate, const QDateTime &executionDate, bool bStartDateHadYear, bool bEndDateHadYear);
/*    bool GetSeasonStartEndDates(SchedulingContext &ctx, int siteId,  QDateTime &startTime, QDateTime &endTime, const QDateTime &executionDate,
                                const ConfigurationParameterValueMap &requestOverrideCfgValues);
    bool GetSeasonStartEndDates(const ConfigurationParameterValueMap &seasonCfgValues, int siteId,
                                QDateTime &startTime, QDateTime &endTime, const QString &keyStart, const QString &keyEnd,
                                const QDateTime &executionDate);
*/
    bool GetSeasonStartEndDates(SchedulingContext &ctx, int siteId,
                                   QDateTime &startTime, QDateTime &endTime,
                                   const QDateTime &executionDate,
                                   const ConfigurationParameterValueMap &requestOverrideCfgValues);
    QStringList GetL2AInputProducts(EventProcessingContext &ctx, const JobSubmittedEvent &event);
    QStringList GetL2AInputProductsTiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                    QMap<QString, QStringList> &mapProductToTilesMetaFiles);
    QStringList GetL2AInputProductsTiles(EventProcessingContext &ctx, const JobSubmittedEvent &event);
    QString GetL2AProductForTileMetaFile(const QMap<QString, QStringList> &mapProductToTilesMetaFiles, const QString &tileMetaFile);
    bool GetParameterValueAsInt(const QJsonObject &parameters, const QString &key, int &outVal);

    QMap<QString, TileTemporalFilesInfo> GroupTiles(EventProcessingContext &ctx, int siteId, int jobId, const QStringList &listAllProductsTiles, ProductType productType);
    QString GetProductFormatterTile(const QString &tile);

    void SubmitTasks(EventProcessingContext &ctx, int jobId, const QList<std::reference_wrapper<TaskToSubmit> > &tasks);
    QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> GetSiteTiles(EventProcessingContext &ctx, int siteId);
    ProcessorHandlerHelper::SatelliteIdType GetSatIdForTile(const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &mapSatTiles,
                                                                           const QString &tileId);
    QString BuildProcessorOutputFileName(const std::map<QString, QString> &configParameters, const QString &fileName,
                                         bool compress=false, bool bigTiff=false);
    bool IsCloudOptimizedGeotiff(const std::map<QString, QString> &configParameters);
    QString GetMapValue(const std::map<QString, QString> &configParameters, const QString &key, const QString &defVal = "");

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
