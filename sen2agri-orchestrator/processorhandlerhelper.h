#ifndef PROCESSORHANDLERHELPER_H
#define PROCESSORHANDLERHELPER_H

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
    static QMap<QString, QStringList> GroupHighLevelProductTiles(const QStringList &listAllProductFolders);

    static QString GetL2ATileMainImageFilePath(const QString &tileMetadataPath);
    static QString GetL2AProductTypeFromTile(const QString &tileMetadataPath);
    static QString GetL2AProductDateFromPath(const QString &path);
};

#endif // PROCESSORHANDLERHELPER_H
