#pragma once

#include "processorhandler.hpp"

class CropMaskHandler : public ProcessorHandler
{
private:
    void HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) override;
    void HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                const TaskFinishedEvent &event) override;

    ProcessorJobDefinitionParams GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues) override;

    void HandleInsituJob(EventProcessingContext &ctx, const JobSubmittedEvent &event);
    void HandleNoInsituJob(EventProcessingContext &ctx, const JobSubmittedEvent &event);

    QStringList GetBandsExtractorArgs(const QString &mission, const QString &outImg, const QString &mask, const QString &statusFlags,
                                        const QString &outDates, const QString &shape, const QStringList &inputProducts, int resolution);
    QStringList GetCompressionArgs(const QString &inImg, const QString &outImg);
    QStringList GetConfusionMatrixArgs(const QString &inRaster, const QString &outCsv, const QString &refIn, const QString &refType="raster", const QString &refField = "");
    QStringList GetGdalWarpArgs(const QString &inImg, const QString &outImg, const QString &dtsNoData,
                                const QString &gdalwarpMem, const QString &shape, const QString &resolutionStr="");
};
