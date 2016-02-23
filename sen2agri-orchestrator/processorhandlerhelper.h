#ifndef PROCESSORHANDLERHELPER_H
#define PROCESSORHANDLERHELPER_H

class ProcessorHandlerHelper
{
public:
    ProcessorHandlerHelper();

    static QString GetTileId(const QString &xmlFileName, bool *ok = 0);
    static QString GetTileId(const QStringList &xmlFileNames, bool *ok = 0);
    static QStringList GetProductTileIds(const QStringList &listFiles);
    static QMap<QString, QStringList> GroupTiles(const QStringList &listAllProductsTiles);
};

#endif // PROCESSORHANDLERHELPER_H
