#include "processorhandler.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>

#include "schedulingcontext.h"
#include "processorhandlerhelper.h"

bool removeDir(const QString & dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}

bool compareProductDates(const QString& path1,const QString& path2)
{
  QDateTime dtProd1=ProcessorHandlerHelper::GetL2AProductDateFromPath(path1);
  QDateTime dtProd2=ProcessorHandlerHelper::GetL2AProductDateFromPath(path2);
  return (dtProd1 < dtProd2);
}

ProcessorHandler::~ProcessorHandler() {}

void ProcessorHandler::HandleProductAvailable(EventProcessingContext &ctx,
                                              const ProductAvailableEvent &event)
{
    HandleProductAvailableImpl(ctx, event);
}

void ProcessorHandler::HandleJobSubmitted(EventProcessingContext &ctx,
                                          const JobSubmittedEvent &event)
{
    HandleJobSubmittedImpl(ctx, event);
}

void ProcessorHandler::HandleTaskFinished(EventProcessingContext &ctx,
                                          const TaskFinishedEvent &event)
{
    HandleTaskFinishedImpl(ctx, event);
}

ProcessorJobDefinitionParams ProcessorHandler::GetProcessingDefinition(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                        const ConfigurationParameterValueMap &requestOverrideCfgValues) {
    return GetProcessingDefinitionImpl(ctx, siteId, scheduledDate, requestOverrideCfgValues);
}

void ProcessorHandler::HandleProductAvailableImpl(EventProcessingContext &,
                                                  const ProductAvailableEvent &)
{
}

QString ProcessorHandler::GetFinalProductFolder(EventProcessingContext &ctx, int jobId,
                                                int siteId) {
    auto configParameters = ctx.GetJobConfigurationParameters(jobId, PRODUCTS_LOCATION_CFG_KEY);
    QString siteName = ctx.GetSiteShortName(siteId);

    return GetFinalProductFolder(configParameters, siteName, processorDescr.shortName);
}

QString ProcessorHandler::GetFinalProductFolder(EventProcessingContext &ctx, int jobId, int siteId,
                                                const QString &productName) {
    auto configParameters = ctx.GetJobConfigurationParameters(jobId, PRODUCTS_LOCATION_CFG_KEY);
    QString siteName = ctx.GetSiteShortName(siteId);

    return GetFinalProductFolder(configParameters, siteName, productName);
}

QString ProcessorHandler::GetFinalProductFolder(const std::map<QString, QString> &cfgKeys, const QString &siteName,
                                                const QString &processorName) {
    auto it = cfgKeys.find(PRODUCTS_LOCATION_CFG_KEY);
    if (it == std::end(cfgKeys)) {
        throw std::runtime_error(QStringLiteral("No final product folder configured for site %1 and processor %2")
                                     .arg(siteName)
                                     .arg(processorName)
                                     .toStdString());
    }
    QString folderName = (*it).second;
    folderName = folderName.replace("{site}", siteName);
    folderName = folderName.replace("{processor}", processorName);

    return folderName;
}

bool ProcessorHandler::RemoveJobFolder(EventProcessingContext &ctx, int jobId)
{
    QString jobOutputPath = ctx.GetJobOutputPath(jobId);
    return removeDir(jobOutputPath);
}

QString ProcessorHandler::GetProductFormatterOutputProductPath(EventProcessingContext &ctx,
                                                        const TaskFinishedEvent &event) {
    QString prodFolderOutPath = ctx.GetOutputPath(event.jobId, event.taskId, event.module) +
            "/" + PRODUCT_FORMATTER_OUT_PROPS_FILE;
    QStringList fileLines = ProcessorHandlerHelper::GetTextFileLines(prodFolderOutPath);
    if(fileLines.size() > 0) {
        return fileLines[0].trimmed();
    }
    return "";
}

QString ProcessorHandler::GetProductFormatterProductName(EventProcessingContext &ctx,
                                                        const TaskFinishedEvent &event) {
    QString productPath = GetProductFormatterOutputProductPath(ctx, event);
    if(productPath.length() > 0) {
        QString name = ProcessorHandlerHelper::GetFileNameFromPath(productPath);
        if(name.trimmed() != "") {
            return name;
        }
    }
    return "";
}

