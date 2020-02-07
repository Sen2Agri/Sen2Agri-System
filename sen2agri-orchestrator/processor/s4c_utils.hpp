#ifndef S4C_UTILS_H
#define S4C_UTILS_H

#include "model.hpp"
#include "eventprocessingcontext.hpp"

class S4CUtils
{
public:
    S4CUtils();

    static QJsonArray GetInputProducts(const QJsonObject &parameters, const ProductType &prdType);
    static QStringList GetInputProducts(EventProcessingContext &ctx,
                                        const JobSubmittedEvent &event, const ProductType &prdType,
                                        QDateTime &minDate, QDateTime &maxDate, const QString &processorCfgPrefix,
                                        bool extractFromInputParamOnly = true, bool bExtractTiffFiles = true);
    static QStringList FindL3BProductTiffFiles(const QString &path, const QStringList &s2L8TilesFilter,
                                               const QString &l3bBioIndexType);
    static QString GetSiteYearFromDisk(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                       const QString &siteShortName, const QString &cfgFilesSubPath,
                                       const QString &cfgKeyPrefix, const QString &cfgKey);

private:
    static QString GetShortNameForProductType(const ProductType &prdType);
    static QJsonArray FilterProducts(const QJsonArray &allPrds, const ProductType &prdType);
    static QStringList FindL3BProductTiffFiles(EventProcessingContext &ctx, int siteId,
                                               const QString &path, const QStringList &s2L8TilesFilter);
};

#endif // S4C_UTILS_H
