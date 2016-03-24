#ifndef PROCESSORHANDLERHELPER_H
#define PROCESSORHANDLERHELPER_H

// 23*3600+59*60+59
#define SECONDS_IN_DAY 86400
class ProcessorHandlerHelper
{
public:
    ProcessorHandlerHelper();

    static QString GetTileId(const QString &path, bool *ok = 0);
    static QString GetTileId(const QStringList &xmlFileNames, bool *ok = 0);
    static QStringList GetProductTileIds(const QStringList &listFiles);
    static QMap<QString, QStringList> GroupTiles(const QStringList &listAllProductsTiles);
    static QStringList GetTextFileLines(const QString &filePath);
    static QString GetFileNameFromPath(const QString &filePath);
    static bool IsValidHighLevelProduct(const QString &path);
    static bool GetHigLevelProductAcqDatesFromName(const QString &productName, QDateTime &minDate, QDateTime &maxDate);
    static QString GetHigLevelProductTileFile(const QString &tileDir, const QString &fileIdentif, bool isQiData=false);
    static QMap<QString, QString> GetHigLevelProductFiles(const QString &productDir, const QString &fileIdentif, bool isQiData=false);
    static QMap<QString, QStringList> GroupHighLevelProductTiles(const QStringList &listAllProductFolders);

    //static QString GetL2ATileMainImageFilePath(const QString &tileMetadataPath);
    static QString GetL2AProductTypeFromTile(const QString &tileMetadataPath);
    static QDateTime GetL2AProductDateFromPath(const QString &path);
    static bool IsValidL2AMetadataFileName(const QString &path);
    static bool GetL2AIntevalFromProducts(const QStringList &productsList, QDateTime &minTime, QDateTime &maxTime);
    static bool GetCropReferenceFile(const QString &refDir, QString &shapeFile, QString &referenceRasterFile);

private:
    static QStringList m_supportedSensorPrefixes;
    static QMap<QString, QString> m_mapSensorL2AMetaFilePattern;
};

#endif // PROCESSORHANDLERHELPER_H