QString ProcessorHandler::GetProductFormatterQuicklook(EventProcessingContext &ctx,
                                                        const TaskFinishedEvent &event) {
    QString prodFolderOutPath = ctx.GetOutputPath(event.jobId, event.taskId, event.module) +
            "/" + PRODUCT_FORMATTER_OUT_PROPS_FILE;
    QStringList fileLines = ProcessorHandlerHelper::GetTextFileLines(prodFolderOutPath);
    QString quickLookName("");
    if(fileLines.size() > 0) {
        const QString &mainFolderName = fileLines[0];
        QString legacyFolder = mainFolderName + "/LEGACY_DATA/";

        QDirIterator it(legacyFolder, QStringList() << "*.jpg", QDir::Files);
        // get the last shape file found
        QString quickLookFullName;
        while (it.hasNext()) {
            quickLookFullName = it.next();
            QFileInfo quickLookFileInfo(quickLookFullName);
            QString quickLookTmpName = quickLookFileInfo.fileName();
            if(quickLookTmpName.indexOf("_PVI_")) {
                return quickLookTmpName;
            }

        }
    }
    return quickLookName;
}

QString ProcessorHandler::GetProductFormatterFootprint(EventProcessingContext &ctx,
                                                        const TaskFinishedEvent &event) {
    QString prodFolderOutPath = ctx.GetOutputPath(event.jobId, event.taskId, event.module) +
            "/" + PRODUCT_FORMATTER_OUT_PROPS_FILE;
    QStringList fileLines = ProcessorHandlerHelper::GetTextFileLines(prodFolderOutPath);
    if(fileLines.size() > 0) {
        const QString &mainFolderName = fileLines[0];
        QString legacyFolder = mainFolderName + "/LEGACY_DATA/";

        QDirIterator it(legacyFolder, QStringList() << "*.xml", QDir::Files);
        // get the last shape file found
        QString footprintFullName;
        while (it.hasNext()) {
            footprintFullName = it.next();
            // check only the name if it contains MTD
            QFileInfo footprintFileInfo(footprintFullName);
            if(footprintFileInfo.fileName().indexOf("_MTD_")) {
                // parse the XML file
                QFile inputFile(footprintFullName);
                if (inputFile.open(QIODevice::ReadOnly))
                {
                   QTextStream in(&inputFile);
                   while (!in.atEnd()) {
                        QString curLine = in.readLine();
                        // we assume we have only one line
                        QString startTag("<EXT_POS_LIST>");
                        int extposlistStartIdx = curLine.indexOf(startTag);
                        if(extposlistStartIdx >= 0) {
                            int extposlistEndIdxIdx = curLine.indexOf("</EXT_POS_LIST>");
                            if(extposlistEndIdxIdx >= 0) {
                                int startIdx = extposlistStartIdx + startTag.length();
                                QString extensionPointsStr = curLine.mid(startIdx,extposlistEndIdxIdx-startIdx);
                                QStringList extPointsList = extensionPointsStr.split(" ");
                                if((extPointsList.size() > 8) && (extPointsList.size() % 2) == 0) {
                                    QString footprint = "POLYGON((";
                                    for(int i = 0; i<extPointsList.size(); i++) {
                                        if(i > 0)
                                            footprint.append(", ");
                                        footprint.append(extPointsList[i]);

                                        footprint.append(" ");
                                        footprint.append(extPointsList[++i]);
                                    }
                                    footprint += "))";
                                    inputFile.close();
                                    return footprint;
                                }
                            }
                        }
                   }
                   inputFile.close();
                }
            }

        }
    }
    return "POLYGON((0.0 0.0, 0.0 0.0, 0.0 0.0, 0.0 0.0, 0.0 0.0))";
}

