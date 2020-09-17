#ifndef LAIRETRIEVALHANDLERL3D_HPP
#define LAIRETRIEVALHANDLERL3D_HPP

#include "lairetrhandler_multidt_base.hpp"

class LaiRetrievalHandlerL3D : public LaiRetrievalHandlerMultiDateBase
{
protected:
    QStringList GetSpecificReprocessingArgs(const std::map<QString, QString> &configParameters) override;
    ProductType GetOutputProductType() override;
    QString GetOutputProductShortName() override;
    void WriteExecutionSpecificParamsValues(const std::map<QString, QString> &configParameters, std::ofstream &stream) override;
    QString GetPrdFormatterRasterFlagName() override;
    QString GetPrdFormatterMskFlagName() override;
    QList<QMap<QString, TileTemporalFilesInfo>> ExtractL3BMapTiles(EventProcessingContext &ctx, const JobSubmittedEvent &event,
                                                       const QStringList &l3bProducts,
                                                       const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles) override;
    ProductList GetScheduledJobProductList(SchedulingContext &ctx, int siteId, const QDateTime &seasonStartDate,
                                           const QDateTime &seasonEndDate, const QDateTime &qScheduledDate,
                                           const ConfigurationParameterValueMap &requestOverrideCfgValues) override;
    bool AcceptSchedJobProduct(const QString &l2aPrdHdrPath, ProcessorHandlerHelper::SatelliteIdType satId) override;

    QMap<QString, TileTemporalFilesInfo> GetL3BMapTiles(EventProcessingContext &ctx,
                                                        const QStringList &l3bProducts,
                                                        const QMap<ProcessorHandlerHelper::SatelliteIdType, TileList> &siteTiles);


};

#endif // LAIRETRIEVALHANDLERL3D_HPP

