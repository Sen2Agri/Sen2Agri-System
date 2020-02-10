#include "s4c_utils.hpp"
#include "logger.hpp"
#include "processorhandler.hpp"

QString S4CUtils::GetShortNameForProductType(const ProductType &prdType) {
    switch(prdType) {
        case ProductType::L3BProductTypeId:
            return "NDVI";
        case ProductType::S4CS1L2AmpProductTypeId:
            return "AMP";
        case ProductType::S4CS1L2CoheProductTypeId:
            return "COHE";
        default:
        return "";
    }
}

QJsonArray S4CUtils::FilterProducts(const QJsonArray &allPrds, const ProductType &prdType)
{
    QJsonArray retList;
    for (const auto &prd: allPrds) {
        QFileInfo fi(prd.toString());
        const QString &fileName = fi.fileName();
        switch(prdType) {
            case ProductType::L3BProductTypeId:
                if (fileName.contains("S2AGRI_L3B_PRD_")) {
                    retList.append(prd);
                }
                break;
            case ProductType::S4CS1L2AmpProductTypeId:
                if (fileName.contains("_AMP.tif")) {
                    retList.append(prd);
                }
                break;
            case ProductType::S4CS1L2CoheProductTypeId:
                if (fileName.contains("_COHE.tif")) {
                    retList.append(prd);
                }
                break;
            default:
                Logger::error(QStringLiteral("Unsupported product type %1.").arg((int)prdType));
                return retList;
        }
    }
    return retList;
}

QStringList S4CUtils::FindL3BProductTiffFiles(const QString &path, const QStringList &s2L8TilesFilter, const QString &l3bBioIndexType)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.isDir()) {
        const QString &fileName = fileInfo.fileName();
        const QString substr = QString("S2AGRI_L3B_%1_A").arg(l3bBioIndexType);
        if (fileName.contains(substr) && fileName.endsWith(".TIF")) {
            return QStringList(path);
        }
    }
    QString absPath = path;
    const QMap<QString, QString> &prdTiles = ProcessorHandlerHelper::GetHighLevelProductTilesDirs(absPath);
    QStringList retList;
    for(const auto &tileId : prdTiles.keys()) {
        if (s2L8TilesFilter.size() > 0 && !s2L8TilesFilter.contains(tileId)) {
            continue;
        }
        const QString &tileDir = prdTiles[tileId];
        const QString &laiFileName = ProcessorHandlerHelper::GetHigLevelProductTileFile(tileDir, l3bBioIndexType);
        if (laiFileName.size() > 0) {
            retList.append(laiFileName);
        }
    }

    if (retList.size() == 0) {
        Logger::error(QStringLiteral("Unable to find any TIFF file for the given input product %1.").arg(absPath));
    }
    return retList;
}

QStringList S4CUtils::FindL3BProductTiffFiles(EventProcessingContext &ctx, int siteId,
                                              const QString &path, const QStringList &s2L8TilesFilter)
{
    QFileInfo fileInfo(path);
    QString absPath = path;
    if(!fileInfo.isAbsolute()) {
        // if we have the product name, we need to get the product path from the database
        absPath = ctx.GetProductAbsolutePath(siteId, path);
    }
    return FindL3BProductTiffFiles(absPath, s2L8TilesFilter, "SNDVI");
}

QJsonArray S4CUtils::GetInputProducts(const QJsonObject &parameters, const ProductType &prdType) {
    const QString &prdTypeShortName = GetShortNameForProductType(prdType);
    const std::string &prodsInputKey = "input_" + prdTypeShortName.toStdString();
    if (prodsInputKey == "input_") {
        Logger::error(QStringLiteral("Unsupported product type %1.").arg(prdTypeShortName));
        return QJsonArray();
    }
    auto inputProducts = parameters[prodsInputKey.c_str()].toArray();
    if (inputProducts.size() == 0) {
        // check for the "input_products" key that is provided from the
        inputProducts = parameters["input_products"].toArray();
        // Filter the products by the prdType
        inputProducts = FilterProducts(inputProducts, prdType);
    }
    return inputProducts;
}