bool ProcessorHandler::GetSeasonStartEndDates(SchedulingContext &ctx, int siteId,
                                              QDateTime &startTime, QDateTime &endTime,
                                              const ConfigurationParameterValueMap &requestOverrideCfgValues) {
    QDate currentDate = QDate::currentDate();
    int curYear = currentDate.year();

    // first, get the values from general section
    ConfigurationParameterValueMap seasonCfgValues = ctx.GetConfigurationParameters(SEASON_CFG_KEY_PREFIX, siteId, requestOverrideCfgValues);
    QDate genStartSummerSeasonDate = QDate::fromString(seasonCfgValues[START_OF_SEASON_CFG_KEY].value, "yyyyMMdd");
    QDate genEndSummerSeasonDate = QDate::fromString(seasonCfgValues[END_OF_SEASON_CFG_KEY].value, "yyyyMMdd");
    if(genStartSummerSeasonDate.isValid() && genEndSummerSeasonDate.isValid()) {
        startTime = QDateTime(genStartSummerSeasonDate);
        endTime = QDateTime(genEndSummerSeasonDate);
        return true;
    }


    seasonCfgValues = ctx.GetConfigurationParameters("downloader.", siteId, requestOverrideCfgValues);
    QDate startSummerSeasonDate = QDate::fromString(seasonCfgValues["downloader.summer-season.start"].value, "MMdd").addYears(curYear-1900);
    QDate endSummerSeasonDate = QDate::fromString(seasonCfgValues["downloader.summer-season.end"].value, "MMdd").addYears(curYear-1900);
    QDate startWinterSeasonDate = QDate::fromString(seasonCfgValues["downloader.winter-season.start"].value, "MMdd").addYears(curYear-1900);
    QDate endWinterSeasonDate = QDate::fromString(seasonCfgValues["downloader.winter-season.end"].value, "MMdd").addYears(curYear-1900);
    if(startSummerSeasonDate.isValid() && endSummerSeasonDate.isValid()) {
        if(currentDate >= startSummerSeasonDate && currentDate <= endSummerSeasonDate) {
            startTime = QDateTime(startSummerSeasonDate);
            endTime = QDateTime(endSummerSeasonDate);
            return true;
        }
    }
    if(startWinterSeasonDate.isValid() && endWinterSeasonDate.isValid()) {
        if(currentDate >= startWinterSeasonDate && currentDate <= endWinterSeasonDate) {
            startTime = QDateTime(startWinterSeasonDate);
            endTime = QDateTime(endWinterSeasonDate);
            return true;
        }
    }

    // check the default values
    seasonCfgValues = ctx.GetConfigurationParameters("downloader.", -1, requestOverrideCfgValues);
    QDate defStartSummerSeasonDate = QDate::fromString(seasonCfgValues["downloader.summer-season.start"].value, "MMdd").addYears(curYear-1900);
    QDate defEndSummerSeasonDate = QDate::fromString(seasonCfgValues["downloader.summer-season.end"].value, "MMdd").addYears(curYear-1900);
    QDate defStartWinterSeasonDate = QDate::fromString(seasonCfgValues["downloader.winter-season.start"].value, "MMdd").addYears(curYear-1900);
    QDate defEndWinterSeasonDate = QDate::fromString(seasonCfgValues["downloader.winter-season.end"].value, "MMdd").addYears(curYear-1900);

    if(defStartSummerSeasonDate.isValid() && defEndSummerSeasonDate.isValid()) {
        if(currentDate >= defStartSummerSeasonDate && currentDate <= defEndSummerSeasonDate) {
            startTime = QDateTime(defStartSummerSeasonDate);
            endTime = QDateTime(defEndSummerSeasonDate);
            return true;
        }
    }
    if(defStartWinterSeasonDate.isValid() && defEndWinterSeasonDate.isValid()) {
        if(currentDate >= defStartWinterSeasonDate && currentDate <= defEndWinterSeasonDate) {
            startTime = QDateTime(defStartWinterSeasonDate);
            endTime = QDateTime(defEndWinterSeasonDate);
            return true;
        }
    }
    return false;
}

QStringList ProcessorHandler::GetL2AInputProductsTiles(EventProcessingContext &ctx,
                                const JobSubmittedEvent &event) {
    QStringList listProducts;
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    const auto &inputProducts = parameters["input_products"].toArray();
    if(inputProducts.size() == 0) {
        const auto &startDate = QDateTime::fromString(parameters["date_start"].toString(), "yyyyMMdd");
        const auto &endDateStart = QDateTime::fromString(parameters["date_end"].toString(), "yyyyMMdd");
        // we consider the end of the end date day
        const auto endDate = endDateStart.addSecs(SECONDS_IN_DAY-1);
        ProductList productsList = ctx.GetProducts(event.siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
        for(const auto &product: productsList) {
            listProducts.append(ctx.findProductFiles(product.fullPath));
        }
    } else {
        for (const auto &inputProduct : inputProducts) {
            listProducts.append(ctx.findProductFiles(inputProduct.toString()));
        }
    }

    // sort the input products according to their dates
    qSort(listProducts.begin(), listProducts.end(), compareProductDates);

    return listProducts;
}

bool ProcessorHandler::GetParameterValueAsInt(const QJsonObject &parameters, const QString &key,
                                              int &outVal) {
    bool bRet = false;
    if(parameters.contains(key)) {
        // first try to get it as string
        const auto &value = parameters[key];
        if(value.isDouble()) {
            outVal = value.toInt();
            bRet = true;
        }
        if(value.isString()) {
            outVal = value.toString().toInt(&bRet);
        }
    }
    return bRet;
}
