#include "processorhandlerhelper.h"

ProcessorHandlerHelper::ProcessorHandlerHelper()
{

}

/* static */
QString ProcessorHandlerHelper::GetTileId(const QStringList &xmlFileNames) {
    // we might have combined tiles (from S2 and L8 for example but L8 does not have a tile ID)
    for (const QString &xmlFileName: xmlFileNames) {
        // First remove the extension
        QFileInfo info(xmlFileName);
        QString fileNameWithoutExtension = info.completeBaseName();
        // Split the name by "_" and search the part having _Txxxxx (_T followed by 5 characters)
        QStringList pieces = fileNameWithoutExtension.split("_");
        for(const QString &piece: pieces) {
            if((piece.length() == 6) && (piece.at(0) == 'T')) {
                return QString("TILE_" + piece);
            }
        }
    }
    return QString("TILE_00000");
}