QStringList S4CUtils::GetInputProducts(EventProcessingContext &ctx,
                                       const JobSubmittedEvent &event, const ProductType &prdType,
                                       QDateTime &minDate, QDateTime &maxDate, const QString &processorCfgPrefix,
                                       bool extractFromInputParamOnly, bool bExtractTiffFiles) {
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    auto inputProducts = GetInputProducts(parameters, prdType);

    const std::map<QString, QString> &configParameters = ctx.GetJobConfigurationParameters(event.jobId, processorCfgPrefix);
    const QString &s2L8TilesStr = ProcessorHandlerHelper::GetStringConfigValue(parameters, configParameters, "s2_l8_tiles", processorCfgPrefix);
    const QStringList &s2L8Tiles = s2L8TilesStr.split(',',  QString::SkipEmptyParts);

    QStringList listProducts;

    // get the products from the input_products or based on start_date or date_end
    if(!extractFromInputParamOnly && inputProducts.size() == 0) {
        const auto &startDate = QDateTime::fromString(parameters["start_date"].toString(), "yyyyMMdd");
        const auto &endDateStart = QDateTime::fromString(parameters["end_date"].toString(), "yyyyMMdd");
        if(startDate.isValid() && endDateStart.isValid()) {
            // we consider the end of the end date day
            const auto endDate = endDateStart.addSecs(SECONDS_IN_DAY-1);
            if (!minDate.isValid() || minDate > startDate) {
                minDate = startDate;
            }
            if (!maxDate.isValid() || maxDate < endDate) {
                maxDate = endDate;
            }

            ProductList productsList = ctx.GetProducts(event.siteId, (int)prdType, startDate, endDate);
            for(const auto &product: productsList) {
                if (bExtractTiffFiles && prdType == ProductType::L3BProductTypeId) {
                    const QStringList &tiffFiles = FindL3BProductTiffFiles(ctx, event.siteId, product.fullPath, s2L8Tiles);
                    if (tiffFiles.size() > 0) {
                        listProducts.append(tiffFiles);
                    }
                } else {
                    listProducts.append(product.fullPath);
                }
            }
        }
    } else {
        for (const auto &inputProduct : inputProducts) {
            // if the product is an LAI, we need to extract the TIFF file for the NDVI
            if (prdType == ProductType::L3BProductTypeId) {
                QDateTime prdTime;
                const QString &prdPath = ctx.GetProductAbsolutePath(event.siteId, inputProduct.toString());
                if (ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prdPath, prdTime, prdTime)) {
                    ProcessorHandlerHelper::UpdateMinMaxTimes(prdTime, minDate, maxDate);
                    if (bExtractTiffFiles) {
                        const QStringList &tiffFiles = FindL3BProductTiffFiles(ctx, event.siteId, inputProduct.toString(), s2L8Tiles);
                        listProducts.append(tiffFiles);
                    }
                }
            } else {
                // the S1 AMP and COHE products have directly the path of the tiff in the product table
                const QString &prdPath = ctx.GetProductAbsolutePath(event.siteId, inputProduct.toString());
                if (prdPath.size() > 0) {
                    listProducts.append(prdPath);
                    QDateTime s1MinTime, s1MaxTime;
                    if (ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(prdPath, s1MinTime, s1MaxTime)) {
                        ProcessorHandlerHelper::UpdateMinMaxTimes(s1MaxTime, minDate, maxDate);
                    }
                } else {
                    Logger::error(QStringLiteral("The product path does not exists %1.").arg(inputProduct.toString()));
                    return QStringList();
                }
            }
        }
    }

    return listProducts;
}

QString S4CUtils::GetSiteYearFromDisk(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                      const QString &siteShortName, const QString &cfgFilesSubPath,
                                      const QString &cfgKeyPrefix, const QString &cfgKey)
{
    qDebug() << "Determining year from disk ...";
    QString dataExtrDirName = ProcessorHandlerHelper::GetStringConfigValue(parameters, configParameters, cfgKey, cfgKeyPrefix);
    dataExtrDirName = dataExtrDirName.replace("{site}", siteShortName);
    const QStringList &tokens = dataExtrDirName.split("{year}");
    QString retVal;
    QDateTime lastDirDate;
    if (tokens.length() > 0) {
        const QString &yearsRoot = tokens.at(0);
        QDirIterator iter(yearsRoot, QDir::Dirs | QDir::NoDotAndDotDot);
        while(iter.hasNext()) {
            const QString &subdirFullPath = iter.next();
            QFileInfo fileInfo(subdirFullPath);
            const QString &subDir = fileInfo.fileName();
            const QDateTime &dirDate = fileInfo.lastModified();
            qDebug() << "Checking subfolder " << subDir << " from " << yearsRoot << " to determine the year ...";
            int year = subDir.toInt();
            // check if valid year
            if (year >= 1970 && year < 2100) {
                // check if it the most recent one
                if (!lastDirDate.isValid() || dirDate > lastDirDate) {
                    // check if the config subfolder has files
                    const QFileInfoList &entries = QDir(subdirFullPath + QDir::separator() + cfgFilesSubPath).
                                                    entryInfoList(QDir::NoDotAndDotDot|QDir::Files);
                    if (entries.count() > 0) {
                        retVal = subDir;
                        lastDirDate = dirDate;
                    }
                }
            }
        }
    }
    return retVal;
}

