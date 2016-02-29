#include "processorhandlerhelper.h"

ProcessorHandlerHelper::ProcessorHandlerHelper() {}

/* static */
QString ProcessorHandlerHelper::GetTileId(const QString &xmlFileName, bool *ok)
{
    if(ok) *ok = false;
    // First remove the extension
    QFileInfo info(xmlFileName);
    QString fileNameWithoutExtension = info.completeBaseName();
    QString extension = info.suffix();
    // Split the name by "_" and search the part having _Txxxxx (_T followed by 5 characters)
    QStringList pieces = fileNameWithoutExtension.split("_");
    for (const QString &piece : pieces) {
        if ((piece.length() == 6) && (piece.at(0) == 'T')) {
            if(ok) *ok = true;
            return QString("TILE_" + piece);
        }
    }
    if(QString::compare(extension, "hdr", Qt::CaseInsensitive) == 0) {
        // check if is S2
        if(pieces[0].indexOf("S2") != -1 && pieces.size() == 9 && pieces[5] == "") {
            if(ok) *ok = true;
            return QString("TILE_" + pieces[4]);
        } else if(pieces[0].indexOf("L8") != -1) {
            // TODO
        }
    }

    return QString("TILE_00000");
}

/* static */
QString ProcessorHandlerHelper::GetTileId(const QStringList &xmlFileNames, bool *ok)
{
    bool bOk = false;
    if(ok) *ok = bOk;
    // we might have combined tiles (from S2 and L8 for example but L8 does not have a tile ID)
    for (const QString &xmlFileName : xmlFileNames) {
        QString tileId = GetTileId(xmlFileName, &bOk);
        if(bOk) {
            if(ok) *ok = bOk;
            return tileId;
        }
    }
    return QString("TILE_00000");
}

QMap<QString, QStringList> ProcessorHandlerHelper::GroupTiles(const QStringList &listAllProductsTiles) {
    QMap<QString, QStringList> mapTiles;
    for(const QString &tileFile: listAllProductsTiles) {
        QString tileId = GetTileId(tileFile);
        if(mapTiles.contains(tileId)) {
            mapTiles[tileId].append(tileFile);
        } else {
            mapTiles[tileId] = QStringList{tileFile};
        }
    }
    return mapTiles;
}

/* static */
QStringList ProcessorHandlerHelper::GetProductTileIds(const QStringList &listFiles) {
    bool bOk;
    QStringList result;
    for (const QString &fileName : listFiles) {
        QString tileId = GetTileId(fileName, &bOk);
        if(bOk) {
            result.append(tileId);
        }
    }
    return result;
}
