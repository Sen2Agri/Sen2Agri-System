#ifndef PROCESSORHANDLERHELPER_H
#define PROCESSORHANDLERHELPER_H

// 23*3600+59*60+59
#define SECONDS_IN_DAY 86400
class ProcessorHandlerHelper
{
public:
    typedef enum {UNKNOWN = 0, S2 = 1, L8 = 2, SPOT4 =3, SPOT5 = 4} L2ProductType;
    typedef struct {
        L2ProductType productType;
        // position of the date in the name when split by _
        int dateIdxInName;
        // The expected extension of the tile metadata file
        QString extension;
        //NOT Used yet: maybe a pattern should be used
        QString tileNamePattern;
    } L2MetaTileNameInfos;

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
    static const L2MetaTileNameInfos &GetL2AProductTileNameInfos(const QString &metaFileName);
    static L2ProductType GetL2AProductTypeFromTile(const QString &tileMetadataPath);
    static QDateTime GetL2AProductDateFromPath(const QString &path);
    static bool IsValidL2AMetadataFileName(const QString &path);
    static bool GetL2AIntevalFromProducts(const QStringList &productsList, QDateTime &minTime, QDateTime &maxTime);
    static bool GetCropReferenceFile(const QString &refDir, QString &shapeFile, QString &referenceRasterFile);

private:
    static QMap<QString, L2MetaTileNameInfos> m_mapSensorL2ATileMetaFileInfos;
};

#endif // PROCESSORHANDLERHELPER_H
